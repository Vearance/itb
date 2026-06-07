/**
 * @file cli.h
 * @brief Interactive command loop entry points.
 */

#ifndef MAGI_CLI_CLI_H
#define MAGI_CLI_CLI_H

#include "topology/topology.h"

/**
 * @brief Run the interactive MAGI shell loop.
 *
 * @param topology Mutable topology context used by commands.
 * @return MAGI_OK on graceful exit, or an error code.
 */
int cli_run(Topology* topology);

#endif