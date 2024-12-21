#include "render.h"

bool initialize_display(Display *display) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
    return false;
  }

  display->window = SDL_CreateWindow(
      "Chip-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      SCREEN_WIDTH * PIXEL_SIZE, SCREEN_HEIGHT * PIXEL_SIZE, SDL_WINDOW_SHOWN);
  if (!display->window) {
    printf("Erro ao criar janela: %s\n", SDL_GetError());
    return false;
  }

  display->renderer =
      SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED);
  if (!display->renderer) {
    printf("Erro ao criar renderizador: %s\n", SDL_GetError());
    return false;
  }

  SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
  return true;
}

void render_screen(struct Chip8 *chip8, Display *display) {
  SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
  SDL_RenderClear(display->renderer);

  SDL_SetRenderDrawColor(display->renderer, 255, 255, 255, 255);

  for (int y = 0; y < SCREEN_HEIGHT; ++y) {
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
      if (chip8->screen[y * SCREEN_WIDTH + x]) {
        SDL_Rect pixel = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE,
                          PIXEL_SIZE};
        SDL_RenderFillRect(display->renderer, &pixel);
      }
    }
  }

  SDL_RenderPresent(display->renderer);
}

void shutdown_display(Display *display) {
  SDL_DestroyRenderer(display->renderer);
  SDL_DestroyWindow(display->window);
  SDL_Quit();
}

void processInput(CPU *chip, SDL_Event *event) {
  if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
    bool isKeyDown = event->type == SDL_KEYDOWN;

    switch (event->key.keysym.sym) {
    case SDLK_1:
      chip->keys[0x1] = isKeyDown;
      break;
    case SDLK_2:
      chip->keys[0x2] = isKeyDown;
      break;
    case SDLK_3:
      chip->keys[0x3] = isKeyDown;
      break;
    case SDLK_4:
      chip->keys[0xC] = isKeyDown;
      break;
    case SDLK_q:
      chip->keys[0x4] = isKeyDown;
      break;
    case SDLK_w:
      chip->keys[0x5] = isKeyDown;
      break;
    case SDLK_e:
      chip->keys[0x6] = isKeyDown;
      break;
    case SDLK_r:
      chip->keys[0xD] = isKeyDown;
      break;
    case SDLK_a:
      chip->keys[0x7] = isKeyDown;
      break;
    case SDLK_s:
      chip->keys[0x8] = isKeyDown;
      break;
    case SDLK_d:
      chip->keys[0x9] = isKeyDown;
      break;
    case SDLK_f:
      chip->keys[0xE] = isKeyDown;
      break;
    case SDLK_z:
      chip->keys[0xA] = isKeyDown;
      break;
    case SDLK_x:
      chip->keys[0x0] = isKeyDown;
      break;
    case SDLK_c:
      chip->keys[0xB] = isKeyDown;
      break;
    case SDLK_v:
      chip->keys[0xF] = isKeyDown;
      break;
    default:
      break;
    }
  }
}

void play_sound(int frequency, int duration_ms) {
  SDL_AudioSpec audio_spec;
  SDL_zero(audio_spec);
  audio_spec.freq = 44100;
  audio_spec.format = AUDIO_S16SYS;
  audio_spec.channels = 1;
  audio_spec.samples = 4096;
  audio_spec.callback = NULL;

  SDL_AudioDeviceID audio_device =
      SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
  if (audio_device == 0) {
    printf("Erro ao abrir dispositivo de áudio: %s\n", SDL_GetError());
    return;
  }

  int sample_count = (44100 * duration_ms) / 1000;
  int16_t *buffer = malloc(sample_count * sizeof(int16_t));
  for (int i = 0; i < sample_count; i++) {
    buffer[i] = (i / (44100 / frequency)) % 2 ? 3000 : -3000;
  }

  SDL_QueueAudio(audio_device, buffer, sample_count * sizeof(int16_t));
  SDL_PauseAudioDevice(audio_device, 0);
  SDL_Delay(duration_ms);

  SDL_CloseAudioDevice(audio_device);
  free(buffer);
}

void audio_callback(void *userdata, uint8_t *stream, int len) {
  CPU *chip = (CPU *)userdata;
  int16_t *output = (int16_t *)stream;
  int samples = len / sizeof(int16_t);

  if (!chip || chip->frequency <= 0) {
    memset(stream, 0, len);
    return;
  }

  for (int i = 0; i < samples; i++) {
    if (chip->sound_timer > 0) {
      output[i] = (audio_phase / (AUDIO_SAMPLE_RATE / chip->frequency)) % 2
                      ? AUDIO_VOLUME
                      : -AUDIO_VOLUME;
      audio_phase++;
    } else {
      output[i] = 0;
    }
  }
}

void init_audio(CPU *chip) {

  if (audio_initialized) {
    return;
  }

  SDL_AudioSpec audio_spec;
  SDL_zero(audio_spec);
  audio_spec.freq = AUDIO_SAMPLE_RATE;
  audio_spec.format = AUDIO_S16SYS;
  audio_spec.channels = 1;
  audio_spec.samples = 4096;
  audio_spec.callback = audio_callback;
  audio_spec.userdata = chip;
  if (SDL_OpenAudio(&audio_spec, NULL) < 0) {
    fprintf(stderr, "Erro ao inicializar áudio: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_PauseAudio(0);
}

void handle_audio(CPU *chip) {

  if (chip->sound_timer > 0) {
    if (!chip->audio_playing) {
      play_sound(1440, 100);
      chip->audio_playing = true;
    }
  } else {
    if (chip->audio_playing) {
      SDL_CloseAudio();
      chip->audio_playing = false;
    }
  }
}

void shutdown_audio() {
  if (audio_initialized) {
    SDL_CloseAudio();
    audio_initialized = false;
  }
}

void *audio_thread(void *arg) {
  CPU *chip = (CPU *)arg;
  while (1) {
    handle_audio(chip);
    SDL_Delay(10);
  }
}

void *video_thread(void *arg) {

  ThreadArgs *args = (ThreadArgs *)arg;
  while (1) {
    render_screen(args->chip, args->display);
    SDL_Delay(10);
  }
}
