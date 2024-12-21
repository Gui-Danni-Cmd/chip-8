#include "dram.h"
#include "cpu.h"
#include <string.h>

struct DRAM *initDRAM() {
  struct DRAM *dram = (struct DRAM *)malloc(sizeof(struct DRAM));
  if (!dram) {
    fprintf(stderr, "Erro: Falha ao alocar memória para DRAM.\n");
    return NULL;
  }

  dram->memory = (uint8_t *)malloc(MEM_SIZE);
  if (!dram->memory) {
    fprintf(stderr, "Erro: Falha ao alocar memória para DRAM->memory.\n");
    free(dram);
    return NULL;
  }

  memset(dram->memory, 0, MEM_SIZE);

  return dram;
}
void freeDRAM(struct DRAM *dram) {
  if (dram) {
    free(dram->memory);
    free(dram);
  }
}
