#ifndef m_SCRATCH_H
#define m_SCRATCH_H

typedef struct {
    char* base;
    size_t size;
    size_t offset;
    size_t previous;
} ScratchArena;

typedef enum AlignType {
    ALIGN_DEFAULT   = 0,
    ALIGN_1         = 1,
    ALIGN_2         = 2,
    ALIGN_4         = 4,
    ALIGN_8         = 8,
    ALIGN_16        = 16,
    ALIGN_32        = 32,
    ALIGN_64        = 64,
    ALIGN_128       = 128
} AlignType;

ScratchArena createArena(size_t arena_size) {
    ScratchArena arena;
    arena.base = malloc(arena_size);
    if(!arena.base) {
        fprintf(stderr, "Arena allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    arena.size = arena_size;
    arena.offset = 0;
    arena.previous = arena.offset;
    return arena;
}

static inline size_t AlignPad(size_t offset, size_t alignment) {
    size_t mod = offset % alignment;
    return (mod == 0) ? 0 : (alignment - mod);
}

void* arenaAlloc(ScratchArena* arena, size_t alloc_size, size_t alignment) {
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

            size_t aligned, align_pad, offset_pad;
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

        void* ptr = (void*)(arena->base + arena->offset);
        arena->offset += aligned;
        return ptr;
    }
    fprintf(stderr, "ERROR: Returning null due to unacceptable alignment request on allocation.\n");
    return NULL;
}

void resetArena(ScratchArena* arena) {
    arena->offset = 0;
}

void arenaPush(ScratchArena* arena) {
    arena->previous = arena->offset;
}

void arenaPop(ScratchArena* arena) {
    arena->offset = arena->previous;
}

void destroyArena(ScratchArena* arena) {
    if(arena == NULL) {
        fprintf(stdout, "Arena already free, exit function.\n");
        return;
    }
    else {
        free(arena->base);
        arena->base = NULL;
        arena->size = 0;
        arena->offset = 0;
    }
}

#endif
