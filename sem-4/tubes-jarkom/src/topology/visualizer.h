/**
 * @file visualizer.h
 * @brief File-based topology visualization exporters.
 */

#ifndef MAGI_TOPOLOGY_VISUALIZER_H
#define MAGI_TOPOLOGY_VISUALIZER_H

#include "topology.h"

/**
 * @brief Write the topology as an SVG or DOT graph.
 *
 * Supported output extensions are ".svg" and ".dot". When filename is NULL
 * or empty, "topology.svg" is used.
 *
 * @param topology Topology instance to export.
 * @param filename Output path, or NULL for the default SVG path.
 * @return MAGI_OK on success, otherwise an error code.
 */
int topology_visualize(const Topology* topology, const char* filename);

#endif
