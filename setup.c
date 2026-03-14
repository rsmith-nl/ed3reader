// file: setup.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:41:17 +0100
// Last modified: 2026-03-14T23:10:37+0100

#include "setup.h"
#include "logging.h"
#include "version.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <getopt.h>

char *name = "ed3reader-c";

const char license[] =
  "This is free and unencumbered software released into the public domain.\n\n"
  "Anyone is free to copy, modify, publish, use, compile, sell, or\n"
  "distribute this software, either in source code form or as a compiled\n"
  "binary, for any purpose, commercial or non-commercial, and by any\n"
  "means.\n\n"
  "In jurisdictions that recognize copyright laws, the author or authors\n"
  "of this software dedicate any and all copyright interest in the\n"
  "software to the public domain. We make this dedication for the benefit\n"
  "of the public at large and to the detriment of our heirs and\n"
  "successors. We intend this dedication to be an overt act of\n"
  "relinquishment in perpetuity of all present and future rights to this\n"
  "software under copyright law.\n\n"
  "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
  "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
  "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n"
  "IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR\n"
  "OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,\n"
  "ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR\n"
  "OTHER DEALINGS IN THE SOFTWARE.\n\n"
  "For more information, please refer to <http://unlicense.org>\n\n";


const char help[] =
  "usage: ed3reader [-h] [-v] [-l] [--log=(debug|info|warn|error|crit)] infile [outfile]\n"
  "\n"
  "Program for converting ed3 files from an EBI 40 temperature logger to plain text.\n"
  "\n"
  "positional argument: \n"
  "  infile  -- input file in ed3 format.\n"
  "  outfile -- output file name. If this is not given output goes to stdout.\n"
  "\n"
  "options:\n"
  "  -h, --help            show this help message and exit\n"
  "  -v, --version         show program's version number and exit\n"
  "  -l, --license         print the license\n"
  "  --log                 logging level debug,info,(default) warn,error,crit\n\n";


Options setup(int argc, char *argv[])
{
  Options rv = {0};
  int32_t choice;
  static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'v'},
    {"license", no_argument, 0, 'l'},
    {"log", required_argument, 0, 1000},
    {0,0,0,0}
  };
  logging_configure(name, LOG_WARNING);
  while (1) {
    int32_t option_index = 0;
    choice = getopt_long(argc, argv, "hlv", long_options, &option_index);
    if (choice == -1) {
      break;
    }
    switch (choice) {
      case 'h':
        printf("%s version: %s\n", name, VERSION);
        printf(help);
        exit(0);
        break;
      case 'l':
        printf("%s version: %s\n", name, VERSION);
        printf(license);
        exit(0);
        break;
      case 'v':
        printf("%s version: %s\n", name, VERSION);
        exit(0);
        break;
      case 1000:
        if (strcasecmp(optarg, "debug")==0) {
          logging_configure(0, LOG_DEBUG);
        } else if (strcasecmp(optarg, "info")==0) {
          logging_configure(0, LOG_INFO);
        } else if (strcasecmp(optarg, "error")==0) {
          logging_configure(0, LOG_ERROR);
        } else if (strcasecmp(optarg, "crit")==0) {
          logging_configure(0, LOG_CRITICAL);
        } else {
          warning("unknown logging level \"%s\" ignored", optarg);
        }
        break;
    }
  }
  // Show all arguments when LOG_DEBUG is set.
  debug("argc = %d", argc);
  for (int32_t j = 0; j < argc; j++) {
    debug("argv[%d] = “%s”", j, argv[j]);
  }
  // Save updated values, skipping the executable name.
  rv.argc = argc - optind;
  rv.argv = argv + optind;
  if (rv.argc < 1) {
    warning("no input file given; exiting");
    exit(EXIT_FAILURE);
  }
  rv.infile = rv.argv[0];
  rv.argc--;
  rv.argv++;
  if (rv.argc > 0) {
    rv.outfile = rv.argv[0];
    rv.argc--;
    rv.argv++;
  }
  // Show remaining arguments when LOG_DEBUG is set.
  debug("remaining argc = %d", rv.argc);
  for (int32_t j = 0; j < rv.argc; j++) {
    debug("remaining argv[%d] = “%s”", j, rv.argv[j]);
  }
  return rv;
}
