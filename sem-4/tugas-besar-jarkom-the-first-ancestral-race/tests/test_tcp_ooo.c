#define _POSIX_C_SOURCE 200809L

#include "layer4/tcp_socket.h"
#include "layer4/tcp.h"
#include "core/node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_out_of_order() {
    printf("--- Running TCP Receive Buffer Out-of-Order Test ---\n");

    Node* node = node_new("TestNode");
    TCPSocket* sock = tcp_socket_new(node);
    
    // Simulate ESTABLISHED state
    sock->state = TCP_ESTABLISHED;
    sock->ack_num = 1000;
    
    uint8_t src_ip[4] = {192, 168, 1, 1};
    uint8_t dst_ip[4] = {192, 168, 1, 2};

    // Segment 1: seq 1000, len 5 (in order) -> "HELLO"
    TCPSegment seg1;
    memset(&seg1, 0, sizeof(seg1));
    seg1.seq_num = 1000;
    seg1.payload = (const uint8_t*)"HELLO";
    seg1.payload_len = 5;
    seg1.flags = TCP_FLAG_PSH | TCP_FLAG_ACK;
    
    // Segment 2: seq 1005, len 6 (in order) -> " WORLD"
    TCPSegment seg2;
    memset(&seg2, 0, sizeof(seg2));
    seg2.seq_num = 1005;
    seg2.payload = (const uint8_t*)" WORLD";
    seg2.payload_len = 6;
    seg2.flags = TCP_FLAG_PSH | TCP_FLAG_ACK;

    // Segment 3: seq 1011, len 7 (in order) -> " TESTER"
    TCPSegment seg3;
    memset(&seg3, 0, sizeof(seg3));
    seg3.seq_num = 1011;
    seg3.payload = (const uint8_t*)" TESTER";
    seg3.payload_len = 7;
    seg3.flags = TCP_FLAG_PSH | TCP_FLAG_ACK;

    // We will receive them out of order: Seg 3, then Seg 2, then Seg 1
    printf("1. Receiving Segment 3 (seq=1011, len=7) [OUT OF ORDER]\n");
    tcp_socket_handle_segment(sock, &seg3, node, src_ip, dst_ip);
    printf("   Ack num is now: %u (expected 1000)\n", sock->ack_num);
    
    printf("2. Receiving Segment 2 (seq=1005, len=6) [OUT OF ORDER]\n");
    tcp_socket_handle_segment(sock, &seg2, node, src_ip, dst_ip);
    printf("   Ack num is now: %u (expected 1000)\n", sock->ack_num);

    printf("3. Receiving Segment 1 (seq=1000, len=5) [IN ORDER - triggers reassembly]\n");
    tcp_socket_handle_segment(sock, &seg1, node, src_ip, dst_ip);
    printf("   Ack num is now: %u (expected 1018)\n", sock->ack_num);

    // Read the reassembled buffer
    uint8_t read_buf[64] = {0};
    size_t rd = tcp_recv_buf_read(sock, read_buf, sizeof(read_buf));
    printf("   Read %zu bytes from buffer: '%s'\n", rd, read_buf);

    if (strcmp((char*)read_buf, "HELLO WORLD TESTER") == 0) {
        printf("RESULT: PASS - Receive buffer successfully reassembled disjointed PSH segments.\n");
    } else {
        printf("RESULT: FAIL - Reassembled string does not match expected output.\n");
        exit(1);
    }

    tcp_socket_free(sock);
    node_free(node);
}

int main() {
    test_out_of_order();
    return 0;
}
