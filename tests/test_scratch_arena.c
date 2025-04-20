#include "minunit.h"
#include "scratch.h"

//TODO set up test for scratch arena:

mu_suite_start();
int tests_run = 0;
//1MB:

#define ARENA_SIZE 1024 * 1024

int isArenaValid(ScratchArena* a) {
    return a && a->base;
}

char *test_create_arena() {
    ScratchArena arena = createArena(ARENA_SIZE);
    mu_assert(isArenaValid(&arena), "Failed to create arena");
    destroyArena(&arena);
    fprintf(stdout, "[X] Create Arena Success!\n");
    return NULL;
}

char *test_arena_size_init() {
    ScratchArena arena = createArena(ARENA_SIZE);
    mu_assert(arena.size == 1048576, "Arena size not correct");
    destroyArena(&arena);
    fprintf(stdout, "[X] Arena size correct\n");
    return NULL;
}

char *test_arena_offset_init() {
    ScratchArena arena = createArena(ARENA_SIZE);
    mu_assert(arena.offset == 0, "Arena initial offset misaligned.");
    destroyArena(&arena);
    fprintf(stdout, "[X] Arena offset correct\n");
    return NULL;
}

char *test_arena_previous_init() {
    ScratchArena arena = createArena(ARENA_SIZE);
    mu_assert(arena.previous == 0, "Arena initial previous not cleared.\n");
    destroyArena(&arena);
    return NULL;
}

char *test_alloc_normal() {
    ScratchArena arena = createArena(1024);
    void* ptr = arenaAlloc(&arena, 64, ALIGN_8);
    mu_assert(ptr != NULL, "Expected non-null allocation.");
    mu_assert(arena.offset >= 64, "Offset should have increased by at least 64.");
    destroyArena(&arena);
    fprintf(stdout, "[X] Success void pointer allocation.\n");
    return NULL;
}

char *test_alloc_overflow() {
    ScratchArena arena = createArena(1024);
    arena.offset = 1024 - 16;
    void* ptr = arenaAlloc(&arena, 32, ALIGN_16);
    mu_assert(ptr == NULL, "Expected allocation to fail due to overflow.");
    destroyArena(&arena);
    fprintf(stdout, "[X] Successful allocation fail for overflow.\n");
    return 0;
}

char* test_alloc_zero_size() {
    ScratchArena arena = createArena(1024);
    size_t initial_offset = arena.offset;
    void* ptr = arenaAlloc(&arena, 0, ALIGN_1);
    mu_assert(ptr != NULL, "Expected non-null pointer for zero-sized allocation.");
    mu_assert(arena.offset >= initial_offset, "Offset should not decrease for zero size allocation.");
    destroyArena(&arena);
    fprintf(stdout, "[X] Zero allocation does not decrease offset.\n");
    return NULL;
}

char *test_alloc_exact_boundary() {
    ScratchArena arena = createArena(1024);
    arena.offset = 1023; //size -1, this should not work for aligned scratch memory, rather it is not a situation that would be normal
    void* ptr = arenaAlloc(&arena, 32, ALIGN_8);
    mu_assert(ptr == NULL, "Expected NULL due to boundary overflow.");
    destroyArena(&arena);
    fprintf(stdout, "[X] Overflow check, success\n");
    return NULL;
}

char *test_push_pop() {
    ScratchArena arena = createArena(1024);
    arenaPush(&arena);
    void* ptr = arenaAlloc(&arena, 256, ALIGN_16);
    mu_assert(ptr != NULL, "Expected memory to be allocated");
    mu_assert(arena.previous == 0, "Expected previous to be 0 after push")
    mu_assert(arena.offset > 0, "Offset should have increased after allocation");
    arenaPop(&arena);
    mu_assert(arena.previous == arena.offset, "Pop should return offset to saved position");
    mu_assert(arena.offset == 0, "Offset should return to zero");
    destroyArena(&arena);
    fprintf(stdout, "[X] Successful push/pop.\n");
    return NULL;
}

char *test_hundred_bytes_align() {
    ScratchArena arena = createArena(1024);
    arenaPush(&arena);
    mu_assert(arena.previous == arena.offset, "Expected offset equals previous");
    void* ptr = arenaAlloc(&arena, 100, ALIGN_4);
    mu_assert(ptr != NULL, "Expected non-null pointer for 100 byte allocation");
    mu_assert(arena.offset == arena.previous + 100 + (100 % ALIGN_4), "Expected offset to be allocation aligned");
    arenaPop(&arena);
    mu_assert(arena.offset == arena.previous, "Expected offset return to previous location.");
    destroyArena(&arena);
    fprintf(stdout, "[x] Basic memory alignment pass.\n");
    return NULL;
}

char *test_double_destroy() {
    ScratchArena arena = createArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocated")
    destroyArena(&arena);
    mu_assert(arena.base == NULL, "Expect arena destroyed")
    destroyArena(&arena);
    fprintf(stdout, "[X] Double destroy does not double free memory\n");
    return NULL;
}

char *test_pop_before_push() {
    ScratchArena arena = createArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation\n");
    arenaPop(&arena);
    mu_assert(arena.offset == 0, "Expect offset still zero after no-push pop.\n");
    arenaPush(&arena);
    void* ptr = arenaAlloc(&arena, 64, ALIGN_8);
    mu_assert(ptr != NULL, "Expect pointer allocated memory.\n");
    mu_assert(arena.offset > 0, "Expect offset increase after push.\n");
    arenaPop(&arena);
    mu_assert(arena.offset == 0, "Expect offset return to saved position after pop.\n");
    arenaPop(&arena);
    mu_assert(arena.offset == 0, "Expect offset no-op on one push, two pop\n");
    mu_assert(arena.previous == 0, "Expect previous to remain safe after double pop.\n");
    destroyArena(&arena);
    fprintf(stdout, "[X] One push, two pops. Clear.\n");
    return NULL;
}

