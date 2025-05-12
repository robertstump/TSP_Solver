#include "minunit.h"
#include "arena_base.h"
#include "scratch_arena.h"

mu_suite_start();
s32 tests_run = 0;
//1MB:

#define ARENA_SIZE 1024 * 1024

s32 isArenaValid(ScratchArena* a) {
    return a && a->base;
}

char* test_create_arena() {
    ScratchArena arena = createScratchArena(ARENA_SIZE);
    mu_assert(isArenaValid(&arena), "Failed to create arena");
    destroyScratchArena(&arena);
    PASS_TEST(" Create Arena Success!");
    return NULL;
}

char* test_arena_size_init() {
    ScratchArena arena = createScratchArena(ARENA_SIZE);
    mu_assert(arena.size == 1048576, "Arena size not correct");
    destroyScratchArena(&arena);
    PASS_TEST(" Arena size correct");
    return NULL;
}

char* test_arena_offset_init() {
    ScratchArena arena = createScratchArena(ARENA_SIZE);
    mu_assert(arena.offset == 0, "Arena initial offset misaligned.");
    destroyScratchArena(&arena);
    PASS_TEST(" Arena offset correct");
    return NULL;
}

char* test_arena_previous_init() {
    ScratchArena arena = createScratchArena(ARENA_SIZE);
    mu_assert(arena.previous == 0, "Arena initial previous not cleared.");
    destroyScratchArena(&arena);
    PASS_TEST(" Arena Previous correct");
    return NULL;
}

char* test_alloc_normal() {
    ScratchArena arena = createScratchArena(1024);
    memptr ptr = arenaScratchAlloc(&arena, 64, ALIGN_8);
    mu_assert(ptr != NULL, "Expected non-null allocation.");
    mu_assert(arena.offset >= 64, "Offset should have increased by at least 64.");
    destroyScratchArena(&arena);
    PASS_TEST(" Success void pointer allocation.");
    return NULL;
}

char* test_alloc_overflow() {
    ScratchArena arena = createScratchArena(1024);
    arena.offset = 1024 - 16;
    memptr ptr = arenaScratchAlloc(&arena, 32, ALIGN_16);
    mu_assert(ptr == NULL, "Expected allocation to fail due to overflow.");
    destroyScratchArena(&arena);
    PASS_TEST(" Successful allocation fail for overflow.");
    return 0;
}

char* test_alloc_zero_size() {
    ScratchArena arena = createScratchArena(1024);
    size_t initial_offset = arena.offset;
    memptr ptr = arenaScratchAlloc(&arena, 0, ALIGN_1);
    mu_assert(ptr != NULL, "Expected non-null pointer for zero-sized allocation.");
    mu_assert(arena.offset >= initial_offset, "Offset should not decrease for zero size allocation.");
    destroyScratchArena(&arena);
    PASS_TEST(" Zero allocation does not decrease offset.");
    return NULL;
}

char* test_alloc_exact_boundary() {
    ScratchArena arena = createScratchArena(1024);
    arena.offset = 1023; //size -1, this should not work for aligned scratch memory, rather it is not a situation that would be normal
    memptr ptr = arenaScratchAlloc(&arena, 32, ALIGN_8);
    mu_assert(ptr == NULL, "Expected NULL due to boundary overflow.");
    destroyScratchArena(&arena);
    PASS_TEST(" Overflow check, success");
    return NULL;
}

char* test_push_pop() {
    ScratchArena arena = createScratchArena(1024);
    arenaScratchPush(&arena);
    memptr ptr = arenaScratchAlloc(&arena, 256, ALIGN_16);
    mu_assert(ptr != NULL, "Expected memory to be allocated");
    mu_assert(arena.previous == 0, "Expected previous to be 0 after push")
    mu_assert(arena.offset > 0, "Offset should have increased after allocation");
    arenaScratchPop(&arena);
    mu_assert(arena.previous == arena.offset, "Pop should return offset to saved position");
    mu_assert(arena.offset == 0, "Offset should return to zero");
    destroyScratchArena(&arena);
    PASS_TEST(" Successful push/pop.");
    return NULL;
}

