#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include "arena_base.h"
#include "page_arena.h"

memMap reservePages(usize requestedSize) {
    memMap map;

    if (requestedSize < 1) {
        LOG_ERROR("Allocation must be non-zero positive integer");
        map.start = NULL;
        map.base = NULL;
        map.previous = 0;
        map.offset = 0;
        return map;
    }

    map.pageSize = sysconf(_SC_PAGESIZE);
    usize remainder = requestedSize % map.pageSize;
    usize pageOffset = (remainder == 0) ? 0 : (map.pageSize - remainder);
    map.size = requestedSize + pageOffset;
    map.limit = map.size + 2 * map.pageSize;

    map.start = mmap(NULL, map.limit, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (map.start == MAP_FAILED) {
        fprintf(stderr, "FAILED TO MAP MEMORY PAGES");
        map.start = NULL;
        map.base = NULL;
        map.limit = 0;
        return map;
    }
    mprotect((byte)map.start + map.pageSize, map.size, PROT_READ | PROT_WRITE);
    map.base = map.start + map.pageSize;
    map.previous = 0;
    map.offset = 0;
    return map;
}

void pageAlign(memMap* map, usize arenaSize) {
    usize remainder = arenaSize % map->pageSize; 
    usize alignPad = (remainder == 0) ? 0 : (map->pageSize - remainder);
    if (map->offset + alignPad + arenaSize < map->limit) {
        map->offset += arenaSize + alignPad;
    }
}

void memMapPush(memMap* m) {
    m->previous = m->offset;
}

void memMapPop(memMap* m) {
    m->offset = m->previous;
}

void releasePages(memMap* map) {
    munmap(map->start, map->limit);
    map->start = NULL;
    map->base = NULL;
}

PageArena createPageArena(memMap* map, usize arenaSize) {
    PageArena arena;
    arena.base = (byte)map->base + map->offset; 
    pageAlign(map, arenaSize);
    if(!arena.base) {
        fprintf(stderr, "Arena allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    arena.size = arenaSize;
    arena.offset = 0;
    arena.previous = arena.offset;
    return arena;
}

memptr arenaPageAlloc(PageArena* arena, usize alloc_size, usize alignment) {
    switch(alignment) {
        case ALIGN_1:
        case ALIGN_2:
        case ALIGN_4:
        case ALIGN_8:
        case ALIGN_16:
        case ALIGN_32:
        case ALIGN_64:
        case ALIGN_128:

            if(arena->size < alloc_size || arena->size - arena->offset < alloc_size || arena->size < alignment) {
                fprintf(stderr, "ERROR: allocation request beyond size of arena, return null\n");
                return NULL;
            }

            usize aligned, align_pad, offset_pad;
            align_pad = AlignPad(alloc_size, alignment);
            aligned = align_pad + alloc_size;

            if (arena->offset % alignment != 0) {
                offset_pad = AlignPad(arena->offset, alignment);
                arena->offset += offset_pad;
            }

            if(aligned + arena->offset > arena->size) {
                fprintf(stderr, "ERROR: Arena overflow!\n");
                return NULL;
            }

        memptr ptr = (memptr)(arena->base + arena->offset);
        arena->offset += aligned;
        return ptr;
    }
    fprintf(stderr, "ERROR: Returning null due to unacceptable alignment request on allocation.\n");
    return NULL;
}

void resetPageArena(PageArena* arena) {
    arena->offset = 0;
}

void arenaPagePush(PageArena* arena) {
    arena->previous = arena->offset;
}

void arenaPagePop(PageArena* arena) {
    arena->offset = arena->previous;
}

void destroyPageArena(PageArena* arena) {
    if(arena == NULL) {
        fprintf(stdout, "Arena already NULL, exit function.\n");
        return;
    }
    else { 
        arena->base = NULL;
        arena->size = 0;
        arena->offset = 0;
    }
}
    
    
