typedef struct {
    char* base;
    size_t size;
    size_t offset;
    size_t previous;
    size_t limit;
} PageArena;
