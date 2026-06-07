#define _POSIX_C_SOURCE 200809L

#include "json_loader.h"

#include "core/interface.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <cJSON.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct NodeList {
  TopologyNodeInfo** items;
  TopologyNodeKind kind;
  size_t count;
  size_t index;
} NodeList;

typedef struct LinkList {
  TopologyLinkInfo** items;
  size_t count;
  size_t index;
} LinkList;

/**
 * @brief Duplicate a string onto the heap.
 */
static char* duplicate_string(const char* text) {
  if (text == NULL) {
    return NULL;
  }

  size_t len = strlen(text) + 1U;
  char* copy = malloc(len);
  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, text, len);
  return copy;
}

/**
 * @brief Copy an optional JSON string field into fixed metadata storage.
 */
static void copy_optional_string(cJSON* object, const char* key, char* out, size_t out_len) {
  if (object == NULL || key == NULL || out == NULL || out_len == 0U) {
    return;
  }

  cJSON* item = cJSON_GetObjectItemCaseSensitive(object, key);
  if (cJSON_IsString(item) && item->valuestring != NULL) {
    snprintf(out, out_len, "%s", item->valuestring);
  }
}

/**
 * @brief Read an entire file into a heap buffer.
 */
static int read_file(const char* path, char** buffer_out, size_t* len_out) {
  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    return MAGI_ERR_BADARGS;
  }

  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    return MAGI_ERR_BADARGS;
  }

  long size = ftell(file);
  if (size < 0) {
    fclose(file);
    return MAGI_ERR_BADARGS;
  }

  if (fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    return MAGI_ERR_BADARGS;
  }

  char* buffer = calloc((size_t)size + 1U, 1U);
  if (buffer == NULL) {
    fclose(file);
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  size_t read = fread(buffer, 1U, (size_t)size, file);
  fclose(file);

  if (read != (size_t)size) {
    free(buffer);
    return MAGI_ERR_BADARGS;
  }

  *buffer_out = buffer;
  if (len_out != NULL) {
    *len_out = (size_t)size;
  }

  return MAGI_OK;
}

/**
 * @brief Compare node metadata pointers for deterministic serialization.
 */
static int compare_node_info_ptrs(const void* lhs, const void* rhs) {
  const TopologyNodeInfo* left = *(const TopologyNodeInfo* const*)lhs;
  const TopologyNodeInfo* right = *(const TopologyNodeInfo* const*)rhs;

  if (left->kind != right->kind) {
    return (int)left->kind - (int)right->kind;
  }

  return strcmp(left->node->name, right->node->name);
}

/**
 * @brief Compare link metadata pointers for deterministic serialization.
 */
static int compare_link_info_ptrs(const void* lhs, const void* rhs) {
  const TopologyLinkInfo* left = *(const TopologyLinkInfo* const*)lhs;
  const TopologyLinkInfo* right = *(const TopologyLinkInfo* const*)rhs;

  int comparison = strcmp(left->node_a, right->node_a);
  if (comparison != 0) {
    return comparison;
  }

  if (left->port_a != right->port_a) {
    return (int)left->port_a - (int)right->port_a;
  }

  comparison = strcmp(left->node_b, right->node_b);
  if (comparison != 0) {
    return comparison;
  }

  return (int)left->port_b - (int)right->port_b;
}

/**
 * @brief Collect matching node metadata into a scratch array.
 */
static void collect_node_info_cb(const char* key, void* value, void* ctx) {
  (void)key;

  NodeList* state = ctx;
  TopologyNodeInfo* info = value;
  if (info != NULL && info->kind == state->kind && state->index < state->count) {
    state->items[state->index++] = info;
  }
}

/**
 * @brief Collect link metadata into a scratch array.
 */
static void collect_link_info_cb(const char* key, void* value, void* ctx) {
  (void)key;

  LinkList* state = ctx;
  if (state->index < state->count) {
    state->items[state->index++] = value;
  }
}

/* Context wrapper for port collection. */
typedef struct PortCollectCtx {
  uint16_t* ports;
  size_t index;
} PortCollectCtx;

typedef struct RouteSaveCtx {
  cJSON* array;
  bool failed;
} RouteSaveCtx;

