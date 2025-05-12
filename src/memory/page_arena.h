#ifndef m_PAGE_ARENA_H
#define m_PAGE_ARENA_H

#include "common_types.h"

typedef struct memMap{
    memptr start;
    memptr structBase;
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

typedef struct {
    byte base;
    usize offset;
    usize size;
    usize previous;
    memMap* parent;
} PageArena;

memMap *initMemMap(usize requestedSize);
void pageAlign(memMap* map, usize arenaSize);
void memMapPush(memMap* m);
void memMapPop(memMap* m);
void releasePages(memMap* map);

PageArena *createPageArena(memMap* map, usize arenaSize);
memptr arenaPageAlloc(PageArena* arena, usize alloc_size, usize alignment);
void resetPageArena(PageArena* arena);
void arenaPagePop(PageArena* arena);
void arenaPagePush(PageArena* arena);
void destroyPageArena(PageArena* arena);

#endif
