#pragma once

#include <stdint.h>
#include <stdlib.h> //

#define _DRAM__

#ifdef _DRAM__

struct DRAM {
  uint8_t *memory;
};

struct DRAM *initDRAM();
void freeDRAM(struct DRAM *dram);

#endif
