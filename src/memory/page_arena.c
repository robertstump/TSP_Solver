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
        .arenaCurrent = NULL,
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
        return;
    }
    LOG_ERROR("ARENA OVERFLOW on offset alginment : %d Arena", map->arenaCount);
}

void releasePages(memMap* map) {
    munmap(map->start, map->limit);
}

PageArena* createPageArena(memMap* map, usize arenaSize) {
    if(arenaSize > map->size - map->offset) {
        LOG_ERROR("Arena Requested more than available memory");
        return NULL;
    }
    byte arenaBase = (byte)map->base + map->offset; 
    pageAlign(map, arenaSize);
    if(!arenaBase) {
        LOG_ERROR("Arena allocation failed.");
        return NULL;
    }
    PageArena tmp;
    tmp.size = arenaSize;
    tmp.offset = 0;
    tmp.previous = tmp.offset;
    tmp.parent = map;
    byte structBase;
    if(tmp.parent->arenaCount < 1) {
        structBase = tmp.parent->structBase + sizeof(memMap);
    } else {
        structBase = tmp.parent->structBase + sizeof(memMap) + tmp.parent->arenaCount * sizeof(PageArena);
    }
    memcpy(structBase, &tmp, sizeof(PageArena));
    tmp.parent->arenaCount++;
    PageArena* arena = (PageArena*)structBase;
    arena->base = arenaBase;
    map->previous = map->offset;
    if(map->arenaCurrent) {
        arena->arenaPrevious = map->arenaCurrent;
    } else {
        arena->arenaPrevious = NULL;
    }
    map->arenaCurrent = arena;
    return arena;
}

memptr arenaPageAlloc(PageArena* arena, usize alloc_size, usize alignment) {
    if(alloc_size < 1) {
        LOG_ERROR("request 0 bit allocation, return NULL");
        return NULL;
    }
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

            usize offset_pad;
            offset_pad = AlignPad(arena->offset, alignment);
            arena->offset += offset_pad;

            if(alloc_size + arena->offset > arena->size) {
                LOG_ERROR("ERROR: Arena overflow!");
                return NULL;
            }

            memptr ptr = (memptr)(arena->base + arena->offset);
            arena->offset += alloc_size;
            return ptr;
    }
    LOG_ERROR("Returning null due to unacceptable alignment request on allocation.");
    return NULL;
}

void arenaPagePop(memMap* map) {
    if(map->arenaCurrent) {
        PageArena* current = map->arenaCurrent;
        current->base = NULL;
        current->size = 0;
        current->offset = 0;

        map->arenaCurrent = current->arenaPrevious;
        map->arenaCount--;
        map->offset = map->previous;

    } else {
        LOG_ERROR("Current arena NULL on pop request");
        //exit(EXIT_FAILURE);
    }
}
