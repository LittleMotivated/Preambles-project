#include "draw.h"
#include "simulation.h"

const SDL_Color COLOR_CLEAR = {0, 0, 0, 0};
const SDL_Color COLOR_WHITE = {255, 255, 255, 255};
const SDL_Color COLOR_BLUE = {0, 0, 255, 255};
const SDL_Color COLOR_RED = {255, 0, 0, 255};
const SDL_Color COLOR_GREEN = {0, 255, 0, 255};
const SDL_Color COLOR_GRAY = {100, 100, 100, 255};

// Рисует основу (номера абонентов, станцию...)
int DrawBase(SDL_Renderer *renderer, TTF_Font *font, int abonent_count, int padding) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect rect_bs = {MARGIN, SCREEN_HEIGHT - MARGIN - 10, SCREEN_WIDTH - MARGIN * 2, 10};
    SDL_RenderFillRect(renderer, &rect_bs);

    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect text_rect;

    // Рисуем номера абонентов
    char buf[10];
    for (int i = 0; i < abonent_count; ++i) {
        /*sprintf(buf, "%d", i + 1);
        surface = TTF_RenderText_Blended(font, buf, COLOR_BLUE);
        texture = SDL_CreateTextureFromSurface(renderer, surface);

        text_rect.h = surface->h;
        text_rect.w = surface->w;
        text_rect.x = MARGIN + padding * i;
        text_rect.y = MARGIN;

        SDL_RenderCopy(renderer, texture, NULL, &text_rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);*/

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderDrawRect(renderer, &(SDL_Rect){MARGIN + padding * i, MARGIN + 50, 5, 5});
    }

    // Рисуем преамбулы
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect rect = {MARGIN + (WORK_WIDTH) / (MAX_PREAMBLES - 1), SCREEN_HEIGHT - MARGIN - 30, 10, 30};
    for (int i = 0; i < MAX_PREAMBLES; ++i) {
        SDL_RenderFillRect(renderer, &rect);
        rect.x += PREAMBLE_PADD;
    }

    // Подпись абонентов
    surface = TTF_RenderUTF8_Blended(font, "Абоненты", COLOR_BLUE);
    texture = SDL_CreateTextureFromSurface(renderer, surface);

    text_rect.h = surface->h;
    text_rect.w = surface->w;
    text_rect.x = MARGIN;
    text_rect.y = MARGIN - 40;

    SDL_RenderCopy(renderer, texture, NULL, &text_rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    // Подпись БС
    surface = TTF_RenderUTF8_Blended(font, "Базовая станция", COLOR_BLUE);
    texture = SDL_CreateTextureFromSurface(renderer, surface);

    text_rect.h = surface->h;
    text_rect.w = surface->w;
    text_rect.x = MARGIN;
    text_rect.y = SCREEN_HEIGHT - MARGIN + 20;

    SDL_RenderCopy(renderer, texture, NULL, &text_rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    // Подпись
    surface = TTF_RenderUTF8_Blended(font, "Пауза - Space Выход - esc", COLOR_GRAY);
    texture = SDL_CreateTextureFromSurface(renderer, surface);

    text_rect.h = surface->h;
    text_rect.w = surface->w;
    text_rect.x = SCREEN_WIDTH - MARGIN - surface->w;
    text_rect.y = SCREEN_HEIGHT - MARGIN + 20;

    SDL_RenderCopy(renderer, texture, NULL, &text_rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

// Каждый кадр обновляет экран
int UpdateScreen(SDL_Renderer *renderer, TTF_Font *font,
                 int abonent_count, int padding, List *list, char colored, int attemption_number) {

    SDL_Delay(70);
    DrawBase(renderer, font, abonent_count, padding);

    // Номер попытки
    char buf[20];
    sprintf(buf, "Попытка № %d", attemption_number);
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, buf, COLOR_BLUE);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect text_rect;
    text_rect.h = surface->h;
    text_rect.w = surface->w;
    text_rect.x = SCREEN_WIDTH / 2 - surface->w / 2;
    text_rect.y = MARGIN - 40;

    SDL_RenderCopy(renderer, texture, NULL, &text_rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    // Рисует линии
    int i = 0;
    int count_usage[64] = {0};
    while (list->calls[i].type == ABONENT_SEND_PREAMBLE) {
        count_usage[list->calls[i].preamble_number]++;
        i++;
    }
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    i = 0;
    while (list->calls[i].type == ABONENT_SEND_PREAMBLE) {
        Call call = list->calls[i];
        if (colored) {
            if (count_usage[call.preamble_number] == 1) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            }
        }
        if (count_usage[call.preamble_number] > 0) {
            SDL_RenderDrawLine(renderer, MARGIN + padding * i, MARGIN + 50,
                               MARGIN + (WORK_WIDTH) / (MAX_PREAMBLES - 1) + PREAMBLE_PADD * call.preamble_number + 5,
                               SCREEN_HEIGHT - MARGIN - 30);
        }
        i++;
    }
    SDL_RenderPresent(renderer);
}
