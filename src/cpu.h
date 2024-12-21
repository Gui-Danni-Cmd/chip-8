#pragma once
#include "dram.h"
#include "files.h"
#include <stdbool.h>
#include <stdint.h>

#define MEM_SIZE 4096
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define NUM_REGISTERS 16
#define STACK_SIZE 16
#define KEYS 16
#define MEMORY_SIZE 4096
#define ROM_START_ADDRESS 0x200
#define AUDIO_BUFFER_SIZE 4096
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_VOLUME 3000

struct Chip8 {
  struct DRAM *dram;
  uint8_t v[NUM_REGISTERS];
  uint16_t i;
  uint16_t pc;
  uint16_t stack[STACK_SIZE];
  uint8_t sp;
  uint8_t delay_timer;
  uint8_t sound_timer;
  uint8_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];
  uint8_t keys[KEYS];

  bool audio_playing;
  int frequency;
};
typedef struct Chip8 CPU;
void initCPU(CPU *chip);
void initROM(CPU *chip, FILEDRAM *file);
