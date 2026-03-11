// file: parser.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2026-03-10 20:58:54 +0100
// Last modified: 2026-03-11T20:38:19+0100

#include "arena.h"
#include "logging.h"
#include "parser.h"
#include "stringview.h"

#include <assert.h>
#include <stddef.h>
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
