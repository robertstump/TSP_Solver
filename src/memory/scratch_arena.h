#ifndef m_SCRATCH_ARENA_H
#define m_SCRATCH_ARENA_H

#include "common_types.h"

typedef struct {
    byte base;
    usize size;
    usize offset;
    usize previous;
} ScratchArena;

ScratchArena createScratchArena(usize arena_size);
void* arenaScratchAlloc(ScratchArena* arena, usize alloc_size, usize alignment);
void resetScratchArena(ScratchArena* arena);
void arenaScratchPush(ScratchArena* arena);
void arenaScratchPop(ScratchArena* arena);
void destroyScratchArena(ScratchArena* arena);

#endif
