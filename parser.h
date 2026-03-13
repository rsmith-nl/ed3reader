// file: parser.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:58:51 +0100
// Last modified: 2026-03-13T14:05:23+0100

#pragma once

#include <stdint.h>
#include "arena.h"
#include "stringview.h"

typedef struct {
  Sv8 name;
  Sv8 serial;
  Sv8 device_id;
  Sv8 firmware_version;
  int32_t battery_capacity;
  Sv8 last_calibration;
  int32_t channel_count;
  // From the channels
  int32_t data_count;
  int32_t timeformat;
  int32_t bits;
  int32_t comma_shift;
  int32_t interval;
  Sv8 interval_units;
  Sv8 date_start;
  bool ok;
} Header;

typedef struct {
  int32_t index;
  int32_t count;
  uint16_t *b16;
  Sv8 tail;
  bool ok;
} DataBlock;

#ifdef __cplusplus
extern "C" {
#endif

extern Sv8 read_file(char *path, Arena *permanent);
extern Header read_header(Sv8 contents);
extern DataBlock read_data_block(Sv8 contents, Arena *permanent);


#ifdef __cplusplus
}
#endif
