#include "draw.h"
#include "simulation.h"

const SDL_Color COLOR_CLEAR = {0, 0, 0, 255};
const SDL_Color COLOR_WHITE = {255, 255, 255, 255};
const SDL_Color COLOR_BLUE = {0, 0, 255, 255};
const SDL_Color COLOR_RED = {255, 0, 0, 255};
const SDL_Color COLOR_GREEN = {0, 255, 0, 255};
const SDL_Color COLOR_GRAY = {100, 100, 100, 255};

static void draw_one(Statistics_data* data, SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b) {
    if (!data->is_processed) {
        return;
    }
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    int prev_x = -1;
    int prev_y = -1;
    for (int i = 1; i <= MAX_ABONENTS_STATISTICS; i++) {
        if (data->data[i].y < 0) {
            continue;
        }
        int x = (int)data->data[i].x;
        int y = (int)data->data[i].y;
        if (prev_x >= 0) {
            SDL_RenderDrawLine(renderer, prev_x, prev_y, x, y);
        }
        prev_x = x;
        prev_y = y;
    }
}

void DrawText(SDL_Renderer *renderer, TTF_Font *font, char *text, int x, int y, int font_size, bool left_align) {
    TTF_SetFontSize(font, font_size);

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, COLOR_BLUE);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect text_rect;
    text_rect.h = surface->h;
    text_rect.w = surface->w;
    text_rect.x = (left_align) ? x : x - surface->w;
    text_rect.y = y;

    SDL_RenderCopy(renderer, texture, NULL, &text_rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

// 1 - центрировать по горизонтали, 2 - по вертикали, 0 - оба
void DrawTextCentered(SDL_Renderer *renderer, TTF_Font *font,
                      char *text, int x, int y, int font_size, int center_by) {
    TTF_SetFontSize(font, font_size);

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, COLOR_BLUE);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect text_rect;
    text_rect.h = surface->h;
    text_rect.w = surface->w;
    text_rect.x = x;
    text_rect.y = y;
    if (center_by == 1 || center_by == 0) {
        text_rect.x -= surface->w / 2;
    }
    if (center_by == 2 || center_by == 0) {
        text_rect.y -= surface->h / 2;
    }

    SDL_RenderCopy(renderer, texture, NULL, &text_rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

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

    DrawText(renderer, font, "Абоненты", MARGIN, MARGIN - 40, 20, true);
    DrawText(renderer, font, "Базовая станция", MARGIN, SCREEN_HEIGHT - MARGIN + 20, 20, true);
    DrawText(renderer, font, "Открыть график - p    Пауза - Space    Выход - esc",
             SCREEN_WIDTH - MARGIN, SCREEN_HEIGHT - MARGIN + 20, 20, false);
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
        /*if (count_usage[call.preamble_number] == 1) {
            SDL_SetRenderDrawColor(renderer, GREEN);
        }
        else {
            SDL_SetRenderDrawColor(renderer, RED);
        }*/
        if (count_usage[call.preamble_number] > 0) {
            SDL_RenderDrawLine(renderer, MARGIN + padding * call.abonent_id, MARGIN + 50,
                               MARGIN + (WORK_WIDTH) / (MAX_PREAMBLES - 1) + PREAMBLE_PADD * call.preamble_number + 5,
                               SCREEN_HEIGHT - MARGIN - 30);
        }
        i++;
    }
    SDL_RenderPresent(renderer);
}

int DrawPlot(SDL_Renderer *renderer, TTF_Font *font, Statistics_data *stat_data1, Statistics_data *stat_data2, Statistics_data *stat_data3) {
    SDL_SetRenderDrawColor(renderer, WHITE);
    SDL_RenderClear(renderer);

    int padding = 80;

    // SDL_SetRenderDrawColor(renderer, BLACK);
    // SDL_RenderDrawLinesF(renderer, stat_data->data, MAX_ABONENTS_STATISTICS);

    // Дополнительные оси
    for (int i = 0; i < MAX_ABONENTS_STATISTICS + 100; i += 10) {
        SDL_SetRenderDrawColor(renderer, GRAY);
        SDL_RenderDrawLine(renderer, padding + i * SCALE_X, padding,
            padding + i * SCALE_X, PLOT_SCREEN_HEIGHT - padding);
        }
    for (int i = 0; i < 25; i += 4) {
        SDL_SetRenderDrawColor(renderer, GRAY);
        SDL_RenderDrawLine(renderer, padding, PLOT_SCREEN_HEIGHT - padding - i * SCALE_Y,
        PLOT_SCREEN_WIDTH - padding, PLOT_SCREEN_HEIGHT - padding - i * SCALE_Y);
    }

    // Рисуем оси
    SDL_SetRenderDrawColor(renderer, BLACK);
    SDL_RenderDrawLine(renderer, padding, PLOT_SCREEN_HEIGHT - padding, padding, padding);
    SDL_RenderDrawLine(renderer, padding, PLOT_SCREEN_HEIGHT - padding,
                       PLOT_SCREEN_WIDTH - padding, PLOT_SCREEN_HEIGHT - padding);

    // Подписи осей
    for (int i = 0; i < MAX_ABONENTS_STATISTICS + 100; i += 10) {
        char buf[10];
        sprintf(buf, "%d", i);
        DrawTextCentered(renderer, font, buf, padding + i * SCALE_X, PLOT_SCREEN_HEIGHT - padding + 7, 19, 1);
        //printf("\n\n%d %d\n", 40 + i * 3, PLOT_SCREEN_HEIGHT - 20);
        //printf(buf);
    }
    for (int i = 0; i < 25; i += 4) {
        char buf[10];
        sprintf(buf, "%d", i);
        DrawTextCentered(renderer, font, buf, padding - 20, PLOT_SCREEN_HEIGHT - padding - i * SCALE_Y, 19, 0);
        //printf("\n\n\n%d %d\n\n\n", 10, PLOT_SCREEN_HEIGHT - 40 - i * 10);
        //printf(buf);
    }
    DrawText(renderer, font, "Количество абонентов",
             PLOT_SCREEN_WIDTH - padding + 20, PLOT_SCREEN_HEIGHT - padding + 30, 20, false);
    DrawText(renderer, font, "Среднее количество попыток для подключения", padding - 40, padding - 30, 20, true);

    // Заголовок
    DrawTextCentered(renderer, font,
                     "Зависимость оценки среднего количества попыток, необходимых для подключения, от количества абонентов",
                    PLOT_SCREEN_WIDTH / 2, 26, 25, 0);

    draw_one(stat_data1, renderer, 0, 0, 255);
    draw_one(stat_data2, renderer, 255, 0, 0);
    draw_one(stat_data3, renderer, 0, 255, 0);

    // Легенда
    int leg_x = PLOT_SCREEN_WIDTH - 180;
    int leg_y = 75;
    int leg_w = 170;
    int leg_h = 110;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220);
    SDL_Rect bg_rect = {leg_x - 10, leg_y - 10, leg_w + 20, leg_h + 20};
    SDL_RenderFillRect(renderer, &bg_rect);
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &bg_rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_Rect box = {leg_x, leg_y, 20, 20};
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &box);
    DrawText(renderer, font, "64 преамбулы", leg_x + 25, leg_y, 18, true);
    box.y += 30;
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &box);
    DrawText(renderer, font, "32 преамбулы", leg_x + 25, leg_y + 30, 18, true);
    box.y += 30;
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &box);
    DrawText(renderer, font, "16 преамбул", leg_x + 25, leg_y + 60, 18, true);

    SDL_RenderPresent(renderer);
    return 0;
}
