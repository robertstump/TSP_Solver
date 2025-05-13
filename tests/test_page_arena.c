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
    PageArena* a = createPageArena(map, ARENA_SIZE);
    mu_assert(isArenaValid(a), "page arena creation failure\n");
    arenaPagePop(map);
    mu_assert(a->size == 0 && a->base == 0 && a->offset == 0 && a->previous == 0, "metadata incorrect after pop");
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
    arenaPagePop(map);
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
    mu_assert(arena2->parent == map, "second page parent not set correctly\n");
    mu_assert(arena->base != arena2->base, "Arena base not same location\n");
    PageArena* arena3 = createPageArena(map, ARENA_SIZE);
    mu_assert(isArenaValid(arena3), "third page arena failed on multi test\n");
    mu_assert(map->arenaCount == 3, "arena count increment on third\n");
    mu_assert(arena3->parent == map, "third parent not correct\n");
    mu_assert(arena3->base != arena2->base, "third base same as second\n");
    mu_assert(arena3->base != arena->base, "third base same as first\n");
    mu_assert(arena3->arenaPrevious == arena2, "chain previous to previous fail\n");
    mu_assert(arena3->arenaPrevious->arenaPrevious == arena, "previous chain back to first\n");
    mu_assert(arena3->arenaPrevious->arenaPrevious->arenaPrevious == NULL, "previous chain back to NULL fail\n");
    arenaPagePop(map);
    mu_assert(map->arenaCount == 2, "arena count decrement on first removal\n");
    arenaPagePop(map);
    mu_assert(map->arenaCount == 1, "arena count decrement with first removal\n");
    arenaPagePop(map);
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
    mu_assert(buffer[19] == 19, "data stored in arena\n");
    mu_assert(arena->offset != arena->previous, "offset moved with allocation\n");
    arenaPagePop(map);
    releasePages(map);
    map = NULL;
    PASS_TEST("Data stored in page Arena.");
    return NULL;
}

char* test_arena_overflow() {
    memMap* map = initMemMap(MiB(8));
    mu_assert(isMapValid(map), "invalid map for arena overflow\n");
    PageArena* arena = createPageArena(map, ARENA_SIZE);
    mu_assert(!isArenaValid(arena), "arena overflow should handle gracefully\n");
    releasePages(map);
    map = NULL;
    PASS_TEST("Caught arena overflow");
    return NULL;
}

char* test_one_align() {
    memMap* map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "invalid map for one align test\n");
    PageArena* arena = createPageArena(map, ARENA_SIZE);
    mu_assert(isArenaValid(arena), "arena should validate for one align test\n");
    memptr ptr = arenaPageAlloc(arena, 32, ALIGN_1);
    mu_assert(arena->offset == 32, "Expect no padding");
    memptr ptr2 = arenaPageAlloc(arena, 32, ALIGN_1);
    mu_assert(arena->offset == 64, "Expect no padding");
    memptr ptr3 = arenaPageAlloc(arena, 32, ALIGN_1);
    mu_assert(arena->offset == 96, "Expect no padding");
    mu_assert(ptr != NULL, "Expect allocation 1.");
    mu_assert(ptr2 != NULL, "Expect allocation 1.");
    mu_assert(ptr3 != NULL, "Expect allocation 1.");
    arenaPagePop(map);
    releasePages(map);
    map = NULL;
    PASS_TEST("ALIGN_1 Pass");
    return NULL;
}

char* test_mix_align() {
    memMap* map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "invalid map for mix align test\n");
    PageArena *arena = createPageArena(map, 1024);
    mu_assert(isArenaValid(arena), "Expect arena allocation."); 
    memptr ptr = arenaPageAlloc(arena, 20, ALIGN_8);
    memptr ptr2 = arenaPageAlloc(arena, 8, ALIGN_8);
    memptr ptr3 = arenaPageAlloc(arena, 5, ALIGN_8);
    mu_assert((usize)ptr % ALIGN_8 == 0, "Expect allocation 24");
    mu_assert((usize)ptr2 % ALIGN_8 == 0, "Expect allocation 8");
    mu_assert((usize)ptr3 % ALIGN_8 == 0, "Expect allocation 8");
    mu_assert(arena->offset == 37, "Expect offset increase");
    memptr ptr4 = arenaPageAlloc(arena, 1, ALIGN_2);
    memptr ptr5 = arenaPageAlloc(arena, 1, ALIGN_2);
    memptr ptr6 = arenaPageAlloc(arena, 1, ALIGN_2);
    mu_assert(arena->offset == 43, "Expect offset increase.");
    mu_assert((usize)ptr4 % ALIGN_2 == 0, "Expect allocation 2");
    mu_assert((usize)ptr5 % ALIGN_2 == 0, "Expect allocation 2");
    mu_assert((usize)ptr6 % ALIGN_2 == 0, "Expect allocation 2");
    memptr ptr7 = arenaPageAlloc(arena, 3, ALIGN_4);
    memptr ptr8 = arenaPageAlloc(arena, 31, ALIGN_4);
    memptr ptr9 = arenaPageAlloc(arena, 16, ALIGN_4);
    mu_assert(arena->offset == 96, "Expect offset increase due to allocation.");
    mu_assert((usize)ptr7 % ALIGN_4 == 0, "Expect allocation on 4");
    mu_assert((usize)ptr8 % ALIGN_4 == 0, "Expect second allocation on 4");
    mu_assert((usize)ptr9 % ALIGN_4 == 0, "Expect third allocation on 4");
    arenaPagePop(map);
    releasePages(map);
    map = NULL;
    PASS_TEST("Allocated mixed alignment");
    return NULL;
}

char* test_arena_page_bound() {
    memMap* map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "map failed on page bound test\n");
    PageArena* arena = createPageArena(map, map->pageSize - 1);
    mu_assert(isArenaValid(arena), "arena failed on page bound test\n");
    PageArena* arena2 = createPageArena(map, map->pageSize);
    mu_assert(isArenaValid(arena2), "arena2 failed on page bound test\n");
    mu_assert(arena2->base == arena->base + map->pageSize, "arena2 not on correct page bound\n");
    arenaPagePop(map);
    releasePages(map);
    map = NULL;
    PASS_TEST("Page bound correct on arena creations");
    return NULL;
}

char* test_pop_without_create() {
    //same result as double pop on single arena
    memMap* map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "map failed on pop without test\n");
    arenaPagePop(map);
    releasePages(map);
    map = NULL;
    PASS_TEST("No-op on pop without create");
    return NULL;
}

char* test_zero_alloc() {
    memMap* map = initMemMap(MEM_MAP_SIZE);
    mu_assert(isMapValid(map), "map failed on offset test\n");
    PageArena* arena = createPageArena(map, ARENA_SIZE);
    memptr ptr = arenaPageAlloc(arena, 0, ALIGN_8);
    mu_assert(ptr == NULL, "ptr not null of 0 alloc request\n");
    arenaPagePop(map);
    releasePages(map);
    map = NULL;
    PASS_TEST("Zero allocation handled");
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
    mu_run_test(test_arena_overflow);
    mu_run_test(test_one_align);
    mu_run_test(test_mix_align);
    mu_run_test(test_arena_page_bound);
    mu_run_test(test_pop_without_create);
    mu_run_test(test_zero_alloc);
    return NULL;
}

RUN_TESTS(all_tests);
