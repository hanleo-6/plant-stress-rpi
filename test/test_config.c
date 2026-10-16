// Unit tests for the config defaults.
// These act as guardrails to make sure no crazy defaults are being used

#include "config.h"
#include "filter.h"
#include "test_util.h"
#include "tests.h"

#include <math.h>
#include <string.h>

int run_test_config(void) {
    config_t cfg;
    config_default(&cfg);

    testcond(cfg.i2c_bus != NULL && strlen(cfg.i2c_bus) > 0,
             "i2c_bus default set");
    testcond(cfg.log_path != NULL && strlen(cfg.log_path) > 0,
             "log_path default set");
    testcond(cfg.filter_window <= FILTER_CAPACITY,
             "default window fits in filter capacity");
    testcond(cfg.validity.dt_min < cfg.validity.dt_max,
             "dT min < dT max holds");
    testcond(cfg.validity.temp_air_min < cfg.validity.temp_air_max,
             "air-temperature min < max holds");
    testcond(APPROX(cfg.validity.humidity_air_min, 0.0, 1e-12) &&
             APPROX(cfg.validity.humidity_air_max, 100.0, 1e-12),
             "humidity range in [0..100]%");
    testcond(cfg.emissivity > 0.0 && cfg.emissivity <= 1.0,
             "emissivity is a fraction in (0,1]");

    return test_report("test_config");
}

