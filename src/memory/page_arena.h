#ifndef m_PAGE_ARENA_H
#define m_PAGE_ARENA_H

#include "common_types.h"

typedef struct {
    usize totalSize;
    memptr start;
    memptr base;
    usize offset;
    usize previous;
    usize limit;
    usize pageSize;
} memMap;

typedef struct {
    byte base;
    usize offset;
    usize size;
    usize previous;
    usize limit;
} PageArena;

memMap reservePages(usize requestedSize);
void pageAlign(memMap* map, usize arenaSize);
void memMapPush(memMap* m);
void memMapPop(memMap* m);
void releastPages(memMap* map);

PageArena createPageArena(memMap* map, usize arenaSize);
memptr arenaPageAlloc(PageArena* arena, usize alloc_size, usize alignment);
void resetPageArena(PageArena* arena);
void arenaPagePop(PageArena* arena);
void arenaPagePush(PageArena* arena);
void destroyPageArena(PageArena* arena);

#endif
