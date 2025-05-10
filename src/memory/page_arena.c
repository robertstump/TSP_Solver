#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "arena_base.h"
#include "page_arena.h"

typedef struct {
    size_t totalSize;
    void* start;
    void* base;
} memMap;

typedef struct {
    char* base;
    size_t offset;
    size_t size;
    size_t previous;
    size_t limit;
} PageArena;

memMap reservePages(size_t requestedSize) {
    memMap map;
    size_t pageSize = sysconf(_SC_PAGESIZE);
    size_t remainder = requestedSize % pageSize;
    size_t pageOffset = (remainder == 0) ? 0 : (pageSize - remainder);
    size_t totalUsableSize = requestedSize + pageOffset;
    map.totalSize = totalUsableSize + 2 * pageSize;

    map.start = mmap(NULL, totalSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (map->start == MAP_FAILED) {
        fprintf(stderr, "FAILED TO MAP MEMORY PAGES");
        map.start = NULL;
        map.base = NULL;
        map.totalSize = 0;
        return map;
    }
    mprotect((char*)map.start + pageSize, totalUsableSize, PROT_READ | PROT_WRITE);
    map.base = map.start + pageSize;
    return memMap;
}


void releasePages(memMap* map) {
    munmap(map->start, map->totalSize);
    map->start = NULL;
    map->base = NULL;
}

PageArena createPageArena(memMap* map, size_t arena_size) {
    
    
