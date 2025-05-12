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
    return m && m->base && m->structBase;
}

char* test_create_memMap() {
    memMap *map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "memMap must not return NULL value");
    releasePages(map);
    map = NULL;
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

    memMap *map = initMemMap(MEM_MAP_SIZE);
    if (sigsetjmp(jump_env, 1) == 0) {
        byte rear_guard = (byte)map->start;
        mu_assert(isMapValid(map), "memMap must not return NULL value\n");
        mu_assert(map->start + map->pageSize == map->structBase, "First page should be before structBase\n");
        mu_assert(map->start + map->pageSize * 3 == map->base, "First page should be before base ptr\n");
        rear_guard[0] = 0xFF; //trip SIGSEGV
        mu_assert(0, "guard page did not trip SIGSEGV\n");
    }
    releasePages(map);    
    map = NULL;
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
    
    memMap *map = initMemMap(MEM_MAP_SIZE);
if (sigsetjmp(jump_env, 1) == 0) {
        byte front_guard = (byte)map->limit;
        mu_assert(isMapValid(map),  "memMap must not return NULL value\n");
        mu_assert(map->pageSize * 4 + map->size == map->limit, "Total allocation should include three gaurd pages and metadata"); 
        front_guard[0] = 0xFF;
        mu_assert(0, "front guard did not trip SIGSEGV\n");
    }
    releasePages(map);
    map = NULL;
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

    memMap *map = initMemMap(MEM_MAP_SIZE);
    if(sigsetjmp(jump_env, 1) == 0) {
        byte rear_guard = (byte)map->start + map->pageSize - 1;
        mu_assert(isMapValid(map), "memMap valid\n");
        rear_guard[1] = 0xFF;
        mu_assert(rear_guard[1] == 0xFF, "should write to rear guard +1\n");
        rear_guard[0] = 0xFF;
        mu_assert(0, "rear guard did not trip SIGSEGV\n");
    }
    releasePages(map);
    map = NULL;
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

    memMap *map = initMemMap(MEM_MAP_SIZE);
    if(sigsetjmp(jump_env, 1) == 0) {
        byte front_guard = (byte)map->limit - map->pageSize - 1;
        mu_assert(isMapValid(map), "memMap is valid\n");
        front_guard[0] = 0xFF;
        mu_assert(front_guard[0] == 0xFF, "shold write to front guard -1\n");
        front_guard[1] = 0xFF;
        mu_assert(0, "front guard SIGSEG not triggered at boundary\n");
    }
    releasePages(map);
    map = NULL;
    PASS_TEST("Tripped SIGSEGV at front guard boundary");
    return NULL;
}

char* test_meta_guard_position_rear() {
    struct sigaction sa; 
    sa.sa_handler = segv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);

    memMap *map = initMemMap(MEM_MAP_SIZE);
    if(sigsetjmp(jump_env, 1) == 0) {
        byte meta_guard = (byte)map->start + (map->pageSize * 2) - 1;
        mu_assert(isMapValid(map), "memMap is valid\n");
        meta_guard[0] = 0xFF;
        mu_assert(meta_guard[0] == 0xFF, "shold write to front guard -1\n");
        meta_guard[1] = 0xFF;
        mu_assert(0, "front guard SIGSEG not triggered at boundary\n");
    }
    releasePages(map);
    map = NULL;
    PASS_TEST("Tripped SIGSEGV at front guard boundary");
    return NULL;
}

char* test_meta_guard_position_front() {
    struct sigaction sa; 
    sa.sa_handler = segv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);

    memMap *map = initMemMap(MEM_MAP_SIZE);
    if(sigsetjmp(jump_env, 1) == 0) {
        byte meta_guard = (byte)map->start + (map->pageSize * 3) - 1;
        mu_assert(isMapValid(map), "memMap is valid\n");
        meta_guard[1] = 0xFF;
        mu_assert(meta_guard[1] == 0xFF, "shold write to front guard -1\n");
        meta_guard[0] = 0xFF;
        mu_assert(0, "front guard SIGSEG not triggered at boundary\n");
    }
    releasePages(map);
    map = NULL;
    PASS_TEST("Tripped SIGSEGV at front guard boundary");
    return NULL;
}

char* test_release_pages() {
    memMap *map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "memMap is valid\n");
    releasePages(map);
    map = NULL;
    mu_assert(!isMapValid(map), "memMap did not release\n");
    PASS_TEST("Released mapped memory");
    return NULL;
}

