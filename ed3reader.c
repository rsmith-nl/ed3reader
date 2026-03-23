// file: ed3reader.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:38:54 +0100
// Last modified: 2026-03-23T22:56:08+0100

#include "arena.h"
#include "logging.h"
#include "parser.h"
#include "setup.h"
#include "stringview.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <io.h> // for _setmode
// instead of including windows.h....
extern int __stdcall SetConsoleOutputCP(unsigned int);
#endif

static void print_info(Header *header, FILE *outfile);
static void print_info_csv(Header *header, FILE *outfile);
static char *fmttime(time_t t);
static char *fmttime_csv(time_t t);

int main(int argc, char *argv[])
{
#ifdef _WIN32
  _setmode(0, 0x8000);
  _setmode(1, 0x8000);
  _setmode(2, 0x8000);
  SetConsoleOutputCP(65001);
#endif
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
  FILE *outfile = stdout;
  if (opt.outfile) {
    outfile = fopen(opt.outfile, "w");
    if (outfile==0) {
      error("could not open \"%s\" for writing.", opt.outfile);
      return EXIT_FAILURE;
    }
  }
  fprintf(stderr, "# File path: %s\n", opt.infile);
  float divisor = powf(10.0, header.comma_shift);
  // Read all data blocks.
  int32_t total_values = 0;
  DataBlock block = read_data_block(contents, &permanent);
  int16_t *data = block.b16;
  while (block.ok && total_values < header.samples_count*header.channel_count) {
    int32_t block_samples = block.count/header.channel_count;
    total_values += block_samples;
    block = read_data_block(block.tail, &permanent);
  }
  if (total_values > header.samples_count) {
    total_values = header.samples_count*header.channel_count;
  }
  if (opt.csv) {
    // Print CSV header
    print_info_csv(&header, outfile);
    fputs("Excel datevalue", outfile);
    for (int32_t j = 1; j <= header.channel_count; j++) {
      fprintf(outfile, ",ch%1d", j);
    }
    fputs("\n", outfile);
    // Print the data.
    time_t current = header.start;
    int32_t count = 0;
    while (count < total_values) {
      fputs(fmttime_csv(current), outfile);
      for (int32_t j = 0; j < header.channel_count; j++) {
        if (data[count]==32766) {
          fputs(",NaN", outfile);
        } else {
          fprintf(outfile, ",%.1f", data[count]/divisor);
        }
        count++;
      }
      fputs("\n", outfile);
      current += header.seconds;
    }
  } else {
    // Print gnuplot header
    print_info(&header, outfile);
    fputs("# ISO8601 datetime", outfile);
    for (int32_t j = 1; j <= header.channel_count; j++) {
      fprintf(outfile, "\tch%1d", j);
    }
    fputs("\n", outfile);
    // Print the data.
    time_t current = header.start;
    int32_t count = 0;
    while (count < total_values) {
      fputs(fmttime(current), outfile);
      for (int32_t j = 0; j < header.channel_count; j++) {
        if (data[count]==32766) {
          fputs(" NaN", outfile);
        } else {
          fprintf(outfile, "\t%.1f", data[count]/divisor);
        }
        count++;
      }
      fputs("\n", outfile);
      current += header.seconds;
    }
  }
  debug("ending ed3reader normally...");
  fclose(outfile);
  return 0;
}

static char *fmttime(time_t t)
{
  static char buf[64];
  memset(buf, 0, 64);
  struct tm *tv = gmtime(&t);
  strftime(buf, 63, "%Y-%m-%dT%H:%M:%S", tv);
  return buf;
}

static char *fmttime_csv(time_t t)
{
  static char buf[64];
  memset(buf, 0, 64);
  double exceldays = (double)t/86400.0;
  exceldays += 25569.0; // days between excel epoch and UNIX epoch.
  snprintf(buf, 63, "%.6f", exceldays);
  return buf;
}

static void print_info(Header *header, FILE *outfile)
{
  fprintf(outfile, "# Device type: %s\n", sv8cstring(header->name));
  fprintf(outfile, "# Serial number: %s\n", sv8cstring(header->serial));
  fprintf(outfile, "# Device Id: %s\n", sv8cstring(header->device_id));
  fprintf(outfile, "# Firmware version: %s\n", sv8cstring(header->firmware_version));
  fprintf(outfile, "# Battery Capacity: %d\n", header->battery_capacity);
  fprintf(outfile, "# Last calibration: %s\n", sv8cstring(header->last_calibration));
  fprintf(outfile, "# Channel count: %d\n", header->channel_count);
  fprintf(outfile, "# Data count: %d samples\n", header->samples_count);
  fprintf(outfile, "# Temperature unit: %s\n", sv8cstring(header->unit));
  fprintf(outfile, "# Bits per sample: %d\n", header->bits);
  fprintf(outfile, "# Comma shift %d positions to the left.\n", header->comma_shift);
  fprintf(outfile, "# Measurement interval %d %s.\n",
          header->interval, sv8cstring(header->interval_units));
  fprintf(outfile, "# Start date: %s\n", fmttime(header->start));
}

static void print_info_csv(Header *header, FILE *outfile)
{
  fprintf(outfile, "Device type,%s\n", sv8cstring(header->name));
  fprintf(outfile, "Serial number,%s\n", sv8cstring(header->serial));
  fprintf(outfile, "Device Id,%s\n", sv8cstring(header->device_id));
  fprintf(outfile, "Firmware version,%s\n", sv8cstring(header->firmware_version));
  fprintf(outfile, "Battery Capacity,%d\n", header->battery_capacity);
  fprintf(outfile, "Last calibration,%s\n", sv8cstring(header->last_calibration));
  fprintf(outfile, "Channel count,%d\n", header->channel_count);
  fprintf(outfile, "Data count,%d samples\n", header->samples_count);
  fprintf(outfile, "Temperature unit,%s\n", sv8cstring(header->unit));
  fprintf(outfile, "Bits per sample,%d\n", header->bits);
  fprintf(outfile, "Comma shift,%d positions to the left\n", header->comma_shift);
  fprintf(outfile, "Measurement interval,%d %s\n",
          header->interval, sv8cstring(header->interval_units));
  fprintf(outfile, "Start date,%s\n", fmttime(header->start));
}
