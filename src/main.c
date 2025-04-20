#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// --- Config ---
#define MAX_CITIES 6
#define MAX_FRAGMENT_LEN 6
#define CACHE_CAPACITY (1 << 20)
#define ARENA_SIZE (10 * 1024 * 1024)

// --- Distance Matrix ---
float distance_matrix[MAX_CITIES][MAX_CITIES] = {
    {0, 10, 15, 20, 25, 30},
    {10, 0, 35, 25, 30, 20},
    {15, 35, 0, 30, 20, 10},
    {20, 25, 30, 0, 15, 35},
    {25, 30, 20, 15, 0, 40},
    {30, 20, 10, 35, 40, 0}
};

// --- Arena Allocator ---
char arena[ARENA_SIZE];
size_t arena_offset = 0;
void* arena_alloc(size_t size) {
    if (arena_offset + size > ARENA_SIZE) return NULL;
    void* ptr = &arena[arena_offset];
    arena_offset += size;
    return ptr;
}

// --- Hash Function (FNV-1a) ---
uint64_t hash_path(const uint8_t* path, uint8_t len) {
    uint64_t hash = 14695981039346656037ULL;
    for (uint8_t i = 0; i < len; ++i) {
        hash ^= path[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

// --- Cache Entry ---
typedef struct {
    uint64_t hash;
    uint8_t path[MAX_FRAGMENT_LEN];
    uint8_t length;
    float value;
    bool used;
} CacheEntry;

CacheEntry cache[CACHE_CAPACITY];

// --- Insert into Cache ---
void insert_fragment(const uint8_t* path, uint8_t len, float value) {
    uint64_t h = hash_path(path, len);
    uint64_t index = h % CACHE_CAPACITY;
    while (cache[index].used) {
        if (cache[index].hash == h && cache[index].length == len && 
            memcmp(cache[index].path, path, len) == 0) return;
        index = (index + 1) % CACHE_CAPACITY;
    }
    cache[index].hash = h;
    cache[index].length = len;
    memcpy(cache[index].path, path, len);
    cache[index].value = value;
    cache[index].used = true;
}

// --- Lookup in Cache ---
bool lookup_fragment(const uint8_t* path, uint8_t len, float* out_value) {
    uint64_t h = hash_path(path, len);
    uint64_t index = h % CACHE_CAPACITY;
    while (cache[index].used) {
        if (cache[index].hash == h && cache[index].length == len && 
            memcmp(cache[index].path, path, len) == 0) {
            *out_value = cache[index].value;
            return true;
        }
        index = (index + 1) % CACHE_CAPACITY;
    }
    return false;
}

// --- Compute Cost of a Path Fragment ---
float compute_cost(const uint8_t* path, uint8_t len) {
    float cost = 0;
    for (uint8_t i = 0; i < len - 1; ++i) {
        cost += distance_matrix[path[i]][path[i + 1]];
    }
    return cost;
}

// --- Placeholder Main ---
int main(void) {
    uint8_t path1[] = {0, 1, 2};
    float cost = compute_cost(path1, 3);
    insert_fragment(path1, 3, cost);

    float cached;
    if (lookup_fragment(path1, 3, &cached)) {
        printf("Cached cost for [0 1 2]: %.2f\n", cached);
    } else {
        printf("Not cached.\n");
    }

    return 0;
}

