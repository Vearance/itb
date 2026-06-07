/**
 * @file magi_error.h
 * @brief Shared MAGI error codes and thread-local error variable.
 */

#ifndef MAGI_UTILS_MAGI_ERROR_H
#define MAGI_UTILS_MAGI_ERROR_H

/**
 * @brief Thread-local last error code.
 */
extern _Thread_local int magi_errno;

/** Operation succeeded. */
#define MAGI_OK 0
/** Memory allocation failed. */
#define MAGI_ERR_NOMEM -1
/** Route not found. */
#define MAGI_ERR_NOROUTE -2
/** Checksum validation failed. */
#define MAGI_ERR_BADCKSUM -3
/** TTL expired. */
#define MAGI_ERR_TTL -4
/** Interface has no attached link. */
#define MAGI_ERR_NOLINK -5
/** Port already in use. */
#define MAGI_ERR_PORTUSED -6
/** Connection was reset. */
#define MAGI_ERR_CONNRESET -7
/** Operation timed out. */
#define MAGI_ERR_TIMEOUT -8
/** Access denied by ACL. */
#define MAGI_ERR_ACL_DENY -9
/** Packet is fragmented or unsupported fragment form. */
#define MAGI_ERR_FRAGMENTED -10
/** Invalid arguments. */
#define MAGI_ERR_BADARGS -11

#endif