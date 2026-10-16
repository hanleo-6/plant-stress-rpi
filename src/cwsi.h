#ifndef CWSI_H
#define CWSI_H

// CWSI (crop water stress index) calculation

// cwsi_params_t: calibration constants.
// The well-watered lower limit is computed as dt_ll = a + b*VPD;
// The water-stressed/non-transpiring upper limit is dt_ul
typedef struct {
    double a;       // lower-limit baseline intercept (C)
    double b;       // lower-limit baseline slope vs VPD (C/kPa)
    double dt_ul;   // upper-limit canopy-air difference (C)
} cwsi_params_t;

// cwsi_reading_t: a single reading output, raw and computed fields
typedef struct {
    double t_canopy; // canopy temperature (C), emissivity-corrected by caller
    double t_air;    // air temperature (C)
    double rh;       // relative humidity (%)
    double vpd;      // vapour pressure deficit (kPa)
    double dt;       // canopy - air temperature (C)
    double cwsi;     // crop water stress index, [0 (ok) .. 1 (stressed)]
} cwsi_reading_t;

cwsi_reading_t cwsi_get_value(double t_canopy, double t_air,
                              double rh, const cwsi_params_t *p);

#endif // CWSI_H

