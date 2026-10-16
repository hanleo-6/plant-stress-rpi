#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>
#include "validity.h"
#include "cwsi.h"

typedef struct {
    const char *i2c_bus; // default "/dev/i2c-1"
    const char *log_path; // CSV log path on SD card
    unsigned sample_period_s; // time (secs) between readings
    size_t filter_window; // no. of samples kept for mean/variance
    double emissivity; // canopy emissivity for the MLX IR correction [0..1]
    validity_config_t validity;
    cwsi_params_t cwsi;
} config_t;

void config_default(config_t *cfg);

#endif // CONFIG_H

