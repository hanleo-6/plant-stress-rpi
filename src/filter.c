#include "filter.h"

// Filter: calculate mean and variance over samples.
//         Mean: used in CWSI calculation so it is more stable.
//         Variance: used for the validity check.
// The filter is implemented with a ring buffer so no dynamic memory
// allocation is needed. It is just a wrapped window, keeping the last
// 'window_size' number of readings.
// The state lives in struct filter_t.

// filter_init: set up the buffer with length window_size
// usage: if (filter_init(&f, 30) != 0) printf("invalid window size");
// preconditions: window_size in [1..FILTER_CAPACITY]
int filter_init(filter_t *f, size_t window_size) {
    if (window_size == 0 || window_size > FILTER_CAPACITY) {
        return -1;
    }
    f->window_size = window_size;
    f->count = 0;
    f->head = 0;
    return 0;
}

// filter_push: store one sample,
//              if full then wrap, so overwrites index 0 (oldest)
// usage: filter_push(&f, reading);
// preconditions: f initialised
void filter_push(filter_t *f, double sample) {
    f->samples[f->head] = sample;
    f->head = (f->head + 1) % f->window_size;
    if (f->count < f->window_size) {
        f->count++;
    }
}

// filter_mean: average of the samples currently held
// usage: double m = filter_mean(&f);
// preconditions: f initialised
double filter_mean(const filter_t *f) {
    if (f->count == 0) { // avoid division by zero
        return 0.0;
    }
    double sum = 0.0;
    for (size_t i = 0; i < f->count; i++) {
        sum += f->samples[i];
    }
    return sum / (double)f->count;
}

// filter_variance: population variance of the samples held
// usage: double v = filter_variance(&f);
// preconditions: f initialised
double filter_variance(const filter_t *f) {
    if (f->count < 2) { // not enough samples
        return 0.0;
    }
    double mean = filter_mean(f);
    double sum_sq = 0.0;
    for (size_t i = 0; i < f->count; i++) {
        double d = f->samples[i] - mean;
        sum_sq += d * d;
    }
    return sum_sq / (double)f->count;
}

// filter_stats: returns mean and population variance together.
//               (Used so that the mean is only computed once and reused
//               for the variance).
// usage: double m, v; filter_stats(&f, &m, &v);
// preconditions: f initialised, mean and variance non-NULL
void filter_stats(const filter_t *f, double *mean, double *variance) {
    double m = filter_mean(f);
    *mean = m;
    if (f->count < 2) { // not enough samples
        *variance = 0.0;
        return;
    }
    double sum_sq = 0.0;
    for (size_t i = 0; i < f->count; i++) {
        double d = f->samples[i] - m;
        sum_sq += d * d;
    }
    *variance = sum_sq / (double)f->count;
}

// filter_full: returns true when the window is full -> warm up complete
// usage: if (filter_full(&f)) ...
// preconditions: f initialised
bool filter_full(const filter_t *f) {
    return f->count == f->window_size;
}

