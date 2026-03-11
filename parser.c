// file: parser.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:58:54 +0100
// Last modified: 2026-03-11T23:54:10+0100

#include "arena.h"
#include "logging.h"
#include "parser.h"
#include "stringview.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

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
ContentElement read_content_element(Sv8 contents, Sv8 name)
{
  ContentElement rv = {0};
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

Header read_header(Sv8 contents)
{
  Header rv = {0};
  return rv;
}

static const char invB64[128] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1,
  -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
  17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30,
  31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
  51, -1, -1, -1, -1, -1
};

// Adapted from base64.c to skip whitespace.
static int b64decode(const char *in, uint32_t inlen, char *out, uint32_t outlen)
{
  int ix = 0;
  uint32_t outcnt = 0;
  unsigned char obuf[5] = {0};
  char *p = out;
  for (uint32_t j = 0; j < inlen; j++) {
    int cur = (unsigned char)in[j];
    if (cur == 61) {
      // Filler
      obuf[ix++] = 0;
    } else if (cur == 9 || cur == 10 || cur == 13 || cur == 32) {
      continue; // Skip whitespace.
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
  return 0;
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
  return rv;
}


typedef struct {
  int32_t index;
  int32_t count;
  uint16_t *b16;
  Sv8 tail;
  bool ok;
} Data;

Data read_data(Sv8 contents, int32_t bits, Arena *permanent)
{
  Data rv = {0}, fail = {0};
  Sv8 current = contents;
  Sv8 dend = SV8("</CodedData>\"");
  Sv8Cut ccut = sv8lpartition(contents, SV8("<CodedData index=\""));
  if (!ccut.ok) {
    return fail;
  }
  Sv8Int num = sv8toi(ccut.tail);
  if (!num.ok) {
    return fail;
  }
  rv.index = num.result;
  ccut = sv8lpartition(contents, SV8("count=\""));
  if (!ccut.ok) {
    return fail;
  }
  num = sv8toi(ccut.tail);
  if (!num.ok) {
    return fail;
  }
  rv.count = num.result;
  current = num.tail;
  current.data += 2;
  current.len -= 2;
  ptrdiff_t endix = sv8find(current, dend);
  if (endix==-1) {
    return fail;
  }
  current.len -= endix;
  // Current now contains base64 encoded data, with embedded newlines.

  return rv;
}
