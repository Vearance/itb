/**
 * @file json_loader.h
 * @brief Topology JSON serialization and deserialization helpers.
 */

#ifndef MAGI_TOPOLOGY_JSON_LOADER_H
#define MAGI_TOPOLOGY_JSON_LOADER_H

#include "topology.h"

/**
 * @brief Load topology state from a JSON file.
 *
 * @param topology Destination topology object.
 * @param path Input file path, or NULL to use topology->source_path.
 * @return MAGI_OK on success, otherwise an error code.
 */
int topology_load_file(Topology* topology, const char* path);

/**
 * @brief Save topology state to a JSON file.
 *
 * @param topology Source topology object.
 * @param path Output file path, or NULL to use topology->source_path.
 * @return MAGI_OK on success, otherwise an error code.
 */
int topology_save_file(const Topology* topology, const char* path);

#endif