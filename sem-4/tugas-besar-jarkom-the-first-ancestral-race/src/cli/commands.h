/**
 * @file commands.h
 * @brief CLI command dispatchers and command handlers.
 */

#ifndef MAGI_CLI_COMMANDS_H
#define MAGI_CLI_COMMANDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "topology/topology.h"

/** Continue CLI loop after command execution. */
#define CLI_CONTINUE 0
/** Exit CLI loop after command execution. */
#define CLI_EXIT_REQUEST 1

/**
 * @brief Dispatch one tokenized CLI command line.
 *
 * @param topology Mutable topology context.
 * @param argc Number of CLI tokens.
 * @param argv Token array.
 * @return MAGI_OK, CLI_EXIT_REQUEST, or an error code.
 */
int commands_dispatch(Topology* topology, int argc, char** argv);

/**
 * @brief Print CLI command usage summary.
 */
void commands_print_help(void);

/**
 * @brief Create a node in the topology.
 *
 * @param topology Mutable topology context.
 * @param type Node type text (host/switch/router).
 * @param name Node identifier.
 * @return MAGI_OK on success, otherwise an error code.
 */
int cmd_create(Topology* topology, const char* type, const char* name);

/**
 * @brief Create a link between two endpoints.
 *
 * @param topology Mutable topology context.
 * @param dev1 Endpoint spec (name or name:port).
 * @param dev2 Endpoint spec (name or name:port).
 * @param delay_ms Link delay in milliseconds.
 * @param mtu Link MTU in bytes.
 * @return MAGI_OK on success, otherwise an error code.
 */
int cmd_link(Topology* topology, const char* dev1, const char* dev2, uint32_t delay_ms,
             uint16_t mtu);

/**
 * @brief Remove an existing link between two endpoints.
 *
 * @param topology Mutable topology context.
 * @param dev1 Endpoint spec (name or name:port).
 * @param dev2 Endpoint spec (name or name:port).
 * @return MAGI_OK on success, otherwise an error code.
 */
int cmd_unlink(Topology* topology, const char* dev1, const char* dev2);

/**
 * @brief Print the current topology summary.
 *
 * @param topology Topology context.
 * @return MAGI_OK.
 */
int cmd_topology(Topology* topology);

/**
 * @brief Save topology JSON to disk.
 *
 * @param topology Topology context.
 * @param filename Output path, or NULL to use current source path.
 * @return MAGI_OK on success, otherwise an error code.
 */
int cmd_save(Topology* topology, const char* filename);

/**
 * @brief Load topology JSON from disk.
 *
 * @param topology Mutable topology context.
 * @param filename Input path, or NULL to use current source path.
 * @return MAGI_OK on success, otherwise an error code.
 */
int cmd_load(Topology* topology, const char* filename);

/**
 * @brief Request clean CLI shutdown.
 *
 * @return CLI_EXIT_REQUEST.
 */
int cmd_exit(void);

#endif