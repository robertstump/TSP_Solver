#ifndef m_PAGE_ARENA_H
#define m_PAGE_ARENA_H

#include "common_types.h"

struct PageArena;

typedef struct memMap{
    memptr start;
    memptr structBase;
    struct PageArena* arenaCurrent;
    byte base;
    usize offset;
    usize previous;
    usize limit;
    usize size;
    usize pageSize;
    usize selfSize;
    u32 arenaCount;
    u32 _pad;
} memMap;

typedef struct PageArena{
    memMap* parent;
    struct PageArena* arenaPrevious;
    byte base;
    usize offset;
    usize size;
    usize previous;
} PageArena;

memMap *initMemMap(usize requestedSize);
void pageAlign(memMap* map, usize arenaSize);
void releasePages(memMap* map);

PageArena *createPageArena(memMap* map, usize arenaSize);
memptr arenaPageAlloc(PageArena* arena, usize alloc_size, usize alignment);
void arenaPagePop(memMap* map);

#endif
