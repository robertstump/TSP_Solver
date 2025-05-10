#ifndef m_SCRATCH_ARENA_H
#define m_SCRATCH_ARENA_H

typedef struct {
char* base;
    size_t size;
    size_t offset;
    size_t previous;
} ScratchArena;

ScratchArena createScratchArena(size_t arena_size);
void* arenaScratchAlloc(ScratchArena* arena, size_t alloc_size, size_t alignment);
void resetScratchArena(ScratchArena* arena);
void arenaScratchPush(ScratchArena* arena);
void arenaScratchPop(ScratchArena* arena);
void destroyScratchArena(ScratchArena* arena);

#endif
