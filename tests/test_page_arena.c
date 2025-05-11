#include "minunit.h"
#include <signal.h>
#include <setjmp.h> 
#include "arena_base.h"
#include "page_arena.h"

static sigjmp_buf jump_env;

mu_suite_start();
s32 tests_run = 0;

#define ARENA_SIZE MiB(10)
#define MEM_MAP_SIZE MiB(64)

void segv_handler(s32 sig) {
    siglongjmp(jump_env, 1);
}

s32 isArenaValid(PageArena* a) {
    return a && a->base;
}

s32 isMapValid(memMap* m) {
    return m && m->base && m->start;
}

char* test_create_memMap() {
    memMap map = reservePages(MEM_MAP_SIZE);
    mu_assert(isMapValid(&map), "memMap must not return NULL value");
    releasePages(&map);
    PASS_TEST("Successfully created memMap");
    return NULL;
}

char* test_rear_guard_check() {
    struct sigaction sa;
    sa.sa_handler = segv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);

    memMap map = reservePages(MEM_MAP_SIZE);
    if (sigsetjmp(jump_env, 1) == 0) {
        byte rear_guard = (byte)map.start;
        mu_assert(isMapValid(&map), "memMap must not return NULL value\n");
        mu_assert(map.start + map.pageSize == map.base, "First page should be before base ptr\n");
        rear_guard[0] = 0xFF; //trip SIGSEGV
        mu_assert(0, "guard page did not trip SIGSEGV\n");
    }
    releasePages(&map);    
    PASS_TEST("Rear Guard raised SIGV correctly on guard write");
    return NULL;
}

char* test_front_guard_check() {
    struct sigaction sa;
    sa.sa_handler = segv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    
    memMap map = reservePages(MEM_MAP_SIZE);
    if (sigsetjmp(jump_env, 1) == 0) {
        byte front_guard = (byte)map.limit;
        mu_assert(isMapValid(&map),  "memMap must not return NULL value\n");
        mu_assert(map.pageSize * 2 + map.size == map.limit, "Total allocation should include two gaurd pages"); 
        front_guard[0] = 0xFF;
        mu_assert(0, "front guard did not trip SIGSEGV\n");
    }
    releasePages(&map);
    PASS_TEST("Front Guard raised SIGV correctly on guard write");
    return NULL;
}

char* test_guard_position_rear() {
    struct sigaction sa;
    sa.sa_handler = segv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);

    memMap map = reservePages(MEM_MAP_SIZE);
    if(sigsetjmp(jump_env, 1) == 0) {
        byte rear_guard = (byte)map.start + map.pageSize - 1;
        mu_assert(isMapValid(&map), "memMap valid\n");
        rear_guard[1] = 0xFF;
        mu_assert(rear_guard[1] == 0xFF, "should write to rear guard +1\n");
        rear_guard[0] = 0xFF;
        mu_assert(0, "rear guard did not trip SIGSEGV\n");
    }
    releasePages(&map);
    PASS_TEST("Tripped SIGSEGV at rear_guard boundary");
    return NULL;
}

char* test_guard_position_front() {
    struct sigaction sa; 
    sa.sa_handler = segv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);

    memMap map = reservePages(MEM_MAP_SIZE);
    if(sigsetjmp(jump_env, 1) == 0) {
        byte front_guard = (byte)map.limit - map.pageSize - 1;
        mu_assert(isMapValid(&map), "memMap is valid\n");
        front_guard[0] = 0xFF;
        mu_assert(front_guard[0] == 0xFF, "shold write to front guard -1\n");
        front_guard[1] = 0xFF;
        mu_assert(0, "front guard SIGSEG not triggered at boundary\n");
    }
    releasePages(&map);
    PASS_TEST("Tripped SIGSEGV at front guard boundary");
    return NULL;
}

static char* all_tests() {
    mu_run_test(test_create_memMap);
    mu_run_test(test_rear_guard_check);
    mu_run_test(test_front_guard_check);
    mu_run_test(test_guard_position_rear);
    mu_run_test(test_guard_position_front);
    return NULL;
}

RUN_TESTS(all_tests);
