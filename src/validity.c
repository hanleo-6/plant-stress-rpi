#include "validity.h"

// Validity: decide whether the current reading was taken under the right
//           conditions for CWSI to be meaningful

// validity_eval: check each factor in order then return validity state
// usage: validity_state_t s = validity_eval(&inputs, &config);
// preconditions: inputs and config non-NULL
validity_state_t validity_eval(const validity_inputs_t *inputs,
                               const validity_config_t *config) {
    // 1. warm-up: variance doesn't matter until the window has filled
    if (!inputs->window_full) {
        return VALIDITY_WARMUP;
    }
    // 2. sanity check sensor ranges
    if (inputs->dt < config->dt_min || inputs->dt > config->dt_max ||
        inputs->temp_air < config->temp_air_min ||
        inputs->temp_air > config->temp_air_max ||
        inputs->humidity_air < config->humidity_air_min ||
        inputs->humidity_air > config->humidity_air_max) {
        return VALIDITY_INVALID_RANGE;
    }
    // 3. conditions must be stable across the sample window
    if (inputs->variance > config->variance_max) {
        return VALIDITY_INVALID_UNSTABLE;
    }
    return VALIDITY_VALID;
}

// validity_to_string: convert state to string for display and logging
// usage: printf("%s\n", validity_to_string(s));
// preconditions: none
const char *validity_to_string(validity_state_t s) {
    switch (s) {
    case VALIDITY_WARMUP:           return "warming up";
    case VALIDITY_VALID:            return "valid";
    case VALIDITY_INVALID_UNSTABLE: return "unstable";
    case VALIDITY_INVALID_RANGE:    return "out of range";
    }
    return "unknown";
}

