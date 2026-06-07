/**
 * @file node_ops.h
 * @brief Concrete topology node hooks registered by the CLI integrator.
 */

#ifndef MAGI_CLI_NODE_OPS_H
#define MAGI_CLI_NODE_OPS_H

#include "topology/topology.h"

/**
 * @brief Return the concrete host/switch/router hooks for topology registries.
 *
 * @return Stable callback table owned by this module.
 */
const TopologyNodeOps* cli_topology_node_ops(void);

#endif
