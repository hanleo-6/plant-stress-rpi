// Unit tests for the ring buffer filter

#include "filter.h"
#include "test_util.h"
#include "tests.h"

#include <math.h>

int run_test_filter(void) {
    // init bounds
    {
        filter_t f;
        testcond(filter_init(&f, 8) == 0, "init accepts a valid window");
        testcond(filter_init(&f, 0) == -1, "init rejects window 0");
        testcond(filter_init(&f, FILTER_CAPACITY + 1) == -1,
                 "init rejects window over capacity");
        testcond(filter_init(&f, FILTER_CAPACITY) == 0,
                 "init accepts window at capacity");
    }

    // edge case empty filter
    {
        filter_t f;
        filter_init(&f, 4);
        testcond(APPROX(filter_mean(&f), 0.0, 1e-9), "empty mean is 0");
        testcond(APPROX(filter_variance(&f), 0.0, 1e-9), "empty variance is 0");
        testcond(!filter_full(&f), "empty filter is not full");
    }

    // single sample test
    {
        filter_t f;
        filter_init(&f, 4);
        filter_push(&f, 21.0);
        testcond(APPROX(filter_mean(&f), 21.0, 1e-9),
                 "mean of single sample");
        testcond(APPROX(filter_variance(&f), 0.0, 1e-9),
                 "variance for one sample returns 0 representing undefined");
    }

    // multiple samples with same value
    {
        filter_t f;
        filter_init(&f, 4);
        for (int i = 0; i < 4; i++) {
            filter_push(&f, 23.5);
        }
        testcond(APPROX(filter_mean(&f), 23.5, 1e-9),
                 "mean of same value samples");
        testcond(APPROX(filter_variance(&f), 0.0, 1e-9),
                "variance of same value samples is 0");
        testcond(filter_full(&f), "filter full after window_size samples");
    }

    // ring-buffer wrap-around
    // window 3: {1,2,3}; pushing 4 evicts the oldest, leaving {2,3,4}
    {
        filter_t f;
        filter_init(&f, 3);
        filter_push(&f, 1.0);
        filter_push(&f, 2.0);
        filter_push(&f, 3.0);
        testcond(APPROX(filter_mean(&f), 2.0, 1e-9), "mean before wrap");
        filter_push(&f, 4.0);
        testcond(APPROX(filter_mean(&f), 3.0, 1e-9),
                 "wrap evicts oldest sample");
        // variance of {2,3,4} = 2/3
        testcond(APPROX(filter_variance(&f), 2.0 / 3.0, 1e-9),
                 "variance after wrap uses only the window");
        testcond(filter_full(&f), "window still full after wrap");
    }

    return test_report("test_filter");
}

