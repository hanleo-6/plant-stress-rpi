#ifndef FILTER_H
#define FILTER_H

#include <stddef.h>
#include <stdbool.h>

#define FILTER_CAPACITY 256

typedef struct {
    double samples[FILTER_CAPACITY];
    size_t window_size; // runtime size [1..FILTER_CAPACITY]
    size_t count;  // samples seen so far, resets at window_size
    size_t head;   // next write index, wraps when full
} filter_t;

int filter_init(filter_t *f, size_t window);
void filter_push(filter_t *f, double sample);

double filter_mean(const filter_t *f);
double filter_variance(const filter_t *f);
void filter_stats(const filter_t *f, double *mean, double *variance);

bool filter_full(const filter_t *f);

#endif // FILTER_H

