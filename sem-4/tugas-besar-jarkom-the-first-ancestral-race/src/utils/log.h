/**
 * @file log.h
 * @brief Lightweight logging macros for stdout/stderr.
 */

#ifndef MAGI_UTILS_LOG_H
#define MAGI_UTILS_LOG_H

#include <stdio.h>

/**
 * @brief Emit an informational log line to stdout.
 *
 * Output format is [node] message.
 */
#define LOG(node_name, fmt, ...)                                                                   \
  do {                                                                                             \
    const char* magi_log_name = (node_name) != NULL ? (node_name) : "?";                           \
    fprintf(stdout, "[%s] " fmt "\n", magi_log_name __VA_OPT__(, ) __VA_ARGS__);                   \
    fflush(stdout);                                                                                \
  } while (0)

/**
 * @brief Emit a warning log line to stderr.
 *
 * Output format is [node][WARN] message.
 */
#define LOG_WARN(node_name, fmt, ...)                                                              \
  do {                                                                                             \
    const char* magi_log_name = (node_name) != NULL ? (node_name) : "?";                           \
    fprintf(stderr, "[%s][WARN] " fmt "\n", magi_log_name __VA_OPT__(, ) __VA_ARGS__);             \
    fflush(stderr);                                                                                \
  } while (0)

#endif