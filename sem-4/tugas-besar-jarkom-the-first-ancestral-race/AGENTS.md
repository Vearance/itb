# AGENTS.md — Magi System

This file is the single source of truth for any coding agent working on this codebase.
Read it fully before touching any file. Every rule here is load-bearing.

---

## 1. What This Project Is

Magi System is a **user-space OSI network simulator** written in **C2Y**. It simulates
Ethernet, ARP, IPv4, ICMP, TCP, UDP, DHCP, DNS, HTTP, and RIPv2 entirely in memory —
no OS network APIs, no sockets, no `<sys/socket.h>`, no `<arpa/inet.h>`, no
`<netinet/in.h>`. Every frame, packet, and segment is built manually as a byte array.

The simulator runs as an interactive CLI (`TFAR> ` prompt). Topologies are loaded from
JSON files. Nodes (hosts, switches, routers) are in-memory structs connected by Link
objects. Packets travel between nodes by calling function pointers on Interface structs.

**Compiler:** `clang`, **Standard:** C2X, **Platforms:** Debian Linux and Windows WSL2.

---

## 2. Repository Layout

```
magi_system/
├── Makefile          # make run | make debug | make async | make gui | make clean | make test
├── README.md
├── topology.json     # default topology loaded on startup
├── vendor/
│   ├── cJSON.h / cJSON.c        # JSON (NOT a network lib — allowed)
│   ├── mongoose.h / mongoose.c  # HTTP server for GUI dashboard only
│   └── README.md                # library provenance
├── src/
│   ├── main.c
│   ├── core/         # Packet, Interface, Link, Node — foundational types
│   ├── utils/        # HashMap, byteops, MAC, arena, log, magi_error
│   ├── layer2/       # EthernetFrame, ARP, Switch, Host (L2)
│   ├── layer3/       # IPv4, ICMP, Router, fragmentation
│   ├── layer4/       # TCP, TCPSocket, UDP
│   ├── layer7/       # MagiSocket, DHCP, DNS, HTTP, RIPv2
│   ├── middleboxes/  # ACL, NAT/PAT
│   ├── async/        # Concurrent engine, thread-safe queue (bonus)
│   ├── cli/          # CLI loop, all command handlers
│   ├── topology/     # In-memory topology registry, JSON load/save
│   └── gui/          # Web dashboard via Mongoose (bonus)
│       └── frontend/ # index.html, app.js, style.css
└── tests/            # Plain C test programs + integration shell scripts
```

---

## 3. Absolute Rules — Read Before Writing Any Code

### 3.1 No OS Network APIs

The following are **banned everywhere** in `src/`:

```
socket()  bind()  connect()  listen()  accept()  send()  recv()
<sys/socket.h>  <netinet/in.h>  <arpa/inet.h>  <netdb.h>
import socket  java.net.*  net/http  (and equivalents in any language)
```

Mongoose is only permitted inside `src/gui/dashboard.c`. It must never be `#include`d
from any file in `core/`, `layer2/`, `layer3/`, `layer4/`, or `layer7/`.

### 3.2 Include Dependency Rules

Layers may only include **downward**. This is enforced by CI. Violations break the build.

| Module | May `#include` from | Must NOT `#include` from |
|---|---|---|
| `core/` | `utils/` | `layer2/` `layer3/` `layer4/` `layer7/` `middleboxes/` |
| `layer2/` | `core/` `utils/` | `layer3/` `layer4/` `layer7/` `middleboxes/` |
| `layer3/` | `core/` `utils/` | `layer2/` (direct) `layer4/` `layer7/` |
| `layer4/` | `core/` `utils/` `layer3/` (IPv4 pseudo-header only) | `layer2/` `layer7/` |
| `layer7/` | `core/` `utils/` `layer3/` `layer4/` | `layer2/` `middleboxes/` |
| `middleboxes/` | `core/` `utils/` `layer3/` | `layer4/` `layer7/` |
| `async/` | `core/` `utils/` | all layer dirs |
| `cli/` | all `src/` dirs | — (top-level integrator) |
| `topology/` | `core/` `utils/` `vendor/cJSON` | layer dirs |
| `gui/` | `topology/` `utils/` `vendor/mongoose` | layer dirs |

### 3.3 No `printf()` in Layer Code

All output goes through `LOG()` or `LOG_WARN()` from `utils/log.h`. Never call
`printf()` or `fprintf()` directly inside `core/`, `layer2/`, `layer3/`, `layer4/`,
`layer7/`, or `middleboxes/`.

```c
#define LOG(node_name, fmt, ...) \
    printf("[%s] " fmt "\n", (node_name), ##__VA_ARGS__)
#define LOG_WARN(node_name, fmt, ...) \
    fprintf(stderr, "[%s][WARN] " fmt "\n", (node_name), ##__VA_ARGS__)
```

### 3.4 Header Guards

Every header must use the pattern `#ifndef MAGI_{DIR}_{FILE}_H`. Example:
`src/layer3/ipv4.h` → `#ifndef MAGI_LAYER3_IPV4_H`.

### 3.5 Feature-Test Macro

Every `.c` file must have this as the **first line** before any `#include`:

```c
#define _POSIX_C_SOURCE 200809L
```

### 3.6 Compiler Flags (do not change these)

```makefile
CC    = clang
CFLAGS = -std=c2x -Wall -Wextra -Wpedantic -Werror -D_POSIX_C_SOURCE=200809L
# Release: append -O2 -DNDEBUG
# Debug:   append -g -fsanitize=address,undefined -fno-omit-frame-pointer
```

Zero warnings is enforced (`-Werror`). Do not suppress warnings with pragmas or casts
unless there is a documented reason in a comment on the same line.

---

## 4. Async-Readiness Rules (apply to every function)

The codebase has two build modes: sequential (default `make run`) and concurrent
(`make async`, which defines `-DMAGI_ASYNC`). The async engine activates pthreads per
node. Every function must be async-ready from the start.

**A. No hidden global state.**
Routing tables, ARP caches, MAC tables, TCP sockets — all live as fields inside their
owning struct. File-scope variables are banned except for compile-time constants
(`static const`). `magi_errno` is `_Thread_local` and is the only permitted global-ish
error carrier.