char *test_zero_with_high_alignment() {
    ScratchArena arena = createArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation.\n");
    arenaPush(&arena);
    void* ptr = arenaAlloc(&arena, 0, ALIGN_64);
    mu_assert(ptr != NULL, "Zero allocation points to zero offset.\n");
    mu_assert(arena.offset == 0, "Zero allocation does not alter offset.\n");
    mu_assert(arena.previous == 0, "Zero allocation does not alter previous.\n");
    destroyArena(&arena);
    fprintf(stdout, "[X] Zero allocation fine.\n");
    return NULL;
}

char *test_overflow_huge_allocation() {
    ScratchArena arena = createArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation.\n");
    arenaPush(&arena);
    void* ptr = arenaAlloc(&arena, SIZE_MAX, ALIGN_1);
    mu_assert(ptr == NULL, "Arena should not allocate memory beyond size\n");
    mu_assert(arena.offset == 0, "Offset remains zero, no memory allocated in overflow.\n");
    destroyArena(&arena);
    fprintf(stdout, "[X] Arena huge overflow safety check.\n");
    return NULL;
}

char *test_odd_alignment() {
    ScratchArena arena = createArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation.\n");
    arenaPush(&arena);
    void* ptr = arenaAlloc(&arena, 64, 3);
    mu_assert(ptr == NULL, "Expect null return on odd alignment request.\n");
    destroyArena(&arena);
    fprintf(stdout, "[X] No allocation on odd alignment request.\n");
    return NULL;
}

//ALIGNMENT CHECKS
char *test_one_align() {
    ScratchArena arena = createArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation.\n");
    arenaPush(&arena);
    void* ptr = arenaAlloc(&arena, 32, ALIGN_1);
    mu_assert(arena.offset == 32, "Expect no padding\n");
    void* ptr2 = arenaAlloc(&arena, 32, ALIGN_1);
    mu_assert(arena.offset == 64, "Expect no padding\n");
    void* ptr3 = arenaAlloc(&arena, 32, ALIGN_1);
    mu_assert(arena.offset == 96, "Expect no padding\n");
    mu_assert(ptr != NULL, "Expect allocation 1.\n");
    mu_assert(ptr2 != NULL, "Expect allocation 1.\n");
    mu_assert(ptr3 != NULL, "Expect allocation 1.\n");
    destroyArena(&arena);
    return NULL;
}

char *test_alignment_is_aligned() {
    ScratchArena arena = createArena(1024);
    mu_assert(arena.base != NULL, "Expect arena allocation.\n"); 
    arenaPush(&arena);
    void* ptr = arenaAlloc(&arena, 20, ALIGN_8);
    void* ptr2 = arenaAlloc(&arena, 8, ALIGN_8);
    void* ptr3 = arenaAlloc(&arena, 5, ALIGN_8);
    mu_assert(arena.offset % ALIGN_8 == 0, "Expect allocation aligns on ALIGN_8.\n");
    mu_assert(ptr != NULL, "Expect allocation 24.\n");
    mu_assert(ptr2 != NULL, "Expect allocation 8.\n");
    mu_assert(ptr3 != NULL, "Expect allocation 8.\n");
    mu_assert(arena.offset == 40, "Expect offset increase\n");
    arenaPop(&arena);
    arenaPush(&arena);
    mu_assert(arena.offset == 0, "Expect offset return to previous state.\n");
    void* ptr4 = arenaAlloc(&arena, 1, ALIGN_2);
    void* ptr5 = arenaAlloc(&arena, 1, ALIGN_2);
    void* ptr6 = arenaAlloc(&arena, 1, ALIGN_2);
    mu_assert(arena.offset % ALIGN_2 == 0, "Expect allocation aligns on ALIGN_2.\n");
    mu_assert(arena.offset == 6, "Expect offset increase.\n");
    mu_assert(ptr4 != NULL, "Expect allocation 2.\n");
    mu_assert(ptr5 != NULL, "Expect allocation 2.\n");
    mu_assert(ptr6 != NULL, "Expect allocation 2.\n");
    arenaPop(&arena);
    arenaPush(&arena);
    mu_assert(arena.offset == 0, "Expect offset returns to previous state.\n");
    void* ptr7 = arenaAlloc(&arena, 3, ALIGN_4);
    void* ptr8 = arenaAlloc(&arena, 31, ALIGN_4);
    void* ptr9 = arenaAlloc(&arena, 16, ALIGN_4);
    mu_assert(arena.offset % ALIGN_4 == 0, "Expect allocation aligns on ALIGN_4\n");
    mu_assert(arena.offset == 52, "Expect offset increase due to allocation.\n");
    mu_assert(ptr7 != NULL, "Expect allocation on 4.\n");
    mu_assert(ptr8 != NULL, "Expect second allocation on 4s.\n");
    mu_assert(ptr9 != NULL, "Expect third allocation on 4s.\n");
    arenaPop(&arena);
    destroyArena(&arena);
    return NULL;
}

//what happens if i alloc, and then pop and then ref the allocated pointer....?

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
