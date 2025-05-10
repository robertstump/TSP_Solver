#ifndef m_ARENA_BASE_H
#define m_ARENA_BASE_H

static inline size_t AlignPad(size_t offset, size_t alignment);

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
 
static inline size_t AlignPad(size_t offset, size_t alignment) {
    size_t mod = offset % alignment;
    return (mod == 0) ? 0 : (alignment - mod);
}

#endif
