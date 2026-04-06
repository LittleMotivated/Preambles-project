#include "draw.h"
#include "simulation.h"

const SDL_Color COLOR_CLEAR = {0, 0, 0, 255};
const SDL_Color COLOR_WHITE = {255, 255, 255, 255};
const SDL_Color COLOR_BLUE = {0, 0, 255, 255};
const SDL_Color COLOR_RED = {255, 0, 0, 255};
const SDL_Color COLOR_GREEN = {0, 255, 0, 255};
const SDL_Color COLOR_GRAY = {100, 100, 100, 255};

// Рисует основу (номера абонентов, станцию...)
int DrawBase(SDL_Renderer *renderer, TTF_Font *font,
             int abonent_count, int padding, int *count_usage, int *ready_list) {
    SDL_SetRenderDrawColor(renderer, WHITE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, GRAY);
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
        if (ready_list != NULL && ready_list[i] == 1) {
            SDL_SetRenderDrawColor(renderer, GREEN);
            SDL_RenderFillRect(renderer, &(SDL_Rect){MARGIN + padding * i, MARGIN + 50, 5, 10});
        } else {
            SDL_SetRenderDrawColor(renderer, BLUE);
            SDL_RenderDrawRect(renderer, &(SDL_Rect){MARGIN + padding * i, MARGIN + 50, 5, 5});
        }
    }

    // Рисуем преамбулы
    SDL_SetRenderDrawColor(renderer, RED);
    SDL_Rect rect = {MARGIN + (WORK_WIDTH) / (MAX_PREAMBLES - 1), SCREEN_HEIGHT - MARGIN - 30, 10, 30};
    for (int i = 0; i < MAX_PREAMBLES; ++i) {
        if (count_usage != NULL) {
            if (count_usage[i] == 0) {
                SDL_SetRenderDrawColor(renderer, GRAY);
            } else if (count_usage[i] == 1) {
                SDL_SetRenderDrawColor(renderer, GREEN);
            } else {
                SDL_SetRenderDrawColor(renderer, RED);
            }
        }
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
    surface = TTF_RenderUTF8_Blended(font, "Открыть график - p    Пауза - Space    Выход - esc", COLOR_GRAY);
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
                 int abonent_count, int padding, List *list,
                 int attemption_number, int *ready_list) {

    SDL_Delay(70);

    int i = 0;
    int count_usage[64] = {0};
    while (list->calls[i].type == ABONENT_SEND_PREAMBLE) {
        count_usage[list->calls[i].preamble_number]++;
        i++;
    }

    DrawBase(renderer, font, abonent_count, padding, count_usage, ready_list);

    // Номер попытки
    char buf[32];
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

    SDL_SetRenderDrawColor(renderer, GRAY);
    i = 0;
    while (list->calls[i].type == ABONENT_SEND_PREAMBLE) {
        Call call = list->calls[i];
        if (count_usage[call.preamble_number] == 1) {
            SDL_SetRenderDrawColor(renderer, GREEN);
        }
        else {
            SDL_SetRenderDrawColor(renderer, RED);
        }
        if (count_usage[call.preamble_number] > 0) {
            SDL_RenderDrawLine(renderer, MARGIN + padding * call.abonent_id, MARGIN + 50,
                               MARGIN + (WORK_WIDTH) / (MAX_PREAMBLES - 1) + PREAMBLE_PADD * call.preamble_number + 5,
                               SCREEN_HEIGHT - MARGIN - 30);
        }
        i++;
    }
    SDL_RenderPresent(renderer);
}

int DrawPlot(SDL_Renderer *renderer, TTF_Font *font, Statistics_data *stat_data) {
    SDL_SetRenderDrawColor(renderer, WHITE);
    SDL_RenderClear(renderer);

    int padding = 40;
    // Рисуем оси
    SDL_SetRenderDrawColor(renderer, BLACK);
    SDL_RenderDrawLine(renderer, padding, PLOT_SCREEN_HEIGHT - padding, padding, padding);
    SDL_RenderDrawLine(renderer, padding, PLOT_SCREEN_HEIGHT - padding,
                       PLOT_SCREEN_WIDTH - padding, PLOT_SCREEN_HEIGHT - padding);

    SDL_RenderDrawLinesF(renderer, stat_data->data, MAX_ABONENTS_STATISTICS);

    SDL_RenderPresent(renderer);
}
