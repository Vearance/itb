/**
 * @file tcp_socket.c
 * @brief TCP socket state machine — exhaustive 35+ transition table.
 *
 * Implements the full state machine from AGENTS.md §11.2 including
 * 3-way handshake, 4-way teardown, RST handling, and out-of-order
 * reassembly in the receive buffer.
 */

#define _POSIX_C_SOURCE 200809L

#include "tcp_socket.h"

#include "core/interface.h"
#include "layer3/ipv4.h"
#include "layer4/tcp.h"
#include "utils/hashmap.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─── State names ─── */

const char* tcp_state_name(TCPState state) {
  switch (state) {
  case TCP_CLOSED:
    return "CLOSED";
  case TCP_LISTEN:
    return "LISTEN";
  case TCP_SYN_SENT:
    return "SYN_SENT";
  case TCP_SYN_RCVD:
    return "SYN_RCVD";
  case TCP_ESTABLISHED:
    return "ESTABLISHED";
  case TCP_FIN_WAIT_1:
    return "FIN_WAIT_1";
  case TCP_FIN_WAIT_2:
    return "FIN_WAIT_2";
  case TCP_CLOSE_WAIT:
    return "CLOSE_WAIT";
  case TCP_LAST_ACK:
    return "LAST_ACK";
  case TCP_TIME_WAIT:
    return "TIME_WAIT";
  case TCP_CLOSING:
    return "CLOSING";
  default:
    return "UNKNOWN";
  }
}

/* ─── Logging helper ─── */

/**
 * @brief Log a TCP state transition.
 *
 * Prints the old state, new state, flags, sequence number, and
 * acknowledgment number for debugging.
 *
 * @param sock      Socket undergoing the transition.
 * @param old_state State before the transition.
 * @param new_state State after the transition.
 * @param flags     TCP flags of the triggering segment.
 * @param seq       Sequence number of the triggering segment.
 * @param ack       Acknowledgment number of the triggering segment.
 */
static void log_transition(TCPSocket* sock, TCPState old_state, TCPState new_state, uint8_t flags,
                           uint32_t seq, uint32_t ack) {
  char addr[80];
  snprintf(addr, sizeof(addr), "%s:%u", sock->node != NULL ? sock->node->name : "?",
           (unsigned)sock->local_port);
  LOG(addr, "TCP state: %s → %s (flags=0x%02X seq=%u ack=%u)", tcp_state_name(old_state),
      tcp_state_name(new_state), (unsigned)flags, (unsigned)seq, (unsigned)ack);
}

/* ─── Segment sending helper ─── */

/**
 * @brief Build and send a TCP segment.
 *
 * Allocates a buffer, serializes the segment via tcp_pack (which
 * computes the pseudo-header checksum), advances sock->seq_num
 * by the number of SYN/FIN flags plus payload bytes, then sends
 * the segment via node->send_ip_packet. The buffer is freed after
 * transmission.
 *
 * @param sock         Socket to send from.
 * @param flags        TCP flags (SYN, ACK, FIN, PSH, etc.).
 * @param ack_num      Acknowledgment number for the segment.
 * @param payload      Payload data (may be NULL if len is 0).
 * @param payload_len  Length of payload in bytes.
 * @return MAGI_OK on success, or an error code from tcp_pack or send_ip_packet.
 */
