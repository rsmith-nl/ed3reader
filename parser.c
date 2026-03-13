// file: parser.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:58:54 +0100
// Last modified: 2026-03-13T15:56:50+0100

#include "arena.h"
#include "logging.h"
#include "parser.h"
#include "stringview.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef struct {
  Sv8 key;
  Sv8 value;
  Sv8 tail;
  bool ok;
} ContentString;

typedef struct {
  Sv8 key;
  int32_t value;
  Sv8 tail;
  bool ok;
} ContentInt;


Sv8 read_file(char *path, Arena *permanent)
{
  assert(path!=0);
  assert(permanent!=0);
  Sv8 contents = {0};
  FILE *inputfile = fopen(path, "r");
  if (inputfile==0) {
    info("could not open file %s", path);
    return contents;
  }
  fseek(inputfile, 0L, SEEK_END);
  // Make space for extra newline.
  ptrdiff_t size = ftell(inputfile) + 1;
  rewind(inputfile);
  contents.data = arena_new(permanent, char, size);
  contents.len = size;
  ptrdiff_t rv = fread(contents.data, sizeof(char), size, inputfile);
  // Append extra newline.
  contents.data[rv++] = '\n';
  fclose(inputfile);
  if (rv != size) {
    info("file “%s” has size %td bytes, but only %td bytes read.", path, size, rv);
  }
  return contents;
}

// This function only reads element in the format <name>value</tag>.
ContentString read_content_string(Sv8 contents, Sv8 name)
{
  ContentString rv = {0};
  Sv8 current = contents;
  // Check for start of element.
  ptrdiff_t index = sv8find(contents, name);
  if (index==-1 || contents.data[index-1]!='<') {
    return rv;
  }
  current.data += index+name.len;
  current.len -= index+name.len;
  // The next character should be ' ' or '>'.
  if (current.data[0]!=' ' && current.data[0]!='>') {
    return rv;
  }
  ptrdiff_t index3 = sv8lindex(current, '>');
  if (index3!=-1) {
    current.data += index3+1;
    current.len -= index3+1;
  } else {
    return rv;
  }
  rv.tail = current;
  ptrdiff_t index2 = sv8find(current, name);
  if (index2==-1) {
    rv.tail.data = 0;
    rv.tail.len = 0;
    return rv;
  }
  // Go back to before "</"
  index2 -= 2;
  rv.key = name;
  rv.value = sv8span(current.data, current.data+index2);
  index2 = name.len + 4;
  rv.tail.data += index2;
  rv.tail.len -= index2;
  rv.ok = true;
  return rv;
}

ContentInt read_content_int(Sv8 contents, Sv8 name)
{
  ContentInt rv = {0};
  ContentString string = read_content_string(contents, name);
  if (!string.ok) {
    return rv;
  }
  Sv8Int bc = sv8toi(string.value);
  if (bc.ok) {
    rv.key = name;
    rv.value = bc.result;
    rv.tail = string.tail;
    rv.ok = true;
  }
  return rv;
}

Header read_header(Sv8 contents)
{
  Header rv = {0}, fail = {0};
  // Get info from the header
  ContentString stringinfo = read_content_string(contents, SV8("Name"));
  if (!stringinfo.ok) {
    return fail;
  }
  rv.name = stringinfo.value;
  stringinfo = read_content_string(contents, SV8("SerialNumber"));
  if (!stringinfo.ok) {
    return fail;
  }
  rv.serial = stringinfo.value;
  stringinfo = read_content_string(contents, SV8("DeviceId"));
  if (!stringinfo.ok) {
    return fail;
  }
  rv.device_id = stringinfo.value;
  stringinfo = read_content_string(contents, SV8("FirmwareVersion"));
  if (!stringinfo.ok) {
    return fail;
  }
  rv.firmware_version = stringinfo.value;
  ContentInt intinfo = read_content_int(contents, SV8("BatteryCapacity"));
  if (!intinfo.ok) {
    return fail;
  }
  rv.battery_capacity = intinfo.value;
  stringinfo = read_content_string(contents, SV8("LastCalibration"));
  if (!stringinfo.ok) {
    return fail;
  }
  rv.last_calibration = stringinfo.value;
  intinfo = read_content_int(contents, SV8("ChannelCount"));
  if (!intinfo.ok) {
    return fail;
  }
  rv.channel_count = intinfo.value;
  rv.ok = true;
  // Get info from the Channel
  intinfo = read_content_int(contents, SV8("DataCount"));
  if (!intinfo.ok) {
    return fail;
  }
  rv.data_count = intinfo.value;
  intinfo = read_content_int(contents, SV8("TimeFormat"));
  if (!intinfo.ok) {
    return fail;
  }
  rv.timeformat = intinfo.value;
  intinfo = read_content_int(contents, SV8("NoBits"));
  if (!intinfo.ok) {
    return fail;
  }
  rv.bits = intinfo.value;
  intinfo = read_content_int(contents, SV8("CommaShift"));
  if (!intinfo.ok) {
    return fail;
  }
  rv.comma_shift = intinfo.value;
  intinfo = read_content_int(contents, SV8("Interval"));
  if (!intinfo.ok) {
    return fail;
  }
  rv.interval = (int32_t)((uint32_t)intinfo.value & 0xfff);
  uint32_t unit = ((uint32_t)intinfo.value & 0xf000)>>12;
  switch (unit) {
    case 8:
      rv.interval_units = SV8("minutes");
      break;
    case 4:
      rv.interval_units = SV8("seconds");
      break;
    default:
      rv.interval_units = SV8("unknown");
  }
  stringinfo = read_content_string(contents, SV8("DateStart"));
  if (!stringinfo.ok) {
    return fail;
  }
  rv.date_start = stringinfo.value;
  return rv;
}


