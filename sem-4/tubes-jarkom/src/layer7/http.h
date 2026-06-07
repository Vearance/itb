/**
 * @file http.h
 * @brief HTTP/1.1 plaintext GET server and client over TCP port 80.
 *
 * Uses only the MagiSocket API. Supports:
 * - Server: listens on port 80, serves static content
 * - Client: sends GET request, receives response
 */

#ifndef MAGI_LAYER7_HTTP_H
#define MAGI_LAYER7_HTTP_H

#include "core/node.h"

#include <stddef.h>

#define HTTP_PORT 80U

/**
 * @brief Start an HTTP server on a host node.
 *
 * Binds a TCP socket to port 80 and enters LISTEN state.
 * The server will serve a default page or the content specified by web_root.
 *
 * @param node     Host node to run the server on.
 * @param web_root Optional content to serve (NULL for default page).
 * @return MAGI_OK on success, otherwise an error code.
 */
int http_server_start(Node* node, const char* web_root);

/**
 * @brief Stop the HTTP server on a host node.
 *
 * Closes the listening socket and unbinds port 80.
 *
 * @param node Host node running the server.
 * @return MAGI_OK on success, otherwise an error code.
 */
int http_server_stop(Node* node);

/**
 * @brief Perform an HTTP GET request from a host node.
 *
 * Parses the URL, performs DNS resolution if needed, connects via TCP,
 * sends a GET request, receives the response, and logs the result.
 *
 * @param node Host node to send from.
 * @param url  Target URL (e.g., "http://10.0.0.1/index.html" or "10.0.0.1/path").
 * @return MAGI_OK on success, otherwise an error code.
 */
int http_get(Node* node, const char* url);

#endif /* MAGI_LAYER7_HTTP_H */