char* test_hundred_bytes_align() {
    ScratchArena arena = createScratchArena(1024);
    arenaScratchPush(&arena);
    mu_assert(arena.previous == arena.offset, "Expected offset equals previous");
    memptr ptr = arenaScratchAlloc(&arena, 100, ALIGN_4);
    mu_assert(ptr != NULL, "Expected non-null pointer for 100 char* allocation");
    mu_assert(arena.offset == arena.previous + 100 + (100 % ALIGN_4), "Expected offset to be allocation aligned");
    arenaScratchPop(&arena);
    mu_assert(arena.offset == arena.previous, "Expected offset return to previous location.");
    destroyScratchArena(&arena);
    PASS_TEST("Basic memory alignment pass.");
    return NULL;
}

char* test_double_destroy() {
    ScratchArena arena = createScratchArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocated")
    destroyScratchArena(&arena);
    mu_assert(arena.base == NULL, "Expect arena destroyed")
    destroyScratchArena(&arena);
    PASS_TEST(" Double destroy does not double free memory");
    return NULL;
}

char* test_pop_before_push() {
    ScratchArena arena = createScratchArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation");
    arenaScratchPop(&arena);
    mu_assert(arena.offset == 0, "Expect offset still zero after no-push pop.");
    arenaScratchPush(&arena);
    memptr ptr = arenaScratchAlloc(&arena, 64, ALIGN_8);
    mu_assert(ptr != NULL, "Expect pointer allocated memory.");
    mu_assert(arena.offset > 0, "Expect offset increase after push.");
    arenaScratchPop(&arena);
    mu_assert(arena.offset == 0, "Expect offset return to saved position after pop.");
    arenaScratchPop(&arena);
    mu_assert(arena.offset == 0, "Expect offset no-op on one push, two pop");
    mu_assert(arena.previous == 0, "Expect previous to remain safe after double pop.");
    destroyScratchArena(&arena);
    PASS_TEST(" One push, two pops. Clear.");
    return NULL;
}

char* test_zero_with_high_alignment() {
    ScratchArena arena = createScratchArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation.");
    arenaScratchPush(&arena);
    memptr ptr = arenaScratchAlloc(&arena, 0, ALIGN_64);
    mu_assert(ptr != NULL, "Zero allocation points to zero offset.");
    mu_assert(arena.offset == 0, "Zero allocation does not alter offset.");
    mu_assert(arena.previous == 0, "Zero allocation does not alter previous.");
    destroyScratchArena(&arena);
    PASS_TEST(" Zero allocation fine.");
    return NULL;
}

char* test_overflow_huge_allocation() {
    ScratchArena arena = createScratchArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation.");
    arenaScratchPush(&arena);
    memptr ptr = arenaScratchAlloc(&arena, SIZE_MAX, ALIGN_1);
    mu_assert(ptr == NULL, "Arena should not allocate memory beyond size");
    mu_assert(arena.offset == 0, "Offset remains zero, no memory allocated in overflow.");
    destroyScratchArena(&arena);
    PASS_TEST(" Arena huge overflow safety check.");
    return NULL;
}

char* test_odd_alignment() {
    ScratchArena arena = createScratchArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation.");
    arenaScratchPush(&arena);
    memptr ptr = arenaScratchAlloc(&arena, 64, 3);
    mu_assert(ptr == NULL, "Expect null return on odd alignment request.");
    destroyScratchArena(&arena);
    PASS_TEST(" No allocation on odd alignment request.");
    return NULL;
}

