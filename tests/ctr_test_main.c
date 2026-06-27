#define main ctr_native_game_main_unused
#include "../main.c"
#undef main

#include <utest.h>
#ifdef CTR_TESTS_ENABLE_FFF
#include <fff.h>
DEFINE_FFF_GLOBALS;
#endif
#include "cases/test_list.c"
#include "cases/test_math.c"

UTEST_MAIN();
