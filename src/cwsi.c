#include "cwsi.h"
#include <math.h>

// cwsi_vpd: vapour pressure deficit (kPa) from air temperature and humidity
// usage: double vpd = cwsi_vpd(t_air, rh);
// preconditions: t_air in Celsius, rh is a percentage [0..100]
static double cwsi_vpd(double t_air, double rh) {
    // Tetens equation
    double es = 0.6108 * exp(17.27 * t_air / (t_air + 237.3));
    // VPD equation
    return es * (1.0 - rh / 100.0);
}

// cwsi_compute: map the canopy-air difference onto the [0, 1] stress index
// dt_ll (well-watered baseline) varies with VPD
// dt_ul is the non-transpiring upper limit.
// CWSI = (dt - dt_ll) / (dt_ul - dt_ll).
static double cwsi_compute(double dt, double vpd, const cwsi_params_t *p) {
    double dt_ll = p->a + p->b * vpd;
    double denom = p->dt_ul - dt_ll;
    if (denom == 0.0) {
        return 0.0;
    }
    double cwsi = (dt - dt_ll) / denom;
    // clamp to correct any values slightly off due to noise or calibration
    // issues, all values shouldn't be much off after division.
    if (cwsi < 0.0) {
        cwsi = 0.0;
    }
    if (cwsi > 1.0) {
        cwsi = 1.0;
    }
    return cwsi;
}

// cwsi_get_value: bundle the reading and its CWSI
// t_canopy is the emissivity-corrected canopy temperature from the MLX90614
// t_air/rh come from the SHT31
cwsi_reading_t cwsi_get_value(double t_canopy, double t_air,
                              double rh, const cwsi_params_t *p) {

    double vpd = cwsi_vpd(t_air, rh);
    double dt = t_canopy - t_air;

    cwsi_reading_t r = {
        .t_canopy = t_canopy,
        .t_air = t_air,
        .rh = rh,
        .vpd = vpd,
        .dt = dt,
        .cwsi = cwsi_compute(dt, vpd, p),
    };
    return r;
}

