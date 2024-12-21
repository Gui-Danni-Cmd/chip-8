#pragma once

#include "cpu.h"
#include "render.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void emu(struct Chip8 *chip) {
  uint16_t opcode =
      (chip->dram->memory[chip->pc] << 8) | chip->dram->memory[chip->pc + 1];
  chip->pc += 2;
#if DEBUG_MODE
  printf("PC: %04X Opcode: %04X\n", chip->pc, opcode);
#endif
  switch (opcode & 0xF000) {

  case 0x0000:
    if ((opcode & 0x00FF) == 0xE0) {
      memset(chip->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT); // Clear screen
    } else if ((opcode & 0x00FF) == 0xEE) {
      chip->pc = chip->stack[--chip->sp]; // Return from subroutine
    } else {
      // printf("Opcode 0NNN não implementado (ignorado).\n");
    }
    break;

  case 0x1000:
    // 1NNN: Jump to address NNN
    chip->pc = opcode & 0x0FFF;
    break;

  case 0x2000:
    // 2NNN: Call subroutine at NNN
    chip->stack[chip->sp++] = chip->pc;
    chip->pc = opcode & 0x0FFF;
    break;

  case 0x3000:
    // 3XNN: Skip next instruction if VX == NN
    if (chip->v[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
      chip->pc += 2;
    }
    break;

  case 0x4000:
    // 4XNN: Skip next instruction if VX != NN
    if (chip->v[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
      chip->pc += 2;
    }
    break;

  case 0x5000:
    // 5XY0: Skip next instruction if VX == VY
    if (chip->v[(opcode & 0x0F00) >> 8] == chip->v[(opcode & 0x00F0) >> 4]) {
      chip->pc += 2;
    }
    break;

  case 0x6000:
    // 6XNN: Set VX = NN
    chip->v[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
    break;

  case 0x7000:
    // 7XNN: Add NN to VX (no carry)
    chip->v[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
    break;

  case 0x8000:
    switch (opcode & 0x000F) {
    case 0x0:
      // 8XY0: Set VX = VY
      chip->v[(opcode & 0x0F00) >> 8] = chip->v[(opcode & 0x00F0) >> 4];
      break;

    case 0x1:
      // 8XY1: Set VX = VX | VY
      chip->v[(opcode & 0x0F00) >> 8] |= chip->v[(opcode & 0x00F0) >> 4];
      break;

    case 0x2:
      // 8XY2: Set VX = VX & VY
      chip->v[(opcode & 0x0F00) >> 8] &= chip->v[(opcode & 0x00F0) >> 4];
      break;

    case 0x3:
      // 8XY3: Set VX = VX ^ VY
      chip->v[(opcode & 0x0F00) >> 8] ^= chip->v[(opcode & 0x00F0) >> 4];
      break;

    case 0x4: {
      // 8XY4: Add VY to VX. VF = 1 on carry, 0 otherwise
      uint16_t sum =
          chip->v[(opcode & 0x0F00) >> 8] + chip->v[(opcode & 0x00F0) >> 4];
      chip->v[0xF] = (sum > 0xFF);
      chip->v[(opcode & 0x0F00) >> 8] = sum & 0xFF;
      break;
    }

    case 0x5:
      // 8XY5: Subtract VY from VX. VF = 0 on borrow, 1 otherwise
      chip->v[0xF] =
          chip->v[(opcode & 0x0F00) >> 8] > chip->v[(opcode & 0x00F0) >> 4];
      chip->v[(opcode & 0x0F00) >> 8] -= chip->v[(opcode & 0x00F0) >> 4];
      break;

    case 0x6:
      // 8XY6: Shift VX right by 1. VF = least significant bit
      chip->v[0xF] = chip->v[(opcode & 0x0F00) >> 8] & 0x1;
      chip->v[(opcode & 0x0F00) >> 8] >>= 1;
      break;

    case 0x7:
      // 8XY7: Set VX = VY - VX. VF = 0 on borrow, 1 otherwise
      chip->v[0xF] =
          chip->v[(opcode & 0x00F0) >> 4] > chip->v[(opcode & 0x0F00) >> 8];
      chip->v[(opcode & 0x0F00) >> 8] =
          chip->v[(opcode & 0x00F0) >> 4] - chip->v[(opcode & 0x0F00) >> 8];
      break;

    case 0xE:
      // 8XYE: Shift VX left by 1. VF = most significant bit
      chip->v[0xF] = chip->v[(opcode & 0x0F00) >> 8] >> 7;
      chip->v[(opcode & 0x0F00) >> 8] <<= 1;
      break;

    default:
      printf("Opcode desconhecido: 0x%X\n", opcode);
    }
    break;

  case 0xA000:
    // ANNN: Set I = NNN
    chip->i = opcode & 0x0FFF;
    break;

  case 0xB000:
    // BNNN: Jump to address NNN + V0
    chip->pc = (opcode & 0x0FFF) + chip->v[0];
    break;

  case 0xC000:
    // CXNN: Set VX = random byte AND NN
    chip->v[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
    break;

  case 0xD000: {
    // DXYN: Draw sprite at (VX, VY) with N bytes of sprite data starting at I
    uint8_t x = chip->v[(opcode & 0x0F00) >> 8] % SCREEN_WIDTH;
    uint8_t y = chip->v[(opcode & 0x00F0) >> 4] % SCREEN_HEIGHT;
    uint8_t height = opcode & 0x000F;
    chip->v[0xF] = 0;

    for (int row = 0; row < height; ++row) {
      uint8_t sprite = chip->dram->memory[chip->i + row];
      for (int col = 0; col < 8; ++col) {
        if (sprite & (0x80 >> col)) {
          size_t index = (y + row) * SCREEN_WIDTH + (x + col);
          if (chip->screen[index]) {
            chip->v[0xF] = 1;
          }
          chip->screen[index] ^= 1;
        }
      }
    }
    break;
  }

  case 0xE000:
    switch (opcode & 0x00FF) {
    case 0x9E:
      // EX9E: Skip next instruction if key VX is pressed
      if (chip->keys[chip->v[(opcode & 0x0F00) >> 8]]) {
        chip->pc += 2;
      }
      break;

    case 0xA1:
      // EXA1: Skip next instruction if key VX is not pressed
      if (!chip->keys[chip->v[(opcode & 0x0F00) >> 8]]) {
        chip->pc += 2;
      }
      break;

    default:
      printf("Opcode desconhecido: 0x%X\n", opcode);
    }
    break;

  case 0xF000:
    switch (opcode & 0x00FF) {
    case 0x07:
      // FX07: Set VX = delay timer
      chip->v[(opcode & 0x0F00) >> 8] = chip->delay_timer;
      break;

    case 0x15:
      // FX15: Set delay timer = VX
      chip->delay_timer = chip->v[(opcode & 0x0F00) >> 8];
      break;
    case 0x20:
      // Fx20: Armazena VX na memória em I + X
      chip->dram->memory[chip->i + ((opcode & 0x0F00) >> 8)] =
          chip->v[(opcode & 0x0F00) >> 8];
      // printf("Opcode Fx20: Armazenou V%X (%X) em memória[%X]\n",(opcode &
      // 0x0F00) >> 8, chip->v[(opcode & 0x0F00) >> 8],chip->i + ((opcode &
      // 0x0F00) >> 8));
      break;

    case 0x1E:
      // FX1E: Add VX to I
      chip->i += chip->v[(opcode & 0x0F00) >> 8];
      break;
    case 0x0A:
      if ((opcode & 0xF0FF) == 0xF20A) {
        // F20A: Espera por entrada de teclado e armazena o valor em VX
        uint8_t vx_index = (opcode & 0x0F00) >> 8;
        printf("Opcode F20A: Aguardando entrada de teclado para V%X\n",
               vx_index);

        bool key_pressed = false;

        // Loop para aguardar uma tecla ser pressionada
        for (int i = 0; i < 16; i++) {
          if (chip->keys[i]) { // Supondo que chip->keys[i] indica se a tecla i
                               // foi pressionada
            chip->v[vx_index] = i; // Armazena a tecla pressionada em VX
            key_pressed = true;
            // printf("Opcode F20A: Tecla %X pressionada, armazenada em V%X\n",
            // i, vx_index);
            break;
          }
        }

        // Continua aguardando se nenhuma tecla foi pressionada
        if (!key_pressed) {
          chip->pc -= 2; // Reexecuta a instrução no próximo ciclo
        }
        break;
      }
      break;
    case 0x90:
      // Fx90: Inverte os bits do valor em VX
      chip->v[(opcode & 0x0F00) >> 8] ^= 0xFF;
      // printf("Opcode Fx90: Inverteu os bits de V%X\n", (opcode & 0x0F00) >>
      // 8);
      break;
    case 0x55:
      // Fx55: Armazena os registradores V0 até VX na memória começando em I
      for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
        chip->dram->memory[chip->i + i] = chip->v[i];
      }

      break;

    case 0x65:
      // Fx65: Carrega os valores da memória em I para os registradores V0 até
      // VX
      for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
        chip->v[i] = chip->dram->memory[chip->i + i];
      }
    case 0x33: // FX33
      if ((opcode & 0xF0FF) == 0xFE33) {
        // FE33: Store BCD representation of VX in memory at I
        uint8_t value = chip->v[(opcode & 0x0F00) >> 8];
        chip->dram->memory[chip->i] = value / 100;           // Centenas
        chip->dram->memory[chip->i + 1] = (value / 10) % 10; // Dezenas
        chip->dram->memory[chip->i + 2] = value % 10;        // Unidades
        // printf("Opcode FE33: Valor BCD de V%X armazenado em
        // memória[%X]\n",(opcode & 0x0F00) >> 8, chip->i);
        break;
      }
      break;

    case 0x29:
      if ((opcode & 0xF0FF) == 0xF129) {
        // FX29: Set I to the sprite address for the hexadecimal digit in VX
        chip->i =
            chip->v[(opcode & 0x0F00) >> 8] * 5; // Cada sprite ocupa 5 bytes
        // printf("Opcode F129: I configurado para sprite de V%X em
        // %X\n",(opcode & 0x0F00) >> 8, chip->i);
        break;
      } else if ((opcode & 0xF0FF) == 0xF229) {
        // FX29 (variação): Configurar I para sprites grandes (16x16)
        chip->i =
            chip->v[(opcode & 0x0F00) >> 8] * 10; // Cada sprite ocupa 10 bytes
        // printf("Opcode F229: I configurado para sprite estendido de V%X em
        // %X\n", (opcode & 0x0F00) >> 8, chip->i);
        break;
      }
      break;

    case 0x18:
      if ((opcode & 0xF0FF) == 0xF018) {
        // FX18: Set sound timer to VX
        chip->sound_timer = chip->v[(opcode & 0x0F00) >> 8];
        // printf("Opcode F018: Timer de som configurado para V%X = %X\n",
        //      (opcode & 0x0F00) >> 8, chip->sound_timer);
        break;
      }
      break;
      break;
    default:
      printf("Opcode desconhecido: 0x%X\n", opcode);
    }
    break;

  case 0x9000:
    if ((opcode & 0x00FF) == 0x90) {
      // 9090: Condicional XOR entre V0 e V1; salta se resultado for 0
      uint8_t result = chip->v[0] ^ chip->v[1];
      if (result == 0) {
        chip->pc += 2; // Salta próxima instrução
      }
      // printf("Opcode 9090: Condicional XOR entre V0 e V1. Resultado: %X\n",
      // result);
    } else {
      // printf("Opcode desconhecido: 0x%X\n", opcode);
    }
    break;
  default:
    printf("Opcode desconhecido: 0x%X\n", opcode);
  }
  if (chip->delay_timer > 0)
    chip->delay_timer--;
  if (chip->sound_timer > 0)
    chip->sound_timer--;
}