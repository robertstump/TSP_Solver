#ifndef tsp_TRIE_H
#define tsp_TRIE_H

#include "common_types.h"
#include "page_arena.h"

struct TrieNode;
struct TrieChild;

typedef struct TrieNode{
    struct TrieChild* children;
    memptr lutPtr;
    u16 label;
    u16 childCount;
    u8 isTerminal;
} TrieNode;

typedef struct TrieChild {
    u16 label;
    TrieNode* child;
} TrieChild;

TrieNode* searchChildByLabel(TrieNode* node, u16 label);
void insertChildSorted(PageArena* arena, TrieNode* node, u16 label, TrieNode* child);
TrieNode* insertTrie(PageArena* arena, TrieNode* root, u16* path, u16 pathLen, memptr lutPtr);
TrieNode* searchTrie(TrieNode* root, u16* path, u16 pathLen);

#endif