**B. `nanosleep()` only inside `link_transmit()`.**
No other function may block or sleep. If a function needs to "wait" for a reply (e.g.
ARP pending), it queues the pending work and returns immediately.

**C. Buffer ownership transfers on `link_transmit()`.**
After calling `link_transmit(link, iface, data, len)`, the sender must not read or free
`data`. In sequential mode the receiver frees it inside `handle_receive()`. In async
mode the receiver's thread frees it after dequeuing.

**D. HashMap is not thread-safe.**
`hashmap_set/get/delete` have no internal locking. In async mode, the caller must hold
`node->lock` before touching any node-owned hashmap. Mark every such call site with:
`/* caller holds node->lock */`

**E. `#ifdef MAGI_ASYNC` is the only permitted conditional compilation.**
Use it to select between the sequential and concurrent bodies of `link_transmit()`,
`node_new()`, `node_free()`, and `engine_init()`. Function signatures must be identical
in both modes — only bodies differ.

---

## 5. Naming Conventions

| Category | Convention | Example |
|---|---|---|
| Structs | PascalCase | `EthernetFrame`, `TCPSocket`, `NATEntry` |
| Enums (type) | PascalCase | `TCPState` |
| Enum values | UPPER\_SNAKE | `TCP_ESTABLISHED` |
| Functions | `module_verb_noun()` snake\_case | `hashmap_set()`, `arp_send_request()` |
| Macros | UPPER\_SNAKE\_CASE | `WRITE_U16`, `MAGI_ERR_NOROUTE` |
| Local variables | snake\_case | `recv_buf`, `src_port` |
| Constants | UPPER\_SNAKE with module prefix | `ETHERNET_HDR_LEN`, `IPV4_MAX_TTL` |
| Test files | `test_{module}.c` | `test_hashmap.c` |

---

## 6. Error Handling Convention

- Functions that **perform operations** return `int`: `0` = `MAGI_OK`, negative = error
  code from `utils/magi_error.h`.
- Functions that **construct objects** return a pointer: non-`NULL` = success,
  `NULL` = failure with `magi_errno` set.
- Set `magi_errno` before returning any error. Callers check it for detail.

```c
// utils/magi_error.h
extern _Thread_local int magi_errno;

#define MAGI_OK             0
#define MAGI_ERR_NOMEM     -1   // malloc returned NULL
#define MAGI_ERR_NOROUTE   -2   // no matching routing table entry
#define MAGI_ERR_BADCKSUM  -3   // IPv4/TCP/UDP checksum mismatch
#define MAGI_ERR_TTL       -4   // TTL expired
#define MAGI_ERR_NOLINK    -5   // interface has no attached link
#define MAGI_ERR_PORTUSED  -6   // TCP/UDP port already bound
#define MAGI_ERR_CONNRESET -7   // RST received
#define MAGI_ERR_TIMEOUT   -8   // TCP retransmit timeout
#define MAGI_ERR_ACL_DENY  -9   // packet dropped by ACL rule
#define MAGI_ERR_FRAGMENTED -10 // fragment stored, not yet complete
#define MAGI_ERR_BADARGS   -11  // invalid arguments
```

---

## 7. Memory Management

| Allocation type | Strategy |
|---|---|
| Packet buffers (hot path) | Arena allocator — one per node, reset after each packet |
| Nodes, links, interfaces, sockets | `malloc` / `free` per object |
| HashMap entries (MAC table, ARP cache, routing table) | `malloc` per entry key; freed on table destroy or eviction |
| CLI input | Stack-allocated 1024-byte buffer |
| JSON parse/serialize | cJSON internal alloc; always call `cJSON_Delete()` when done |

The arena struct is:
```c
typedef struct Arena { uint8_t *base; size_t cap; size_t offset; } Arena;
Arena  *arena_new(size_t capacity);   // default node arena = 64 KB
void   *arena_alloc(Arena *a, size_t size); // 8-byte aligned; NULL if full
void    arena_reset(Arena *a);        // bumps offset back to 0, no free
void    arena_free(Arena *a);         // frees base and struct
```

Arena allocations are owned by the node that owns the arena. Never pass an arena
pointer across node boundaries.

---

## 8. Core Data Structures (locked interfaces — do not change signatures)

### 8.1 `utils/hashmap.h`

Open-addressing hash map with linear probing and FNV-1a hashing. Used by MAC table,
ARP cache, routing table, port-to-socket table, NAT table. **Not thread-safe.**
Resize threshold: load factor > 0.7 (doubles capacity, rehashes).

```c
typedef struct HashEntry {
    char    *key;        // heap-allocated copy; NULL = empty slot
    void    *value;      // caller-owned; hashmap_free does NOT free values
    bool     tombstone;  // true = deleted slot for linear probing
} HashEntry;

typedef struct HashMap {
    HashEntry *entries;
    size_t     capacity; // always a power of 2; minimum 16
    size_t     count;
    size_t     tombstones;
} HashMap;

HashMap *hashmap_new(size_t initial_capacity);
void     hashmap_free(HashMap *map);          // frees keys, NOT values
int      hashmap_set(HashMap *map, const char *key, void *value);
void    *hashmap_get(const HashMap *map, const char *key);
int      hashmap_delete(HashMap *map, const char *key);
void     hashmap_foreach(const HashMap *map,
             void (*fn)(const char *key, void *value, void *ctx), void *ctx);
```

### 8.2 `core/packet.h` — abstract base for all protocol messages

Every concrete packet type (`EthernetFrame`, `IPv4Packet`, `TCPSegment`, etc.) embeds
`Packet` as its **first member**, enabling safe casting.

```c
typedef struct Packet {
    uint8_t *buf;   // serialized bytes (arena or heap)
    size_t   len;
    int    (*to_bytes)(struct Packet *self, uint8_t *out, size_t out_len);
    int    (*from_bytes)(struct Packet *self, const uint8_t *in, size_t in_len);
    void   (*destroy)(struct Packet *self);
} Packet;
// Default destroy: free(self->buf); free(self);
```

