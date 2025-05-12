#include <stdio.h>
#include <stdlib.h>
#include "arena_base.h"
#include "scratch_arena.h"

ScratchArena createScratchArena(usize arena_size) {
    ScratchArena arena;
    arena.base = malloc(arena_size);
    if(!arena.base) {
        LOG_ERROR("Arena allocation failed.");
        exit(EXIT_FAILURE);
    }
    arena.size = arena_size;
    arena.offset = 0;
    arena.previous = arena.offset;
    return arena;
}

void* arenaScratchAlloc(ScratchArena* arena, usize alloc_size, usize alignment) {
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
                LOG_ERROR("ERROR: allocation request beyond size of arena, return null");
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

void resetScratchArena(ScratchArena* arena) {
    arena->offset = 0;
}

void arenaScratchPush(ScratchArena* arena) {
    arena->previous = arena->offset;
}

void arenaScratchPop(ScratchArena* arena) {
    arena->offset = arena->previous;
}

void destroyScratchArena(ScratchArena* arena) {
    if(arena == NULL) {
        LOG_ERROR("Arena already free, exit function.");
        return;
    }
    else {
        free(arena->base);
        arena->base = NULL;
        arena->size = 0;
        arena->offset = 0;
    }
}
