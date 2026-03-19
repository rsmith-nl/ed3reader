// file: setup.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-02-22 15:24:00 +0100
// Last modified: 2026-03-19T19:32:48+0100

#pragma once

#include <stdbool.h>

typedef struct {
  bool verbose;
  bool csv;
  int argc;
  char *infile, *outfile;
  char **argv;
} Options;

#ifdef __cplusplus
extern "C" {
#endif

extern Options setup(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

