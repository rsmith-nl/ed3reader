// file: ed3reader.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:38:54 +0100
// Last modified: 2026-03-10T21:33:25+0100


#include "arena.h"
#include "logging.h"
#include "parser.h"
#include "setup.h"

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  Options opt = setup(argc, argv);
  debug("starting ed3reader...");
  Arena permanent = arena_create(32*1024*1024);
  Sv8 contents = read_file(opt.infile, &permanent);
  info("input file %s has a size of %d bytes", opt.infile, contents.len);
  debug("ending ed3reader normally...");
  return 0;
}
