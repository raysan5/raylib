/**********************************************************************************************
*
*   rtest - Minimal single-header test helper for raylib headless tests
*
*   NOTE: raylib has no unit test framework (see 01-discovery), so this helper is intentionally tiny:
*   no dependencies, plain C99, same spirit as the rest of the library.
*
*   USAGE:
*       CHECK(cond)                     -> fails the current test if cond is false
*       CHECK_INT(expected, actual)     -> compares two integers, prints both on failure
*       CHECK_STR(expected, actual)     -> compares two strings (NULL-safe), prints both on failure
*       KNOWN_BUG(cond, "issue/PR")     -> expected failure (XFAIL) tracked upstream:
*                                          - cond false: reported as XFAIL, does NOT fail the run
*                                          - cond true : reported as XPASS (bug fixed, update the test)
*       RTEST_SUMMARY()                 -> prints totals, returns process exit code (0 = ok)
*
**********************************************************************************************/

#ifndef RTEST_H
#define RTEST_H

#include <stdio.h>
#include <string.h>

static int rtestChecks = 0;     // Total checks executed
static int rtestFailed = 0;     // Checks that failed (make the run fail)
static int rtestXfail = 0;      // Known bugs still present
static int rtestXpass = 0;      // Known bugs that now pass

#define CHECK(cond) do { \
    rtestChecks++; \
    if (!(cond)) { rtestFailed++; printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
} while (0)

#define CHECK_INT(expected, actual) do { \
    long long e_ = (long long)(expected), a_ = (long long)(actual); \
    rtestChecks++; \
    if (e_ != a_) { rtestFailed++; printf("  FAIL %s:%d: %s -> expected %lld, got %lld\n", __FILE__, __LINE__, #actual, e_, a_); } \
} while (0)

#define CHECK_STR(expected, actual) do { \
    const char *e_ = (expected), *a_ = (actual); \
    rtestChecks++; \
    if (((e_ == NULL) != (a_ == NULL)) || ((e_ != NULL) && (strcmp(e_, a_) != 0))) { \
        rtestFailed++; \
        printf("  FAIL %s:%d: %s -> expected %s%s%s, got %s%s%s\n", __FILE__, __LINE__, #actual, \
            e_ ? "\"" : "", e_ ? e_ : "NULL", e_ ? "\"" : "", a_ ? "\"" : "", a_ ? a_ : "NULL", a_ ? "\"" : ""); \
    } \
} while (0)

#define KNOWN_BUG(cond, ref) do { \
    rtestChecks++; \
    if (cond) { rtestXpass++; printf("  XPASS %s:%d: %s (known bug %s seems FIXED, promote to CHECK)\n", __FILE__, __LINE__, #cond, ref); } \
    else { rtestXfail++; printf("  XFAIL %s:%d: %s (known bug %s)\n", __FILE__, __LINE__, #cond, ref); } \
} while (0)

#define RUN(test) do { printf("[ RUN ] %s\n", #test); test(); } while (0)

#define RTEST_SUMMARY() ( \
    printf("\n%d checks, %d failed, %d known bugs (XFAIL), %d fixed known bugs (XPASS)\n", \
        rtestChecks, rtestFailed, rtestXfail, rtestXpass), \
    (rtestFailed == 0) ? 0 : 1 )

#endif // RTEST_H
