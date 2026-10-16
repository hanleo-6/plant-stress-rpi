#include "logger.h"

// Logger: append readings to a CSV on the SD card so trends can be plotted
// Each row is one reading: timestamp, fields, and validity state

// logger_open: open the log for appending, writes CSV header if new file
// usage: FILE *f = logger_open("/media/sd/cwsi.csv")
//        -> returns FILE * or NULL on failure
// preconditions: path non-NULL
FILE *logger_open(const char *path) {
    FILE *f = fopen(path, "a");
    if (f == NULL) {
        return NULL;
    }
    // only write header when starting a fresh file
    if (fseek(f, 0, SEEK_END) == 0 && ftell(f) == 0) {
        fprintf(f, "time,t_canopy,t_air,rh,vpd,dt,cwsi,validity\n");
    }
    return f;
}

// logger_write: append one reading row
// usage: if (logger_write(f, time(NULL), &r, v) != 0) ...
// preconditions: f open for writing, r non-NULL
int logger_write(FILE *f, time_t t, const cwsi_reading_t *r,
                 validity_state_t v) {
    // timestamp
    char ts[32];
    struct tm tm_buf;
    localtime_r(&t, &tm_buf);
    strftime(ts, sizeof ts, "%Y-%m-%dT%H:%M:%S", &tm_buf);

    // write to log
    int n = fprintf(f, "%s,%.2f,%.2f,%.1f,%.3f,%.2f,%.3f,%s\n",
                    ts, r->t_canopy, r->t_air, r->rh, r->vpd, r->dt,
                    r->cwsi, validity_to_string(v));
    if (n < 0) {
        return -1;
    }
    // flush so data not lost on cut power
    fflush(f);
    return 0;
}

// logger_close: close the log file
// usage: logger_close(f);
// preconditions: none (NULL f ignored)
void logger_close(FILE *f) {
    if (f != NULL) {
        fclose(f);
    }
}