// In Python:
// invB64 = [B64.index(chr(j)) if chr(j) in B64 else -1 for j in range(128)]
static const char invB64[128] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
  -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27,
  28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
  44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

static int b64decode(const char *in, int32_t inlen, char *out, int32_t outlen)
{
  int ix = 0;
  int32_t outcnt = 0;
  unsigned char obuf[5] = {0};
  char *p = out;
  for (int32_t j = 0; j < inlen; j++) {
    int cur = (unsigned char)in[j];
    if (cur == 61) {
      // Filler
      obuf[ix++] = 0;
    } else if (cur < 0 || invB64[cur] == -1) {
      return -1; // Input contains illegal character
    } else {
      obuf[ix++] = invB64[cur];
    }
    if (ix == 4) {
      uint32_t k = (obuf[0]<<18)|(obuf[1]<<12)|(obuf[2]<<6)|obuf[3];
      memset(obuf, 0, 5);
      ix = 0;
      unsigned char tmp[3] = {(k&0xff0000)>>16, (k&0xff00)>>8, k&0xff};
      for (int32_t q = 0; q < 3; q++) {
        *p++ = tmp[q];
        if (++outcnt >= outlen) {
          return 0;
        }
      }
    }
  }
  return outcnt;
}

// Cut, but then on a stringview.
Sv8Cut sv8lpartition(Sv8 s, Sv8 c)
{
  Sv8Cut rv = {0};
  ptrdiff_t ix = sv8find(s, c);
  if (ix==-1) {
    return rv;
  }
  rv.head = s;
  rv.head.len = ix;
  rv.tail.data = s.data + ix + c.len;
  rv.tail.len = s.len - ix - c.len;
  rv.ok = true;
  return rv;
}

DataBlock read_data_block(Sv8 contents, Arena *permanent)
{
  DataBlock rv = {0}, fail = {0};
  Sv8 dend = SV8("</CodedData>");
  Sv8Cut ccut = sv8lpartition(contents, SV8("<CodedData index=\""));
  if (!ccut.ok) {
    debug("failed to find start of data.");
    return fail;
  }
  Sv8Int num = sv8toi(ccut.tail);
  if (!num.ok) {
    debug("failed read index.");
    return fail;
  }
  rv.index = num.result;
  debug("rv.index = %d", rv.index);
  ccut = sv8lpartition(contents, SV8("count=\""));
  if (!ccut.ok) {
    debug("failed to find count.");
    return fail;
  }
  num = sv8toi(ccut.tail);
  if (!num.ok) {
    debug("failed to read count.");
    return fail;
  }
  rv.count = num.result;
  debug("rv.count = %d", rv.count);
  debug("num.tail.len = %d", num.tail.len);
  Sv8 current = num.tail;
  current.data += 2;
  current.len -= 2;
  ptrdiff_t endix = sv8find(current, dend);
  if (endix==-1) {
    debug("failed to find end of data.");
    return fail;
  }
  debug("endix = %ld", endix);
  rv.tail = sv8lskip(current, endix);
  current.len = endix;
  // Current now contains base64 encoded data, with embedded newlines.
  debug("current.len = %d", current.len);
  char *compacted = arena_new(permanent, char, current.len+1);
  int32_t oi = 0;
  for (int32_t j = 0; j < current.len; j++) {
    if (current.data[j] != '\n') {
      compacted[oi++] = current.data[j];
    }
  }
  debug("oi = %d", oi);
  uint16_t outbuf[1024] = {0};
  int conv = b64decode(compacted, oi, (char*)outbuf, 1024);
  if (conv < 1) {
    debug("failed to decode data.");
    return fail;
  }
  // Append data to arena, so the data should be contiguous in the arena.
  rv.b16 = arena_new(permanent, uint16_t, conv/2);
  memcpy(rv.b16, outbuf, conv);
  rv.ok = true;
  return rv;
}
