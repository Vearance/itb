#define _POSIX_C_SOURCE 200809L

#include "commands.h"

#include "core/interface.h"
#include "core/node.h"
#include "layer2/arp.h"
#include "layer2/host.h"
#include "layer2/switch.h"
#include "layer3/ipv4.h"
#include "layer3/router.h"
#include "layer4/l4_host.h"
#include "layer4/port_registry.h"
#include "layer4/tcp.h"
#include "layer4/tcp_socket.h"
#include "layer7/dhcp.h"
#include "layer7/http.h"
#include "layer7/magi_socket.h"
#include "topology/json_loader.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct EndpointRef {
  TopologyNodeInfo* node_info;
  uint16_t port;
} EndpointRef;

/**
 * @brief Duplicate a string onto the heap.
 */
static char* duplicate_string(const char* text) {
  if (text == NULL) {
    return NULL;
  }

  size_t length = strlen(text) + 1U;
  char* copy = malloc(length);
  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, text, length);
  return copy;
}

/**
 * @brief Parse an unsigned 32-bit integer from text.
 */
static int parse_uint32(const char* text, uint32_t* value_out) {
  if (text == NULL || value_out == NULL) {
    return MAGI_ERR_BADARGS;
  }

  char* end = NULL;
  unsigned long value = strtoul(text, &end, 10);
  if (end == text || *end != '\0' || value > 0xFFFFFFFFUL) {
    return MAGI_ERR_BADARGS;
  }

  *value_out = (uint32_t)value;
  return MAGI_OK;
}

/**
 * @brief Parse an unsigned 16-bit integer from text.
 */
static int parse_uint16(const char* text, uint16_t* value_out) {
  uint32_t value = 0U;
  int status = parse_uint32(text, &value);
  if (status != MAGI_OK || value > 0xFFFFU) {
    return MAGI_ERR_BADARGS;
  }

  *value_out = (uint16_t)value;
  return MAGI_OK;
}

/**
 * @brief Resolve a CLI endpoint specification into a node and port.
 */
static int resolve_endpoint(const Topology* topology, const char* spec, EndpointRef* endpoint_out) {
  if (topology == NULL || spec == NULL || endpoint_out == NULL) {
    LOG("CLI", "Internal error: resolve_endpoint received NULL arguments");
    return MAGI_ERR_BADARGS;
  }

  const char* colon = strchr(spec, ':');
  char node_name[64];
  uint16_t port = 0U;

  if (colon == NULL) {
    TopologyNodeInfo* node_info = topology_get_node_info(topology, spec);
    if (node_info == NULL) {
      LOG("CLI", "Endpoint '%s' does not exist in topology", spec);
      return MAGI_ERR_BADARGS;
    }

    if (node_info->kind != TOPOLOGY_NODE_HOST) {
      LOG("CLI",
          "Endpoint '%s' is not a host (only hosts can be referenced without a port, e.g. H1)",
          spec);
      return MAGI_ERR_BADARGS;
    }

    snprintf(node_name, sizeof(node_name), "%s", spec);
    port = 1U;
    endpoint_out->node_info = node_info;
    endpoint_out->port = port;
    return MAGI_OK;
  }

  size_t name_len = (size_t)(colon - spec);
  if (name_len == 0U) {
    LOG("CLI", "Invalid endpoint '%s': node name is empty before the colon", spec);
    return MAGI_ERR_BADARGS;
  }

  if (name_len >= sizeof(node_name)) {
    LOG("CLI", "Invalid endpoint '%s': node name is too long (max %zu characters)", spec,
        sizeof(node_name) - 1U);
    return MAGI_ERR_BADARGS;
  }

  memcpy(node_name, spec, name_len);
  node_name[name_len] = '\0';
  if (parse_uint16(colon + 1, &port) != MAGI_OK || port == 0U) {
    LOG("CLI", "Invalid endpoint '%s': port must be a positive integer between 1 and 65535", spec);
    return MAGI_ERR_BADARGS;
  }

  TopologyNodeInfo* node_info = topology_get_node_info(topology, node_name);
  if (node_info == NULL) {
    LOG("CLI", "Endpoint '%s': node '%s' does not exist in topology", spec, node_name);
    return MAGI_ERR_BADARGS;
  }

  endpoint_out->node_info = node_info;
  endpoint_out->port = port;
  return MAGI_OK;
}