### 8.3 `core/interface.h`

```c
typedef void (*send_fn_t)(struct Interface*, const uint8_t*, size_t);
typedef void (*recv_fn_t)(struct Interface*, const uint8_t*, size_t);

typedef struct Interface {
    struct Node  *node;
    uint8_t       mac[6];
    uint16_t      port_number;
    struct Link  *link;
    send_fn_t     send_down;   // registered at node init; points to L2 send fn
    recv_fn_t     receive_up;  // registered at node init; points to node handler
#ifdef MAGI_ASYNC
    struct MagiQueue *queue;   // node's inbound message queue
#endif
} Interface;

Interface *interface_new(struct Node *node, uint16_t port);
void       interface_free(Interface *iface);
int        interface_send(Interface *iface, const uint8_t *data, size_t len);
```

Never `sizeof(Interface)` as a constant — it differs between sequential and async builds.

### 8.4 `core/link.h`

```c
typedef struct Link {
    Interface *endpoint_a;
    Interface *endpoint_b;
    uint32_t   delay_ms;
    uint16_t   mtu;   // 0 = unlimited; used for IP fragmentation
} Link;

Link *link_new(Interface *a, Interface *b, uint32_t delay_ms, uint16_t mtu);
void  link_free(Link *link);
int   link_transmit(Link *link, Interface *sender, const uint8_t *data, size_t len);
```

`link_transmit` body under sequential mode: `nanosleep(delay_ms)` then call
`receiver->receive_up(receiver, data, len)`.
`link_transmit` body under `#ifdef MAGI_ASYNC`: enqueue `MagiMsg` onto
`receiver->node->queue` — do NOT call `receive_up` directly.

### 8.5 `core/node.h`

```c
typedef struct Node {
    char       name[64];
    HashMap   *interfaces;   // key = port number as decimal string → Interface*
    void      (*handle_receive)(struct Node*, Interface*, const uint8_t*, size_t);
#ifdef MAGI_ASYNC
    pthread_t        thread;
    struct MagiQueue *queue;
    pthread_mutex_t  lock;
#endif
} Node;

Node      *node_new(const char *name);
void       node_free(Node *node);
Interface *node_add_interface(Node *node, uint16_t port);
Interface *node_get_interface(Node *node, uint16_t port);
int        node_remove_interface(Node *node, uint16_t port);
```

Port key format: `snprintf(key, 16, "%u", port)`.

### 8.6 `utils/byteops.h` — packet serialization macros

All packet fields are written in **network (big-endian) byte order**. Use only these
macros — never type-punning or `*(uint16_t*)ptr` casts.

```c
#define WRITE_U8(buf,off,v)   ((buf)[(off)]=(uint8_t)(v))
#define WRITE_U16(buf,off,v)  do{(buf)[(off)]=(uint8_t)((v)>>8); \
    (buf)[(off)+1]=(uint8_t)((v)&0xFF);}while(0)
#define WRITE_U32(buf,off,v)  do{(buf)[(off)]=(uint8_t)((v)>>24); \
    (buf)[(off)+1]=(uint8_t)((v)>>16); \
    (buf)[(off)+2]=(uint8_t)((v)>>8); \
    (buf)[(off)+3]=(uint8_t)((v)&0xFF);}while(0)
#define READ_U16(buf,off) \
    ((uint16_t)((buf)[(off)]<<8)|(buf)[(off)+1])
#define READ_U32(buf,off) \
    ((uint32_t)((buf)[(off)]<<24)|((buf)[(off)+1]<<16)| \
               ((buf)[(off)+2]<<8)|(buf)[(off)+3])

uint16_t ipv4_checksum(const uint8_t *header, size_t len);
// One's complement sum of 16-bit words. Validating a header: result must be 0x0000.

uint16_t transport_checksum(const uint8_t *pseudo_hdr, size_t ph_len,
                             const uint8_t *segment,   size_t seg_len);
// IPv4 pseudo-header: src_ip[4] + dst_ip[4] + 0x00 + protocol[1] + seg_len[2] = 12 bytes
// Used for both TCP and UDP checksums. UDP checksum of 0x0000 stored as 0xFFFF (RFC 768).
```

### 8.7 `utils/mac.h`

```c
void mac_generate(uint8_t out[6], const char *node_name, uint16_t port);
// FNV-1a hash of node_name XOR port. Sets locally-administered bit (byte[0] bit1=1).
// Sets unicast bit (byte[0] bit0=0). Deterministic across save/load cycles.

void mac_to_str(const uint8_t mac[6], char out[18]); // "AA:BB:CC:DD:EE:FF"
int  mac_from_str(const char *str, uint8_t out[6]);   // MAGI_OK or MAGI_ERR_BADARGS
```

MAC addresses are **never stored in topology JSON** — always regenerated on load via
`mac_generate`.

---

## 9. Layer 2 — Data Link

### 9.1 `layer2/ethernet.h`

```c
typedef struct EthernetFrame {
    Packet   base;
    uint8_t  dst_mac[6];
    uint8_t  src_mac[6];
    bool     vlan_tagged;
    uint16_t vlan_id;       // 12-bit VID; 0 if not tagged
    uint16_t ether_type;    // 0x0800=IPv4, 0x0806=ARP, 0x8100=VLAN tag
    uint8_t *payload;       // points into base.buf after header
    size_t   payload_len;
} EthernetFrame;

int ethernet_pack(EthernetFrame *frame, uint8_t *out, size_t out_len);
int ethernet_unpack(EthernetFrame *frame, const uint8_t *in, size_t in_len);
```

Wire layout untagged: `dst[6] | src[6] | ethertype[2] | payload` (14-byte header).
Wire layout 802.1Q tagged: `dst[6] | src[6] | 0x8100[2] | TCI[2] | ethertype[2] | payload`
(18-byte header). TCI = `(priority<<13)|(dei<<12)|vid`.

### 9.2 `layer2/arp.h`

ARP packet is 28 bytes: `htype[2]=0x0001 | ptype[2]=0x0800 | hlen[1]=6 | plen[1]=4 |`
`oper[2] | sender_mac[6] | sender_ip[4] | target_mac[6] | target_ip[4]`.

