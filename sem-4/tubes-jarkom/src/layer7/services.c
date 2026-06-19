#define _POSIX_C_SOURCE 200809L

#include "services.h"

#include "layer7/magi_socket.h"
#include "utils/hashmap.h"

#include <stdlib.h>

struct Layer7Services {
  struct MagiSocket* http_server;
  char* http_content;
  struct MagiSocket* dns_server;
  HashMap* dns_records;
  bool owns_dns_records;
  struct MagiSocket* dhcp_server;
  void* dhcp_state;
  void (*dhcp_state_free)(void* data);
};

static void free_record_value(const char* key, void* value, void* ctx) {
  (void)key;
  (void)ctx;
  free(value);
}

struct MagiSocket* layer7_services_get_http_server(Layer7Services* services) {
  return services != NULL ? services->http_server : NULL;
}

void layer7_services_set_http_server(Layer7Services* services, struct MagiSocket* sock) {
  if (services != NULL) {
    services->http_server = sock;
  }
}

char* layer7_services_get_http_content(Layer7Services* services) {
  return services != NULL ? services->http_content : NULL;
}

void layer7_services_set_http_content_owned(Layer7Services* services, char* content) {
  if (services == NULL) {
    free(content);
    return;
  }

  free(services->http_content);
  services->http_content = content;
}

struct MagiSocket* layer7_services_get_dns_server(Layer7Services* services) {
  return services != NULL ? services->dns_server : NULL;
}

void layer7_services_set_dns_server(Layer7Services* services, struct MagiSocket* sock) {
  if (services != NULL) {
    services->dns_server = sock;
  }
}

struct HashMap* layer7_services_get_dns_records(Layer7Services* services) {
  return services != NULL ? services->dns_records : NULL;
}

void layer7_services_set_dns_records(Layer7Services* services, struct HashMap* records,
                                     bool owns_records) {
  if (services == NULL) {
    return;
  }

  if (services->dns_records != NULL && services->owns_dns_records &&
      services->dns_records != records) {
    hashmap_foreach(services->dns_records, free_record_value, NULL);
    hashmap_free(services->dns_records);
  }

  services->dns_records = records;
  services->owns_dns_records = owns_records;
}

struct HashMap* layer7_services_ensure_dns_records(Layer7Services* services) {
  if (services == NULL) {
    return NULL;
  }

  if (services->dns_records == NULL) {
    HashMap* records = hashmap_new(8U);
    if (records == NULL) {
      return NULL;
    }
    services->dns_records = records;
    services->owns_dns_records = true;
  }

  return services->dns_records;
}

void layer7_services_clear_dhcp(Layer7Services* services) {
  if (services == NULL) {
    return;
  }

  if (services->dhcp_server != NULL) {
    (void)magi_close(services->dhcp_server);
    services->dhcp_server = NULL;
  }
  if (services->dhcp_state_free != NULL && services->dhcp_state != NULL) {
    services->dhcp_state_free(services->dhcp_state);
  }
  services->dhcp_state = NULL;
  services->dhcp_state_free = NULL;
}

void layer7_services_set_dhcp_server(Layer7Services* services, struct MagiSocket* sock) {
  if (services != NULL) {
    services->dhcp_server = sock;
  }
}

void layer7_services_set_dhcp_state(Layer7Services* services, void* state,
                                    void (*state_free)(void* data)) {
  if (services != NULL) {
    services->dhcp_state = state;
    services->dhcp_state_free = state_free;
  }
}

Layer7Services* layer7_services_get(Node* node) {
  if (node == NULL) {
    return NULL;
  }

  if (node->l7_data != NULL) {
    return (Layer7Services*)node->l7_data;
  }

  Layer7Services* services = calloc(1U, sizeof(*services));
  if (services == NULL) {
    return NULL;
  }

  node->l7_data = services;
  node->l7_data_free = layer7_services_free;
  return services;
}

void layer7_services_free(void* data) {
  Layer7Services* services = (Layer7Services*)data;
  if (services == NULL) {
    return;
  }

  if (services->http_server != NULL) {
    (void)magi_close(services->http_server);
  }
  free(services->http_content);

  if (services->dns_server != NULL) {
    (void)magi_close(services->dns_server);
  }
  if (services->dns_records != NULL && services->owns_dns_records) {
    hashmap_foreach(services->dns_records, free_record_value, NULL);
    hashmap_free(services->dns_records);
  }

  if (services->dhcp_server != NULL) {
    (void)magi_close(services->dhcp_server);
  }
  if (services->dhcp_state_free != NULL && services->dhcp_state != NULL) {
    services->dhcp_state_free(services->dhcp_state);
  }

  free(services);
}
