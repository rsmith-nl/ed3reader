
#include "setup.h"
#include "logging.h"

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  Options opt = setup(argc, argv);
  debug("starting deploy...");
  switch (opt.cmd) {
    case CHECK:
    break;
    case STATUS:
    break;
    case DIFF:
    break;
    case INSTALL:
    break;
  }
  debug("ending deploy normally...");
  return 0;
}
