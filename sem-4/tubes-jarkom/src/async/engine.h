/**
 * @file engine.h
 * @brief Async packet delivery engine.
 */

#ifndef MAGI_ASYNC_ENGINE_H
#define MAGI_ASYNC_ENGINE_H

struct Topology;

/**
 * @brief Start async workers for all nodes in a topology.
 *
 * In sequential builds this is a no-op so top-level code can call it
 * unconditionally.
 *
 * @param topology Topology whose nodes should receive worker threads.
 * @return MAGI_OK on success, otherwise an error code.
 */
int engine_init(struct Topology* topology);

/**
 * @brief Stop all running async workers and timer threads.
 *
 * In sequential builds this is a no-op.
 */
void engine_shutdown(void);

#endif