/**
 * @brief Report a not-yet-implemented node action.
 */
static int print_unimplemented(const char* command_name, const char* node_name) {
  LOG(node_name != NULL ? node_name : "CLI", "%s is not implemented yet", command_name);
  return MAGI_OK;
}

/**
 * @brief Check whether a string is a valid IPv4 address.
 *
 * Delegates to arp_ipv4_from_string() for validation.
 *
 * @param target String to validate.
 * @return true if target is a valid dotted-decimal IPv4 address, false otherwise.
 */
static bool is_ipv4_target(const char* target) {
  uint8_t ip[4];
  return arp_ipv4_from_string(target, ip) == MAGI_OK;
}

/**
 * @brief Print CLI command usage summary to the log.
 *
 * Lists all available topology commands, host actions, router actions,
 * switch actions, and not-yet-implemented features.
 */
void commands_print_help(void) {
  LOG("CLI", "=== Topology Commands ===");
  LOG("CLI", "  create <host|switch|router> <name>");
  LOG("CLI", "  link <dev1> <dev2> [delay_ms] [mtu]");
  LOG("CLI", "  unlink <dev1> <dev2>");
  LOG("CLI", "  topology");
  LOG("CLI", "  save [filename]");
  LOG("CLI", "  load [filename]");
  LOG("CLI", "  help");
  LOG("CLI", "  exit | quit");
  LOG("CLI", "");
  LOG("CLI", "=== Host Actions ===");
  LOG("CLI", "  <host> ping <ip>");
  LOG("CLI", "  <host> traceroute <ip> [max_hops]");
  LOG("CLI", "  <host> arp");
  LOG("CLI", "  <host> tcp_connect <ip> <port>");
  LOG("CLI", "");
  LOG("CLI", "=== Router Actions ===");
  LOG("CLI", "  <router> route");
  LOG("CLI", "  <router> route add <dest_cidr> <next_hop|direct> <out_port>");
  LOG("CLI", "  <router> route del <dest_cidr>");
  LOG("CLI", "  <router> arp");
  LOG("CLI", "");
  LOG("CLI", "=== Switch Actions ===");
  LOG("CLI", "  <switch> mac");
  LOG("CLI", "");
  LOG("CLI", "=== Not Yet Implemented ===");
  LOG("CLI", "  http_get, http_server, dhcp_discover, visualize, acl, rip");
}

/**
 * @brief Create a new node in the topology.
 *
 * Validates the node type, checks for duplicate names, creates the node
 * via topology_add_node(), and logs the result.
 *
 * @param topology Mutable topology context.
 * @param type Node type text ("host", "switch", or "router").
 * @param name Unique node identifier.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS on invalid input or duplicate.
 */
