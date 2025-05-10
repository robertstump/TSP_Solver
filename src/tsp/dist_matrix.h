#ifndef tsp_DIST_MATRIX_H
#define tsp_DIST_MATRIX_H

#define DM_INDEX(dm, i, j) ((i < j) ? ((dm).rowOffset[i] + ((j) - (i) - 1)) : ((dm).rowOffset[j] + ((i) - (j) - 1)))

typedef float Vec2[2];

typedef struct {
    float* distances;
    unsigned int* rowOffset;
} DistanceMatrix;

unsigned int CountDataSize(const char *filename);
Vec2* LoadDistances(ScratchArena *arena, const char *filename, unsigned int size);
DistanceMatrix CreateDistanceMatrix(ScratchArena *arena, Vec2* coords, unsigned int count);

#endif