static void collect_port_info_cb(const char* key, void* value, void* ctx) {
  (void)value;
  PortCollectCtx* state = ctx;
  unsigned long port = strtoul(key, NULL, 10);
  if (port <= 0xFFFFUL) {
    state->ports[state->index++] = (uint16_t)port;
  }
}

/**
 * @brief Find the highest configured interface port on a node.
 */
static uint16_t node_max_port(const Node* node) {
  if (node == NULL || node->interfaces == NULL) {
    return 0U;
  }

  uint16_t max_port = 0U;
  for (size_t index = 0; index < node->interfaces->capacity; ++index) {
    HashEntry* entry = &node->interfaces->entries[index];
    if (entry->key != NULL && !entry->tombstone) {
      unsigned long port = strtoul(entry->key, NULL, 10);
      if (port > max_port && port <= 0xFFFFUL) {
        max_port = (uint16_t)port;
      }
    }
  }

  return max_port;
}

/**
 * @brief Compare 16-bit port numbers in ascending order.
 */
static int compare_uint16_values(const void* lhs, const void* rhs) {
  uint16_t left = *(const uint16_t*)lhs;
  uint16_t right = *(const uint16_t*)rhs;

  if (left < right) {
    return -1;
  }
  if (left > right) {
    return 1;
  }
  return 0;
}

/**
 * @brief Collect nodes of one kind into a deterministic array.
 */
