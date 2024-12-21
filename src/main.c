#include "cpu.h"
#include "emu.h"
#include "files.h"
#include "render.h"
#include <stdio.h>


#ifndef TEST_MODE


int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Erro: Arquivo de ROM não especificado.\n");
    fprintf(stderr, "Uso: %s <caminho_para_o_arquivo_de_ROM>\n", argv[0]);
    return -1;
  }

  FILEDRAM *file = initFILE(argv[1]);
  if (file == NULL) {
    fprintf(stderr, "Erro: Falha ao abrir o arquivo %s.\n", argv[1]);
    return -1;
  }
  CPU chip;
  Display display;
  initCPU(&chip);
  initROM(&chip, file);
  if (!initialize_display(&display)) {
    return -1;
  }
  bool running = true;
  SDL_Event event;

  while (running) {

    int keys[KEYS] = {0};

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        printf("Fim\n");
        running = false;
      } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {

        processInput(&chip, &event);
      }
    }
    emu(&chip);
    handle_audio(&chip);

    render_screen(&chip, &display);

    SDL_Delay(10);
  }
  return 0;
}

#else

#include <CUnit/Basic.h>

void test_clear_screen() {
    struct Chip8 chip = {0};
    chip.dram = malloc(sizeof(struct DRAM));
    memset(chip.screen, 1, SCREEN_WIDTH * SCREEN_HEIGHT); // Simular tela cheia
    chip.dram->memory[0] = 0x00;
    chip.dram->memory[1] = 0xE0; // Opcode 00E0: Clear screen

    emu(&chip);

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
        CU_ASSERT_EQUAL(chip.screen[i], 0);
    }
    free(chip.dram);
}

void test_jump() {
    struct Chip8 chip = {0};
    chip.dram = malloc(sizeof(struct DRAM));
    chip.dram->memory[0] = 0x12;
    chip.dram->memory[1] = 0x34; // Opcode 1234: Jump to address 0x234

    emu(&chip);

    CU_ASSERT_EQUAL(chip.pc, 0x0234);
    free(chip.dram);
}

void test_call_subroutine() {
    struct Chip8 chip = {0};
    chip.dram = malloc(sizeof(struct DRAM));
    chip.dram->memory[0] = 0x22;
    chip.dram->memory[1] = 0x34; // Opcode 2234: Call subroutine at 0x234
    chip.pc = 0x200;

    emu(&chip);

    CU_ASSERT_EQUAL(chip.pc, 0x0234);
    CU_ASSERT_EQUAL(chip.stack[0], 0x202);
    CU_ASSERT_EQUAL(chip.sp, 1);
    free(chip.dram);
}

void test_skip_if_equal() {
    struct Chip8 chip = {0};
    chip.dram = malloc(sizeof(struct DRAM));
    chip.dram->memory[0] = 0x31;
    chip.dram->memory[1] = 0x23; // Opcode 3123: Skip if V1 == 0x23
    chip.v[1] = 0x23;            // V1 set to 0x23
    chip.pc = 0x200;

    emu(&chip);

    CU_ASSERT_EQUAL(chip.pc, 0x204);
    free(chip.dram);
}

void add_tests() {
    CU_pSuite suite = CU_add_suite("Chip8 Emulator Tests", NULL, NULL);
    CU_add_test(suite, "Clear Screen", test_clear_screen);
    CU_add_test(suite, "Jump Instruction", test_jump);
    CU_add_test(suite, "Call Subroutine", test_call_subroutine);
    CU_add_test(suite, "Skip If Equal", test_skip_if_equal);
}

int main() {
    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    add_tests();

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

#endif