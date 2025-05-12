#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include "arena_base.h"
#include "page_arena.h"

memMap* initMemMap(usize requestedSize) {
    if (requestedSize < 1) {
        LOG_ERROR("Allocation must be non-zero positive integer");
        return NULL;
    }

    usize pageSize = sysconf(_SC_PAGESIZE);
    usize remainder = requestedSize % pageSize;
    usize pageOffset = (remainder == 0) ? 0 : (pageSize - remainder);
    usize usableSize = requestedSize + pageOffset;
    usize total = usableSize + 4 * pageSize;

    memptr raw = mmap(NULL, total, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw == MAP_FAILED) {
        LOG_ERROR("FAILED TO MAP MEMORY PAGES, size: %zu", total);
        return NULL;
    }

    byte structBase = (byte)raw + pageSize;
    mprotect(structBase, pageSize, PROT_READ | PROT_WRITE);
    byte usableStart = (byte)raw + pageSize * 3;
    mprotect(usableStart, usableSize, PROT_READ | PROT_WRITE);

    memMap tmp = { 
        .start = raw,
        .base = usableStart,
        .structBase = NULL,
        .limit = total,
        .pageSize = pageSize,
        .offset = 0,
        .previous = 0,
        .size = usableSize,
        .arenaCount = 0
    };

    memcpy(structBase, &tmp, sizeof(memMap));
    memMap* map = (memMap*)structBase;
    map->structBase = map;
    memset(usableStart, 0, 1);
 
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
}

PageArena createPageArena(memMap* map, usize arenaSize) {
    //TODO: Refactor to store PageArena at structBase of arena.parent
    PageArena arena;
    arena.base = (byte)map->base + map->offset; 
    pageAlign(map, arenaSize);
    if(!arena.base) {
        LOG_ERROR("Arena allocation failed.");
        exit(EXIT_FAILURE);
    }
    arena.size = arenaSize;
    arena.offset = 0;
    arena.previous = arena.offset;
    arena.parent = map;
    arena.parent.arenaCount++;
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
                LOG_ERROR("allocation request beyond size of arena, return null");
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
                LOG_ERROR("ERROR: Arena overflow!");
                return NULL;
            }

        memptr ptr = (memptr)(arena->base + arena->offset);
        arena->offset += aligned;
        return ptr;
    }
    LOG_ERROR("Returning null due to unacceptable alignment request on allocation.");
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
        LOG_ERROR("Arena already NULL, exit function.");
        return;
    }
    else { 
        arena->base = NULL;
        arena->size = 0;
        arena->offset = 0;
        if (arena->parent->arenaCount > 0) {
            arena->parent->areanCount--;
        }
    }
}
    
    
