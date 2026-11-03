// Copyright 2026 João Vinícius Bueno.

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void put_pixel(int x, int y, uint32_t color, uint16_t width, uint16_t height,
               uint32_t fb[]) {
  if (x < 0 || x >= width || y < 0 || y >= height) {
    return;
  }
  // convert x and y into array index
  fb[width * y + x] = color;
}

void clear(uint32_t color, uint16_t width, uint16_t height, uint32_t fb[]) {
  for (uint32_t i = 0; i < width * height; ++i) {
    fb[i] = color;
  }
}

int fgoto_line(FILE *file, uint16_t target_line) {
  char buffer[1024];
  uint16_t current_line = 0;

  rewind(file);
  while (current_line < target_line &&
         fgets(buffer, sizeof(buffer), file) != NULL) {
    ++current_line;
  }

  if (current_line == target_line) {
    // the file cursor is now at the beginning of target_line
    return 0;
  }
  // target_line does not exist (EOF reached)
  return -1;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "[error] Usage: %s <filename>\n", argv[0]);
    return EXIT_FAILURE;
  }

  FILE *file = fopen(argv[1], "rb");
  if (file == NULL) {
    fprintf(stderr, "[error] Cannot open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  if (fgoto_line(file, 2)) {
    fprintf(stderr, "[error] Cannot read the file %s\n", argv[1]);
    fclose(file);
    return EXIT_FAILURE;
  }
  char image_sizes[15];
  fgets(image_sizes, 15, file);

  uint16_t image_width = atoi(strtok(image_sizes, " ")); // NOLINT
  uint16_t image_height = atoi(strtok(NULL, " "));       // NOLINT

  // printf("%d %d\n", image_width, image_height);
  uint32_t framebuffer[image_width * image_height]; // NOLINT

  if (fgoto_line(file, 4)) {
    fprintf(stderr, "[error] Cannot read the file %s\n", argv[1]);
    fclose(file);
    return EXIT_FAILURE;
  }

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "[error] SDL_Init failed: %s\n", SDL_GetError());
    fclose(file);
    return EXIT_FAILURE;
  }

  SDL_Window *window =
      SDL_CreateWindow("PPM Image Viewer", image_width, image_height, 0);
  if (window == NULL) {
    fprintf(stderr, "[error] SDL_CreateWindow failed: %s\n", SDL_GetError());
    SDL_Quit();
    fclose(file);
    return EXIT_FAILURE;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
  if (renderer == NULL) {
    fprintf(stderr, "[error] SDL_CreateRenderer failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    fclose(file);
    return EXIT_FAILURE;
  }

  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888,
                        SDL_TEXTUREACCESS_STREAMING, image_width, image_height);
  if (texture == NULL) {
    fprintf(stderr, "[error] SDL_CreateTexture failed: %s\n", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    fclose(file);
    return EXIT_FAILURE;
  }

  SDL_Event event;

  bool is_running = true;

  uint8_t r, g, b;

  clear(0x000000, image_width, image_height, framebuffer);
  for (uint16_t j = 0; j < image_height; ++j) {
    for (uint16_t i = 0; i < image_width; ++i) {
      r = getc(file);
      g = getc(file);
      b = getc(file);

      uint32_t color = (r << 16) + (g << 8) + b;
      put_pixel(i, j, color, image_width, image_height, framebuffer);
    }
  }

  while (is_running) {
    uint64_t start = SDL_GetPerformanceCounter();

    // Poll the events
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        is_running = false;
      }
    }

    // Manipulate the framebuffer array as we wish

    // Copy the contents od the framebuffer to the texture
    // Pitch: the width of the image/texture in bytes
    SDL_UpdateTexture(texture, NULL, framebuffer,
                      image_width * sizeof(uint32_t));

    // Display the window and the renderer
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    uint64_t end = SDL_GetPerformanceCounter();

    double elapsed =
        (double)(end - start) / (double)SDL_GetPerformanceFrequency();

    // Cap the framerate to 60 FPS
    if (elapsed < (1.0 / 60.0)) {
      SDL_Delay((uint32_t)(((1.0 / 60.0) - elapsed) * 1000.0));
    }
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  fclose(file);

  return EXIT_SUCCESS;
}
