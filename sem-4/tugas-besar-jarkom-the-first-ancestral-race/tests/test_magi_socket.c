#define _POSIX_C_SOURCE 200809L

#include "core/node.h"
#include "layer4/l4_host.h"
#include "layer4/port_registry.h"
#include "layer4/tcp_socket.h"
#include "layer4/udp_socket.h"
#include "layer7/magi_socket.h"
#include "utils/magi_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT(cond, msg)                                                                         \
  do {                                                                                            \
    tests_run++;                                                                                  \
    if (cond) {                                                                                   \
      printf("  PASS: %s\n", (msg));                                                              \
      tests_passed++;                                                                             \
    } else {                                                                                      \
      printf("  FAIL: %s\n", (msg));                                                              \
    }                                                                                             \
  } while (0)

/* -----------------------------------------------------------------------
 * Test 1: MagiSocket creation and destruction (TCP & UDP)
 * ----------------------------------------------------------------------- */
static void test_socket_creation(void) {
  printf("\n--- Test: MagiSocket Creation ---\n");

  Node* node = node_new("TestHost");
  l4_host_attach(node);

  /* TCP socket */
  MagiSocket* tcp_sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_STREAM);
  ASSERT(tcp_sock != NULL, "Create TCP MagiSocket");
  ASSERT(tcp_sock->type == MAGI_SOCK_STREAM, "TCP socket type correct");
  ASSERT(tcp_sock->transport != NULL, "TCP transport allocated");
  ASSERT(tcp_sock->bound == false, "TCP socket initially unbound");
  magi_close(tcp_sock);

  /* UDP socket */
  MagiSocket* udp_sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_DGRAM);
  ASSERT(udp_sock != NULL, "Create UDP MagiSocket");
  ASSERT(udp_sock->type == MAGI_SOCK_DGRAM, "UDP socket type correct");
  ASSERT(udp_sock->transport != NULL, "UDP transport allocated");
  ASSERT(udp_sock->bound == false, "UDP socket initially unbound");
  magi_close(udp_sock);

  /* Invalid family */
  MagiSocket* bad = magi_socket(node, (MagiAddrFamily)99, MAGI_SOCK_STREAM);
  ASSERT(bad == NULL, "Reject invalid address family");

  /* NULL node */
  MagiSocket* null_sock = magi_socket(NULL, MAGI_AF_INET, MAGI_SOCK_STREAM);
  ASSERT(null_sock == NULL, "Reject NULL node");

  node_free(node);
}

/* -----------------------------------------------------------------------
 * Test 2: magi_bind
 * ----------------------------------------------------------------------- */
static void test_bind(void) {
  printf("\n--- Test: MagiSocket Bind ---\n");

  Node* node = node_new("BindHost");
  l4_host_attach(node);

  MagiSocket* sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_DGRAM);
  ASSERT(sock != NULL, "Create UDP socket for bind test");

  int status = magi_bind(sock, "192.168.1.10", 5353);
  ASSERT(status == MAGI_OK, "Bind to 192.168.1.10:5353 succeeds");
  ASSERT(sock->bound == true, "Socket marked as bound");
  ASSERT(sock->local_port == 5353, "Local port set correctly");

  /* Verify in port registry */
  void* reg = l4_host_get_registry(node);
  void* found = port_registry_lookup(reg, PORT_PROTOCOL_UDP, 5353);
  ASSERT(found != NULL, "Socket found in port registry");

  /* Bind with invalid args */
  int bad_status = magi_bind(sock, NULL, 80);
  ASSERT(bad_status != MAGI_OK, "Reject bind with NULL IP");

  int zero_port = magi_bind(sock, "0.0.0.0", 0);
  ASSERT(zero_port != MAGI_OK, "Reject bind with port 0");

  magi_close(sock);
  node_free(node);
}

/* -----------------------------------------------------------------------
 * Test 3: magi_listen (TCP only)
 * ----------------------------------------------------------------------- */
static void test_listen(void) {
  printf("\n--- Test: MagiSocket Listen ---\n");

  Node* node = node_new("ListenHost");
  l4_host_attach(node);

  MagiSocket* sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_STREAM);
  ASSERT(sock != NULL, "Create TCP socket for listen test");

  /* Listen before bind should fail */
  int status = magi_listen(sock, 5);
  ASSERT(status != MAGI_OK, "Listen before bind fails");

  /* Bind then listen */
  magi_bind(sock, "10.0.0.1", 80);
  status = magi_listen(sock, 5);
  ASSERT(status == MAGI_OK, "Listen after bind succeeds");
  ASSERT(sock->listening == true, "Socket marked as listening");

  /* Verify TCP state */
  TCPSocket* tcp = (TCPSocket*)sock->transport;
  ASSERT(tcp->state == TCP_LISTEN, "TCP state is LISTEN");

  /* UDP socket cannot listen */
  MagiSocket* udp = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_DGRAM);
  magi_bind(udp, "0.0.0.0", 8080);
  int udp_listen = magi_listen(udp, 5);
  ASSERT(udp_listen != MAGI_OK, "UDP socket cannot listen");

  magi_close(udp);
  magi_close(sock);
  node_free(node);
}

