/**
 * @file cli.c
 * @brief CLI shell — line editing, history, command dispatch
 *
 * SPDX-License-Identifier: MIT
 */

#include "cli.h"
#include "uniproger/config.h"
#include "uniproger/version.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* ── Command registry ────────────────────────────────────────────────── */

#define CLI_MAX_COMMANDS  32

static const cli_command_t *s_commands[CLI_MAX_COMMANDS];
static size_t s_cmd_count = 0;

/* ── History ─────────────────────────────────────────────────────────── */

static char s_history[UP_CLI_HISTORY_DEPTH][UP_CLI_LINE_SIZE];
static size_t s_hist_count = 0;
static size_t s_hist_idx = 0;

static void cli_history_add(const char *line)
{
    if (line[0] == '\0') return;
    if (s_hist_count > 0 &&
        strcmp(s_history[(s_hist_count - 1) % UP_CLI_HISTORY_DEPTH], line) == 0) {
        return; /* Skip duplicate */
    }
    strncpy(s_history[s_hist_count % UP_CLI_HISTORY_DEPTH], line, UP_CLI_LINE_SIZE - 1);
    s_hist_count++;
    s_hist_idx = s_hist_count;
}

/* ── Parsing ─────────────────────────────────────────────────────────── */

static int cli_parse_args(char *line, char *argv[], int max_args)
{
    int argc = 0;
    char *p = line;

    while (*p && argc < max_args) {
        /* Skip whitespace */
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        /* Handle quoted strings */
        if (*p == '"') {
            p++;
            argv[argc++] = p;
            while (*p && *p != '"') p++;
            if (*p) *p++ = '\0';
        } else {
            argv[argc++] = p;
            while (*p && *p != ' ' && *p != '\t') p++;
            if (*p) *p++ = '\0';
        }
    }
    return argc;
}

/* ── Built-in commands ───────────────────────────────────────────────── */

static up_status_t cmd_help(int argc, char *argv[])
{
    (void)argc; (void)argv;
    cli_printf("\n  Available commands:\n\n");
    for (size_t i = 0; i < s_cmd_count; i++) {
        cli_printf("  %-12s %s\n", s_commands[i]->name, s_commands[i]->help);
    }
    cli_printf("\n");
    return UP_OK;
}

static up_status_t cmd_version(int argc, char *argv[])
{
    (void)argc; (void)argv;
    cli_printf("\n  %s v%s\n", UNIPROGER_PROJECT_NAME, UNIPROGER_VERSION_STRING);
    cli_printf("  %s\n\n", UNIPROGER_PROJECT_DESC);
    return UP_OK;
}

static const cli_command_t builtin_commands[] = {
    { "help",    "Show available commands",   "help",    cmd_help },
    { "version", "Show firmware version",     "version", cmd_version },
    { "?",       "Show available commands",   "?",       cmd_help },
};

/* ── Public API ──────────────────────────────────────────────────────── */

up_status_t cli_init(void)
{
    s_cmd_count = 0;
    s_hist_count = 0;
    s_hist_idx = 0;

    /* Register built-in commands */
    for (size_t i = 0; i < sizeof(builtin_commands) / sizeof(builtin_commands[0]); i++) {
        cli_register(&builtin_commands[i]);
    }

    return UP_OK;
}

up_status_t cli_register(const cli_command_t *cmd)
{
    if (!cmd || s_cmd_count >= CLI_MAX_COMMANDS) return UP_ERROR_OVERFLOW;
    s_commands[s_cmd_count++] = cmd;
    return UP_OK;
}

up_status_t cli_process_line(const char *line)
{
    if (!line || line[0] == '\0') return UP_OK;

    char buf[UP_CLI_LINE_SIZE];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *argv[UP_CLI_MAX_ARGS];
    int argc = cli_parse_args(buf, argv, UP_CLI_MAX_ARGS);
    if (argc == 0) return UP_OK;

    /* Find command */
    for (size_t i = 0; i < s_cmd_count; i++) {
        if (strcmp(argv[0], s_commands[i]->name) == 0) {
            return s_commands[i]->handler(argc, argv);
        }
    }

    cli_printf("  Unknown command: '%s'. Type 'help' for list.\n", argv[0]);
    return UP_ERROR_NOT_FOUND;
}

void cli_run(void)
{
    char line[UP_CLI_LINE_SIZE];
    size_t pos = 0;

    cli_printf("\n");
    cli_printf("  ╔═══════════════════════════════════════════╗\n");
    cli_printf("  ║        UniProger v%s               ║\n", UNIPROGER_VERSION_STRING);
    cli_printf("  ║   Universal Programmer-Analyzer           ║\n");
    cli_printf("  ║   Type 'help' for available commands      ║\n");
    cli_printf("  ╚═══════════════════════════════════════════╝\n");
    cli_printf("\n");
    cli_print_prompt();

    while (1) {
        int c = getchar();
        if (c == EOF) continue;

        if (c == '\r' || c == '\n') {
            printf("\n");
            line[pos] = '\0';

            if (pos > 0) {
                cli_history_add(line);
                cli_process_line(line);
            }

            pos = 0;
            cli_print_prompt();
        } else if (c == '\b' || c == 127) {
            /* Backspace */
            if (pos > 0) {
                pos--;
                printf("\b \b");
            }
        } else if (c == 0x03) {
            /* Ctrl+C */
            printf("^C\n");
            pos = 0;
            cli_print_prompt();
        } else if (pos < UP_CLI_LINE_SIZE - 1) {
            line[pos++] = (char)c;
            putchar(c);
        }
    }
}

void cli_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void cli_print_prompt(void)
{
    printf("\033[36mup>\033[0m ");
}
