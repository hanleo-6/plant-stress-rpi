#include "test_util.h"

#include <stdio.h>

// failure count for the current testsuite
static int test_failures = 0;

// testcond: report a single assertion
// Taken from C Lectures by Duncan White
void testcond(bool ok, const char *testname) {
    printf("T %s: %s\n", testname, ok ? "OK" : "FAIL");
    if (!ok) {
        test_failures++;
    }
}

// test_report: print a summary for the suite, reset the per-suite failure
// count, then return 0 if all of its assertions passed else 1
int test_report(const char *suite) {
    int failures = test_failures;
    test_failures = 0;
    if (failures == 0) {
        printf("%s: all assertions passed\n", suite);
        return 0;
    }
    fprintf(stderr, "%s: %d failure(s)\n", suite, failures);
    return 1;
}

