#include <common.h>

struct Data data = {
    .trigApprox = {[0] = {.sin = 0x0000, .cos = 0x1000}},
};

#include <utest.h>

#include "../cases/test_math.c"

UTEST_MAIN();
