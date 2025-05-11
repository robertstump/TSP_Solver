#ifndef tsp_DIST_MATRIX_H
#define tsp_DIST_MATRIX_H

#include "common_types.h"
#include "scratch_arena.h"

#define DM_INDEX(dm, i, j) ((i < j) ? ((dm).rowOffset[i] + ((j) - (i) - 1)) : ((dm).rowOffset[j] + ((i) - (j) - 1)))

typedef f32 Vec2[2];

typedef struct {
    f32* distances;
    u32* rowOffset;
} DistanceMatrix;

u32 CountDataSize(const char *filename);
Vec2* LoadDistances(ScratchArena *arena, const char *filename, u32 size);
DistanceMatrix CreateDistanceMatrix(ScratchArena *arena, Vec2* coords, u32 count);

#endif
