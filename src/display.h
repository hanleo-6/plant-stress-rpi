#include <stdbool.h>
#include "validity.h"

#define DISPLAY_ADDR 0x3C

int display_init(int fd);
int display_render(int fd, double cwsi, bool valid, validity_state_t state);
int display_clear(int fd);

