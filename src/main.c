// CWSI plant water-stress monitor.
//
// One cycle:
// 1. read the SHT31 (air temperature + humidity) and the MLX90614 canopy
//    temperature (emissivity-corrected inside the driver)
// 2. smooth the canopy temperature through a mean + variance filter
// 3. compute the CWSI
// 4. judge whether conditions make that CWSI valid
// 5. show validity state and reading if valid on the OLED and
//    append a row to the log.
//
// Usage: cwsi_monitor [-o log_path] [-p sample_period_s]

#include "config.h"
#include "validity.h"
#include "cwsi.h"
#include "filter.h"
#include "logger.h"
#include "display.h"
#include "mlx90614.h"
#include "sht31.h"
#include "i2c.h"

#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static volatile sig_atomic_t running = 1;

// handle_sigint: exit the main loop cleanly on ctrl-c
static void handle_sigint(int sig) {
    (void)sig;
    running = 0;
}

// sleep_seconds: wait between samples
static void sleep_seconds(unsigned n) {
    struct timespec t = { .tv_sec = (time_t)n, .tv_nsec = 0 };
    nanosleep(&t, NULL);
}

// parse_args: apply command line arguments onto cfg:
// -o (log path)
// -p (sample period, seconds)
static int parse_args(int argc, char **argv, config_t *cfg) {
    int opt;
    while ((opt = getopt(argc, argv, "o:p:")) != -1) {
        switch (opt) {
        case 'o':
            cfg->log_path = optarg;
            break;
        case 'p': {
            int secs = atoi(optarg);
            if (secs <= 0) {
                fprintf(stderr, "invalid sample period: %s\n", optarg);
                return -1;
            }
            cfg->sample_period_s = (unsigned)secs;
            break;
        }
        default:
            fprintf(stderr, "usage: %s [-o log_path] [-p sample_period_s]\n",
                    argv[0]);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    // initialise config with the built-in defaults, then apply any overrides
    config_t cfg;
    config_default(&cfg);
    if (parse_args(argc, argv, &cfg) != 0) {
        return 1;
    }

    signal(SIGINT, handle_sigint);

    // open the two sensors.
    // both live at fixed addresses on the same bus.
    int mlx = i2c_open(cfg.i2c_bus, MLX90614_ADDR);
    int sht = i2c_open(cfg.i2c_bus, SHT31_ADDR);
    if (mlx < 0 || sht < 0) {
        fprintf(stderr, "could not open sensors on %s\n", cfg.i2c_bus);
        i2c_close(mlx);
        i2c_close(sht);
        return 1;
    }
    if (mlx90614_init(mlx) != 0) {
        fprintf(stderr, "MLX90614 did not respond\n");
        i2c_close(mlx);
        i2c_close(sht);
        return 1;
    }

    // attempt to connect to OLED display, continue without if it fails
    int oled = i2c_open(cfg.i2c_bus, DISPLAY_ADDR);
    if (oled >= 0 && display_init(oled) != 0) {
        fprintf(stderr, "display did not respond, continuing headless\n");
        i2c_close(oled);
        oled = -1;
    }

    // attempt to open log, continue without if it fails to open
    FILE *log = logger_open(cfg.log_path);
    if (log == NULL) {
        fprintf(stderr, "could not open log %s, continuing without it\n",
                cfg.log_path);
    }

    // initialise filter with window size from cfg
    filter_t canopy;
    filter_init(&canopy, cfg.filter_window);

    // start monitoring
    printf("# CWSI monitor on %s, logging to %s\n", cfg.i2c_bus,
           (log != NULL) ? cfg.log_path : "(none)");
    while (running) {
        // get readings from sensors, skip the cycle if any sensor read fails
        double t_air, rh, t_canopy;
        if (sht31_read(sht, &t_air, &rh) != 0 ||
            mlx90614_read_object_corrected(mlx, cfg.emissivity,
                                           &t_canopy) != 0) {
            fprintf(stderr, "sensor read failed, retrying\n");
            sleep_seconds(cfg.sample_period_s);
            continue;
        }

        // filter the canopy temperature to get mean and variance
        filter_push(&canopy, t_canopy);
        double canopy_mean, canopy_var;
        filter_stats(&canopy, &canopy_mean, &canopy_var);

        // calculate CWSI from the smoothed canopy temperature
        cwsi_reading_t r = cwsi_get_value(canopy_mean, t_air, rh, &cfg.cwsi);

        // check validity of the reading
        validity_inputs_t in = {
            .variance = canopy_var,
            .dt = r.dt,
            .temp_air = r.t_air,
            .humidity_air = r.rh,
            .window_full = filter_full(&canopy),
        };
        validity_state_t v = validity_eval(&in, &cfg.validity);

        bool valid = (v == VALIDITY_VALID);
        // display appropriately, write the log row, then echo to stdout
        if (oled >= 0) {
            display_render(oled, r.cwsi, valid, v);
        }

        if (log != NULL) {
            logger_write(log, time(NULL), &r, v);
        }
        printf("cwsi=%.3f  dt=%.2f  vpd=%.3f  %s\n",
               r.cwsi, r.dt, r.vpd, validity_to_string(v));
        fflush(stdout);

        // sleep and repeat
        sleep_seconds(cfg.sample_period_s);
    }
    // cleanup
    if (oled >= 0) {
        display_clear(oled);
        i2c_close(oled);
    }
    if (log != NULL) {
        logger_close(log);
    }
    i2c_close(mlx);
    i2c_close(sht);
    return 0;
}

