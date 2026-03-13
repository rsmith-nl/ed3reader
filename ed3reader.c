// file: ed3reader.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:38:54 +0100
// Last modified: 2026-03-13T01:51:22+0100


#include "arena.h"
#include "logging.h"
#include "parser.h"
#include "setup.h"
#include "stringview.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
//#include <stdio.h>
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
  Header  header = read_header(contents);
  if (!header.ok) {
    fprintf(stderr, "Failed to read header");
    return EXIT_FAILURE;
  }
  fprintf(stderr, "Name: %s\n", sv8cstring(header.name));
  fprintf(stderr, "Serial number: %s\n", sv8cstring(header.serial));
  fprintf(stderr, "Device Id: %s\n", sv8cstring(header.device_id));
  fprintf(stderr, "Firmware version: %s\n", sv8cstring(header.firmware_version));
  fprintf(stderr, "Battery Capacity %d\n", header.battery_capacity);
  fprintf(stderr, "Last calibration: %s\n", sv8cstring(header.last_calibration));
  fprintf(stderr, "Channel count %d\n", header.channel_count);

  // Get info from the channels
  ContentElement headerinfo = read_content_element(contents, SV8("ChannelType"));
  if (headerinfo.ok) {
    fprintf(stderr, "Channel type: %s\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("DataCount"));
  if (headerinfo.ok) {
    fprintf(stderr, "Data count: %s\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("Type"));
  if (headerinfo.ok) {
    fprintf(stderr, "Data type: %s\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("TimeFormat"));
  if (headerinfo.ok) {
    fprintf(stderr, "Time format: %s\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("Unit"));
  if (headerinfo.ok) {
    fprintf(stderr, "Unit: %s\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("NoBits"));
  if (headerinfo.ok) {
    fprintf(stderr, "Bits per datapoint: %s\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("CommaShift"));
  if (headerinfo.ok) {
    fprintf(stderr, "Comma is shifted %s positions to the left.\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("Interval"));
  if (headerinfo.ok) {
    //fprintf(stderr, "Interval %s\n", sv8cstring(headerinfo.value));
    Sv8Int intval = sv8toi(headerinfo.value);
    if (intval.ok) {
      uint32_t count = (uint32_t)intval.result & 0xfff;
      uint32_t unit = ((uint32_t)intval.result & 0xf000)>>12;
      char *units = 0;
      switch (unit) {
        case 8:
          units = "minutes";
          break;
        case 4:
          units = "seconds";
          break;
        default:
          units = "unknown";
      }
      fprintf(stderr, "Interval: %d %s\n", count, units);
    }
  }
  headerinfo = read_content_element(contents, SV8("DateStart"));
  if (headerinfo.ok) {
    fprintf(stderr, "Starting date: %s\n", sv8cstring(headerinfo.value));
  }
  ContentElement data = read_content_element(contents, SV8("CodedData"));
  if (data.ok) {
    fprintf(stderr, "Read data element. Length %ld bytes\n", data.value.len);
  }
  Data block = read_data(contents, &permanent);
  if (block.ok) {
    fprintf(stderr, "read %d values from block %d\n", block.count, block.index);
    for (int32_t j = 0; j < 10; j++) {
      fprintf(stderr, "Data.b16[%d] = %u\n", j, block.b16[j]);
    }
  }
  debug("ending ed3reader normally...");
  return 0;
}