int cmd_create(Topology* topology, const char* type, const char* name) {
  if (topology == NULL || type == NULL || name == NULL) {
    LOG("CLI", "create: missing arguments. Usage: create <type> <name>");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  TopologyNodeKind kind = topology_parse_kind(type);
  if (kind < TOPOLOGY_NODE_HOST || kind > TOPOLOGY_NODE_ROUTER) {
    LOG("CLI", "create: invalid node type '%s'. Valid types are: host, switch, router", type);
    return MAGI_ERR_BADARGS;
  }

  if (topology_get_node_info(topology, name) != NULL) {
    LOG("CLI", "create: node '%s' already exists in topology", name);
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (topology_add_node(topology, kind, name) == NULL) {
    LOG("CLI", "create: failed to create node '%s' (out of memory?)", name);
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  const char* kind_name = kind == TOPOLOGY_NODE_HOST     ? "host"
                          : kind == TOPOLOGY_NODE_SWITCH ? "switch"
                                                         : "router";
  LOG("TOPO", "Created %s %s", kind_name, name);
  return MAGI_OK;
}

/**
 * @brief Create a link between two topology endpoints.
 *
 * Resolves both endpoint specs, then calls topology_add_link() to establish
 * the bidirectional connection.
 *
 * @param topology Mutable topology context.
 * @param dev1 First endpoint spec ("Name" or "Name:Port").
 * @param dev2 Second endpoint spec ("Name" or "Name:Port").
 * @param delay_ms Link propagation delay in milliseconds.
 * @param mtu Link maximum transmission unit in bytes.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS if endpoints are invalid or link fails.
 */
int cmd_link(Topology* topology, const char* dev1, const char* dev2, uint32_t delay_ms,
             uint16_t mtu) {
  if (topology == NULL || dev1 == NULL || dev2 == NULL) {
    LOG("CLI", "link: missing arguments. Usage: link <dev1> <dev2> [delay_ms] [mtu]");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  EndpointRef endpoint_a = {0};
  EndpointRef endpoint_b = {0};

  if (resolve_endpoint(topology, dev1, &endpoint_a) != MAGI_OK ||
      resolve_endpoint(topology, dev2, &endpoint_b) != MAGI_OK) {
    return MAGI_ERR_BADARGS;
  }

  if (topology_add_link(topology, endpoint_a.node_info->node->name, endpoint_a.port,
                        endpoint_b.node_info->node->name, endpoint_b.port, delay_ms, mtu) == NULL) {
    LOG("CLI",
        "link: failed to create link between %s and %s. Possible causes: link already "
        "exists, or one/both interfaces are already connected to another link",
        dev1, dev2);
    return MAGI_ERR_BADARGS;
  }

  LOG("TOPO", "Linked %s <-> %s delay=%ums mtu=%u", dev1, dev2, (unsigned)delay_ms, (unsigned)mtu);
  return MAGI_OK;
}

/**
 * @brief Remove an existing link between two topology endpoints.
 *
 * Resolves both endpoint specs, then calls topology_remove_link() to tear
 * down the connection.
 *
 * @param topology Mutable topology context.
 * @param dev1 First endpoint spec ("Name" or "Name:Port").
 * @param dev2 Second endpoint spec ("Name" or "Name:Port").
 * @return MAGI_OK on success, MAGI_ERR_BADARGS if endpoints are invalid or no link exists.
 */
int cmd_unlink(Topology* topology, const char* dev1, const char* dev2) {
  if (topology == NULL || dev1 == NULL || dev2 == NULL) {
    LOG("CLI", "unlink: missing arguments. Usage: unlink <dev1> <dev2>");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  EndpointRef endpoint_a = {0};
  EndpointRef endpoint_b = {0};

  if (resolve_endpoint(topology, dev1, &endpoint_a) != MAGI_OK ||
      resolve_endpoint(topology, dev2, &endpoint_b) != MAGI_OK) {
    return MAGI_ERR_BADARGS;
  }

  if (topology_remove_link(topology, endpoint_a.node_info->node->name, endpoint_a.port,
                           endpoint_b.node_info->node->name, endpoint_b.port) != MAGI_OK) {
    LOG("CLI", "unlink: no link found between %s:%u and %s:%u", dev1, endpoint_a.port, dev2,
        endpoint_b.port);
    return MAGI_ERR_BADARGS;
  }

  LOG("TOPO", "Unlinked %s <-> %s", dev1, dev2);
  return MAGI_OK;
}

/**
 * @brief Print the current topology summary.
 *
 * Delegates to topology_print() which enumerates all nodes and links.
 *
 * @param topology Topology context to display.
 * @return MAGI_OK.
 */
int cmd_topology(Topology* topology) {
  topology_print(topology);
  return MAGI_OK;
}

/**
 * @brief Save the current topology to a JSON file on disk.
 *
 * Uses the provided filename, or falls back to topology->source_path,
 * or finally to "topology.json". Updates topology->source_path on success.
 *
 * @param topology Mutable topology context.
 * @param filename Output file path, or NULL to use current source path.
 * @return MAGI_OK on success, otherwise an error code.
 */
int cmd_save(Topology* topology, const char* filename) {
  if (topology == NULL) {
    LOG("CLI", "save: internal error (topology is NULL)");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  const char* requested_path = filename != NULL ? filename : topology->source_path;
  if (requested_path == NULL) {
    requested_path = "topology.json";
  }

  int status = topology_save_file(topology, requested_path);
  if (status != MAGI_OK) {
    LOG("CLI", "save: unable to save topology to '%s': %s", requested_path, strerror(errno));
    return status;
  }

  char* new_path = duplicate_string(requested_path);
  if (new_path != NULL) {
    free(topology->source_path);
    topology->source_path = new_path;
    LOG("TOPO", "Saved topology to %s", topology->source_path);
  } else {
    LOG("TOPO", "Saved topology to %s", requested_path);
  }
  return MAGI_OK;
}

/**
 * @brief Load a topology from a JSON file on disk.
 *
 * Replaces the entire current topology with the one loaded from the file.
 * Uses the provided filename, or falls back to topology->source_path,
 * or finally to "topology.json".
 *
 * @param topology Mutable topology context.
 * @param filename Input file path, or NULL to use current source path.
 * @return MAGI_OK on success, otherwise an error code.
 */
int cmd_load(Topology* topology, const char* filename) {
  if (topology == NULL) {
    LOG("CLI", "load: internal error (topology is NULL)");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  const char* target = filename != NULL ? filename : topology->source_path;
  if (target == NULL) {
    target = "topology.json";
  }

  int status = topology_load_file(topology, target);
  if (status != MAGI_OK) {
    LOG("CLI", "load: unable to load topology from '%s': %s", target, strerror(errno));
    return status;
  }

  LOG("TOPO", "Loaded %zu hosts, %zu switches, %zu routers, %zu links",
      topology_count_nodes_of_kind(topology, TOPOLOGY_NODE_HOST),
      topology_count_nodes_of_kind(topology, TOPOLOGY_NODE_SWITCH),
      topology_count_nodes_of_kind(topology, TOPOLOGY_NODE_ROUTER), topology_count_links(topology));
  return MAGI_OK;
}

/**
 * @brief Dispatch node-scoped subcommands by node type.
 *
 * Given a command line where argv[0] is a node name, looks up the node
 * and dispatches argv[1] as an action (ping, traceroute, arp, mac,
 * tcp_connect, route, etc.) with remaining arguments. Each action
 * validates that the node kind supports the operation.
 *
 * @param topology Mutable topology context.
 * @param argc Number of tokens in argv.
 * @param argv Token array; argv[0] = node name, argv[1] = action.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS on invalid node/action/args.
 */
static int dispatch_node_action(Topology* topology, int argc, char** argv) {
  if (argc < 2) {
    LOG("CLI", "Node '%s': missing subcommand. Usage: <node> <action> [args...]", argv[0]);
    return MAGI_ERR_BADARGS;
  }

  TopologyNodeInfo* node_info = topology_get_node_info(topology, argv[0]);
  if (node_info == NULL) {
    LOG("CLI", "Unknown command '%s'. Type 'help' for usage.", argv[0]);
    return MAGI_ERR_BADARGS;
  }

  if (strcmp(argv[1], "ping") == 0) {
    if (node_info->kind != TOPOLOGY_NODE_HOST) {
      LOG("CLI", "ping is only available on hosts");
      return MAGI_ERR_BADARGS;
    }
    if (argc < 3) {
      LOG("CLI", "ping: missing target IP. Usage: <host> ping <ip>");
      return MAGI_ERR_BADARGS;
    }

    if (!is_ipv4_target(argv[2])) {
      LOG("CLI", "ping: target '%s' must be an IPv4 address", argv[2]);
      return MAGI_ERR_BADARGS;
    }

    return ipv4_host_ping(node_info->node, argv[2]);
  }

  if (strcmp(argv[1], "traceroute") == 0) {
    if (node_info->kind != TOPOLOGY_NODE_HOST) {
      LOG("CLI", "traceroute is only available on hosts");
      return MAGI_ERR_BADARGS;
    }
    if (argc < 3) {
      LOG("CLI", "traceroute: missing target IP. Usage: <host> traceroute <ip> [max_hops]");
      return MAGI_ERR_BADARGS;
    }
    if (!is_ipv4_target(argv[2])) {
      LOG("CLI", "traceroute: target '%s' must be an IPv4 address", argv[2]);
      return MAGI_ERR_BADARGS;
    }

    uint16_t max_hops = 30U;
    if (argc >= 4 &&
        (parse_uint16(argv[3], &max_hops) != MAGI_OK || max_hops == 0U || max_hops > 255U)) {
      LOG("CLI", "traceroute: max_hops must be an integer between 1 and 255");
      return MAGI_ERR_BADARGS;
    }

    return ipv4_host_traceroute(node_info->node, argv[2], (uint8_t)max_hops);
  }

  if (strcmp(argv[1], "arp") == 0) {
    if (node_info->kind == TOPOLOGY_NODE_HOST) {
      host_print_arp_cache(host_from_node_const(node_info->node));
      return MAGI_OK;
    }
    if (node_info->kind == TOPOLOGY_NODE_ROUTER) {
      Router* router = router_from_node(node_info->node);
      router_print_arp_cache(router);
      return MAGI_OK;
    }
    LOG("CLI", "arp inspection is implemented for hosts and routers only");
    return MAGI_ERR_BADARGS;
  }

  if (strcmp(argv[1], "mac") == 0) {
    if (node_info->kind != TOPOLOGY_NODE_SWITCH) {
      LOG("CLI", "mac inspection is only available on switches");
      return MAGI_ERR_BADARGS;
    }
    switch_print_mac_table(switch_from_node_const(node_info->node));
    return MAGI_OK;
  }

  if (strcmp(argv[1], "tcp_connect") == 0) {
    if (node_info->kind != TOPOLOGY_NODE_HOST) {
      LOG("CLI", "tcp_connect is only available on hosts");
      return MAGI_ERR_BADARGS;
    }
    if (argc < 4) {
      LOG("CLI", "tcp_connect: missing arguments. Usage: <host> tcp_connect <ip> <port>");
      return MAGI_ERR_BADARGS;
    }

    uint16_t remote_port = 0U;
    if (parse_uint16(argv[3], &remote_port) != MAGI_OK || remote_port == 0U) {
      LOG("CLI", "tcp_connect: port must be an integer between 1 and 65535");
      return MAGI_ERR_BADARGS;
    }

    Node* node = node_info->node;
    HashMap* reg = (HashMap*)l4_host_get_registry(node);
    if (reg == NULL) {
      LOG(argv[0], "tcp_connect: L4 is not attached (port registry missing)");
      return MAGI_ERR_BADARGS;
    }

    /* Create TCP socket */
    TCPSocket* sock = tcp_socket_new(node);
    if (sock == NULL) {
      LOG(argv[0], "tcp_connect: failed to create socket (out of memory)");
      return MAGI_ERR_NOMEM;
    }

    /* Bind to ephemeral local port and register */
    sock->local_port = (uint16_t)(49152U + ((uintptr_t)sock & 0x3FFFU));
    uint16_t local_port_tmp = sock->local_port;
    int status = port_registry_bind(reg, PORT_PROTOCOL_TCP, local_port_tmp, sock);
    if (status != MAGI_OK) {
      LOG(argv[0], "tcp_connect: failed to bind to local port %u", (unsigned)local_port_tmp);
      tcp_socket_free(sock);
      return status;
    }

    /* Initiate connection */
    LOG(argv[0], "TCP connect to %s:%u (local port %u)...", argv[2], (unsigned)remote_port,
        (unsigned)local_port_tmp);
    status = tcp_socket_connect(sock, node, argv[2], remote_port);
    if (status != MAGI_OK) {
      LOG(argv[0], "TCP connect failed (err=%d)", status);
      port_registry_unbind(reg, PORT_PROTOCOL_TCP, local_port_tmp);
      tcp_socket_free(sock);
      return status;
    }

    if (sock->state == TCP_ESTABLISHED) {
      LOG(argv[0], "TCP connection established (3-way handshake complete)");

      /* Send data */
      static const uint8_t hello[] = "HELLO\n";
      status = tcp_socket_send(sock, node, hello, sizeof(hello) - 1U);
      if (status == MAGI_OK) {
        LOG(argv[0], "TCP sent %zu bytes", sizeof(hello) - 1U);
      }

      /* Read any data that arrived */
      if (tcp_socket_has_data(sock)) {
        uint8_t recv_buf[1024];
        size_t rd = tcp_recv_buf_read(sock, recv_buf, sizeof(recv_buf) - 1U);
        recv_buf[rd] = '\0';
        LOG(argv[0], "TCP received %zu bytes: %s", rd, (const char*)recv_buf);
      }
    } else {
      LOG(argv[0], "TCP connection in progress (state=%s)", tcp_state_name(sock->state));
    }

    return MAGI_OK;
  }

  if (strcmp(argv[1], "http_get") == 0) {
    if (node_info->kind != TOPOLOGY_NODE_HOST) {
      LOG("CLI", "http_get is only available on hosts");
      return MAGI_ERR_BADARGS;
    }
    if (argc < 3) {
      LOG("CLI", "http_get: missing URL. Usage: <host> http_get <url>");
      return MAGI_ERR_BADARGS;
    }
    return http_get(node_info->node, argv[2]);
  }

  if (strcmp(argv[1], "http_server") == 0) {
    if (node_info->kind != TOPOLOGY_NODE_HOST) {
      LOG("CLI", "http_server is only available on hosts");
      return MAGI_ERR_BADARGS;
    }
    if (argc >= 3 && strcmp(argv[2], "stop") == 0) {
      return http_server_stop(node_info->node);
    }
    if (argc >= 3 && strcmp(argv[2], "start") == 0) {
      const char* web_root = (argc >= 4) ? argv[3] : NULL;
      return http_server_start(node_info->node, web_root);
    }
    LOG("CLI", "http_server: Usage: <host> http_server start [file] | <host> http_server stop");
    return MAGI_ERR_BADARGS;
  }

  if (strcmp(argv[1], "dhcp_discover") == 0) {
    if (node_info->kind != TOPOLOGY_NODE_HOST) {
      LOG("CLI", "dhcp_discover is only available on hosts");
      return MAGI_ERR_BADARGS;
    }
    return dhcp_client_discover(node_info->node);
  }

  if (strcmp(argv[1], "route") == 0) {
    if (node_info->kind != TOPOLOGY_NODE_ROUTER) {
      LOG("CLI", "route inspection is only available on routers");
      return MAGI_ERR_BADARGS;
    }

    Router* router = router_from_node(node_info->node);
    if (argc == 2) {
      router_print_routes(router);
      return MAGI_OK;
    }

    if (strcmp(argv[2], "add") == 0) {
      if (argc < 6) {
        LOG("CLI", "route add: Usage: <router> route add <dest_cidr> <next_hop|direct> <out_port>");
        return MAGI_ERR_BADARGS;
      }

      uint16_t out_port = 0U;
      if (parse_uint16(argv[5], &out_port) != MAGI_OK || out_port == 0U) {
        LOG("CLI", "route add: out_port must be a positive integer");
        return MAGI_ERR_BADARGS;
      }

      int status = router_add_route(router, argv[3], argv[4], out_port);
      if (status == MAGI_OK) {
        LOG(argv[0], "Route added: %s via %s port %u", argv[3], argv[4], (unsigned)out_port);
      }
      return status;
    }

    if (strcmp(argv[2], "del") == 0 || strcmp(argv[2], "delete") == 0 ||
        strcmp(argv[2], "remove") == 0) {
      if (argc < 4) {
        LOG("CLI", "route del: Usage: <router> route del <dest_cidr>");
        return MAGI_ERR_BADARGS;
      }

      int status = router_remove_route(router, argv[3]);
      if (status == MAGI_OK) {
        LOG(argv[0], "Route removed: %s", argv[3]);
      }
      return status;
    }

    LOG("CLI", "route: Usage: <router> route | <router> route add <dest_cidr> <next_hop|direct> "
               "<out_port> | <router> route del <dest_cidr>");
    return MAGI_ERR_BADARGS;
  }

  if (strcmp(argv[1], "visualize") == 0) {
    return print_unimplemented("visualize", argv[0]);
  }

  if (strcmp(argv[1], "acl") == 0) {
    return print_unimplemented("acl", argv[0]);
  }

  if (strcmp(argv[1], "rip") == 0) {
    return print_unimplemented("rip", argv[0]);
  }

  LOG("CLI", "Node '%s': unknown action '%s'. Type 'help' for usage.", argv[0], argv[1]);
  return MAGI_ERR_BADARGS;
}

/**
 * @brief Request clean shutdown of the CLI loop.
 *
 * @return CLI_EXIT_REQUEST, which signals cli_run() to break the input loop.
 */
int cmd_exit(void) {
  return CLI_EXIT_REQUEST;
}

/**
 * @brief Dispatch one tokenized CLI command line.
 *
 * Matches argv[0] against known root-level commands (help, exit, quit,
 * create, link, unlink, topology, save, load). If no match is found,
 * falls through to dispatch_node_action() which treats argv[0] as a
 * node name for node-scoped subcommands.
 *
 * @param topology Mutable topology context.
 * @param argc Number of tokens in argv.
 * @param argv Token array from the parsed input line.
 * @return MAGI_OK, CLI_EXIT_REQUEST, or an error code from the dispatched handler.
 */
int commands_dispatch(Topology* topology, int argc, char** argv) {
  if (topology == NULL || argc <= 0 || argv == NULL || argv[0] == NULL) {
    LOG("CLI", "Internal error: invalid command dispatch arguments");
    return MAGI_ERR_BADARGS;
  }

  if (strcmp(argv[0], "help") == 0) {
    commands_print_help();
    return MAGI_OK;
  }

  if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0) {
    return cmd_exit();
  }

  if (strcmp(argv[0], "create") == 0) {
    if (argc < 3) {
      LOG("CLI", "create: missing arguments. Usage: create <type> <name>");
      return MAGI_ERR_BADARGS;
    }

    return cmd_create(topology, argv[1], argv[2]);
  }

  if (strcmp(argv[0], "link") == 0) {
    uint32_t delay_ms = 0U;
    uint16_t mtu = 1500U;

    if (argc < 3) {
      LOG("CLI", "link: missing arguments. Usage: link <dev1> <dev2> [delay_ms] [mtu]");
      return MAGI_ERR_BADARGS;
    }

    if (argc >= 4) {
      if (parse_uint32(argv[3], &delay_ms) != MAGI_OK) {
        LOG("CLI", "link: invalid delay value '%s'. Must be a non-negative integer", argv[3]);
        return MAGI_ERR_BADARGS;
      }
    }

    if (argc >= 5) {
      if (parse_uint16(argv[4], &mtu) != MAGI_OK) {
        LOG("CLI", "link: invalid MTU value '%s'. Must be an integer between 1 and 65535", argv[4]);
        return MAGI_ERR_BADARGS;
      }
    }

    return cmd_link(topology, argv[1], argv[2], delay_ms, mtu);
  }

  if (strcmp(argv[0], "unlink") == 0) {
    if (argc < 3) {
      LOG("CLI", "unlink: missing arguments. Usage: unlink <dev1> <dev2>");
      return MAGI_ERR_BADARGS;
    }

    return cmd_unlink(topology, argv[1], argv[2]);
  }

  if (strcmp(argv[0], "topology") == 0) {
    return cmd_topology(topology);
  }

  if (strcmp(argv[0], "save") == 0) {
    return cmd_save(topology, argc >= 2 ? argv[1] : NULL);
  }

  if (strcmp(argv[0], "load") == 0) {
    return cmd_load(topology, argc >= 2 ? argv[1] : NULL);
  }

  return dispatch_node_action(topology, argc, argv);
}
