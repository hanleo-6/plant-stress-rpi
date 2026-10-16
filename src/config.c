#include "config.h"

// config.c: contains the default values for the config. These can be
//           overrided through command-line-arguments
// Validity thresholds and CWSI baselines need to be calibrated per-plant and per-setup.
//
#define DEFAULT_BUS "/dev/i2c-1"
#define DEFAULT_LOG "~/logs/cwsi_log.csv"

// config_default: populate cfg with the defaults
// usage: config_t cfg; config_default(&cfg);
// preconditions: cfg non-NULL
void config_default(config_t *cfg) {
    cfg->i2c_bus = DEFAULT_BUS;
    cfg->log_path = DEFAULT_LOG;
    cfg->sample_period_s = 1;
    cfg->filter_window = 30;
    cfg->emissivity = 0.97; // standard leaf emissivity for the MLX correction

    cfg->validity = (validity_config_t){
        .variance_max = 0.30, // NOTE: could be lower
        .dt_min = -15.0,
        .dt_max = 15.0,
        .temp_air_min = 0.0,
        .temp_air_max = 45.0,
        .humidity_air_min = 0.0,
        .humidity_air_max = 100.0,
    };

    cfg->cwsi = (cwsi_params_t){
        .a = -6.7,
        .b = 0.0,
// NOTE: set b = 0 since we are only using one plant
        // dt_ll = a + b*VPD
        .dt_ul = -0.49,
    };
}

