#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdbool.h>
#include <math.h>

void testcond(bool ok, const char *testname);

int test_report(const char *suite);

// APPROX: comparison with tolerance for floating point errors
#define APPROX(a, b, eps) (fabs((a) - (b)) < (eps))

#endif // TEST_UTIL_H

