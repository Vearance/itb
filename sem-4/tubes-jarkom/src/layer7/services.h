#ifndef MAGI_LAYER7_SERVICES_H
#define MAGI_LAYER7_SERVICES_H

#include "core/node.h"
#include <stdbool.h>

struct HashMap;
struct MagiSocket;

typedef struct Layer7Services Layer7Services;

Layer7Services* layer7_services_get(Node* node);
void layer7_services_free(void* data);

struct MagiSocket* layer7_services_get_http_server(Layer7Services* services);
void layer7_services_set_http_server(Layer7Services* services, struct MagiSocket* sock);
char* layer7_services_get_http_content(Layer7Services* services);
void layer7_services_set_http_content_owned(Layer7Services* services, char* content);

struct MagiSocket* layer7_services_get_dns_server(Layer7Services* services);
void layer7_services_set_dns_server(Layer7Services* services, struct MagiSocket* sock);
struct HashMap* layer7_services_get_dns_records(Layer7Services* services);
void layer7_services_set_dns_records(Layer7Services* services, struct HashMap* records,
                                     bool owns_records);
struct HashMap* layer7_services_ensure_dns_records(Layer7Services* services);

void layer7_services_clear_dhcp(Layer7Services* services);
void layer7_services_set_dhcp_server(Layer7Services* services, struct MagiSocket* sock);
void layer7_services_set_dhcp_state(Layer7Services* services, void* state,
                                    void (*state_free)(void* data));

#endif /* MAGI_LAYER7_SERVICES_H */
