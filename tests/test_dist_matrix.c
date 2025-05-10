#include "minunit.h"
#include "arena_base.h"
#include "scratch_arena.h"
#include "page_arena.h"
#include "dist_matrix.h"

#define ARENA_SIZE 1024 * 1024

mu_suite_start();
int tests_run = 0;

char* test_count_lines() {
    int count;
    count = CountDataSize("test_data/fake.tsp");
    mu_assert(count == 10, "Count should equal file lines!\n");
    fprintf(stdout, "[X] Count Data Size Success\n");
    return NULL;
}

char* test_real_data() { 
    int count;
    count = CountDataSize("test_data/ca4663.tsp");
    mu_assert(count == 4663, "Count should equal number of cities!\n");
    fprintf(stdout, "[X] Count Cities Success\n");
    return NULL;
}

char* test_italian_cities() {
    int count;
    count = CountDataSize("test_data/it16862.tsp");
    mu_assert(count == 16862, "Count should equal number of italian cities\n");
    fprintf(stdout, "[X] Count Italian Cities Succes\n");
    return NULL;
}

char* test_dist_loading() {
    ScratchArena arena = createScratchArena(ARENA_SIZE);
    int count;
    const char* filename = "test_data/fake.tsp";
    count = CountDataSize(filename);
    mu_assert(count == 10, "Count should equal data size!\n");
    arenaScratchPush(&arena);
    Vec2* dist;
    dist = LoadDistances(&arena, filename, count);
    mu_assert(dist != NULL, "Dist should point to distances array!\n");
    mu_assert(dist[count - 1][0] != 0, "Value should be greater than zero.\n");
    mu_assert(dist[count - 1][1] != 0, "Value should be greater than zero.\n");
    mu_assert(dist[count - 1][0] == 42150.0000f, "Correct x values should be placed in array.\n");
    mu_assert(dist[count - 1][1] == 82966.6667f, "Correct y values should be place in array.\n");
    fprintf(stdout, "[X] Distance Values loaded properly.\n");
    destroyScratchArena(&arena);
    return NULL;
}

char* test_canada_cities() {
    ScratchArena arena = createScratchArena(ARENA_SIZE);
    int count;
    const char* filename = "test_data/ca4663.tsp";
    count = CountDataSize(filename);
    mu_assert(count == 4663, "Count must equal size of data.\n");
    arenaScratchPush(&arena);
    Vec2* dist;
    dist = LoadDistances(&arena, filename, count);
    mu_assert(dist != NULL, "'dist' must point to distance array.\n");
    mu_assert(dist[count - 1][0] == 82483.3333f, "Correct x values in array.\n");
    mu_assert(dist[count - 1][1] == 62250.0000f, "Correct y values in array.\n");
    fprintf(stdout, "[X] Distance Values loaded from real data.\n");
    destroyScratchArena(&arena);
    return NULL;
}

static char* all_tests() {
    mu_run_test(test_count_lines);
    mu_run_test(test_real_data);
    mu_run_test(test_italian_cities);
    mu_run_test(test_dist_loading);
    mu_run_test(test_canada_cities);
    return NULL;
}

RUN_TESTS(all_tests);
