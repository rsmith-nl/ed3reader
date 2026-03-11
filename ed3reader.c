// file: ed3reader.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:38:54 +0100
// Last modified: 2026-03-11T21:22:18+0100


#include "arena.h"
#include "logging.h"
#include "parser.h"
#include "setup.h"
#include "stringview.h"

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
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
  // Get info from the header
  ContentElement headerinfo = read_content_element(contents, SV8("Name"));
  if (headerinfo.ok) {
    fprintf(stderr, "Name: %s\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("SerialNumber"));
  if (headerinfo.ok) {
    fprintf(stderr, "Serial number: %s\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("DeviceId"));
  if (headerinfo.ok) {
    fprintf(stderr, "Device Id: %s\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("LastCalibration"));
  if (headerinfo.ok) {
    fprintf(stderr, "Last calibration: %s\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("ChannelCount"));
  if (headerinfo.ok) {
    fprintf(stderr, "Channel count: %s\n", sv8cstring(headerinfo.value));
  }
  headerinfo = read_content_element(contents, SV8("ChannelType"));
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
  debug("ending ed3reader normally...");
  return 0;
}