/* -----------------------------------------------------------------------
 * Test 4: magi_has_data and magi_recv (UDP)
 * ----------------------------------------------------------------------- */
static void test_udp_data_flow(void) {
  printf("\n--- Test: UDP Data Flow ---\n");

  Node* node = node_new("UDPHost");
  l4_host_attach(node);

  MagiSocket* sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_DGRAM);
  magi_bind(sock, "0.0.0.0", 12345);

  /* Initially no data */
  ASSERT(magi_has_data(sock) == false, "No data initially");

  /* Simulate incoming datagram by delivering directly to UDP socket */
  UDPSocketState* udp = (UDPSocketState*)sock->transport;
  uint8_t test_payload[] = "Hello UDP!";
  uint8_t src_ip[4] = {10, 0, 0, 1};
  udp_socket_deliver(udp, src_ip, 9999, test_payload, sizeof(test_payload) - 1);

  ASSERT(magi_has_data(sock) == true, "Data available after delivery");

  /* Recv */
  uint8_t buf[64] = {0};
  int rd = magi_recv(sock, buf, sizeof(buf));
  ASSERT(rd == 10, "Received 10 bytes");
  ASSERT(memcmp(buf, "Hello UDP!", 10) == 0, "Payload matches");
  ASSERT(magi_has_data(sock) == false, "No data after reading");

  magi_close(sock);
  node_free(node);
}

/* -----------------------------------------------------------------------
 * Test 5: magi_recvfrom (UDP with sender info)
 * ----------------------------------------------------------------------- */
static void test_udp_recvfrom(void) {
  printf("\n--- Test: UDP RecvFrom ---\n");

  Node* node = node_new("RecvFromHost");
  l4_host_attach(node);

  MagiSocket* sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_DGRAM);
  magi_bind(sock, "0.0.0.0", 5353);

  /* Deliver a datagram from 192.168.1.100:4444 */
  UDPSocketState* udp = (UDPSocketState*)sock->transport;
  uint8_t src_ip[4] = {192, 168, 1, 100};
  uint8_t payload[] = "test query";
  udp_socket_deliver(udp, src_ip, 4444, payload, sizeof(payload) - 1);

  /* RecvFrom */
  uint8_t buf[64] = {0};
  char sender_ip[16] = {0};
  uint16_t sender_port = 0;
  int rd = magi_recvfrom(sock, buf, sizeof(buf), sender_ip, &sender_port);

  ASSERT(rd == 10, "RecvFrom got 10 bytes");
  ASSERT(strcmp(sender_ip, "192.168.1.100") == 0, "Sender IP is 192.168.1.100");
  ASSERT(sender_port == 4444, "Sender port is 4444");

  magi_close(sock);
  node_free(node);
}

/* -----------------------------------------------------------------------
 * Test 6: magi_close with NULL is safe
 * ----------------------------------------------------------------------- */
static void test_close_null(void) {
  printf("\n--- Test: Close NULL Safety ---\n");

  int status = magi_close(NULL);
  ASSERT(status == MAGI_OK, "magi_close(NULL) returns OK");
}

/* -----------------------------------------------------------------------
 * Test 7: Port registry cleanup after close
 * ----------------------------------------------------------------------- */
static void test_port_cleanup(void) {
  printf("\n--- Test: Port Cleanup on Close ---\n");

  Node* node = node_new("CleanupHost");
  l4_host_attach(node);

  MagiSocket* sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_DGRAM);
  magi_bind(sock, "0.0.0.0", 7777);

  void* reg = l4_host_get_registry(node);
  ASSERT(port_registry_lookup(reg, PORT_PROTOCOL_UDP, 7777) != NULL, "Port bound before close");

  magi_close(sock);
  ASSERT(port_registry_lookup(reg, PORT_PROTOCOL_UDP, 7777) == NULL, "Port unbound after close");

  node_free(node);
}

/* ======================================================================= */

int main(void) {
  printf("=== MagiSocket Unit Tests ===\n");

  test_socket_creation();
  test_bind();
  test_listen();
  test_udp_data_flow();
  test_udp_recvfrom();
  test_close_null();
  test_port_cleanup();

  printf("\n=== Results: %d/%d tests passed ===\n", tests_passed, tests_run);

  if (tests_passed != tests_run) {
    printf("RESULT: FAIL\n");
    return 1;
  }
  printf("RESULT: PASS\n");
  return 0;
}