//ALIGNMENT CHECKS
char* test_one_align() {
    ScratchArena arena = createScratchArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation.");
    arenaScratchPush(&arena);
    memptr ptr = arenaScratchAlloc(&arena, 32, ALIGN_1);
    mu_assert(arena.offset == 32, "Expect no padding");
    memptr ptr2 = arenaScratchAlloc(&arena, 32, ALIGN_1);
    mu_assert(arena.offset == 64, "Expect no padding");
    memptr ptr3 = arenaScratchAlloc(&arena, 32, ALIGN_1);
    mu_assert(arena.offset == 96, "Expect no padding");
    mu_assert(ptr != NULL, "Expect allocation 1.");
    mu_assert(ptr2 != NULL, "Expect allocation 1.");
    mu_assert(ptr3 != NULL, "Expect allocation 1.");
    destroyScratchArena(&arena);
    return NULL;
}

char* test_alignment_is_aligned() {
    ScratchArena arena = createScratchArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation."); 
    arenaScratchPush(&arena);
    memptr ptr = arenaScratchAlloc(&arena, 20, ALIGN_8);
    memptr ptr2 = arenaScratchAlloc(&arena, 8, ALIGN_8);
    memptr ptr3 = arenaScratchAlloc(&arena, 5, ALIGN_8);
    mu_assert(arena.offset % ALIGN_8 == 0, "Expect allocation aligns on ALIGN_8.");
    mu_assert(ptr != NULL, "Expect allocation 24.");
    mu_assert(ptr2 != NULL, "Expect allocation 8.");
    mu_assert(ptr3 != NULL, "Expect allocation 8.");
    mu_assert(arena.offset == 40, "Expect offset increase");
    arenaScratchPop(&arena);
    arenaScratchPush(&arena);
    mu_assert(arena.offset == 0, "Expect offset return to previous state.");
    memptr ptr4 = arenaScratchAlloc(&arena, 1, ALIGN_2);
    memptr ptr5 = arenaScratchAlloc(&arena, 1, ALIGN_2);
    memptr ptr6 = arenaScratchAlloc(&arena, 1, ALIGN_2);
    mu_assert(arena.offset % ALIGN_2 == 0, "Expect allocation aligns on ALIGN_2.");
    mu_assert(arena.offset == 6, "Expect offset increase.");
    mu_assert(ptr4 != NULL, "Expect allocation 2.");
    mu_assert(ptr5 != NULL, "Expect allocation 2.");
    mu_assert(ptr6 != NULL, "Expect allocation 2.");
    arenaScratchPop(&arena);
    arenaScratchPush(&arena);
    mu_assert(arena.offset == 0, "Expect offset returns to previous state.");
    memptr ptr7 = arenaScratchAlloc(&arena, 3, ALIGN_4);
    memptr ptr8 = arenaScratchAlloc(&arena, 31, ALIGN_4);
    memptr ptr9 = arenaScratchAlloc(&arena, 16, ALIGN_4);
    mu_assert(arena.offset % ALIGN_4 == 0, "Expect allocation aligns on ALIGN_4");
    mu_assert(arena.offset == 52, "Expect offset increase due to allocation.");
    mu_assert(ptr7 != NULL, "Expect allocation on 4.");
    mu_assert(ptr8 != NULL, "Expect second allocation on 4s.");
    mu_assert(ptr9 != NULL, "Expect third allocation on 4s.");
    arenaScratchPop(&arena);
    destroyScratchArena(&arena);
    return NULL;
}

static char* all_tests() {
    mu_run_test(test_create_arena);
    mu_run_test(test_alloc_overflow);
    mu_run_test(test_alloc_zero_size);
    mu_run_test(test_alloc_exact_boundary);
    mu_run_test(test_arena_size_init);
    mu_run_test(test_arena_offset_init);
    mu_run_test(test_arena_previous_init);
    mu_run_test(test_alloc_normal);
    mu_run_test(test_hundred_bytes_align);
    mu_run_test(test_push_pop);
    mu_run_test(test_double_destroy);
    mu_run_test(test_pop_before_push);
    mu_run_test(test_zero_with_high_alignment);
    mu_run_test(test_overflow_huge_allocation);
    mu_run_test(test_odd_alignment);
    mu_run_test(test_one_align);
    mu_run_test(test_alignment_is_aligned);
    return NULL;
}

RUN_TESTS(all_tests);
