#define _POSIX_C_SOURCE 200809L

#include "http.h"

#include "core/interface.h"
#include "layer3/ipv4.h"
#include "layer7/dns.h"
#include "layer7/magi_socket.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Default HTML page served when no web_root is specified */
static const char* DEFAULT_PAGE =
    "<html><head><title>Magi System</title></head>"
    "<body><h1>Welcome to Magi System HTTP Server</h1>"
    "<p>This page is served by the Magi network simulator.</p></body></html>";

/**
 * @brief Parse a URL into host and path components.
 *
 * Handles "http://host/path", "host/path", and "host" forms.
 */
static void parse_url(const char* url, char* host_out, size_t host_len, char* path_out,
                      size_t path_len) {
  const char* start = url;

  /* Skip "http://" prefix if present */
  if (strncmp(start, "http://", 7U) == 0) {
    start += 7U;
  }

  /* Find the path separator */
  const char* slash = strchr(start, '/');
  if (slash != NULL) {
    size_t hlen = (size_t)(slash - start);
    if (hlen >= host_len) {
      hlen = host_len - 1U;
    }
    memcpy(host_out, start, hlen);
    host_out[hlen] = '\0';
    snprintf(path_out, path_len, "%s", slash);
  } else {
    snprintf(host_out, host_len, "%s", start);
    snprintf(path_out, path_len, "/");
  }
}

int http_server_start(Node* node, const char* web_root) {
  if (node == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  const char* content = (web_root != NULL && web_root[0] != '\0') ? web_root : DEFAULT_PAGE;

  MagiSocket* sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_STREAM);
  if (sock == NULL) {
    return MAGI_ERR_NOMEM;
  }

  /* Determine local IP */
  Interface* iface = node_get_interface(node, 1U);
  const char* bind_ip = "0.0.0.0";
  if (iface != NULL && iface->ip_address[0] != '\0') {
    uint8_t lip[4];
    if (ipv4_parse_address(iface->ip_address, lip) == MAGI_OK) {
      static char lip_str[16];
      ipv4_address_to_string(lip, lip_str);
      bind_ip = lip_str;
    }
  }

  int status = magi_bind(sock, bind_ip, HTTP_PORT);
  if (status != MAGI_OK) {
    LOG(node->name, "HTTP server: failed to bind port 80");
    magi_close(sock);
    return status;
  }

  status = magi_listen(sock, 5);
  if (status != MAGI_OK) {
    LOG(node->name, "HTTP server: failed to listen");
    magi_close(sock);
    return status;
  }

  LOG(node->name, "HTTP server: listening on port 80 (content: %zu bytes)", strlen(content));

  /* In sequential mode, check if there's a pending connection */
  if (magi_has_data(sock)) {
    /* Accept the connection */
    MagiSocket* conn = magi_accept(sock);
    if (conn == NULL) {
      LOG(node->name, "HTTP server: accept failed");
      return MAGI_OK;
    }

    /* Read the GET request */
    uint8_t req_buf[1024];
    int rd = magi_recv(conn, req_buf, sizeof(req_buf) - 1U);
    if (rd > 0) {
      req_buf[rd] = '\0';
      LOG(node->name, "HTTP server: received request (%d bytes):\n%s", rd, (const char*)req_buf);
    }

    /* Build and send response */
    size_t content_len = strlen(content);
    char response[2048];
    int resp_len = snprintf(response, sizeof(response),
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Length: %zu\r\n"
                            "Connection: close\r\n"
                            "\r\n"
                            "%s",
                            content_len, content);

    LOG(node->name, "HTTP server: sending response (%d bytes)", resp_len);
    (void)magi_send(conn, (const uint8_t*)response, (size_t)resp_len);

    /* Server initiates close */
    magi_close(conn);
    LOG(node->name, "HTTP server: connection closed");
  }

  /* Keep the server socket open — don't close it */
  return MAGI_OK;
}

int http_server_stop(Node* node) {
  if (node == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  LOG(node->name, "HTTP server: stopped");
  return MAGI_OK;
}

int http_get(Node* node, const char* url) {
  if (node == NULL || url == NULL) {
    magi_errno = MAGI_ERR_BADARGS;
    return MAGI_ERR_BADARGS;
  }

  char host[64];
  char path[256];
  parse_url(url, host, sizeof(host), path, sizeof(path));

  LOG(node->name, "HTTP GET: host=%s path=%s", host, path);

  /* Check if host is an IP address; if not, DNS-resolve it */
  uint8_t target_ip[4];
  char resolved_ip[16];
  if (ipv4_parse_address(host, target_ip) != MAGI_OK) {
    /* Host is a name — attempt DNS resolution */
    const char* dns_server = node->default_gateway;
    if (dns_server[0] == '\0') {
      LOG(node->name, "HTTP GET: no DNS server (set default_gateway)");
      magi_errno = MAGI_ERR_BADARGS;
      return MAGI_ERR_BADARGS;
    }
    LOG(node->name, "HTTP GET: resolving '%s' via DNS server %s", host, dns_server);
    int status = dns_query(node, dns_server, host, target_ip);
    if (status != MAGI_OK) {
      LOG(node->name, "HTTP GET: DNS resolution failed for '%s'", host);
      return status;
    }
    ipv4_address_to_string(target_ip, resolved_ip);
  } else {
    ipv4_address_to_string(target_ip, resolved_ip);
  }

  /* Create TCP connection */
  MagiSocket* sock = magi_socket(node, MAGI_AF_INET, MAGI_SOCK_STREAM);
  if (sock == NULL) {
    return MAGI_ERR_NOMEM;
  }

  int status = magi_connect(sock, resolved_ip, HTTP_PORT);
  if (status != MAGI_OK) {
    LOG(node->name, "HTTP GET: connection to %s:%u failed", host, (unsigned)HTTP_PORT);
    magi_close(sock);
    return status;
  }

  LOG(node->name, "HTTP GET: connected to %s:%u", host, (unsigned)HTTP_PORT);

  /* Build GET request */
  char request[512];
  int req_len = snprintf(request, sizeof(request),
                         "GET %s HTTP/1.1\r\n"
                         "Host: %s\r\n"
                         "Connection: close\r\n"
                         "\r\n",
                         path, host);

  status = magi_send(sock, (const uint8_t*)request, (size_t)req_len);
  if (status != MAGI_OK) {
    LOG(node->name, "HTTP GET: failed to send request");
    magi_close(sock);
    return status;
  }

  LOG(node->name, "HTTP GET: sent request (%d bytes)", req_len);

  /* Receive response */
  uint8_t resp_buf[2048];
  int rd = magi_recv(sock, resp_buf, sizeof(resp_buf) - 1U);
  if (rd > 0) {
    resp_buf[rd] = '\0';
    LOG(node->name, "HTTP GET: received response (%d bytes):\n%s", rd, (const char*)resp_buf);
  } else {
    LOG(node->name, "HTTP GET: no response data received");
  }

  magi_close(sock);
  return MAGI_OK;
}
