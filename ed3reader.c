// file: ed3reader.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:38:54 +0100
// Last modified: 2026-03-13T22:18:28+0100

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

static void print_info(Header *header, FILE *outfile);

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
  //print_info(&header, stderr);
  FILE *outfile = stdout;
  if (opt.outfile) {
    outfile = fopen(opt.outfile, "w");
    if (outfile==0) {
      error("could not open \"%s\" for writing.", opt.outfile);
      return EXIT_FAILURE;
    }
  }
  if (outfile != stdout) {
    print_info(&header, outfile);
  } else {
    print_info(&header, stderr);
  }
  float divisor = powf(10.0, header.comma_shift);
  // Read data blocks.
  int32_t total_count = 0;
  DataBlock block = read_data_block(contents, &permanent);
  while (block.ok && total_count < header.data_count) {
    int32_t block_samples = block.count/header.channel_count;
    total_count += block_samples;
    fprintf(stderr, "# Read %d samples from block %d (total %d)\n",
            block_samples, block.index, total_count);
    //for (int32_t j = 0; j < block_samples; j++) {
    //  fprintf(stderr, "Data.b16[%d] = %.1f\n", j, block.b16[j]/divisor);
    //}
    for (int32_t k = 0; k < block.count; k++) {
      fprintf(outfile,"%.1f ", block.b16[k]/divisor);
      if ((k+1) % header.channel_count == 0) {
        fputs("\n", outfile);
      }
    }
    block = read_data_block(block.tail, &permanent);
  }
  debug("ending ed3reader normally...");
  fclose(outfile);
  return 0;
}

void print_info(Header *header, FILE *outfile)
{
  fprintf(outfile, "# Name: %s\n", sv8cstring(header->name));
  fprintf(outfile, "# Serial number: %s\n", sv8cstring(header->serial));
  fprintf(outfile, "# Device Id: %s\n", sv8cstring(header->device_id));
  fprintf(outfile, "# Firmware version: %s\n", sv8cstring(header->firmware_version));
  fprintf(outfile, "# Battery Capacity: %d\n", header->battery_capacity);
  fprintf(outfile, "# Last calibration: %s\n", sv8cstring(header->last_calibration));
  fprintf(outfile, "# Channel count: %d\n", header->channel_count);
  fprintf(outfile, "# Data count: %d samples\n", header->data_count);
  fprintf(outfile, "# Bits per sample: %d\n", header->bits);
  fprintf(outfile, "# Comma is shifted %d positions to the left.\n", header->comma_shift);
  fprintf(outfile, "# Measurement interval %d %s.\n",
          header->interval, sv8cstring(header->interval_units));
  fprintf(outfile, "# Start date: %s\n", sv8cstring(header->date_start));
}

