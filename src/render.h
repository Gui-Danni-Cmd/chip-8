#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define PIXEL_SIZE 10
#include "cpu.h"
#include <stdbool.h>

static int16_t audio_buffer[AUDIO_BUFFER_SIZE];
static int audio_buffer_position = 0;
static int audio_phase = 0;
static bool audio_initialized = false;

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
} Display;
typedef struct {
  CPU *chip;
  Display *display;
} ThreadArgs;

bool initialize_display(Display *display);
void render_screen(struct Chip8 *chip8, Display *display);
void shutdown_display(Display *display);
void processInput(struct Chip8 *chip, SDL_Event *event);
void play_sound(int frequency, int duration_ms);
void handle_audio(CPU *chip);
void audio_callback(void *userdata, uint8_t *stream, int len);
void init_audio(CPU *chip);
void *audio_thread(void *arg);

void *video_thread(void *arg);