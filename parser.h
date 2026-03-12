// file: parser.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:58:51 +0100
// Last modified: 2026-03-12T07:59:32+0100

#pragma once

#include <stdint.h>
#include "arena.h"
#include "stringview.h"

typedef struct {
  Sv8 name;
  int32_t index;
  int32_t channel_type;
  int32_t count;
  int32_t status;
  int32_t type;
  int32_t coding_type;
  int32_t timeformat;
  int32_t unit;
  int32_t bits;
  int32_t comma_shift;
  int32_t interval;
  Sv8 date_start;
} Channel;

typedef struct {
  Sv8 name;
  Sv8 device_id;
  Sv8 firmware_version;
  int32_t battery_capacity;
  int32_t num_channels;
  Channel *channels;
  bool ok;
} Header;

typedef struct {
  Sv8 key;
  Sv8 value;
  Sv8 tail;
  bool ok;
} ContentElement;

typedef struct {
  int32_t index;
  int32_t count;
  uint16_t *b16;
  Sv8 tail;
  bool ok;
} Data;

#ifdef __cplusplus
extern "C" {
#endif

extern Sv8 read_file(char *path, Arena *permanent);
extern Header read_header(Sv8 contents);
extern ContentElement read_content_element(Sv8 contents, Sv8 name);
extern Data read_data(Sv8 contents, int32_t bits, Arena *permanent);

#ifdef __cplusplus
}
#endif
