#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scratch.h"

#define LINE_BUFFER 1024
#define FALSE 0
#define TRUE 1

#define DM_INDEX(dm, i, j) ((i < j) ? ((dm).rowOffset[i] + ((j) - (i) - 1)) : ((dm).rowOffset[j] + ((i) - (j) - 1)))

typedef float Vec2[2];

typedef struct { 
    float* distances;
    unsigned int* rowOffset;
} DistanceMatrix;

unsigned int CountDataSize(const char *filename) {

    FILE * file = fopen(filename, "r");
    if(!file) {
        fprintf(stderr, "ERROR: File not found!\n");
        return FALSE;
    } else {
        fprintf(stderr, "INFO: Opened file successfully!\n");
    }
    
    char buffer[LINE_BUFFER];
    int found = 0;
    int count = 0;

    while(fgets(buffer, sizeof(buffer), file)) {
        if(!found) {
            if(strncmp(buffer, "NODE_COORD_SECTION", 17) == 0) {
                found = 1;
            }
        } else if (strncmp(buffer, "EOF", 3) == 0) {
            break;
        } else {
            count++;
        }
    }

    fclose(file);
    return count;
}

//float (*LoadDistances(ScratchArena *arena, const char *filename, int size))[2] 
Vec2* LoadDistances(ScratchArena *arena, const char *filename, unsigned int size) {
    FILE *file = fopen(filename, "r");
    if(!file) {
        fprintf(stderr, "ERROR: File not found!\n");
        return NULL;
    } else {
        fprintf(stdout, "INFO: Opened file successfully!\n");
    }

    char buffer[LINE_BUFFER];
    int found = 0;

   Vec2* dist_matrix = arenaAlloc(arena, sizeof(*dist_matrix) * size, ALIGN_4);

    while(fgets(buffer, sizeof(buffer), file)) {
        if(!found) {
            if(strncmp(buffer, "NODE_COORD_SECTION", 17) == 0) {
                found = 1;
            }
        }  else if (strncmp(buffer, "EOF", 3) == 0) {
            break;
        } else {
            int index;
            float x, y;

            if(sscanf(buffer, "%d %f %f", &index, &x, &y) == 3) {
                dist_matrix[index - 1][0] = x;
                dist_matrix[index - 1][1] = y;
            }
        }
    }
    fclose(file);
    return dist_matrix;
}

DistanceMatrix CreateDistanceMatrix(ScratchArena *arena, Vec2* coords, unsigned int count) {
    unsigned int flatSize = ((uint64_t)count * (count - 1)) >> 1;
    DistanceMatrix dm;
    dm.distances = arenaAlloc(arena, (flatSize * sizeof(float)) + sizeof(float), ALIGN_4);
    arenaPush(arena);
    dm.rowOffset = arenaAlloc(arena, sizeof(float) * count, ALIGN_4);
    unsigned int index = 0;
    for(int i = 0; i < count; i++) {
        dm.rowOffset[i] = index;
        for(int j = i + 1; j < count; j++) {
            float dx = coords[j][0] - coords[i][0];
            float dy = coords[j][1] - coords[i][1];
            //flatArray[index] = sqrtf(dx*dx + dy*dy);
            dm.distances[index] = dx*dx + dy*dy;
            index++;
        }
    }
    flatArray[flatSize] = 0;
    return dm;
}