static int tcp_send_segment(TCPSocket* sock, uint8_t flags, uint32_t ack_num,
                            const uint8_t* payload, size_t payload_len) {
  if (sock == NULL || sock->node == NULL || sock->node->send_ip_packet == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* Cache the sequence number for this segment before any recursive calls. */
  uint32_t used_seq = sock->seq_num;

  TCPSegment seg;
  memset(&seg, 0, sizeof(seg));
  seg.src_port = sock->local_port;
  seg.dst_port = sock->remote_port;
  seg.seq_num = used_seq;
  seg.ack_num = ack_num;
  seg.data_offset = TCP_DATA_OFFSET_DEFAULT;
  seg.flags = flags;
  seg.window_size = TCP_WINDOW_SIZE_DEFAULT;
  seg.payload = payload;
  seg.payload_len = payload_len;

  size_t total_len = TCP_HEADER_LEN + payload_len;
  uint8_t* buf = malloc(total_len);
  if (buf == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return MAGI_ERR_NOMEM;
  }

  int status = tcp_pack(&seg, sock->local_ip, sock->remote_ip, buf, total_len);
  if (status != MAGI_OK) {
    free(buf);
    return status;
  }

  /* Advance seq_num BEFORE the synchronous send in sequential mode.
     This ensures that when send_ip_packet triggers a recursive receive
     (e.g. SYN+ACK arrives during tcp_socket_connect), sock->seq_num
     already reflects the consumed SYN so the ack validation passes. */
  uint32_t seq_advance = (flags & TCP_FLAG_SYN) ? 1U : 0U;
  seq_advance += (flags & TCP_FLAG_FIN) ? 1U : 0U;
  if (payload_len > 0U) {
    seq_advance += (uint32_t)payload_len;
  }
  sock->seq_num += seq_advance;

  status = sock->node->send_ip_packet(sock->node, sock->local_ip, sock->remote_ip,
                                      IPV4_PROTOCOL_TCP, IPV4_DEFAULT_TTL, buf, total_len);
  free(buf);
  return status;
}

/* ─── RST segment sender (static helper for connection refused) ─── */

/**
 * @brief Send a bare RST segment (connection refused / reset).
 *
 * Builds a minimal RST segment with no payload and sends it via the
 * node's send_ip_packet callback. Used for closed-port rejection and
 * error handling in the state machine.
 *
 * @param node     Node to send from.
 * @param src_ip   Source IPv4 address (4 bytes).
 * @param dst_ip   Destination IPv4 address (4 bytes).
 * @param src_port Source port.
 * @param dst_port Destination port.
 * @param seq_num  Sequence number for the RST segment.
 * @param ack_num  Acknowledgment number for the RST segment.
 */
void tcp_send_rst_packet(struct Node* node, const uint8_t src_ip[4], const uint8_t dst_ip[4],
                         uint16_t src_port, uint16_t dst_port, uint32_t seq_num, uint32_t ack_num) {
  if (node == NULL || src_ip == NULL || dst_ip == NULL || node->send_ip_packet == NULL) {
    return;
  }

  TCPSegment seg;
  memset(&seg, 0, sizeof(seg));
  seg.src_port = src_port;
  seg.dst_port = dst_port;
  seg.seq_num = seq_num;
  seg.ack_num = ack_num;
  seg.data_offset = TCP_DATA_OFFSET_DEFAULT;
  seg.flags = TCP_FLAG_RST;
  seg.window_size = 0U;

  uint8_t buf[TCP_HEADER_LEN];
  if (tcp_pack(&seg, src_ip, dst_ip, buf, sizeof(buf)) != MAGI_OK) {
    return;
  }

  (void)node->send_ip_packet(node, src_ip, dst_ip, IPV4_PROTOCOL_TCP, IPV4_DEFAULT_TTL, buf,
                             sizeof(buf));
}

/* ─── Send bare ACK helper ─── */

/**
 * @brief Send a bare ACK segment.
 *
 * Convenience wrapper around tcp_send_segment that sends a pure ACK
 * (no SYN, FIN, or payload) using the socket's current ack_num.
 *
 * @param sock  Socket to send from.
 * @return MAGI_OK on success, or an error code from tcp_send_segment.
 */
static int tcp_send_ack(TCPSocket* sock) {
  return tcp_send_segment(sock, TCP_FLAG_ACK, sock->ack_num, NULL, 0U);
}

/* ─── Receive buffer: insert a segment ─── */

/**
 * @brief Insert a received TCP segment into the receive buffer.
 *
 * Handles three cases:
 *   - Duplicate/old segment (seq < ack_num): resend ACK, drop.
 *   - In-order data (seq == ack_num): append to recv_buf, advance
 *     ack_num, then flush any contiguous out-of-order segments.
 *   - Out-of-order data (seq > ack_num): store in the out_of_order
 *     linked list, sorted by sequence number. Does NOT send ACK.
 *
 * After in-order insertion, sends an ACK for the updated ack_num.
 *
 * @param sock  Socket receiving the data.
 * @param seg   Parsed incoming TCP segment.
 * @return MAGI_OK on success, MAGI_ERR_NOMEM on allocation failure.
 */
static int tcp_recv_buf_insert_internal(TCPSocket* sock, TCPSegment* seg) {
  if (sock == NULL || seg == NULL) {
    return MAGI_ERR_BADARGS;
  }

  /* Duplicate / old segment */
  if (seg->seq_num < sock->ack_num) {
    /* Resend ACK for expected seq */
    (void)tcp_send_ack(sock);
    return MAGI_OK;
  }

  /* In-order data */
  if (seg->seq_num == sock->ack_num) {
    /* Append to recv buffer */
    size_t needed = sock->recv_buf_len + seg->payload_len;
    if (needed > sock->recv_buf_cap) {
      /* Buffer full — drop (simplified) */
      return MAGI_OK;
    }

    if (seg->payload_len > 0U && seg->payload != NULL) {
      memcpy(sock->recv_buf + sock->recv_buf_len, seg->payload, seg->payload_len);
      sock->recv_buf_len += seg->payload_len;
    }

    /* Advance ack_num */
    uint32_t advance = (uint32_t)seg->payload_len;
    if (seg->flags & TCP_FLAG_FIN) {
      advance += 1U;
    }
    sock->ack_num += advance;

    /* Flush contiguous out-of-order segments */
    OOOSegment** pp = &sock->out_of_order;
    while (*pp != NULL) {
      OOOSegment* ooo = *pp;
      if (ooo->seq_num == sock->ack_num) {
        size_t need2 = sock->recv_buf_len + ooo->len;
        if (need2 <= sock->recv_buf_cap) {
          memcpy(sock->recv_buf + sock->recv_buf_len, ooo->data, ooo->len);
          sock->recv_buf_len += ooo->len;
        }
        sock->ack_num += (uint32_t)ooo->len;
        *pp = ooo->next;
        free(ooo->data);
        free(ooo);
      } else {
        pp = &(*pp)->next;
      }
    }

    /* Send ACK for the updated ack_num */
    (void)tcp_send_ack(sock);
    return MAGI_OK;
  }

  /* Out-of-order data (seq_num > ack_num) — store for later */
  if (seg->seq_num > sock->ack_num && seg->payload_len > 0U) {
    OOOSegment* ooo = malloc(sizeof(*ooo));
    if (ooo == NULL) {
      return MAGI_ERR_NOMEM;
    }

    ooo->seq_num = seg->seq_num;
    ooo->len = seg->payload_len;
    ooo->data = malloc(seg->payload_len);
    if (ooo->data == NULL && seg->payload_len > 0U) {
      free(ooo);
      return MAGI_ERR_NOMEM;
    }
    if (seg->payload_len > 0U) {
      memcpy(ooo->data, seg->payload, seg->payload_len);
    }
    ooo->next = NULL;

    /* Insert in sequence order */
    OOOSegment** pp = &sock->out_of_order;
    while (*pp != NULL && (*pp)->seq_num < ooo->seq_num) {
      pp = &(*pp)->next;
    }
    ooo->next = *pp;
    *pp = ooo;

    /* Do NOT send ACK for out-of-order per spec */
    return MAGI_OK;
  }

  return MAGI_OK;
}

/* ─── Public receive buffer API ─── */

/**
 * @brief Read contiguous data from the socket's receive buffer.
 *
 * Copies up to len bytes from recv_buf into out, then shifts
 * remaining data to the front. Updates recv_buf_len accordingly.
 *
 * @param sock  Socket with received data.
 * @param out   Output buffer for the copied bytes.
 * @param len   Maximum number of bytes to read.
 * @return Number of bytes actually copied (may be less than len).
 */
size_t tcp_recv_buf_read(TCPSocket* sock, uint8_t* out, size_t len) {
  if (sock == NULL || out == NULL || len == 0U) {
    return 0U;
  }

  size_t to_copy = sock->recv_buf_len < len ? sock->recv_buf_len : len;
  if (to_copy == 0U) {
    return 0U;
  }

  memcpy(out, sock->recv_buf, to_copy);
  size_t remaining = sock->recv_buf_len - to_copy;
  if (remaining > 0U) {
    memmove(sock->recv_buf, sock->recv_buf + to_copy, remaining);
  }
  sock->recv_buf_len = remaining;
  return to_copy;
}

/**
 * @brief Check whether the socket has received data available.
 *
 * @param sock  TCP socket to check.
 * @return true if recv_buf_len > 0, false otherwise (or if sock is NULL).
 */
bool tcp_socket_has_data(const TCPSocket* sock) {
  return sock != NULL && sock->recv_buf_len > 0U;
}

/* ─── Socket lifecycle ─── */

/**
 * @brief Allocate and initialise a TCP socket.
 *
 * Allocates a TCPSocket in CLOSED state with a 16 KB receive buffer.
 * The out_of_order list is initialised to NULL.
 *
 * @param node  Owning node (used for sending responses).
 * @return New TCPSocket pointer, or NULL on allocation failure.
 */
TCPSocket* tcp_socket_new(struct Node* node) {
  TCPSocket* sock = calloc(1U, sizeof(*sock));
  if (sock == NULL) {
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  sock->state = TCP_CLOSED;
  sock->node = node;

  /* 16 KB receive buffer */
  sock->recv_buf_cap = 16384U;
  sock->recv_buf = malloc(sock->recv_buf_cap);
  if (sock->recv_buf == NULL) {
    free(sock);
    magi_errno = MAGI_ERR_NOMEM;
    return NULL;
  }

  sock->recv_buf_len = 0U;
  sock->out_of_order = NULL;

  return sock;
}

/**
 * @brief Destroy a TCP socket and free all resources.
 *
 * Frees the receive buffer, the out-of-order segment linked list
 * (including each segment's data), and the socket struct itself.
 * Does NOT free the owning node. NULL-safe.
 *
 * @param sock  Socket to free. May be NULL.
 */
void tcp_socket_free(TCPSocket* sock) {
  if (sock == NULL) {
    return;
  }

  free(sock->recv_buf);

  OOOSegment* ooo = sock->out_of_order;
  while (ooo != NULL) {
    OOOSegment* next = ooo->next;
    free(ooo->data);
    free(ooo);
    ooo = next;
  }

  free(sock);
}

/* ─── State Transition Dispatcher ─── */

/**
 * @brief Test whether a flags byte contains all bits in a mask.
 *
 * @param flags  The flags byte to test.
 * @param mask   The mask to check against.
 * @return true if (flags & mask) == mask.
 */
static bool has_flags(uint8_t flags, uint8_t mask) {
  return (flags & mask) == mask;
}

/**
 * @brief Process an incoming TCP segment through the socket state machine.
 *
 * Implements the exhaustive transition table from AGENTS.md §11.2,
 * covering all 11 states and every valid flag combination. Handles
 * RST universally, dispatches to state-specific logic, and falls
 * through to a send-RST-and-close catch-all for unexpected inputs.
 * Every transition is logged.
 *
 * @param sock    Target socket for the incoming segment.
 * @param seg     Parsed incoming TCP segment.
 * @param node    Owning node (for sending response segments).
 * @param src_ip  Source IPv4 address of the received segment.
 * @param dst_ip  Destination IPv4 address of the received segment.
 * @return MAGI_OK on success, MAGI_ERR_CONNRESET on RST or unexpected input,
 *         or an error code from the segment send helper.
 */
int tcp_socket_handle_segment(TCPSocket* sock, TCPSegment* seg, struct Node* node,
                              const uint8_t src_ip[4], const uint8_t dst_ip[4]) {
  if (sock == NULL || seg == NULL || node == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  TCPState old_state = sock->state;
  uint8_t flags = seg->flags;
  uint32_t seq = seg->seq_num;
  uint32_t ack = seg->ack_num;

  /* Log every received segment */
  {
    char addr[80];
    snprintf(addr, sizeof(addr), "%s:%u", node->name, (unsigned)sock->local_port);
    LOG(addr, "TCP received flags=0x%02X seq=%u ack=%u len=%zu", (unsigned)flags, (unsigned)seq,
        (unsigned)ack, seg->payload_len);
  }

  /* ── RST handling (universal) ── */
  if (flags & TCP_FLAG_RST) {
    TCPState old = sock->state;
    sock->state = TCP_CLOSED;
    log_transition(sock, old, TCP_CLOSED, flags, seq, ack);
    LOG(node->name, "TCP connection reset by peer");
    return MAGI_ERR_CONNRESET;
  }

  switch (sock->state) {

  /* ═══════════════ CLOSED ═══════════════ */
  case TCP_CLOSED:
    if (has_flags(flags, TCP_FLAG_SYN) && !(flags & TCP_FLAG_ACK)) {
      /* Passive open: CLOSED + SYN → SYN_RCVD */
      sock->seq_num = (uint32_t)(rand() & 0xFFFF) | 0x10000000U; /* ISS */
      sock->ack_num = seq + 1U;
      memcpy(sock->remote_ip, src_ip, 4U);
      sock->remote_port = seg->src_port;
      /* Update local_ip from incoming segment */
      memcpy(sock->local_ip, dst_ip, 4U);
      sock->state = TCP_SYN_RCVD;
      log_transition(sock, old_state, TCP_SYN_RCVD, flags, seq, ack);
      return tcp_send_segment(sock, TCP_FLAG_SYN | TCP_FLAG_ACK, sock->ack_num, NULL, 0U);
    }
    /* Any other segment on CLOSED → send RST unless it's already an RST */
    {
      char addr[80];
      snprintf(addr, sizeof(addr), "%s:%u", node->name, (unsigned)sock->local_port);
      LOG(addr, "TCP CLOSED: send RST for unexpected segment flags=0x%02X", (unsigned)flags);
      tcp_send_rst_packet(node, sock->local_ip, sock->remote_ip, sock->local_port,
                          sock->remote_port, seg->ack_num,
                          seq + (uint32_t)(seg->payload_len > 0U ? seg->payload_len : 1U));
    }
    /* Stay CLOSED */
    return MAGI_ERR_CONNRESET;

  /* ═══════════════ LISTEN ═══════════════ */
  case TCP_LISTEN:
    if (has_flags(flags, TCP_FLAG_SYN) && !(flags & TCP_FLAG_ACK)) {
      /* Passive open */
      sock->seq_num = (uint32_t)(rand() & 0xFFFF) | 0x10000000U; /* ISS */
      sock->ack_num = seq + 1U;
      memcpy(sock->remote_ip, src_ip, 4U);
      sock->remote_port = seg->src_port;
      memcpy(sock->local_ip, dst_ip, 4U);
      sock->state = TCP_SYN_RCVD;
      log_transition(sock, old_state, TCP_SYN_RCVD, flags, seq, ack);
      return tcp_send_segment(sock, TCP_FLAG_SYN | TCP_FLAG_ACK, sock->ack_num, NULL, 0U);
    }
    /* All others → drop silently */
    return MAGI_OK;

  /* ═══════════════ SYN_SENT ═══════════════ */
  case TCP_SYN_SENT:
    if (has_flags(flags, TCP_FLAG_SYN | TCP_FLAG_ACK)) {
      /* SYN+ACK received */
      if (ack != sock->seq_num) {
        LOG(node->name, "TCP SYN_SENT: bad ack %u (expected %u), send RST", (unsigned)ack,
            (unsigned)sock->seq_num);
        tcp_send_rst_packet(node, sock->local_ip, sock->remote_ip, sock->local_port,
                            sock->remote_port, ack, 0U);
        sock->state = TCP_CLOSED;
        log_transition(sock, old_state, TCP_CLOSED, flags, seq, ack);
        return MAGI_ERR_CONNRESET;
      }
      /* Advance seq_num (SYN consumed) */
      sock->ack_num = seq + 1U;
      sock->state = TCP_ESTABLISHED;
      log_transition(sock, old_state, TCP_ESTABLISHED, flags, seq, ack);
      return tcp_send_ack(sock);
    }

    if (has_flags(flags, TCP_FLAG_SYN) && !(flags & TCP_FLAG_ACK)) {
      /* Simultaneous open */
      sock->ack_num = seq + 1U;
      sock->state = TCP_SYN_RCVD;
      log_transition(sock, old_state, TCP_SYN_RCVD, flags, seq, ack);
      return tcp_send_segment(sock, TCP_FLAG_SYN | TCP_FLAG_ACK, sock->ack_num, NULL, 0U);
    }

    if (flags & TCP_FLAG_ACK) {
      /* ACK-only with no SYN */
      tcp_send_rst_packet(node, sock->local_ip, sock->remote_ip, sock->local_port,
                          sock->remote_port, seg->ack_num, 0U);
      sock->state = TCP_CLOSED;
      log_transition(sock, old_state, TCP_CLOSED, flags, seq, ack);
      return MAGI_ERR_CONNRESET;
    }

    /* Fall through to catch-all */
    goto send_rst_and_close;

  /* ═══════════════ SYN_RCVD ═══════════════ */
  case TCP_SYN_RCVD:
    if (has_flags(flags, TCP_FLAG_ACK)) {
      if (ack != sock->seq_num) {
        LOG(node->name, "TCP SYN_RCVD: bad ack %u (expected %u)", (unsigned)ack,
            (unsigned)sock->seq_num);
        return MAGI_ERR_CONNRESET;
      }
      sock->state = TCP_ESTABLISHED;
      log_transition(sock, old_state, TCP_ESTABLISHED, flags, seq, ack);
      /* If there is payload data, process it */
      if (seg->payload_len > 0U) {
        return tcp_recv_buf_insert_internal(sock, seg);
      }
      return MAGI_OK;
    }

    if (has_flags(flags, TCP_FLAG_FIN | TCP_FLAG_ACK)) {
      sock->ack_num = seq + 1U;
      sock->state = TCP_CLOSE_WAIT;
      log_transition(sock, old_state, TCP_CLOSE_WAIT, flags, seq, ack);
      return tcp_send_ack(sock);
    }

    if (has_flags(flags, TCP_FLAG_SYN)) {
      goto send_rst_and_close;
    }

    goto send_rst_and_close;

  /* ═══════════════ ESTABLISHED ═══════════════ */
  case TCP_ESTABLISHED:
    if (flags & TCP_FLAG_ACK) {
      /* ACK received — update send window */
      /* (Simplified: no congestion control, just ack validation) */
      if (seq == sock->ack_num && seg->payload_len == 0U) {
        /* Pure ACK with no data */
        return MAGI_OK;
      }
    }

    if ((flags & TCP_FLAG_PSH) || seg->payload_len > 0U) {
      /* Data segment */
      return tcp_recv_buf_insert_internal(sock, seg);
    }

    if (has_flags(flags, TCP_FLAG_FIN) || has_flags(flags, TCP_FLAG_FIN | TCP_FLAG_ACK)) {
      sock->ack_num = seq + 1U;
      sock->state = TCP_CLOSE_WAIT;
      log_transition(sock, old_state, TCP_CLOSE_WAIT, flags, seq, ack);
      return tcp_send_ack(sock);
    }

    if (flags & TCP_FLAG_SYN) {
      /* Unexpected SYN — half-open detection */
      goto send_rst_and_close;
    }

    /* Duplicate PSH or unrecognized flags: resend ACK */
    if (seg->payload_len > 0U) {
      return tcp_send_ack(sock);
    }
    return MAGI_OK;

  /* ═══════════════ FIN_WAIT_1 ═══════════════ */
  case TCP_FIN_WAIT_1:
    if (has_flags(flags, TCP_FLAG_ACK)) {
      /* ACK of our FIN */
      sock->state = TCP_FIN_WAIT_2;
      log_transition(sock, old_state, TCP_FIN_WAIT_2, flags, seq, ack);
      return MAGI_OK;
    }

    if (has_flags(flags, TCP_FLAG_FIN)) {
      /* Simultaneous close */
      sock->ack_num = seq + 1U;
      sock->state = TCP_CLOSING;
      log_transition(sock, old_state, TCP_CLOSING, flags, seq, ack);
      return tcp_send_ack(sock);
    }

    if (has_flags(flags, TCP_FLAG_FIN | TCP_FLAG_ACK)) {
      sock->ack_num = seq + 1U;
      sock->state = TCP_TIME_WAIT;
      log_transition(sock, old_state, TCP_TIME_WAIT, flags, seq, ack);
      return tcp_send_ack(sock);
    }

    goto send_rst_and_close;

  /* ═══════════════ FIN_WAIT_2 ═══════════════ */
  case TCP_FIN_WAIT_2:
    if (has_flags(flags, TCP_FLAG_FIN)) {
      sock->ack_num = seq + 1U;
      sock->state = TCP_TIME_WAIT;
      log_transition(sock, old_state, TCP_TIME_WAIT, flags, seq, ack);
      return tcp_send_ack(sock);
    }

    goto send_rst_and_close;

  /* ═══════════════ CLOSING ═══════════════ */
  case TCP_CLOSING:
    if (has_flags(flags, TCP_FLAG_ACK)) {
      sock->state = TCP_TIME_WAIT;
      log_transition(sock, old_state, TCP_TIME_WAIT, flags, seq, ack);
      return MAGI_OK;
    }
    goto send_rst_and_close;

  /* ═══════════════ CLOSE_WAIT ═══════════════ */
  case TCP_CLOSE_WAIT:
    /* Application should close, at which point we send FIN */
    /* For now, any segment triggers RST since we shouldn't be receiving */
    goto send_rst_and_close;

  /* ═══════════════ LAST_ACK ═══════════════ */
  case TCP_LAST_ACK:
    if (has_flags(flags, TCP_FLAG_ACK)) {
      sock->state = TCP_CLOSED;
      log_transition(sock, old_state, TCP_CLOSED, flags, seq, ack);
      return MAGI_OK;
    }
    goto send_rst_and_close;

  /* ═══════════════ TIME_WAIT ═══════════════ */
  case TCP_TIME_WAIT:
    if (has_flags(flags, TCP_FLAG_FIN)) {
      /* Retransmitted FIN; resend ACK */
      return tcp_send_ack(sock);
    }
    goto send_rst_and_close;

  /* ═══════════════ Default ═══════════════ */
  default:
    break;
  }

send_rst_and_close: {
  char addr[80];
  snprintf(addr, sizeof(addr), "%s:%u", node->name, (unsigned)sock->local_port);
  TCPState old = sock->state;
  LOG(addr, "TCP unexpected flags=0x%02X in state %s, send RST", (unsigned)flags,
      tcp_state_name(old));
  tcp_send_rst_packet(node, sock->local_ip, sock->remote_ip, sock->local_port, sock->remote_port,
                      sock->seq_num, sock->ack_num);
  sock->state = TCP_CLOSED;
  log_transition(sock, old, TCP_CLOSED, flags, seq, ack);
}
  return MAGI_ERR_CONNRESET;
}

/* ─── High-level API ─── */

/**
 * @brief Bind a TCP socket to a local port and enter LISTEN state.
 *
 * Finds the node's first IPv4 interface to determine the local IP,
 * sets sock->local_ip and sock->local_port, and transitions the
 * socket state to TCP_LISTEN.
 *
 * @param sock  TCP socket to bind (must be in CLOSED state).
 * @param node  Owning node (used to discover local IP).
 * @param port  Local port number to bind.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS if no IPv4 interface
 *         exists or parameters are invalid.
 */
int tcp_socket_bind_listen(TCPSocket* sock, struct Node* node, uint16_t port) {
  if (sock == NULL || node == NULL || port == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* Set local IP from the node's first interface */
  Interface* iface = NULL;
  if (node->interfaces != NULL) {
    for (size_t i = 0U; i < node->interfaces->capacity; ++i) {
      HashEntry* entry = &node->interfaces->entries[i];
      if (entry->key != NULL && !entry->tombstone) {
        Interface* candidate = (Interface*)entry->value;
        if (candidate->ip_address[0] != '\0') {
          iface = candidate;
          break;
        }
      }
    }
  }
  if (iface == NULL) {
    LOG(node->name, "Cannot bind TCP socket: node has no IPv4 interface");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (ipv4_parse_address(iface->ip_address, sock->local_ip) != MAGI_OK) {
    LOG(node->name, "Cannot bind TCP socket: invalid interface IP");
    return MAGI_ERR_BADARGS;
  }

  sock->local_port = port;
  sock->node = node;
  sock->state = TCP_LISTEN;
  log_transition(sock, TCP_CLOSED, TCP_LISTEN, 0U, 0U, 0U);
  return MAGI_OK;
}

/**
 * @brief Initiate an active TCP connection (client open).
 *
 * Resolves the destination IP, determines the local IP from the
 * node's first interface, assigns an ephemeral local port if not
 * already bound, generates an initial sequence number (ISS), and
 * sends a SYN segment. The socket transitions to SYN_SENT state.
 *
 * @param sock    TCP socket (must be in CLOSED state).
 * @param node    Owning node.
 * @param dst_ip  Destination IPv4 address as a dotted-decimal string.
 * @param dst_port Destination port number.
 * @return MAGI_OK on success, MAGI_ERR_BADARGS on invalid IP or
 *         missing interface, or an error from the SYN transmission.
 */
int tcp_socket_connect(TCPSocket* sock, struct Node* node, const char* dst_ip, uint16_t dst_port) {
  if (sock == NULL || node == NULL || dst_ip == NULL || dst_port == 0U) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  /* Resolve destination IP */
  uint8_t dst_ip_bytes[4];
  if (ipv4_parse_address(dst_ip, dst_ip_bytes) != MAGI_OK) {
    LOG(node->name, "TCP connect: invalid destination IP '%s'", dst_ip);
    return MAGI_ERR_BADARGS;
  }

  /* Set local IP from first interface */
  Interface* iface = NULL;
  if (node->interfaces != NULL) {
    for (size_t i = 0U; i < node->interfaces->capacity; ++i) {
      HashEntry* entry = &node->interfaces->entries[i];
      if (entry->key != NULL && !entry->tombstone) {
        Interface* candidate = (Interface*)entry->value;
        if (candidate->ip_address[0] != '\0') {
          iface = candidate;
          break;
        }
      }
    }
  }
  if (iface == NULL) {
    LOG(node->name, "TCP connect: node has no IPv4 interface");
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (ipv4_parse_address(iface->ip_address, sock->local_ip) != MAGI_OK) {
    LOG(node->name, "TCP connect: invalid interface IP");
    return MAGI_ERR_BADARGS;
  }

  /* Assign ephemeral local port if not already bound */
  if (sock->local_port == 0U) {
    /* Use a simple ephemeral port (49152 + hash of pointer) for now */
    sock->local_port = (uint16_t)(49152U + ((uintptr_t)sock & 0x3FFFU));
  }

  memcpy(sock->remote_ip, dst_ip_bytes, 4U);
  sock->remote_port = dst_port;
  sock->node = node;

  /* Initial sequence number (ISS) */
  sock->seq_num = (uint32_t)(rand() & 0xFFFF) | 0x20000000U;

  /* Send SYN */
  sock->state = TCP_SYN_SENT;
  log_transition(sock, TCP_CLOSED, TCP_SYN_SENT, TCP_FLAG_SYN, sock->seq_num, 0U);

  int status = tcp_send_segment(sock, TCP_FLAG_SYN, 0U, NULL, 0U);
  if (status != MAGI_OK) {
    sock->state = TCP_CLOSED;
    log_transition(sock, TCP_SYN_SENT, TCP_CLOSED, 0U, 0U, 0U);
    return status;
  }

  return MAGI_OK;
}

/**
 * @brief Send data on an ESTABLISHED TCP connection.
 *
 * Sends the payload with PSH+ACK flags. The socket must be in
 * ESTABLISHED state; otherwise MAGI_ERR_CONNRESET is returned.
 *
 * @param sock  TCP socket (must be ESTABLISHED).
 * @param node  Owning node.
 * @param data  Payload bytes to send.
 * @param len   Number of bytes to send.
 * @return MAGI_OK on success, MAGI_ERR_CONNRESET if not ESTABLISHED,
 *         MAGI_ERR_BADARGS on null input.
 */
int tcp_socket_send(TCPSocket* sock, struct Node* node, const uint8_t* data, size_t len) {
  if (sock == NULL || node == NULL || (len > 0U && data == NULL)) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (sock->state != TCP_ESTABLISHED) {
    LOG(node->name, "TCP send: socket not ESTABLISHED (state=%s)", tcp_state_name(sock->state));
    magi_errno = MAGI_ERR_CONNRESET;
    return MAGI_ERR_CONNRESET;
  }

  return tcp_send_segment(sock, TCP_FLAG_PSH | TCP_FLAG_ACK, sock->ack_num, data, len);
}

/**
 * @brief Initiate a graceful TCP close (send FIN).
 *
 * From ESTABLISHED: sends FIN+ACK, transitions to FIN_WAIT_1.
 * From CLOSE_WAIT: sends FIN+ACK, transitions to LAST_ACK.
 * In all other states the close is rejected with MAGI_ERR_CONNRESET.
 *
 * @param sock  TCP socket to close.
 * @param node  Owning node.
 * @return MAGI_OK on success, MAGI_ERR_CONNRESET if the socket
 *         state does not permit a close.
 */
int tcp_socket_close(TCPSocket* sock, struct Node* node) {
  if (sock == NULL || node == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  if (sock->state == TCP_ESTABLISHED) {
    sock->state = TCP_FIN_WAIT_1;
    log_transition(sock, TCP_ESTABLISHED, TCP_FIN_WAIT_1, TCP_FLAG_FIN, sock->seq_num,
                   sock->ack_num);
    return tcp_send_segment(sock, TCP_FLAG_FIN | TCP_FLAG_ACK, sock->ack_num, NULL, 0U);
  }

  if (sock->state == TCP_CLOSE_WAIT) {
    sock->state = TCP_LAST_ACK;
    log_transition(sock, TCP_CLOSE_WAIT, TCP_LAST_ACK, TCP_FLAG_FIN, sock->seq_num, sock->ack_num);
    return tcp_send_segment(sock, TCP_FLAG_FIN | TCP_FLAG_ACK, sock->ack_num, NULL, 0U);
  }

  LOG(node->name, "TCP close: cannot close socket in state %s", tcp_state_name(sock->state));
  return MAGI_ERR_CONNRESET;
}
