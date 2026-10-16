#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "validity.h"
#include "cwsi.h"

FILE *logger_open(const char *path);

int logger_write(FILE* f, time_t t, const cwsi_reading_t *r,
                 validity_state_t v);

void logger_close(FILE *f);

