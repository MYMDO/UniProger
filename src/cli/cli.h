/**
 * @file cli.h
 * @brief CLI shell interface
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UNIPROGER_CLI_H
#define UNIPROGER_CLI_H

#include "uniproger/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** CLI command handler signature */
typedef up_status_t (*cli_cmd_handler_t)(int argc, char *argv[]);

/** CLI command definition */
typedef struct {
    const char        *name;
    const char        *help;
    const char        *usage;
    cli_cmd_handler_t  handler;
} cli_command_t;

/** Initialize CLI subsystem */
up_status_t cli_init(void);

/** Register a command */
up_status_t cli_register(const cli_command_t *cmd);

/** Run CLI main loop (blocking) */
void cli_run(void);

/** Process a single line */
up_status_t cli_process_line(const char *line);

/** Print formatted output */
void cli_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/** Print prompt */
void cli_print_prompt(void);

#ifdef __cplusplus
}
#endif

#endif /* UNIPROGER_CLI_H */
