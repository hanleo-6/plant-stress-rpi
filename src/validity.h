#ifndef VALIDITY_H
#define VALIDITY_H

#include <stdbool.h>

typedef enum {
    VALIDITY_WARMUP,
    VALIDITY_VALID,
    VALIDITY_INVALID_UNSTABLE,
    VALIDITY_INVALID_RANGE
} validity_state_t;

typedef struct {
    double variance;
    double dt;
    double temp_air;
    double humidity_air;
    bool window_full;
} validity_inputs_t;

typedef struct {
    double variance_max;
    double dt_min;
    double dt_max;
    double temp_air_min;
    double temp_air_max;
    double humidity_air_min;
    double humidity_air_max;
} validity_config_t;

validity_state_t validity_eval(const validity_inputs_t *inputs,
                               const validity_config_t *config);

const char *validity_to_string(validity_state_t s);

#endif // VALIDITY_H

