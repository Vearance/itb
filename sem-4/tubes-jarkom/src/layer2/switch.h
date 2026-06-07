/**
 * @file switch.h
 * @brief Switch node wrapper APIs.
 */

#ifndef MAGI_LAYER2_SWITCH_H
#define MAGI_LAYER2_SWITCH_H

#include <stdbool.h>
#include <stdint.h>

#include "core/node.h"

/** @brief Opaque switch specialization of Node. */
typedef struct Switch Switch;

/** @brief Switch VLAN mode for one port. */
typedef enum SwitchPortMode {
  SWITCH_PORT_ACCESS,
  SWITCH_PORT_TRUNK
} SwitchPortMode;

/** @brief Switch VLAN configuration for one port. */
typedef struct SwitchPortConfig {
  SwitchPortMode mode;
  uint16_t vlan_id;
} SwitchPortConfig;

/**
 * @brief Create a switch node.
 *
 * @param name Switch name.
 * @return Switch instance, or NULL on failure.
 */
Switch* switch_new(const char* name);

/**
 * @brief Destroy a switch node.
 *
 * @param sw Switch to destroy. NULL is allowed.
 */
void switch_free(Switch* sw);

/**
 * @brief View a Switch as its embedded generic Node.
 *
 * @param sw Switch instance.
 * @return Generic node pointer, or NULL.
 */
Node* switch_as_node(Switch* sw);

/**
 * @brief View a Switch as its embedded generic Node.
 *
 * @param sw Switch instance.
 * @return Generic node pointer, or NULL.
 */
const Node* switch_as_node_const(const Switch* sw);

/**
 * @brief View a generic Node known to be a switch as Switch.
 *
 * @param node Generic node.
 * @return Switch pointer, or NULL.
 */
Switch* switch_from_node(Node* node);

/**
 * @brief View a generic Node known to be a switch as Switch.
 *
 * @param node Generic node.
 * @return Switch pointer, or NULL.
 */
const Switch* switch_from_node_const(const Node* node);

/**
 * @brief Ensure switch ports exist up to the requested count.
 *
 * @param sw Switch node.
 * @param num_ports Declared port count.
 * @return MAGI_OK on success, otherwise an error code.
 */
int switch_configure_num_ports(Switch* sw, uint16_t num_ports);

/**
 * @brief Configure a switch port as access or trunk.
 *
 * @param sw Switch node.
 * @param port Port number.
 * @param mode_text "access" or "trunk".
 * @param vlan_id Access VLAN id; zero maps to VLAN 1.
 * @return MAGI_OK on success, otherwise an error code.
 */
int switch_configure_port(Switch* sw, uint16_t port, const char* mode_text, uint16_t vlan_id);

/**
 * @brief Fetch explicit switch port VLAN configuration.
 *
 * @param sw Switch node.
 * @param port Port number.
 * @param out Destination config.
 * @return true when an explicit config exists.
 */
bool switch_get_port_config(const Switch* sw, uint16_t port, SwitchPortConfig* out);

/**
 * @brief Print the learned MAC table.
 *
 * @param sw Switch node.
 */
void switch_print_mac_table(const Switch* sw);

#endif
