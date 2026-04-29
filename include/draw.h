#ifndef DRAW_H_
#define DRAW_H_

#include "simulation.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    int x1, y1, x2, y2;
    int r, g, b, a;
} Line;

#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768
#define PLOT_SCREEN_WIDTH 1366
#define PLOT_SCREEN_HEIGHT 768
#define MARGIN (int)(SCREEN_WIDTH * 0.05)
#define WORK_WIDTH SCREEN_WIDTH - MARGIN * 2
#define WORK_HEIGHT SCREEN_HEIGHT - MARGIN * 2
#define PREAMBLE_PADD (SCREEN_WIDTH - MARGIN * 2) / (MAX_PREAMBLES - 1)
#define WHITE 255, 255, 255, 255
#define GRAY 180, 180, 180, 255
#define BLACK 0, 0, 0, 255
#define RED 220, 0, 0, 255
#define GREEN 13, 151, 44, 255
#define BLUE 0, 0, 255, 255

void DrawText(SDL_Renderer *renderer, TTF_Font *font, char *text,
              int x, int y, int font_size, bool left_align);

void DrawTextCentered(SDL_Renderer *renderer, TTF_Font *font,
                      char *text, int x, int y, int font_size, int center_by);

// Рисует основу (номера абонентов, станцию...)
int DrawBase(SDL_Renderer *renderer, TTF_Font *font,
             int abonent_count, int padding, int *count_usage, int *ready_list);

// Каждый кадр обновляет экран
int UpdateScreen(SDL_Renderer *renderer, TTF_Font *font,
                 int abonent_count, int padding, List *list,
                 int attemption_number, int *ready_list);

int DrawPlot(SDL_Renderer *renderer, TTF_Font *font, Statistics_data *stat_data1, Statistics_data *stat_data2, Statistics_data *stat_data3);

#endif // DRAW_H_
