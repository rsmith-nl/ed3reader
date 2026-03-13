// file: ed3reader.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:38:54 +0100
// Last modified: 2026-03-13T14:31:03+0100

#include "arena.h"
#include "logging.h"
#include "parser.h"
#include "setup.h"
#include "stringview.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  Options opt = setup(argc, argv);
  debug("starting ed3reader...");
  Arena permanent = arena_create(32*1024*1024);
  Sv8 contents = read_file(opt.infile, &permanent);
  info("input file %s has a size of %d bytes", opt.infile, contents.len);
  Header header = read_header(contents);
  if (!header.ok) {
    fprintf(stderr, "Failed to read header");
    return EXIT_FAILURE;
  }
  fprintf(stderr, "# Name: %s\n", sv8cstring(header.name));
  fprintf(stderr, "# Serial number: %s\n", sv8cstring(header.serial));
  fprintf(stderr, "# Device Id: %s\n", sv8cstring(header.device_id));
  fprintf(stderr, "# Firmware version: %s\n", sv8cstring(header.firmware_version));
  fprintf(stderr, "# Battery Capacity: %d\n", header.battery_capacity);
  fprintf(stderr, "# Last calibration: %s\n", sv8cstring(header.last_calibration));
  fprintf(stderr, "# Channel count: %d\n", header.channel_count);
  fprintf(stderr, "# Data count: %d samples\n", header.data_count);
  fprintf(stderr, "# Bits per sample: %d\n", header.bits);
  fprintf(stderr, "# Comma is shifted %d positions to the left.\n", header.comma_shift);
  fprintf(stderr, "# Measurement interval %d %s.\n",
          header.interval, sv8cstring(header.interval_units));
  fprintf(stderr, "# Start date: %s\n", sv8cstring(header.date_start));
  float divisor = powf(10.0, header.comma_shift);
  // Read first data block.
  DataBlock block = read_data_block(contents, &permanent);
  if (block.ok) {
    fprintf(stderr, "read %d values from block %d\n", block.count, block.index);
    for (int32_t j = 0; j < 10; j++) {
      fprintf(stderr, "Data.b16[%d] = %.1f\n", j, block.b16[j]/divisor);
    }
  } else {
    fprintf(stderr, "Failed to read first data block\n");
  }
  debug("ending ed3reader normally...");
  return 0;
}
