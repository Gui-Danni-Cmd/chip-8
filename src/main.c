#include "cpu.h"
#include "emu.h"
#include "files.h"
#include "render.h"
#include <stdio.h>


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

    SDL_Delay(16);
  }
  return 0;
}

