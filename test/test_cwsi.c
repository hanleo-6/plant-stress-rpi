#include "cwsi.h"
#include "test_util.h"
#include "tests.h"

#include <math.h>

int run_test_cwsi(void) {
    // vpd
    {
        cwsi_params_t p = { .a = 0.0, .b = 0.0, .dt_ul = 1.0 };
        // saturated air (rh = 100%) has zero deficit
        cwsi_reading_t sat = cwsi_get_value(25.0, 25.0, 100.0, &p);
        testcond(APPROX(sat.vpd, 0.0, 1e-9), "VPD is zero at 100% humidity");

        // Tetens at 25C gives es ~ 3.168 kPa, so at 50% rh VPD ~ 1.584 kPa
        cwsi_reading_t half = cwsi_get_value(25.0, 25.0, 50.0, &p);
        testcond(APPROX(half.vpd, 1.584, 0.02), "VPD ~1.584 kPa at 25C/50%");

        // VPD decreases monotonically with rising humidity
        cwsi_reading_t dry = cwsi_get_value(25.0, 25.0, 20.0, &p);
        testcond(dry.vpd > half.vpd, "VPD rises as humidity falls");
    }

    // canopy-air diff
    {
        cwsi_params_t p = { .a = 0.0, .b = 0.0, .dt_ul = 5.0 };
        cwsi_reading_t r = cwsi_get_value(27.5, 25.0, 50.0, &p);
        testcond(APPROX(r.dt, 2.5, 1e-9), "dT = canopy - air");
    }

    // index mapping and clamping
    // baseline dt_ll = a + b*VPD = 0 (b = 0), upper limit dt_ul = 5
    {
        cwsi_params_t p = { .a = 0.0, .b = 0.0, .dt_ul = 5.0 };
        // dT halfway between the limits -> 0.5
        testcond(APPROX(cwsi_get_value(27.5, 25.0, 50.0, &p).cwsi,
                        0.5, 1e-9),
                 "midpoint dT gives CWSI 0.5");
        // dT at the upper limit -> 1
        testcond(APPROX(cwsi_get_value(30.0, 25.0, 50.0, &p).cwsi,
                        1.0, 1e-9),
                 "dT at upper limit gives CWSI 1");
        // dT below the lower limit clamps to 0
        testcond(APPROX(cwsi_get_value(23.0, 25.0, 50.0, &p).cwsi,
                        0.0, 1e-9),
                 "dT below lower limit clamps to 0");
        // dT above the upper limit clamps to 1
        testcond(APPROX(cwsi_get_value(33.0, 25.0, 50.0, &p).cwsi,
                        1.0, 1e-9),
                 "dT above upper limit clamps to 1");
    }

    return test_report("test_cwsi");
}

