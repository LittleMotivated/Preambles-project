#ifndef SIMULATION_H_
#define SIMULATION_H_

#include <stdbool.h>
#include <stddef.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define MAX_PREAMBLES 64
#define MAX_ATTEMPTS 500
#define MAX_ABONENTS_STATISTICS 100 // 300; max = ~350
#define STAT_ATTEMPTS_NUMBER 200 // 200

#define SCALE_X 12
#define SCALE_Y 23

typedef enum States_e{
    ABONENT_SEND_PREAMBLE,
    BS_CHECK,
    ABONENT_ANSWER,
    BS_SUCCESS,
    BS_COLLISION,
    ABONENT_RETRY,
} Call_type;

typedef struct Call_s {
    Call_type type;
    int abonent_id; //id абонента, если вызов совершает абонент, если БС = -1
    int preamble_number; // номер преамбулы, если вызов совершает абонент, если БС = -1
} Call;

typedef struct Call_list_s {
    Call* calls;
    size_t size;
    size_t capacity;
} List;

int list_init(List* l);
int list_add(List* l, Call_type type, int abonent_id, int preamble_number);
void list_free(List* l);
int attempt(int abonent_count, int preamble_count, int* ready_list, List* out_list);

typedef struct {
    SDL_FPoint data[MAX_ABONENTS_STATISTICS + 1];
    char is_processed;
    int preamble_count;
} Statistics_data;

int process_data(Statistics_data *stat_data, int preamble_count);

#endif // SIMULATION_H_
