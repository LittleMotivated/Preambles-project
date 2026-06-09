#include "util.h"

void swap_min(Sint16 *a, Sint16 *b) {
    Sint16 tmp = *a;
    *a = *b;
    *b = tmp;
}
