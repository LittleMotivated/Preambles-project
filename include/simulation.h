#ifndef SIMULATION_H_
#define SIMULATION_H_

#include <stddef.h>

#define MAX_PREAMBLES 64

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
int attempt(int abonent_count, int* ready_list, List* out_list);

#endif // SIMULATION_H_
