#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  const char *name;
  long size;
  uint8_t *buffer;
} FILEDRAM;

static FILE *openfile(const char *name) {
  FILE *file = fopen(name, "rb+");
  if (file == NULL) {
    perror("Error ao abrir o arquivo");
    return NULL;
  }
  return file;
}

static long sizeBin(FILE *file) {
  fseek(file, 0, SEEK_END);
  long size = ftell(file);

  fseek(file, 0, SEEK_SET);
  return size;
}

static uint8_t *buffercreate(long size, FILE *file) {
  uint8_t *buffer = (uint8_t *)malloc(size);
  if (buffer == NULL) {
    perror("Error na alocação de memoria!");
    return NULL;
  }
  size_t byteRead = fread(buffer, 1, size, file);
  if (byteRead != size) {
    perror("Erro na leitura do arquivo");
    free(buffer);
    return NULL;
  }
  return buffer;
}

static FILEDRAM *initFILE(const char *filename) {
  FILEDRAM *file_t = NULL;
  FILE *file = openfile(filename);
  if (file == NULL) {
    return NULL;
  }
  long size = sizeBin(file);
  uint8_t *buffer = buffercreate(size, file);
  if (buffer == NULL) {
    return NULL;
  }
  file_t = (FILEDRAM *)malloc(sizeof(FILEDRAM));
  if (file_t == NULL) {
    return NULL;
  }
  file_t->name = filename;
  file_t->size = size;
  file_t->buffer = buffer;
  return file_t;
}
