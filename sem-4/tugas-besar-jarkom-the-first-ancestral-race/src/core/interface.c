#define _POSIX_C_SOURCE 200809L

#include "interface.h"

#include "core/link.h"
#include "core/node.h"
#include "utils/mac.h"
#include "utils/magi_error.h"

#include <stdlib.h>
#include <string.h>

Interface* interface_new(struct Node* node, uint16_t port) {
  if (node == NULL || port == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return NULL;
  }

  Interface* iface = malloc(sizeof(*iface));
  if (iface == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  iface->node = node;
  memset(iface->ip_address, 0, sizeof(iface->ip_address));
  iface->vlan_id = 0U;
  iface->port_number = port;
  iface->link = NULL;
  iface->send_down = NULL;
  iface->receive_up = interface_receive;
#ifdef MAGI_ASYNC
  iface->queue = NULL;
#endif
  mac_generate(iface->mac, node->name, port);
  return iface;
}

void interface_free(Interface* iface) {
  free(iface);
}

int interface_send(Interface* iface, const uint8_t* data, size_t len) {
  if (iface == NULL || data == NULL) {
    free((void*)data);
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (iface->link == NULL) {
    free((void*)data);
    magi_errno = MAGI_ERR_NOLINK;
    return MAGI_ERR_NOLINK;
  }

  return link_transmit(iface->link, iface, data, len);
}

void interface_receive(Interface* iface, const uint8_t* data, size_t len) {
  if (iface == NULL || iface->node == NULL || data == NULL) {
    return;
  }

  if (iface->node->handle_receive != NULL) {
    iface->node->handle_receive(iface->node, iface, data, len);
  }
}
