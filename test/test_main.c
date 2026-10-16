#include "tests.h"

#include <stddef.h>
#include <stdio.h>

typedef struct {
    const char *name;
    int (*run)(void);
} test_suite;

int main(void) {
    test_suite suites[] = {
        { "test_cwsi", run_test_cwsi },
        { "test_filter", run_test_filter },
        { "test_validity", run_test_validity },
        { "test_config", run_test_config },
    };
    const size_t n = sizeof suites / sizeof suites[0];

    int fail = 0;
    for (size_t i = 0; i < n; i++) {
        printf("=== %s ===\n", suites[i].name);
        if (suites[i].run() != 0) {
            fail = 1;
        }
    }

    if (fail == 0) {
        printf("YAY ALL TESTS PASSED\n");
        return 0;
    }
    printf("OH NO SOME TESTS FAILED...\n");
    return 1;
}

