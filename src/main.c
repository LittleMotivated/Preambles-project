#include "draw.h"
#include "simulation.h"

#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define FRAME_DELAY 2500

int init_window_and_renderer(SDL_Window **window, char *title, SDL_Renderer **renderer) {
    *window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    if (*window == NULL) {
        printf("Error: Failed to create window.\n");
        return -1;
    }
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        printf("Error: Failed to create renderer.\n");
        SDL_DestroyWindow(*window);
        return -1;
    }
    return 0;
}

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

    SDL_Window *window;
    SDL_Renderer *renderer;
    if (init_window_and_renderer(&window, "Base station simulation", &renderer) != 0) {
        goto delete_window_and_renderer;
    }

    SDL_Window *plot_window;
    SDL_Renderer *plot_renderer;
    if (init_window_and_renderer(&plot_window, "Plot with statistics", &plot_renderer) != 0) {
        goto delete_plot_window_and_renderer;
    }
    SDL_HideWindow(plot_window);

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
    DrawBase(renderer, font, abonent_count, padding, NULL, NULL);

    int* ready_list = calloc(abonent_count, sizeof(int));
    if (!ready_list) {
        printf("Error: Memory allocation failed.\n");
        goto delete_ready_list;
    }
    List l;
    if (list_init(&l) != 0) {
        printf("Error: List initialization failed.\n");
        goto delete_list;
    }
    Statistics_data stat_data64 = {0};
    Statistics_data stat_data32 = {0};
    Statistics_data stat_data16 = {0};
    stat_data64.is_processed = false;
    stat_data32.is_processed = false;
    stat_data16.is_processed = false;

    int attemption_number = 0;
    int success = 0;
    int attempt_res = 0;
    int running = 1;
    size_t i = 0;
    SDL_Event event;
    Call_type simulation_step = ABONENT_SEND_PREAMBLE;
    Uint32 prev_time = SDL_GetTicks() - FRAME_DELAY;
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
                case SDLK_p: // Запустить симуляции и открыть график
                    if (stat_data64.is_processed == false) {
                        process_data(&stat_data64, 64);
                        process_data(&stat_data32, 32);
                        process_data(&stat_data16, 16);
                    }
                    SDL_ShowWindow(plot_window);
                    DrawPlot(plot_renderer, font, &stat_data64, &stat_data32, &stat_data16);
                    break;
                case SDLK_SPACE:
                    running = -running; // Пауза
                    break;
                case SDLK_ESCAPE:
                    if (event.window.windowID == SDL_GetWindowID(plot_window)) {
                        SDL_HideWindow(plot_window);
                        break;
                    }
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
        if (running == -1) {
            continue;
        }
        if (i == l.size) {
            if (SDL_GetTicks() - prev_time < FRAME_DELAY) {
                continue;
            }
            prev_time = SDL_GetTicks();

            UpdateScreen(renderer, font, abonent_count, padding, &l, attemption_number, ready_list);

            if (success == abonent_count) {
                //break;
                continue;
            }
            if (attemption_number >= MAX_ATTEMPTS) {
                printf("Достигнуто максимальное количество попыток. Критический сбой.\n");
                break;
            }
            attemption_number++;
            l.size = 0;
            i = 0;
            attempt_res = attempt(abonent_count, 64, ready_list, &l);
            if (attempt_res == -1) {
                printf("Error: Attempt failed with code -1.\n");
                goto delete_all;
            }
            success += attempt_res;
        }
        if (l.size == 0) {
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
        i++;
        printf("%d\n", SDL_GetTicks());
    }

    if (success == abonent_count) {
        printf("Установлено успешное соединение для %d абонентов после %d попыток.\n", abonent_count, attemption_number);
    } else {
        printf("Достигнуто максимальное количество попыток. Критический сбой.\n");
    }

    list_free(&l);
    free(ready_list);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;

delete_all:
    list_free(&l);
delete_list:
    free(ready_list);
delete_ready_list:
    TTF_CloseFont(font);
delete_font:
    TTF_Quit();
delete_ttf:
    SDL_DestroyRenderer(plot_renderer);
    SDL_DestroyWindow(plot_window);
delete_plot_window_and_renderer:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
delete_window_and_renderer:
    SDL_Quit();
delete:
    return -1;
}
