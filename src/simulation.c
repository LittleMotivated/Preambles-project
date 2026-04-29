#include "draw.h"
#include "simulation.h"
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <stdlib.h>

#define START_CAPACITY 16

//возвращаемые значения: -1 - ошибка, 0 - успех, 1 - коллизия

int list_init(List* l) {
    l->calls = malloc(START_CAPACITY * sizeof(Call)); 
    if (!l->calls) {
        printf("Error: Memory allocation failed.\n");
        return -1;
    }
    l->size = 0;
    l->capacity = START_CAPACITY;
    return 0;
}

int list_add(List* l, Call_type type, int abonent_id, int preamble_number) {
    if (l->size + 1 > l->capacity) {
        size_t new_capacity = l->capacity * 2;
        Call* new_calls = realloc(l->calls, sizeof(Call) * new_capacity);
        if (!new_calls) {
            printf("Error: Memory allocation failed.\n");
            return -1;
        }
        l->calls = new_calls;
        l->capacity = new_capacity;
    }
    Call call;
    call.type = type;
    call.abonent_id = abonent_id;
    call.preamble_number = preamble_number;
    l->calls[l->size++] = call;
    return 0;
}

void list_free(List* l) {
    free(l->calls);
    l->calls = NULL;
    l->size = l->capacity = 0;
}

int attempt(int abonent_count, int preamble_count, int* ready_list, List* out_list) {
    if (out_list == NULL || ready_list == NULL) {
        printf("Error: out_list or ready_list is NULL.\n");
        return -1;
    }
    if (abonent_count <= 0) {
        printf("Error: Incorrect number of abonents. Connection impossible.\n");
        return -1;
    }
    if (preamble_count <= 0) {
        printf("Error: Incorrect number of preambles.\n");
        return -1;
    }

    static int random_seed = 0;
    if (random_seed == 0) {
        srand(time(NULL));
        random_seed = 1;
    }

    int* preambles = malloc(abonent_count * sizeof(int));
    int* count_usage = calloc(preamble_count, sizeof(int));
    if (!preambles || !count_usage) {
        printf("Error: Memory allocation failed.\n");
        free(preambles);
        free(count_usage);
        return -1;
    }
    

    for (int i = 0; i < abonent_count; i++) {
        if (ready_list[i] != 0) {
            continue;
        }
        int preamble = rand() % preamble_count;
        preambles[i] = preamble;
        count_usage[preamble]++;
        if (list_add(out_list, ABONENT_SEND_PREAMBLE, i, preamble) != 0) {
            free(preambles);
            free(count_usage);
            return -1;
        }
    }

    for (int i = 0; i < preamble_count; i++) {
        if (count_usage[i] == 0) {
            continue;
        }
        if (list_add(out_list, BS_CHECK, -1, i) != 0) {
            free(preambles);
            free(count_usage);
            return -1;
        }
        for (int j = 0; j < abonent_count; j++) {
            if (ready_list[j] != 0) {
                continue;
            }
            if (preambles[j] == i) {
                if (list_add(out_list, ABONENT_ANSWER, j, i) != 0) {
                    free(preambles);
                    free(count_usage);
                    return -1;
                }
            }
        }
    }
    int success = 0;
    int collision = 0;
    for (int i = 0; i < preamble_count; i++) {
        if (count_usage[i] > 1) {
            collision = 1;
            break;
        }
    }

    if (collision == 0) {
        for (int i = 0; i < abonent_count; i++) {
            if (ready_list[i] == 0) {
                if (list_add(out_list, BS_SUCCESS, i, preambles[i]) != 0) {
                    free(preambles);
                    free(count_usage);
                    return -1;
                }
                ready_list[i] = 1;
                success++;
            }
        }
        free(preambles);
        free(count_usage);
        return success;
    } else {
        if (list_add(out_list, BS_COLLISION, -1, -1) != 0) {
            free(preambles);
            free(count_usage);
            return -1;
        }
        for (int i = 0; i < preamble_count; i++) {
            if (count_usage[i] == 0) {
                continue;
            }
            if (count_usage[i] == 1) {
                for (int j = 0; j < abonent_count; j++) {
                    if (ready_list[j] == 0 && preambles[j] == i) {
                         if (list_add(out_list, BS_SUCCESS, j, i) != 0) {
                            free(preambles);
                            free(count_usage);
                            return -1;
                        }
                        ready_list[j] = 1;
                        success++;
                        break;
                    }
                }
            } else {
                for (int j = 0; j < abonent_count; j++) {
                    if (ready_list[j] == 0 && preambles[j] == i) {
                         if (list_add(out_list, ABONENT_RETRY, j, i) != 0) {
                            free(preambles);
                            free(count_usage);
                            return -1;
                        }
                    }
                }
            }
        }
        free(preambles);
        free(count_usage);
        return success;
    }
}

int process_data(Statistics_data *stat_data, int p_count) {
    stat_data->preamble_count = p_count;
    int debug_total = 0;
    stat_data->data[0].x = 80;
    stat_data->data[0].y = PLOT_SCREEN_HEIGHT - 80;
    int max_attempts = 500;
    if (stat_data->preamble_count == 32) {
        max_attempts = 2000;
    } else if (stat_data->preamble_count == 16) {
        max_attempts = 5000;
    }
    for (int abonent_count = 1; abonent_count <= MAX_ABONENTS_STATISTICS; ++abonent_count) {
        long double attempts_sum = 0;
        long long total_attempts = 0;
        for (int i = 0; i < STAT_ATTEMPTS_NUMBER; ++i) {
            int success = 0;
            int attemption_number = 0;
            int attempts = 0;
            int* ready_list = calloc(abonent_count, sizeof(int));
            if (!ready_list) {
                printf("Error: Memory allocation failed.\n");
            }
            List l;
            if (list_init(&l) != 0) {
                printf("Error: List initialization failed.\n");
                free(ready_list);
            }
            while (success < abonent_count && attempts < max_attempts) {
                attempts++;
                int res = attempt(abonent_count, p_count, ready_list, &l);
                success += res;
                attemption_number++;
                if (res == 0 || res == -1) { // Нет обработки ошибки res == -1
                    break;
                }
            }
            if (success == abonent_count) {
                total_attempts++;
                attempts_sum += attemption_number - 1; // Учитывается лишняя попытка при res == 0
                debug_total++;
            }
            list_free(&l);
            free(ready_list);
        }
        stat_data->data[abonent_count].x = abonent_count * SCALE_X + 80;
        long double y;
        if (total_attempts > 0) {
            y = PLOT_SCREEN_HEIGHT - 80 - attempts_sum * SCALE_Y / total_attempts;
        } else {
            y = -1; // точка не нарисована
        }
        stat_data->data[abonent_count].y = y;
        printf("%d %Lf\n", abonent_count, (attempts_sum / total_attempts));
        printf("%Lf %lld\n\n", attempts_sum, total_attempts);
    }
    stat_data->is_processed = true;
    //printf("\n\nDebug total: %d\n\n", debug_total);
}