static int collect_node_array(const Topology* topology, TopologyNodeKind kind,
                              TopologyNodeInfo*** items_out, size_t* count_out) {
  size_t count = topology_count_nodes_of_kind(topology, kind);
  TopologyNodeInfo** items = calloc(count > 0U ? count : 1U, sizeof(*items));
  if (items == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  NodeList state = {.items = items, .kind = kind, .count = count, .index = 0U};
  hashmap_foreach(topology->nodes, collect_node_info_cb, &state);
  qsort(items, state.index, sizeof(*items), compare_node_info_ptrs);

  *items_out = items;
  *count_out = state.index;
  return MAGI_OK;
}

static void save_route_cb(const char* dest_cidr, const char* next_hop_ip, uint16_t out_port,
                          void* ctx) {
  RouteSaveCtx* state = ctx;
  if (state == NULL || state->array == NULL || state->failed) {
    return;
  }

  cJSON* route_obj = cJSON_CreateObject();
  if (route_obj == NULL) {
    state->failed = true;
    return;
  }

  const char* next_hop = next_hop_ip != NULL && next_hop_ip[0] != '\0' ? next_hop_ip : "direct";
  if (!cJSON_AddStringToObject(route_obj, "dest_cidr", dest_cidr) ||
      !cJSON_AddStringToObject(route_obj, "next_hop", next_hop) ||
      !cJSON_AddNumberToObject(route_obj, "out_port", out_port)) {
    cJSON_Delete(route_obj);
    state->failed = true;
    return;
  }

  cJSON_AddItemToArray(state->array, route_obj);
}

/**
 * @brief Collect links into a deterministic array.
 */
static int collect_link_array(const Topology* topology, TopologyLinkInfo*** items_out,
                              size_t* count_out) {
  size_t count = topology_count_links(topology);
  TopologyLinkInfo** items = calloc(count > 0U ? count : 1U, sizeof(*items));
  if (items == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  LinkList state = {.items = items, .count = count, .index = 0U};
  hashmap_foreach(topology->links, collect_link_info_cb, &state);
  qsort(items, state.index, sizeof(*items), compare_link_info_ptrs);

  *items_out = items;
  *count_out = state.index;
  return MAGI_OK;
}

/**
 * @brief Parse a node endpoint string "name:port" into components.
 */
static int parse_endpoint(const char* endpoint, char* node_name, size_t node_name_len,
                          uint16_t* port_out) {
  const char* colon = strchr(endpoint, ':');
  if (colon == NULL) {
    size_t name_len = strlen(endpoint);
    if (name_len == 0U || name_len >= node_name_len) {
      return MAGI_ERR_BADARGS;
    }
    snprintf(node_name, node_name_len, "%s", endpoint);
    *port_out = 1U;
    return MAGI_OK;
  }

  size_t name_len = (size_t)(colon - endpoint);
  if (name_len == 0U || name_len >= node_name_len) {
    return MAGI_ERR_BADARGS;
  }

  memcpy(node_name, endpoint, name_len);
  node_name[name_len] = '\0';

  unsigned long port = strtoul(colon + 1, NULL, 10);
  if (port == 0U || port > 0xFFFFUL) {
    return MAGI_ERR_BADARGS;
  }

  *port_out = (uint16_t)port;
  return MAGI_OK;
}

/**
 * @brief Load nodes of one kind from a cJSON array into the topology.
 */
static int load_node_array(cJSON* array, Topology* topology, TopologyNodeKind kind) {
  if (!cJSON_IsArray(array)) {
    return MAGI_ERR_BADARGS;
  }

  cJSON* item = NULL;
  cJSON_ArrayForEach(item, array) {
    if (!cJSON_IsObject(item)) {
      return MAGI_ERR_BADARGS;
    }

    cJSON* name_item = cJSON_GetObjectItemCaseSensitive(item, "name");
    if (!cJSON_IsString(name_item)) {
      return MAGI_ERR_BADARGS;
    }

    const char* name = name_item->valuestring;
    TopologyNodeInfo* info = topology_add_node(topology, kind, name);
    if (info == NULL) {
      return MAGI_ERR_BADARGS;
    }

    if (kind == TOPOLOGY_NODE_HOST) {
      copy_optional_string(item, "ip_address", info->ip_address, sizeof(info->ip_address));
      copy_optional_string(item, "default_gateway", info->default_gateway,
                           sizeof(info->default_gateway));
      if (topology_configure_host(topology, name, info->ip_address, info->default_gateway) !=
          MAGI_OK) {
        return MAGI_ERR_BADARGS;
      }
    }

    if (kind == TOPOLOGY_NODE_SWITCH) {
      cJSON* num_ports = cJSON_GetObjectItemCaseSensitive(item, "num_ports");
      if (cJSON_IsNumber(num_ports) && num_ports->valuedouble >= 0.0 &&
          num_ports->valuedouble <= 65535.0) {
        if (topology_configure_switch_num_ports(topology, name, (uint16_t)num_ports->valuedouble) !=
            MAGI_OK) {
          return MAGI_ERR_BADARGS;
        }
      }

      cJSON* vlans = cJSON_GetObjectItemCaseSensitive(item, "vlans");
      if (cJSON_IsArray(vlans)) {
        cJSON* vlan_item = NULL;
        cJSON_ArrayForEach(vlan_item, vlans) {
          if (!cJSON_IsObject(vlan_item)) {
            return MAGI_ERR_BADARGS;
          }

          cJSON* port_item = cJSON_GetObjectItemCaseSensitive(vlan_item, "port");
          cJSON* mode_item = cJSON_GetObjectItemCaseSensitive(vlan_item, "mode");
          cJSON* vlan_id_item = cJSON_GetObjectItemCaseSensitive(vlan_item, "vlan_id");
          if (!cJSON_IsNumber(port_item) || !cJSON_IsString(mode_item)) {
            return MAGI_ERR_BADARGS;
          }

          if (port_item->valuedouble <= 0.0 || port_item->valuedouble > 65535.0) {
            return MAGI_ERR_BADARGS;
          }

          uint16_t vlan_id = 0U;
          if (cJSON_IsNumber(vlan_id_item)) {
            if (vlan_id_item->valuedouble < 0.0 || vlan_id_item->valuedouble > 4094.0) {
              return MAGI_ERR_BADARGS;
            }
            vlan_id = (uint16_t)vlan_id_item->valuedouble;
          }

          if (topology_configure_switch_port(topology, name, (uint16_t)port_item->valuedouble,
                                             mode_item->valuestring, vlan_id) != MAGI_OK) {
            return MAGI_ERR_BADARGS;
          }
        }
      } else if (vlans != NULL) {
        return MAGI_ERR_BADARGS;
      }
    }

    cJSON* interfaces = cJSON_GetObjectItemCaseSensitive(item, "interfaces");
    if (cJSON_IsArray(interfaces)) {
      cJSON* port_item = NULL;
      cJSON_ArrayForEach(port_item, interfaces) {
        int port = 0;
        const char* ip_address = NULL;
        uint16_t vlan_id = 0U;

        if (cJSON_IsNumber(port_item)) {
          port = (int)port_item->valuedouble;
        } else if (cJSON_IsObject(port_item)) {
          cJSON* port_object = cJSON_GetObjectItemCaseSensitive(port_item, "port");
          cJSON* ip_object = cJSON_GetObjectItemCaseSensitive(port_item, "ip_address");
          cJSON* vlan_object = cJSON_GetObjectItemCaseSensitive(port_item, "vlan_id");
          if (!cJSON_IsNumber(port_object)) {
            return MAGI_ERR_BADARGS;
          }
          port = (int)port_object->valuedouble;
          if (cJSON_IsString(ip_object)) {
            ip_address = ip_object->valuestring;
          }
          if (cJSON_IsNumber(vlan_object)) {
            if (vlan_object->valuedouble < 0.0 || vlan_object->valuedouble > 4094.0) {
              return MAGI_ERR_BADARGS;
            }
            vlan_id = (uint16_t)vlan_object->valuedouble;
          }
        } else {
          return MAGI_ERR_BADARGS;
        }

        if (port <= 0 || port > 65535) {
          return MAGI_ERR_BADARGS;
        }
        Interface* iface = node_add_interface(topology_get_node(topology, name), (uint16_t)port);
        if (iface == NULL) {
          return MAGI_ERR_BADARGS;
        }
        if (ip_address != NULL) {
          snprintf(iface->ip_address, sizeof(iface->ip_address), "%s", ip_address);
        }
        iface->vlan_id = vlan_id;
      }
    } else if (interfaces != NULL) {
      /* interfaces key exists but is not an array */
      return MAGI_ERR_BADARGS;
    }

    if (kind == TOPOLOGY_NODE_ROUTER) {
      cJSON* routing_table = cJSON_GetObjectItemCaseSensitive(item, "routing_table");
      if (cJSON_IsArray(routing_table)) {
        cJSON* route_item = NULL;
        cJSON_ArrayForEach(route_item, routing_table) {
          if (!cJSON_IsObject(route_item)) {
            return MAGI_ERR_BADARGS;
          }

          cJSON* dest_item = cJSON_GetObjectItemCaseSensitive(route_item, "dest_cidr");
          if (!cJSON_IsString(dest_item)) {
            dest_item = cJSON_GetObjectItemCaseSensitive(route_item, "destination");
          }
          if (!cJSON_IsString(dest_item)) {
            dest_item = cJSON_GetObjectItemCaseSensitive(route_item, "network");
          }

          cJSON* next_hop_item = cJSON_GetObjectItemCaseSensitive(route_item, "next_hop");
          if (!cJSON_IsString(next_hop_item)) {
            next_hop_item = cJSON_GetObjectItemCaseSensitive(route_item, "next_hop_ip");
          }

          cJSON* out_port_item = cJSON_GetObjectItemCaseSensitive(route_item, "out_port");
          if (!cJSON_IsNumber(out_port_item)) {
            out_port_item = cJSON_GetObjectItemCaseSensitive(route_item, "out_interface");
          }
          if (!cJSON_IsNumber(out_port_item)) {
            out_port_item = cJSON_GetObjectItemCaseSensitive(route_item, "port");
          }

          if (!cJSON_IsString(dest_item) || !cJSON_IsNumber(out_port_item) ||
              out_port_item->valuedouble <= 0.0 || out_port_item->valuedouble > 65535.0 ||
              topology->node_ops == NULL || topology->node_ops->configure_router_route == NULL) {
            return MAGI_ERR_BADARGS;
          }

          const char* next_hop =
              cJSON_IsString(next_hop_item) ? next_hop_item->valuestring : "direct";
          if (topology->node_ops->configure_router_route(
                  info->node, dest_item->valuestring, next_hop,
                  (uint16_t)out_port_item->valuedouble) != MAGI_OK) {
            return MAGI_ERR_BADARGS;
          }
        }
      } else if (routing_table != NULL) {
        return MAGI_ERR_BADARGS;
      }
    }
  }

  return MAGI_OK;
}

/**
 * @brief Load links from a cJSON array into the topology.
 */
static int load_link_array(cJSON* array, Topology* topology) {
  if (!cJSON_IsArray(array)) {
    return MAGI_ERR_BADARGS;
  }

  cJSON* item = NULL;
  cJSON_ArrayForEach(item, array) {
    if (!cJSON_IsObject(item)) {
      return MAGI_ERR_BADARGS;
    }

    cJSON* endpoints_item = cJSON_GetObjectItemCaseSensitive(item, "endpoints");
    cJSON* a_item = cJSON_GetObjectItemCaseSensitive(item, "a");
    cJSON* b_item = cJSON_GetObjectItemCaseSensitive(item, "b");
    cJSON* delay_item = cJSON_GetObjectItemCaseSensitive(item, "delay");
    if (delay_item == NULL) {
      delay_item = cJSON_GetObjectItemCaseSensitive(item, "delay_ms");
    }
    cJSON* mtu_item = cJSON_GetObjectItemCaseSensitive(item, "mtu");

    const char* endpoint_a_text = NULL;
    const char* endpoint_b_text = NULL;
    if (cJSON_IsArray(endpoints_item) && cJSON_GetArraySize(endpoints_item) == 2) {
      cJSON* first = cJSON_GetArrayItem(endpoints_item, 0);
      cJSON* second = cJSON_GetArrayItem(endpoints_item, 1);
      if (!cJSON_IsString(first) || !cJSON_IsString(second)) {
        return MAGI_ERR_BADARGS;
      }
      endpoint_a_text = first->valuestring;
      endpoint_b_text = second->valuestring;
    } else if (cJSON_IsString(a_item) && cJSON_IsString(b_item)) {
      endpoint_a_text = a_item->valuestring;
      endpoint_b_text = b_item->valuestring;
    } else {
      return MAGI_ERR_BADARGS;
    }

    char node_a[64];
    char node_b[64];
    uint16_t port_a = 0U;
    uint16_t port_b = 0U;

    if (parse_endpoint(endpoint_a_text, node_a, sizeof(node_a), &port_a) != MAGI_OK) {
      return MAGI_ERR_BADARGS;
    }
    if (parse_endpoint(endpoint_b_text, node_b, sizeof(node_b), &port_b) != MAGI_OK) {
      return MAGI_ERR_BADARGS;
    }

    uint32_t delay_ms = 0U;
    if (cJSON_IsNumber(delay_item)) {
      delay_ms = (uint32_t)delay_item->valuedouble;
    }

    uint16_t mtu = 1500U;
    if (cJSON_IsNumber(mtu_item)) {
      mtu = (uint16_t)mtu_item->valuedouble;
    }

    if (topology_add_link(topology, node_a, port_a, node_b, port_b, delay_ms, mtu) == NULL) {
      return MAGI_ERR_BADARGS;
    }
  }

  return MAGI_OK;
}

/**
 * @brief Parse the full topology JSON document using cJSON.
 */
static int parse_topology_json(Topology* topology, const char* buffer) {
  cJSON* root = cJSON_Parse(buffer);
  if (root == NULL) {
    const char* error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
      LOG("JSON", "Parse error before: %.20s", error_ptr);
    }
    return MAGI_ERR_BADARGS;
  }

  if (!cJSON_IsObject(root)) {
    cJSON_Delete(root);
    return MAGI_ERR_BADARGS;
  }

  int status = MAGI_OK;

  cJSON* hosts = cJSON_GetObjectItemCaseSensitive(root, "hosts");
  cJSON* switches = cJSON_GetObjectItemCaseSensitive(root, "switches");
  cJSON* routers = cJSON_GetObjectItemCaseSensitive(root, "routers");
  cJSON* links = cJSON_GetObjectItemCaseSensitive(root, "links");

  if (hosts != NULL && (status = load_node_array(hosts, topology, TOPOLOGY_NODE_HOST)) != MAGI_OK) {
    cJSON_Delete(root);
    return status;
  }
  if (switches != NULL &&
      (status = load_node_array(switches, topology, TOPOLOGY_NODE_SWITCH)) != MAGI_OK) {
    cJSON_Delete(root);
    return status;
  }
  if (routers != NULL &&
      (status = load_node_array(routers, topology, TOPOLOGY_NODE_ROUTER)) != MAGI_OK) {
    cJSON_Delete(root);
    return status;
  }
  if (links != NULL && (status = load_link_array(links, topology)) != MAGI_OK) {
    cJSON_Delete(root);
    return status;
  }

  cJSON_Delete(root);
  return MAGI_OK;
}

/**
 * @brief Serialize one node kind section into a cJSON object.
 */
static cJSON* save_node_array(const Topology* topology, TopologyNodeKind kind) {
  cJSON* array = cJSON_CreateArray();
  if (array == NULL) {
    return NULL;
  }

  TopologyNodeInfo** items = NULL;
  size_t count = 0U;

  if (collect_node_array(topology, kind, &items, &count) != MAGI_OK) {
    cJSON_Delete(array);
    return NULL;
  }

  for (size_t index = 0U; index < count; ++index) {
    TopologyNodeInfo* info = items[index];
    cJSON* node_obj = cJSON_CreateObject();
    if (node_obj == NULL) {
      goto fail;
    }

    if (!cJSON_AddStringToObject(node_obj, "name", info->node->name)) {
      cJSON_Delete(node_obj);
      goto fail;
    }

    if (kind == TOPOLOGY_NODE_HOST) {
      if (info->ip_address[0] != '\0' &&
          !cJSON_AddStringToObject(node_obj, "ip_address", info->ip_address)) {
        cJSON_Delete(node_obj);
        goto fail;
      }
      if (info->default_gateway[0] != '\0' &&
          !cJSON_AddStringToObject(node_obj, "default_gateway", info->default_gateway)) {
        cJSON_Delete(node_obj);
        goto fail;
      }
      cJSON_AddItemToArray(array, node_obj);
      continue;
    }

    size_t port_count = info->node->interfaces != NULL ? info->node->interfaces->count : 0U;
    if (kind == TOPOLOGY_NODE_SWITCH) {
      uint16_t port_limit =
          info->num_ports > node_max_port(info->node) ? info->num_ports : node_max_port(info->node);
      if (!cJSON_AddNumberToObject(node_obj, "num_ports", port_limit)) {
        cJSON_Delete(node_obj);
        goto fail;
      }

      cJSON* vlans = cJSON_CreateArray();
      if (vlans == NULL) {
        cJSON_Delete(node_obj);
        goto fail;
      }

      for (uint16_t port = 1U; port <= port_limit; ++port) {
        char mode[8] = {0};
        uint16_t vlan_id = 0U;
        if (!topology_get_switch_port_config(topology, info->node->name, port, mode, sizeof(mode),
                                             &vlan_id)) {
          continue;
        }

        cJSON* vlan_obj = cJSON_CreateObject();
        if (vlan_obj == NULL) {
          cJSON_Delete(node_obj);
          goto fail;
        }

        if (!cJSON_AddNumberToObject(vlan_obj, "port", port) ||
            !cJSON_AddStringToObject(vlan_obj, "mode", mode)) {
          cJSON_Delete(vlan_obj);
          cJSON_Delete(node_obj);
          goto fail;
        }

        if (strcmp(mode, "access") == 0 && !cJSON_AddNumberToObject(vlan_obj, "vlan_id", vlan_id)) {
          cJSON_Delete(vlan_obj);
          cJSON_Delete(node_obj);
          goto fail;
        }

        cJSON_AddItemToArray(vlans, vlan_obj);
      }

      cJSON_AddItemToObject(node_obj, "vlans", vlans);
      cJSON_AddItemToArray(array, node_obj);
      continue;
    }

    cJSON* iface_array = cJSON_CreateArray();
    if (iface_array == NULL) {
      cJSON_Delete(node_obj);
      goto fail;
    }

    if (port_count > 0U) {
      uint16_t* ports = calloc(port_count, sizeof(*ports));
      if (ports == NULL) {
        cJSON_Delete(iface_array);
        cJSON_Delete(node_obj);
        goto fail;
      }

      PortCollectCtx ctx = {.ports = ports, .index = 0U};
      hashmap_foreach(info->node->interfaces, collect_port_info_cb, &ctx);
      qsort(ports, ctx.index, sizeof(*ports), compare_uint16_values);

      for (size_t port_index = 0U; port_index < ctx.index; ++port_index) {
        Interface* iface = node_get_interface(info->node, ports[port_index]);
        cJSON* iface_obj = cJSON_CreateObject();
        if (iface_obj == NULL) {
          free(ports);
          cJSON_Delete(iface_array);
          cJSON_Delete(node_obj);
          goto fail;
        }
        if (!cJSON_AddNumberToObject(iface_obj, "port", ports[port_index])) {
          free(ports);
          cJSON_Delete(iface_obj);
          cJSON_Delete(iface_array);
          cJSON_Delete(node_obj);
          goto fail;
        }
        if (iface != NULL && iface->ip_address[0] != '\0' &&
            !cJSON_AddStringToObject(iface_obj, "ip_address", iface->ip_address)) {
          free(ports);
          cJSON_Delete(iface_obj);
          cJSON_Delete(iface_array);
          cJSON_Delete(node_obj);
          goto fail;
        }
        if (iface != NULL && iface->vlan_id != 0U &&
            !cJSON_AddNumberToObject(iface_obj, "vlan_id", iface->vlan_id)) {
          free(ports);
          cJSON_Delete(iface_obj);
          cJSON_Delete(iface_array);
          cJSON_Delete(node_obj);
          goto fail;
        }
        cJSON_AddItemToArray(iface_array, iface_obj);
      }
      free(ports);
    }

    cJSON_AddItemToObject(node_obj, "interfaces", iface_array);
    cJSON* routing_table = cJSON_CreateArray();
    if (routing_table == NULL) {
      cJSON_Delete(node_obj);
      goto fail;
    }
    if (kind == TOPOLOGY_NODE_ROUTER && topology->node_ops != NULL &&
        topology->node_ops->foreach_router_route != NULL) {
      RouteSaveCtx route_ctx = {.array = routing_table, .failed = false};
      topology->node_ops->foreach_router_route(info->node, save_route_cb, &route_ctx);
      if (route_ctx.failed) {
        cJSON_Delete(node_obj);
        goto fail;
      }
    }
    cJSON_AddItemToObject(node_obj, "routing_table", routing_table);
    cJSON_AddItemToArray(array, node_obj);
  }

  free(items);
  return array;

fail:
  free(items);
  cJSON_Delete(array);
  return NULL;
}

/**
 * @brief Serialize the topology link section into a cJSON array.
 */
static cJSON* save_link_array(const Topology* topology) {
  cJSON* array = cJSON_CreateArray();
  if (array == NULL) {
    return NULL;
  }

  TopologyLinkInfo** items = NULL;
  size_t count = 0U;

  if (collect_link_array(topology, &items, &count) != MAGI_OK) {
    cJSON_Delete(array);
    return NULL;
  }

  for (size_t index = 0U; index < count; ++index) {
    TopologyLinkInfo* info = items[index];
    cJSON* link_obj = cJSON_CreateObject();
    if (link_obj == NULL) {
      goto fail;
    }

    char endpoint_a[80];
    char endpoint_b[80];
    TopologyNodeInfo* node_a = topology_get_node_info(topology, info->node_a);
    TopologyNodeInfo* node_b = topology_get_node_info(topology, info->node_b);
    if (node_a != NULL && node_a->kind == TOPOLOGY_NODE_HOST && info->port_a == 1U) {
      snprintf(endpoint_a, sizeof(endpoint_a), "%s", info->node_a);
    } else {
      snprintf(endpoint_a, sizeof(endpoint_a), "%s:%u", info->node_a, (unsigned)info->port_a);
    }
    if (node_b != NULL && node_b->kind == TOPOLOGY_NODE_HOST && info->port_b == 1U) {
      snprintf(endpoint_b, sizeof(endpoint_b), "%s", info->node_b);
    } else {
      snprintf(endpoint_b, sizeof(endpoint_b), "%s:%u", info->node_b, (unsigned)info->port_b);
    }

    cJSON* endpoints = cJSON_CreateArray();
    if (endpoints == NULL) {
      cJSON_Delete(link_obj);
      goto fail;
    }
    cJSON_AddItemToArray(endpoints, cJSON_CreateString(endpoint_a));
    cJSON_AddItemToArray(endpoints, cJSON_CreateString(endpoint_b));

    if (!cJSON_AddItemToObject(link_obj, "endpoints", endpoints) ||
        !cJSON_AddNumberToObject(link_obj, "delay", info->link->delay_ms) ||
        !cJSON_AddNumberToObject(link_obj, "mtu", info->link->mtu)) {
      cJSON_Delete(link_obj);
      goto fail;
    }

    cJSON_AddItemToArray(array, link_obj);
  }

  free(items);
  return array;

fail:
  free(items);
  cJSON_Delete(array);
  return NULL;
}

/**
 * @brief Save topology state to a JSON file.
 *
 * Serialises all hosts, switches, routers, and links into a JSON
 * document and writes it to the given path.  If @p path is NULL, the
 * topology's internal @c source_path is used instead.
 *
 * @param topology Source topology object.
 * @param path     Output file path, or NULL to use topology->source_path.
 * @return MAGI_OK on success, or an error code on failure.
 */
int topology_save_file(const Topology* topology, const char* path) {
  if (topology == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  const char* resolved_path = path != NULL ? path : topology->source_path;
  if (resolved_path == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  cJSON* root = cJSON_CreateObject();
  if (root == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  cJSON* hosts = save_node_array(topology, TOPOLOGY_NODE_HOST);
  cJSON* switches = save_node_array(topology, TOPOLOGY_NODE_SWITCH);
  cJSON* routers = save_node_array(topology, TOPOLOGY_NODE_ROUTER);
  cJSON* links = save_link_array(topology);

  if (hosts == NULL || switches == NULL || routers == NULL || links == NULL) {
    cJSON_Delete(root);
    cJSON_Delete(hosts);
    cJSON_Delete(switches);
    cJSON_Delete(routers);
    cJSON_Delete(links);
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  cJSON_AddItemToObject(root, "hosts", hosts);
  cJSON_AddItemToObject(root, "switches", switches);
  cJSON_AddItemToObject(root, "routers", routers);
  cJSON_AddItemToObject(root, "links", links);

  char* json_string = cJSON_Print(root);
  cJSON_Delete(root);

  if (json_string == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  FILE* file = fopen(resolved_path, "wb");
  if (file == NULL) {
    free(json_string);
    return MAGI_ERR_BADARGS;
  }

  fputs(json_string, file);
  fclose(file);
  free(json_string);

  return MAGI_OK;
}

/**
 * @brief Load topology state from a JSON file.
 *
 * Reads the file at the given path, parses the JSON into host, switch,
 * router, and link definitions, and populates the topology.  If @p path
 * is NULL, the topology's internal @c source_path is used instead.
 * Loading is performed into a staging topology first; on success the
 * staging state is atomically swapped into the destination topology
 * via topology_replace_contents().
 *
 * @param topology Destination topology object.
 * @param path     Input file path, or NULL to use topology->source_path.
 * @return MAGI_OK on success, or an error code on failure.
 */
int topology_load_file(Topology* topology, const char* path) {
  if (topology == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  const char* resolved_path = path != NULL ? path : topology->source_path;
  if (resolved_path == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  char* buffer = NULL;
  size_t length = 0U;
  int status = read_file(resolved_path, &buffer, &length);
  if (status != MAGI_OK) {
    return status;
  }

  Topology* staging = topology_new();
  if (staging == NULL) {
    free(buffer);
    return MAGI_ERR_NOMEM;
  }
  topology_set_node_ops(staging, topology->node_ops);

  free(staging->source_path);
  staging->source_path = duplicate_string(resolved_path);
  if (staging->source_path == NULL) {
    topology_free(staging);
    free(buffer);
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  status = parse_topology_json(staging, buffer);
  free(buffer);

  if (status != MAGI_OK) {
    topology_free(staging);
    return status;
  }

  status = topology_replace_contents(topology, staging);
  topology_free(staging);

  return status;
}