```c
int  arp_request_pack(ARPMessage *msg, uint8_t *out, size_t out_len);
int  arp_reply_pack(ARPMessage *msg, uint8_t *out, size_t out_len);
int  arp_unpack(ARPMessage *msg, const uint8_t *in, size_t in_len);

int  arp_send_request(Host *host, const uint8_t target_ip[4]);
int  arp_cache_lookup(Host *host, const uint8_t ip[4], uint8_t mac_out[6]);
void arp_cache_insert(Host *host, const uint8_t ip[4], const uint8_t mac[6]);
int  arp_queue_packet(Host *host, const uint8_t ip[4],
                      const uint8_t *pkt, size_t len);
void arp_flush_queue(Host *host, const uint8_t ip[4], const uint8_t mac[6]);
```

ARP cache key = dotted-decimal IP string. Value = `uint8_t[6]` MAC (heap-allocated).
Pending queue key = IP string. Value = linked list of `malloc`'d packet copies.
`arp_flush_queue` inserts into cache then sends all pending packets via `host_send_l2`.

### 9.3 `layer2/switch.h`

Switch embeds `Node` as first member. MAC table key = `"VLANID:AA:BB:CC:DD:EE:FF"`.

```c
Switch *switch_new(const char *name, uint16_t num_ports);
void    switch_free(Switch *sw);
void    switch_handle_receive(Node *node, Interface *in_iface,
                              const uint8_t *data, size_t len);
```

Forwarding rules: broadcast (`FF:FF:FF:FF:FF:FF`) → flood to all ports same VLAN except
ingress. Known unicast → unicast to stored port. Unknown unicast → flood.
Access ports: strip VLAN tag on egress, add VLAN tag on ingress.
Trunk ports: pass 802.1Q tag intact.

### 9.4 `layer2/host.h` (L2 portion)

```c
int host_send_l2(Host *host, const uint8_t *dst_mac,
                 uint16_t ethertype, const uint8_t *payload, size_t payload_len);
void host_handle_receive(Node *node, Interface *iface,
                          const uint8_t *data, size_t len);
```

