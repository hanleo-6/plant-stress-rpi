// Unit tests for the validity classifier

#include "validity.h"
#include "config.h"
#include "test_util.h"
#include "tests.h"

// a good input
static validity_inputs_t good(void) {
    return (validity_inputs_t){
        .variance = 0.01,
        .dt = -1.0,
        .temp_air = 23.0,
        .humidity_air = 60.0,
        .window_full = true,
    };
}

int run_test_validity(void) {
    config_t cfg;
    config_default(&cfg);
    validity_config_t c = cfg.validity;

    // warm-up
    {
        validity_inputs_t in = good();
        in.window_full = false;
        testcond(validity_eval(&in, &c) == VALIDITY_WARMUP,
                 "not-full window is WARMUP");
    }

    // good path
    {
        validity_inputs_t in = good();
        testcond(validity_eval(&in, &c) == VALIDITY_VALID,
                 "full, in-range, stable is VALID");
    }

    // range sanity
    {
        validity_inputs_t in = good();
        in.dt = c.dt_max + 1.0;
        testcond(validity_eval(&in, &c) == VALIDITY_INVALID_RANGE,
                 "dT above max is out of range");
        in = good();
        in.dt = c.dt_min - 1.0;
        testcond(validity_eval(&in, &c) == VALIDITY_INVALID_RANGE,
                 "dT below min is out of range");
        in = good();
        in.temp_air = c.temp_air_max + 1.0;
        testcond(validity_eval(&in, &c) == VALIDITY_INVALID_RANGE,
                 "air temperature out of range");
        in = good();
        in.humidity_air = c.humidity_air_max + 1.0;
        testcond(validity_eval(&in, &c) == VALIDITY_INVALID_RANGE,
                 "humidity out of range");
    }

    // stability (variance)
    {
        validity_inputs_t in = good();
        in.variance = c.variance_max + 0.1;
        testcond(validity_eval(&in, &c) == VALIDITY_INVALID_UNSTABLE,
                 "variance over threshold is UNSTABLE");
        // edge case variance = threshold should be valid
        in = good();
        in.variance = c.variance_max;
        testcond(validity_eval(&in, &c) == VALIDITY_VALID,
                 "variance exactly at threshold is VALID");
    }

    return test_report("test_validity");
}