char* test_create_arena() {
    memMap *map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "invalid memMap for arena creation\n");
    PageArena* arena = createPageArena(map, ARENA_SIZE);
    mu_assert(isArenaValid(arena), "page arena creation failure\n");
    destroyPageArena(arena);
    releasePages(map);
    map = NULL;
    PASS_TEST("Created Page Arena in mapped memory");
    return NULL;
}

char* test_arena_size() {
    memMap *map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "invalid memMap for arena size test.\n");
    PageArena* arena = createPageArena(map, ARENA_SIZE);
    mu_assert(isArenaValid(arena), "page arena failure on size test\n");
    mu_assert(arena->size == ARENA_SIZE, "arena size incorrent on size test\n");
    usize remainder = ARENA_SIZE % map->pageSize;
    usize alignPad = (remainder == 0) ? 0 : (map->pageSize - remainder);
    usize nextOffset = ARENA_SIZE + alignPad;
    mu_assert(map->offset == nextOffset, "memMap Memory offset not ready for next value");
    destroyPageArena(arena);
    releasePages(map);
    map = NULL;
    PASS_TEST("Arena size corrent, map ready");
    return NULL;
}

char* test_multi_arena() {
    memMap *map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "invalid memMap for multi arena test\n");
    PageArena* arena = createPageArena(map, ARENA_SIZE);
    mu_assert(isArenaValid(arena), "first page arena failed on multi test\n");
    mu_assert(map->arenaCount == 1, "arena count increment on first\n");
    mu_assert(arena->parent == map, "first page parent set correct\n");
    PageArena* arena2 = createPageArena(map, ARENA_SIZE);
    mu_assert(isArenaValid(arena2), "second page arena failed on multi test\n");
    mu_assert(map->arenaCount == 2, "arena count increment on second\n");
    mu_assert(arena2->parent == map, "second page parent set correctly\n");
    mu_assert(arena->base != arena2->base, "Arena base not same location\n");
    destroyPageArena(arena);
    mu_assert(map->arenaCount == 1, "arena count decrement with first removal\n");
    destroyPageArena(arena2);
    mu_assert(map->arenaCount == 0, "arena count decrement with second removal\n");
    releasePages(map);
    map = NULL;
    PASS_TEST("Multi Arena Success");
    return NULL;
}

char* test_arena_data_allocation() {
    memMap *map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "invalid memMap for data test\n");
    PageArena* arena = createPageArena(map, ARENA_SIZE);
    mu_assert(isArenaValid(arena), "arena creation failure on data test\n");
    mu_assert(arena->offset == arena->previous, "arena offset/previous not zero'd\n");
    u8* buffer = arenaPageAlloc(arena, 20 * sizeof(u8), ALIGN_4);
    for (int i = 0; i < 20; i++) {
        buffer[i] = i;
    }
    mu_assert(buffer[19] = 19, "data stored in arena\n");
    mu_assert(arena->offset != arena->previous, "offset moved with allocation\n");
    arenaPagePush(arena);
    mu_assert(arena->offset == arena->previous, "stack aligned after push\n");
    destroyPageArena(arena);
    releasePages(map);
    map = NULL;
    PASS_TEST("Data stored in page Arena.");
    return NULL;
}

char* test_destroy_recreate_arena() {
    memMap *map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "invalid memMap for recreation test\n");
    PageArena* arena = createPageArena(map, ARENA_SIZE);
    mu_assert(isArenaValid(arena), "arena creation fail for recreation test\n");
    mu_assert(map->offset > 0, "offset failed to shift correctly with arena\n");
    mu_assert(map->arenaCount = 1, "area failed to increment map arena count\n");
    destroyPageArena(arena);
    mu_assert(!isArenaValid(arena), "arena not destroyed\n");
    mu_assert(map->offset = 0, "map offset not restored on page destroy\n");
    arena = createPageArena(map, ARENA_SIZE);
    mu_assert(isArenaValid(arena), "arena recreation failed after destroy\n");
    PASS_TEST("Recreate Page Arena after destory");
    return NULL;
}

static char* all_tests() {
    mu_run_test(test_create_memMap);
    mu_run_test(test_front_guard_check);
    mu_run_test(test_rear_guard_check);
    mu_run_test(test_guard_position_rear);
    mu_run_test(test_guard_position_front);
    mu_run_test(test_meta_guard_position_front);
    mu_run_test(test_meta_guard_position_rear);
    mu_run_test(test_release_pages);
    mu_run_test(test_create_arena);
    mu_run_test(test_arena_size);
    mu_run_test(test_multi_arena);
    mu_run_test(test_arena_data_allocation);
    mu_run_test(test_destroy_recreate_arena);
    return NULL;
}

RUN_TESTS(all_tests);
