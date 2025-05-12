#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dist_matrix.h"
#include "arena_base.h"
#include "scratch_arena.h"

#define LINE_BUFFER 1024

u32 CountDataSize(const char *filename) {

    FILE * file = fopen(filename, "r");
    if(!file) {
        LOG_ERROR("File not found!");
        return EXIT_FAILURE;
    } else {
        LOG_ERROR("INFO: Opened file successfully!");
    }
    
    char buffer[LINE_BUFFER];
    s32 found = 0;
    s32 count = 0;

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
Vec2* LoadDistances(ScratchArena *arena, const char *filename, u32 size) {
    FILE *file = fopen(filename, "r");
    if(!file) {
        LOG_ERROR("File not found!");
        return NULL;
    } else {
        LOG_ERROR("INFO: Opened file successfully!");
    }

    char buffer[LINE_BUFFER];
    s32 found = 0;

   Vec2* dist_matrix = arenaScratchAlloc(arena, sizeof(*dist_matrix) * size, ALIGN_4);

    while(fgets(buffer, sizeof(buffer), file)) {
        if(!found) {
            if(strncmp(buffer, "NODE_COORD_SECTION", 17) == 0) {
                found = 1;
            }
        }  else if (strncmp(buffer, "EOF", 3) == 0) {
            break;
        } else {
            s32 index;
            f32 x, y;

            if(sscanf(buffer, "%d %f %f", &index, &x, &y) == 3) {
                dist_matrix[index - 1][0] = x;
                dist_matrix[index - 1][1] = y;
            }
        }
    }
    fclose(file);
    return dist_matrix;
}

DistanceMatrix CreateDistanceMatrix(ScratchArena *arena, Vec2* coords, u32 count) {
    u32 flatSize = ((u32)count * (count - 1)) >> 1;
    DistanceMatrix dm;
    arenaScratchPush(arena);
    dm.distances = arenaScratchAlloc(arena, (flatSize * sizeof(f32)) + sizeof(f32), ALIGN_4);
    dm.rowOffset = arenaScratchAlloc(arena, sizeof(f32) * count, ALIGN_4);
    u32 index = 0;
    for(u32 i = 0; i < count; i++) {
        dm.rowOffset[i] = index;
        for(u32 j = i + 1; j < count; j++) {
            f32 dx = coords[j][0] - coords[i][0];
            f32 dy = coords[j][1] - coords[i][1];
            //flatArray[index] = sqrtf(dx*dx + dy*dy);
            dm.distances[index] = dx*dx + dy*dy;
            index++;
        }
    }
    return dm;
}

