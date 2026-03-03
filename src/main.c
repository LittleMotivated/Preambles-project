#include "draw.h"
#include "simulation.h"

#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define MAX_ATTEMPTS 500

int main(int argc, char* argv[]) {
    int abonent_count;
    if (argc > 1) {
        abonent_count = atoi(argv[1]);
        if (abonent_count <= 0) {
            printf("Error: Incorrect number of abonents. Connection impossible.\n");
            goto delete;
        }
    }
    else {
        printf("Error: No input.\n");
        goto delete;
    }
    
    // Инициализация SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error: Failed to init SDL.\n");
        goto delete;
    }
    SDL_Window *window = SDL_CreateWindow("Base station simulation",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (window == NULL) {
        printf("Error: Failed to create window.\n");
        goto delete_window;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Error: Failed to create renderer.\n");
        goto delete_renderer;
    }

    if (TTF_Init() != 0) {
        printf("Error: Failed to init TTF.\n");
        goto delete_ttf;
    }
    TTF_Font *font = TTF_OpenFont("./fonts/LiberationSans-Regular.ttf", 20);
    if (font == NULL) {
        printf("Error: Failed to load font file.\n");
        goto delete_font;
    }

    int padding = (SCREEN_WIDTH - MARGIN * 2) / abonent_count;
    DrawBase(renderer, font, abonent_count, padding);

    int* ready_list = calloc(abonent_count, sizeof(int));
    if (!ready_list) {
        printf("Error: Memory allocation failed.\n");
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    List l;
    if (list_init(&l) != 0) {
        printf("Error: List initialization failed.\n");
        free(ready_list);
        goto delete_list;
    }
    int attemption_number = 0;
    int success = 0;
    int attempt_res = 0;
    size_t i = 0;
    int running = 1;
    SDL_Event event;
    Call_type simulation_step = ABONENT_SEND_PREAMBLE;
    // Основной цикл
    while (running) {
        // Обработка событий
        while (SDL_PollEvent(&event)) {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_SPACE:
                    running = -running; // Пауза
                    break;
                case SDLK_ESCAPE:
                    running = 0; // Выход
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        if (i == l.size) {
            if (success == abonent_count) {
                break;
            }
            if (attemption_number >= MAX_ATTEMPTS) {
                printf("Достигнуто максимальное количество попыток. Критический сбой.\n");
                break;
            }
            attemption_number++;
            l.size = 0;
            i = 0;
            attempt_res = attempt(abonent_count, ready_list, &l);
            if (attempt_res == -1) {
                printf("Error: Attempt failed with code -1.\n");
                list_free(&l);
                goto delete_list;
            }
            success += attempt_res;
        }
        if (l.size == 0) {
            continue;
        }
        if (running == -1) {
            continue;
        }

        Call call = l.calls[i];
        switch (call.type) {
        case ABONENT_SEND_PREAMBLE:
            printf("Абонент %d: Преамбула %d.\n", call.abonent_id + 1, call.preamble_number);
            break;
        case BS_CHECK:
            printf("БС: Кто отправил преамбулу %d?\n", call.preamble_number);
            break;
        case ABONENT_ANSWER:
            printf("Я, Абонент %d, отправил преамбулу %d.\n", call.abonent_id + 1, call.preamble_number);
            break;
        case BS_SUCCESS:
            printf("БС: Абонент %d готов.\n", call.abonent_id + 1);
            break;
        case BS_COLLISION:
            printf("БС: Квитанция об ошибке (коллизия).\n");
            printf("БС: Запуск повторной процедуры подключения.\n");
            break;
        case ABONENT_RETRY:
            printf("Повторное подключение.\n");
            break;
        default:
            printf("Error: Unknown call.\n");
            break;
        }
        UpdateScreen(renderer, font, abonent_count, padding, &l, 1, attemption_number);
        i++;

        //SDL_Delay(0);
    }

    if (success == abonent_count) {
        printf("Установлено успешное соединение для %d абонентов после %d попыток.\n", abonent_count, attemption_number);
    } else {
        printf("Достигнуто максимальное количество попыток. Критический сбой.\n");
    }


    
    return 0;
delete_list:
    list_free(&l);
delete_font:
    TTF_CloseFont(font);
delete_ttf:
    TTF_Quit();
delete_renderer:
    SDL_DestroyRenderer(renderer);
delete_window:
    SDL_DestroyWindow(window);
delete_sdl:
    SDL_Quit();
delete:
    return -1;
}
