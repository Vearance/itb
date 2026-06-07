#define _POSIX_C_SOURCE 200809L

#include "cli.h"

#include "commands.h"

#include "utils/log.h"
#include "utils/magi_error.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Run the interactive MAGI shell loop.
 *
 * Reads input line-by-line from stdin, strips trailing newline/carriage-return,
 * tokenizes the line on whitespace, and dispatches to commands_dispatch().
 * The loop continues until commands_dispatch() returns CLI_EXIT_REQUEST
 * (triggered by "exit" or "quit") or EOF is received on stdin.
 *
 * @param topology Mutable topology context used by all commands.
 * @return MAGI_OK on graceful exit, MAGI_ERR_BADARGS if topology is NULL.
 */
int cli_run(Topology* topology) {
  if (topology == NULL) {
    return MAGI_ERR_BADARGS;
  }

  char line[1024];

  for (;;) {
    fputs("TFAR> ", stdout);
    fflush(stdout);

    if (fgets(line, sizeof(line), stdin) == NULL) {
      fputc('\n', stdout);
      break;
    }

    size_t len = strlen(line);
    while (len > 0U && (line[len - 1U] == '\n' || line[len - 1U] == '\r')) {
      line[--len] = '\0';
    }

    char* argv[32] = {0};
    int argc = 0;
    char* token = strtok(line, " \t");

    while (token != NULL && argc < (int)(sizeof(argv) / sizeof(argv[0]))) {
      argv[argc++] = token;
      token = strtok(NULL, " \t");
    }

    if (argc == 0) {
      continue;
    }

    int status = commands_dispatch(topology, argc, argv);
    if (status == CLI_EXIT_REQUEST) {
      break;
    }
  }

  return MAGI_OK;
}