`host_handle_receive` dispatches on ethertype: `0x0806` → ARP handler,
`0x0800` → IPv4 handler. Frames with wrong `dst_mac` (not host's MAC and not broadcast)
are silently dropped with no log entry.

---

## 10. Layer 3 — Network

### 10.1 `layer3/ipv4.h`

Fixed 20-byte header (no IP options). `version=4`, `IHL=5`.

```c
typedef struct IPv4Packet {
    Packet   base;
    uint8_t  version_ihl;   // 0x45
    uint8_t  tos;
    uint16_t total_len;
    uint16_t identification; // per-host counter; atomic_fetch_add under MAGI_ASYNC
    uint16_t flags_frag_off; // bit15=reserved, bit14=DF, bit13=MF, bits12-0=offset
    uint8_t  ttl;
    uint8_t  protocol;       // 1=ICMP, 6=TCP, 17=UDP
    uint16_t checksum;
    uint8_t  src_ip[4];
    uint8_t  dst_ip[4];
    uint8_t *payload;
    size_t   payload_len;
} IPv4Packet;

int ipv4_pack(IPv4Packet *pkt, uint8_t *out, size_t out_len);
int ipv4_unpack(IPv4Packet *pkt, const uint8_t *in, size_t in_len);

// Bonus: IP fragmentation
int ipv4_fragment(const IPv4Packet *pkt, uint16_t mtu,
                  IPv4Packet **frags_out, size_t *count_out);
int ipv4_try_reassemble(Host *host, IPv4Packet *frag, IPv4Packet *out);
```

`ipv4_pack`: set checksum field to 0, call `ipv4_checksum`, write result back.
`ipv4_unpack`: call `ipv4_checksum` on raw 20 bytes — result must be `0x0000` or return
`MAGI_ERR_BADCKSUM`. Set `payload` pointer at byte offset 20 into `in`.

Fragment offset is in units of 8 bytes. Reassembly uses a per-host hashmap keyed on
`identification`. Complete when `MF=0` and all offsets contiguous.

### 10.2 `layer3/icmp.h`

```c
typedef struct ICMPMessage {
    Packet  base;
    uint8_t  type;      // 0=Echo Reply, 3=Unreachable, 8=Echo Request, 11=Time Exceeded
    uint8_t  code;
    uint16_t checksum;  // covers ICMP header + data (no pseudo-header)
    uint8_t *payload;
    size_t   payload_len;
} ICMPMessage;

int icmp_pack(ICMPMessage *msg, uint8_t *out, size_t out_len);
int icmp_unpack(ICMPMessage *msg, const uint8_t *in, size_t in_len);
```

Types 3 and 11 include the original IP header (20 bytes) + first 8 bytes of original
transport header starting at payload offset 8 (after unused[4]).
Echo: header = `type[1]|code[1]|checksum[2]|id[2]|seq[2]` then data.

### 10.3 `layer3/router.h`

```c
typedef struct RoutingTableEntry {
    uint8_t  network[4];
    uint8_t  mask[4];
    int      prefix_len;
    uint8_t  next_hop[4];
    uint16_t out_port;
    uint8_t  metric;     // used by RIPv2; 1 for directly connected
} RoutingTableEntry;

Router *router_new(const char *name);
void    router_free(Router *r);
int     router_add_route(Router *r, const char *dest_cidr,
                          const char *next_hop_ip, uint16_t out_port);
int     router_remove_route(Router *r, const char *dest_cidr);
RoutingTableEntry *lpm_lookup(Router *r, const uint8_t dst_ip[4]);
void    router_handle_receive(Node *node, Interface *in_iface,
                               const uint8_t *data, size_t len);
```

Routing table is a **sorted array** (not a hashmap) — sorted descending by
`prefix_len` so LPM iterates and first match wins. Re-sort after every
`router_add_route`.

`router_handle_receive` pipeline:
1. Unpack Ethernet frame; strip VLAN tag if trunk port.
2. Unpack IPv4; validate checksum — drop on failure.
3. Check ACL (if configured) — drop on `MAGI_ERR_ACL_DENY`.
4. Decrement TTL. If TTL == 0 → send ICMP Time Exceeded (type=11) to src; drop.
5. LPM lookup. If NULL → send ICMP Unreachable (type=3, code=0) to src; drop.
6. Recalculate IPv4 checksum (TTL changed).
7. ARP lookup for next hop. If miss → queue; send ARP request.
8. Re-encapsulate in new Ethernet frame; add VLAN tag if egress is trunk; call
   `interface_send`.

---

## 11. Layer 4 — Transport

### 11.1 `layer4/tcp.h`

TCP flags byte: `bit0=FIN(0x01)`, `bit1=SYN(0x02)`, `bit2=RST(0x04)`,
`bit3=PSH(0x08)`, `bit4=ACK(0x10)`. `data_offset=5` (20-byte header, no options).

```c
typedef struct TCPSegment {
    Packet   base;
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t  data_offset;  // = 5
    uint8_t  flags;
    uint16_t window_size;
    uint16_t checksum;     // computed with IPv4 pseudo-header
    uint8_t *payload;
    size_t   payload_len;
} TCPSegment;

int tcp_pack(TCPSegment *seg, const uint8_t src_ip[4],
             const uint8_t dst_ip[4], uint8_t *out, size_t out_len);
int tcp_unpack(TCPSegment *seg, const uint8_t src_ip[4],
               const uint8_t dst_ip[4], const uint8_t *in, size_t in_len);
```

### 11.2 `layer4/tcp_socket.h` — state machine

```c
typedef enum {
    TCP_CLOSED, TCP_LISTEN, TCP_SYN_SENT, TCP_SYN_RCVD,
    TCP_ESTABLISHED, TCP_FIN_WAIT_1, TCP_FIN_WAIT_2,
    TCP_CLOSE_WAIT, TCP_LAST_ACK, TCP_TIME_WAIT
} TCPState;

typedef struct TCPSocket {
    TCPState  state;
    uint8_t   local_ip[4];
    uint16_t  local_port;
    uint8_t   remote_ip[4];
    uint16_t  remote_port;
    uint32_t  seq_num;
    uint32_t  ack_num;
    uint8_t  *recv_buf;      // heap-allocated 16 KB
    size_t    recv_buf_len;
    size_t    recv_buf_cap;
    HashMap  *out_of_order;  // seq_num (decimal string) → TCPSegment*
} TCPSocket;

TCPSocket *tcp_socket_new(void);
void       tcp_socket_free(TCPSocket *sock);
int        tcp_socket_handle_segment(TCPSocket *sock, TCPSegment *seg, Host *host);
int        tcp_recv_buf_insert(TCPSocket *sock, TCPSegment *seg);
int        tcp_recv_buf_read(TCPSocket *sock, uint8_t *out, size_t len);
```

**State transition table (exhaustive):**

| Current state | Received flags | Action | Next state |
|---|---|---|---|
| CLOSED | SYN | Send SYN+ACK; store remote seq+1 as ack\_num; generate ISS | SYN\_RCVD |
| CLOSED | any other | Send RST if RST not set; drop | CLOSED |
| LISTEN | SYN | Send SYN+ACK; store remote seq+1 as ack\_num; generate ISS | SYN\_RCVD |
| LISTEN | RST | Drop silently | LISTEN |
| LISTEN | any other | Drop silently | LISTEN |
| SYN\_SENT | SYN+ACK | Validate ack\_num==ISS+1; send ACK; set seq\_num=ISS+1 | ESTABLISHED |
| SYN\_SENT | SYN (simultaneous open) | Send SYN+ACK | SYN\_RCVD |
| SYN\_SENT | RST | If ack\_num valid: teardown; log "Connection refused" | CLOSED |
| SYN\_SENT | ACK only (no SYN) | Send RST; drop | CLOSED |
| SYN\_SENT | timeout (async only) | Retransmit SYN; after 3 retries → CLOSED | SYN\_SENT / CLOSED |
| SYN\_RCVD | ACK | Validate ack\_num==ISS+1 | ESTABLISHED |
| SYN\_RCVD | RST | Passive open → LISTEN; active open → CLOSED | LISTEN / CLOSED |
| SYN\_RCVD | SYN | Send RST; close | CLOSED |
| SYN\_RCVD | FIN+ACK | Validate ACK; send ACK; log premature FIN | CLOSE\_WAIT |
| ESTABLISHED | PSH / data | Validate seq\_num; insert recv\_buffer; send ACK | ESTABLISHED |
| ESTABLISHED | ACK | Update send window; advance if ack > last unacked | ESTABLISHED |
| ESTABLISHED | FIN | Send ACK; signal application | CLOSE\_WAIT |
| ESTABLISHED | FIN+ACK | Send ACK; signal application | CLOSE\_WAIT |
| ESTABLISHED | RST | Teardown immediately; no ACK | CLOSED |
| ESTABLISHED | SYN (unexpected) | Send RST; close (half-open) | CLOSED |
| ESTABLISHED | out-of-order PSH | Buffer by seq\_num; no ACK yet | ESTABLISHED |
| ESTABLISHED | duplicate PSH | Drop; resend ACK for expected seq | ESTABLISHED |
| FIN\_WAIT\_1 | ACK (of our FIN) | FIN acknowledged | FIN\_WAIT\_2 |
| FIN\_WAIT\_1 | FIN (simultaneous close) | Send ACK | CLOSING |
| FIN\_WAIT\_1 | FIN+ACK | Send ACK | TIME\_WAIT |
| FIN\_WAIT\_1 | RST | Teardown | CLOSED |
| FIN\_WAIT\_2 | FIN | Send ACK; start TIME\_WAIT (async: 2×MSL; sequential: immediate) | TIME\_WAIT |
| FIN\_WAIT\_2 | RST | Teardown | CLOSED |
| CLOSING | ACK (of our FIN) | Start TIME\_WAIT timer | TIME\_WAIT |
| CLOSE\_WAIT | app calls close() | Send FIN; increment seq\_num | LAST\_ACK |
| LAST\_ACK | ACK (of our FIN) | Free socket resources | CLOSED |
| LAST\_ACK | RST | Treat as ACK; close | CLOSED |
| TIME\_WAIT | FIN (retransmit) | Resend ACK; restart timer | TIME\_WAIT |
| TIME\_WAIT | timer expires (async) / immediate (sequential) | Free resources | CLOSED |
| TIME\_WAIT | RST | Teardown | CLOSED |
| any | RST | Always log. CLOSED. Never send RST in response to RST. | CLOSED |

Any `(state, flags)` pair not in the table → send RST, transition to CLOSED.

Log every transition: `[HostName:port] TCP state: OLD → NEW (flags=0xXX seq=N ack=M)`

**Receive buffer (`tcp_recv_buf_insert`):**
- `seg->seq_num == sock->ack_num` (in-order): append payload to `recv_buf`, advance
  `ack_num`, send ACK. Then flush contiguous `out_of_order` segments.
- `seg->seq_num > sock->ack_num` (out-of-order): store in `out_of_order` hashmap, no ACK.
- `seg->seq_num < sock->ack_num` (duplicate): drop, resend ACK.

### 11.3 `layer4/udp.h`

```c
typedef struct UDPDatagram {
    Packet   base;
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;    // 8 + payload_len
    uint16_t checksum;  // uses transport_checksum with pseudo-header
    uint8_t *payload;
    size_t   payload_len;
} UDPDatagram;

int udp_pack(UDPDatagram *dgram, uint8_t *out, size_t out_len);
int udp_unpack(UDPDatagram *dgram, const uint8_t *in, size_t in_len);
```

UDP header: `src_port[2] | dst_port[2] | length[2] | checksum[2]` (8 bytes).
Checksum of `0x0000` after computation is stored as `0xFFFF` (RFC 768).

---

## 12. Layer 7 — Application

### 12.1 `layer7/magi_socket.h` — wrapper API

All application protocols (DHCP, DNS, HTTP, RIPv2) use only this API. They never call
`tcp_pack`, `udp_pack`, or any layer4 function directly.

```c
typedef enum { MAGI_AF_INET = 2 } MagiAddrFamily;
typedef enum { MAGI_SOCK_STREAM = 1, MAGI_SOCK_DGRAM = 2 } MagiSockType;

typedef struct MagiSocket {
    MagiAddrFamily  family;
    MagiSockType    type;
    struct Node    *node;
    void           *transport;  // TCPSocket* or UDPSocket*
    bool            bound;
    bool            listening;
} MagiSocket;

MagiSocket *magi_socket(Node *node, MagiAddrFamily, MagiSockType);
int         magi_bind(MagiSocket*, const char *ip, uint16_t port);
int         magi_listen(MagiSocket*, int backlog);
MagiSocket *magi_accept(MagiSocket*);
int         magi_connect(MagiSocket*, const char *ip, uint16_t port);
int         magi_send(MagiSocket*, const uint8_t *data, size_t len);
int         magi_recv(MagiSocket*, uint8_t *buf, size_t buf_len);
int         magi_close(MagiSocket*);
```

In sequential mode, `magi_connect` completes the full 3-way handshake before returning.
`magi_accept` on a LISTEN socket completes the handshake synchronously before returning
the new socket.

### 12.2 Protocol ports

| Protocol | Transport | Port(s) |
|---|---|---|
| DHCP server | UDP | 67 |
| DHCP client | UDP | 68 |
| DNS | UDP | 53 |
| HTTP | TCP | 80 |
| RIPv2 | UDP broadcast | 520 |

### 12.3 DHCP — DORA flow

`dhcp_client_discover` broadcasts DISCOVER (`src_ip=0.0.0.0`, `dst=255.255.255.255`).
Server replies OFFER. Client sends REQUEST. Server sends ACK. Client sets
`host->ip_address`. Required fields: `op`, `htype=1`, `hlen=6`, `xid` (random),
`yiaddr`, `siaddr`, options (message type, subnet mask, router, lease time).

### 12.4 DNS — simplified wire format

Not standard RFC 1035 binary. Simplified:
`query_id[2] | qr[1] | qname[N]\0 | qtype[2]`.
Response appends `rdata[4]` (IPv4 address). Server looks up in a hashmap of
`name → ip_string` records.

### 12.5 HTTP — plaintext GET

Client sends: `GET /path HTTP/1.1\r\nHost: hostname\r\nConnection: close\r\n\r\n`
Server replies: `HTTP/1.1 200 OK\r\nContent-Length: N\r\n\r\n<body>`
Server initiates `magi_close` after sending response (connection: close).

### 12.6 RIPv2

Bellman-Ford over UDP broadcast port 520. Update message contains
`(network_cidr, metric)` pairs. Metric=1 for directly connected; infinity=16.
Triggered by: `rip trigger` CLI command, any `link`/`unlink` event, or every 30 seconds
under `MAGI_ASYNC`.

---

## 13. Middleboxes (Bonus)

### 13.1 `middleboxes/acl.h`

```c
typedef struct ACLRule {
    char     src_cidr[20]; // CIDR notation or "any"
    char     dst_cidr[20];
    uint8_t  protocol;     // 0=any, 1=ICMP, 6=TCP, 17=UDP
    uint16_t src_port;     // 0=any
    uint16_t dst_port;     // 0=any
    bool     permit;       // true=allow, false=deny
} ACLRule;

ACLTable *acl_table_new(void);
int       acl_add_rule(ACLTable *t, ACLRule rule);
int       acl_check(ACLTable *t, const IPv4Packet *pkt);
void      acl_table_free(ACLTable *t);
```

Called in `router_handle_receive` **after LPM, before forwarding**. First matching rule
wins. Default (no match) = permit. Deny → silent drop + log
`[RouterName][ACL] DENY proto=X src=A:P dst=B:P matched rule N`.

### 13.2 `middleboxes/nat.h`

```c
NATTable *nat_table_new(const uint8_t public_ip[4]);
void      nat_table_free(NATTable *t);
int       nat_translate_out(NATTable *t, IPv4Packet *pkt, TCPSegment *seg);
int       nat_translate_in(NATTable *t, IPv4Packet *pkt, TCPSegment *seg);
```

Forward map keyed on `"proto:pub_port"`. Reverse map keyed on `"proto:priv_ip:priv_port"`.
`nat_translate_out`: rewrite `src_ip=public_ip`, `src_port=pub_port`; recompute
IPv4 and TCP checksums.
`nat_translate_in`: lookup by `dst_port`; rewrite `dst_ip=priv_ip`, `dst_port=priv_port`;
recompute checksums.
Public ports allocated starting at 1024, incrementing per session.

---

## 14. Async Engine (Bonus — `make async`)

### 14.1 `async/queue.h`

```c
typedef struct MagiMsg {
    struct Interface *src_iface;
    uint8_t          *data;   // heap-allocated; receiver calls free(data)
    size_t            len;
} MagiMsg;

typedef struct MagiQueue {
    MagiMsg         *buf;
    size_t           head, tail, cap;
    pthread_mutex_t  lock;
    pthread_cond_t   not_empty;
} MagiQueue;

MagiQueue *queue_new(size_t capacity);  // default 256
void       queue_free(MagiQueue *q);
int        queue_push(MagiQueue *q, MagiMsg msg);  // MAGI_ERR_NOMEM if full
int        queue_pop(MagiQueue *q, MagiMsg *out);  // blocks until message available
```

`queue_push`: lock → check full → write at `buf[tail]` → advance tail → signal
`not_empty` → unlock.
`queue_pop`: lock → `pthread_cond_wait` while empty → read `buf[head]` → advance head
→ unlock.

### 14.2 `async/engine.h`

```c
int  engine_init(struct Topology *topo);
void engine_shutdown(void);
```

`engine_init`: `pthread_create` one thread per node; each thread loops on `queue_pop`,
acquires `node->lock`, calls `node->handle_receive`, frees `msg.data`, releases lock.
Also spawns `rip_timer_thread` that calls `rip_send_update` on all routers every 30s.
`engine_shutdown`: sets `atomic_bool stop_flag=true`, pushes sentinel `MagiMsg`
(data=NULL) to each queue to unblock `queue_pop`, then `pthread_join` all threads.

---

## 15. CLI Commands

All commands parsed in `cli/commands.c`. The parser tokenizes on whitespace, dispatches
on first token. Unknown commands print usage hint and continue — never call `exit()`.

| Command syntax | Handler | Notes |
|---|---|---|
| `create <host\|router\|switch> <name>` | `cmd_create()` | Duplicate name → error |
| `link <dev1> <dev2> [delay_ms]` | `cmd_link()` | Format: `Name` or `Name:Port`; host defaults to port 1 |
| `unlink <dev1> <dev2>` | `cmd_unlink()` | Sets both interface->link = NULL |
| `topology` | `cmd_topology()` | Prints all nodes + links |
| `save [filename]` | `cmd_save()` | JSON, no MAC addresses written |
| `load [filename]` | `cmd_load()` | Regenerates MACs via `mac_generate` |
| `<host> ping <ip>` | `cmd_ping()` | TTL=64; prints RTT = sum of link delays |
| `<host> traceroute <ip>` | `cmd_traceroute()` | TTL=1..30; prints each hop |
| `<host> tcp_connect <ip> <port>` | `cmd_tcp_connect()` | Full handshake + teardown |
| `<host> http_server start [file]` | `cmd_http_server_start()` | Listens on port 80 |
| `<host> http_server stop` | `cmd_http_server_stop()` | |
| `<host> http_get <url>` | `cmd_http_get()` | Parses URL, DNS-resolves, fetches |
| `<host> dhcp_discover` | `cmd_dhcp_discover()` | Full DORA logged |
| `<router> route` | `cmd_route()` | Prints table sorted by prefix length desc |
| `<switch> mac` | `cmd_mac_table()` | Prints VLAN:MAC → port |
| `<host\|router> arp` | `cmd_arp()` | Prints IP → MAC cache |
| `acl <router> add <rule>` | `cmd_acl_add()` | Bonus |
| `rip trigger` | `cmd_rip_trigger()` | Bonus; triggers Bellman-Ford |
| `visualize [output.png]` | `cmd_visualize()` | Bonus; falls back to ASCII |
| `exit` / `quit` | `cmd_exit()` | |

---

## 16. Topology JSON Schema

```json
{
  "hosts": [
    { "name": "H1", "ip_address": "192.168.1.10/24", "default_gateway": "192.168.1.1" }
  ],
  "switches": [
    {
      "name": "SW1", "num_ports": 24,
      "vlans": [
        { "port": 1, "mode": "access", "vlan_id": 10 },
        { "port": 24, "mode": "trunk" }
      ]
    }
  ],
  "routers": [
    {
      "name": "R1",
      "interfaces": [
        { "port": 1, "ip_address": "192.168.1.1/24" },
        { "port": 2, "ip_address": "10.0.0.1/8" }
      ],
      "routing_table": [
        { "destination": "0.0.0.0/0", "next_hop": "10.0.0.254", "interface": 2 }
      ]
    }
  ],
  "links": [
    { "endpoints": ["H1", "SW1:1"], "delay": 10 },
    { "endpoints": ["SW1:24", "R1:1"], "delay": 5 }
  ]
}
```

`mac_address` is intentionally absent — always generated at load time via
`mac_generate(node_name, port)`. `delay` is in milliseconds.

---

## 17. Testing

No test framework. Tests are plain C programs in `tests/` linked against the same source
files. `make test` compiles and runs all `test_*.c` files.

### Unit test files

| File | Covers |
|---|---|
| `test_hashmap.c` | Insert, get, delete, collision, resize, tombstone |
| `test_byteops.c` | `WRITE_U16/U32`, `READ_U16/U32`, `ipv4_checksum`, `transport_checksum` |
| `test_mac.c` | `mac_generate` uniqueness and determinism, `mac_to_str/from_str` round-trip |
| `test_ethernet.c` | Frame pack/unpack with and without VLAN tag |
| `test_arp.c` | ARP cache insert/lookup, pending queue flush on reply |
| `test_ipv4.c` | Pack/unpack round-trip, checksum validation, TTL decrement |
| `test_tcp_state.c` | All 35 state transitions, 3-way handshake, 4-way teardown, RST |
| `test_tcp_buffer.c` | Out-of-order reassembly in receive buffer |
| `test_fragmentation.c` | Fragment + reassemble at various MTU sizes |
| `test_lpm.c` | Longest Prefix Match with overlapping routes |
| `test_acl.c` | Permit/deny rules — IP, protocol, port |
| `test_nat.c` | Outbound translation, inbound reverse lookup, port reuse |

### Integration scenarios

All must pass before tagging any release:

1. Same-subnet ping: H1 → SW1 → H2 same VLAN — verify ARP + ICMP round trip.
2. Cross-subnet ping: H1 → SW1 → R1 → SW2 → H2 — verify TTL decrement, RTT = Σ delays.
3. ICMP TTL exceeded: TTL=1, verify Time Exceeded from first router.
4. ICMP Destination Unreachable: send to non-routable IP.
5. TCP handshake + data: `H1 tcp_connect H2:80`, PSH "HELLO\n", teardown.
6. HTTP: `H2 http_server start`, `H1 http_get http://H2/`, verify 200 OK.
7. DHCP: `H3 dhcp_discover`, verify IP assigned; DISCOVER/OFFER/REQUEST/ACK logged.
8. DNS: resolve `www.magi.nerv`, verify IP returned.
9. VLAN isolation: H1 (VLAN10) cannot reach H2 (VLAN20) through same switch.
10. RIPv2 convergence: unlink a route, `rip trigger`, verify routing tables update.
11. NAT: private H1 → NAT router → public H3, verify translation logged both ways.
12. ACL deny: deny rule on R1, verify TCP SYN silently dropped (no SYN+ACK).

---

## 18. Makefile Targets

| Target | What it builds |
|---|---|
| `make run` (default) | Release: `clang -std=c2x -O2 -DNDEBUG`; launches `TFAR>` |
| `make debug` | Debug: appends `-g -fsanitize=address,undefined`; launches `TFAR>` |
| `make async` | Release + `-DMAGI_ASYNC` + `src/async/`; concurrent engine active |
| `make gui` | Release + `src/gui/` + `vendor/mongoose.c`; dashboard at port 8080 |
| `make test` | Compiles and runs all `tests/test_*.c` |
| `make clean` | Removes `build/` |

Source discovery: `$(shell find src -name '*.c')`. `src/async/` and `src/gui/` are
excluded from the default build and added only by their respective targets.

cJSON vendored files compile with an additional `-Wno-pedantic` flag applied only to
`vendor/` to suppress its internal warnings without affecting project code.

---

## 19. Function Ownership Quick Reference

| Function(s) | File | Milestone |
|---|---|---|
| `hashmap_*` | `utils/hashmap.c` | 0 |
| `mac_generate/to_str/from_str` | `utils/mac.c` | 0 |
| `ipv4_checksum`, `transport_checksum` | `utils/byteops.c` | 0 |
| `arena_*` | `utils/arena.c` | 3 |
| `packet` default destroy | `core/packet.c` | 0 |
| `interface_new/free/send` | `core/interface.c` | 0 |
| `node_new/free/add_interface/get_interface/remove_interface` | `core/node.c` | 0 |
| `link_new/free/transmit` | `core/link.c` | 0 |
| `ethernet_pack/unpack` | `layer2/ethernet.c` | 1 |
| `switch_new/free/handle_receive` | `layer2/switch.c` | 1 |
| `host_send_l2` | `layer2/host.c` | 1 |
| `arp_*` | `layer2/arp.c` | 1 |
| `host_handle_receive` | `layer2/host.c` | 1 |
| `ipv4_pack/unpack`, `ipv4_fragment/try_reassemble` | `layer3/ipv4.c` | 2 |
| `icmp_pack/unpack` | `layer3/icmp.c` | 2 |
| `router_new/free/add_route/remove_route`, `lpm_lookup`, `router_handle_receive` | `layer3/router.c` | 2 |
| `tcp_pack/unpack` | `layer4/tcp.c` | 3 |
| `tcp_socket_new/free`, `tcp_socket_handle_segment`, `tcp_recv_buf_insert/read` | `layer4/tcp_socket.c` | 3 |
| `udp_pack/unpack` | `layer4/udp.c` | 3 |
| `magi_socket/bind/listen/accept/connect/send/recv/close` | `layer7/magi_socket.c` | 4 |
| `dhcp_server_start`, `dhcp_client_discover` | `layer7/dhcp.c` | 4 |
| `dns_server_start/add_record`, `dns_resolve` | `layer7/dns.c` | 4 |
| `http_server_start/stop`, `http_get` | `layer7/http.c` | 4 |
| `rip_send_update/recv_update/convergence_check` | `layer7/rip.c` | 4 |
| `acl_table_new/free`, `acl_add_rule`, `acl_check` | `middleboxes/acl.c` | 3 (bonus) |
| `nat_table_new/free`, `nat_translate_out/in` | `middleboxes/nat.c` | 4 (bonus) |
| `queue_new/free/push/pop` | `async/queue.c` | 4 (bonus) |
| `engine_init/shutdown` | `async/engine.c` | 4 (bonus) |
| `cli_run` | `cli/cli.c` | 0 |
| `cmd_create/link/unlink/topology/save/load` | `cli/commands.c` | 0 |
| `cmd_mac`, `cmd_arp` | `cli/commands.c` | 1 |
| `cmd_ping`, `cmd_traceroute` | `cli/commands.c` | 2 |
| `cmd_tcp_connect` | `cli/commands.c` | 3 |
| `cmd_http_*`, `cmd_dhcp_*`, `cmd_rip_*`, `cmd_acl_*`, `cmd_visualize` | `cli/commands.c` | 4 |
| `dashboard_start/stop/push_log/update_topology` | `gui/dashboard.c` | 4 (bonus) |
| `cmd_visualize` | `cli/commands.c` | 4 (bonus) |
