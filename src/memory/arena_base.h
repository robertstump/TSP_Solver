#ifndef m_ARENA_BASE_H
#define m_ARENA_BASE_H

#include "common_types.h"

static inline usize AlignPad(usize offset, usize alignment);

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
 
static inline usize AlignPad(usize offset, usize alignment) {
    usize mod = offset % alignment;
    return (mod == 0) ? 0 : (alignment - mod);
}

#endif
