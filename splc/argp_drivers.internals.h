#ifndef ARGP_DRIVERS_INTERNALS_H
#define ARGP_DRIVERS_INTERNALS_H

#include <errno.h>
#include <limits.h>
#include <string.h>

#include "config.h"
#include "common.h"
#include "wrappers.h"
#include "argp_drivers.h"

static opt_driver_t drive_help;
static opt_driver_t drive_debug;
static opt_driver_t drive_kawaii;
static opt_driver_t drive_version;
static opt_driver_t drive_dump;
static opt_driver_t drive_optimize;
static opt_driver_t drive_direct;
static opt_driver_t drive_verbose;
static opt_driver_t drive_dryrun;
static opt_driver_t drive_job;
static opt_driver_t drive_output;
static opt_driver_t drive_standard;
static opt_driver_t drive_language;
static opt_driver_t drive_target;
static opt_driver_t drive_asmbly;
static opt_driver_t drive_disasm;
static opt_driver_t drive_pedantic;
static opt_driver_t drive_warning;

static opt_errhandler_t errhandle_unrecognized;

/*==================*
 | STATIC VARIABLES |
 *==================*/
optmd_t optmds[] = {
   /* Note: .seen be initialized with 0 */
   /* Note: [0] must be "help" and [1] "version" */
   {
      .sname = 'h',
      .lname = "help",
      .drive = drive_help,
      .reqv = false,
      .help = "prints the manual page",
      .unique = true
   },
   {
      .sname = 'v',
      .lname = "version",
      .drive = drive_version,
      .reqv = false,
      .help = "prints the current version",
      .unique = false
   },
   {
      .sname = 'g',
      .lname = "debug",
      .drive = drive_debug,
      .reqv = false,
      .help = "emits debug symbols",
      .unique = true
   },
   {
      .sname = 'k',
      .lname = "kawaii",
      .drive = drive_kawaii,
      .reqv = false,
      .help = "uses a kawaii tone",
      .unique = true
   },
   {
      .sname = 'S',
      .lname = "dump",
      .drive = drive_dump,
      .reqv = false,
      .help = "dumps intermediate representation",
      .unique = true
   },
   {
      .sname = 'O',
      .lname = "optimize",
      .drive = drive_optimize,
      .reqv = false,
      .help = "optimizes intermediate representation",
      .unique = true
   },
   {
      .sname = 'd',
      .lname = "direct",
      .drive = drive_direct,
      .reqv = false,
      .help = "generates .splo directly from AST",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "verbose",
      .drive = drive_verbose,
      .reqv = false,
      .help = "prints full program message",
      .unique = true
   },
   {
      .sname = 'n',
      .lname = "dry-run",
      .drive = drive_dryrun,
      .reqv = false,
      .help = "doesn't produce actual files",
      .unique = true
   },
   {
      .sname = 'j',
      .lname = "job",
      .drive = drive_job,
      .reqv = true,
      .help = "uses multi-thread",
      .unique = true
   },
   {
      .sname = 'o',
      .lname = "output",
      .drive = drive_output,
      .reqv = true,
      .help = "specifies the name of the output file",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "std",
      .drive = drive_standard,
      .reqv = true,
      .help = "specifies the language spec to use",
      .unique = true,
   },
   {
      .sname = '\0',
      .lname = "lang",
      .drive = drive_language,
      .reqv = true,
      .help = "specifies the system language to use",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "target",
      .drive = drive_target,
      .reqv = true,
      .help = "specifies the target langauge for transpiling",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "asm",
      .drive = drive_asmbly,
      .reqv = false,
      .help = "generates output from .splasm input file",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "disasm",
      .drive = drive_disasm,
      .reqv = false,
      .help = "generates .splasm from .splo",
      .unique = true
   },
   {
      .sname = '\0',
      .lname = "pedantic",
      .drive = drive_pedantic,
      .reqv = false,
      .help = "checks strictly. enables all -W flags",
      .unique = true
   },
   {
      .sname = 'W',
      .lname = "warn",
      .drive = drive_warning,
      .reqv = true,
      .help = "issues warnings for the specified criterion",
      .unique = false
   }
};
const size_t optmds_len = ARRLEN(optmds);

#endif
