#define _POSIX_C_SOURCE 200809L

#include "cli/cli.h"
#include "cli/node_ops.h"
#include "topology/json_loader.h"
#include "utils/log.h"
#include "utils/magi_error.h"

#include <stdio.h>

/**
 * @brief Program entry point.
 *
 * Initialises the topology, sets the concrete CLI node operations,
 * loads the topology from the default JSON file (or continues with an
 * empty topology on failure), enters the CLI event loop, and cleans up
 * before exiting.
 *
 * @return 0 on success, 1 on initialisation or runtime failure.
 */
int main(void) {
  Topology* topology = topology_new();
  if (topology == NULL) {
    LOG("MAGI", "Failed to initialize topology");
    return 1;
  }
  topology_set_node_ops(topology, cli_topology_node_ops());

  if (topology_load_file(topology, topology->source_path) != MAGI_OK) {
    LOG("MAGI", "Starting with an empty topology");
  }

  int status = cli_run(topology);
  topology_free(topology);
  return status == MAGI_OK ? 0 : 1;
}
