/**
 * @file ethernet.h
 * @brief Ethernet II frame serialization helpers with optional 802.1Q tags.
 */

#ifndef MAGI_LAYER2_ETHERNET_H
#define MAGI_LAYER2_ETHERNET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ETHERNET_MAC_LEN 6U
#define ETHERNET_MIN_FRAME_LEN 14U
#define ETHERNET_VLAN_FRAME_LEN 18U
#define ETHERNET_TYPE_IPV4 0x0800U
#define ETHERNET_TYPE_ARP 0x0806U
#define ETHERNET_TYPE_VLAN 0x8100U
#define ETHERNET_VLAN_ID_NONE 0U

/**
 * @brief Parsed Ethernet II frame.
 */
typedef struct EthernetFrame {
  uint8_t dst_mac[ETHERNET_MAC_LEN];
  uint8_t src_mac[ETHERNET_MAC_LEN];
  uint16_t ethertype;
  bool vlan_present;
  uint16_t vlan_id;
  const uint8_t* payload;
  size_t payload_len;
} EthernetFrame;

/**
 * @brief Check whether a MAC address is the Ethernet broadcast address.
 *
 * @param mac Input MAC address.
 * @return true if all bytes are 0xFF.
 */
bool ethernet_mac_is_broadcast(const uint8_t mac[ETHERNET_MAC_LEN]);

/**
 * @brief Check whether a MAC address is multicast.
 *
 * @param mac Input MAC address.
 * @return true if the multicast bit is set.
 */
bool ethernet_mac_is_multicast(const uint8_t mac[ETHERNET_MAC_LEN]);

/**
 * @brief Check whether two MAC addresses are identical.
 *
 * @param lhs Left MAC address.
 * @param rhs Right MAC address.
 * @return true if both 6-byte addresses match.
 */
bool ethernet_mac_equal(const uint8_t lhs[ETHERNET_MAC_LEN], const uint8_t rhs[ETHERNET_MAC_LEN]);

/**
 * @brief Populate a MAC address with FF:FF:FF:FF:FF:FF.
 *
 * @param out Destination 6-byte buffer.
 */
void ethernet_mac_broadcast(uint8_t out[ETHERNET_MAC_LEN]);

/**
 * @brief Parse raw bytes into an EthernetFrame view.
 *
 * The returned payload pointer references the input buffer.
 *
 * @param data Raw frame bytes.
 * @param len Raw frame length.
 * @param frame_out Destination parsed frame.
 * @return MAGI_OK on success, otherwise an error code.
 */
int ethernet_frame_from_bytes(const uint8_t* data, size_t len, EthernetFrame* frame_out);

/**
 * @brief Serialize an EthernetFrame into a heap buffer.
 *
 * @param frame Source frame.
 * @param bytes_out Owned output buffer.
 * @param len_out Output buffer length.
 * @return MAGI_OK on success, otherwise an error code.
 */
int ethernet_frame_to_bytes(const EthernetFrame* frame, uint8_t** bytes_out, size_t* len_out);

#endif
