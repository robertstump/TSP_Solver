#include "trie.h"
#include "arena_base.h"
#include "page_arena.h"

TrieNode* searchChildByLabel(TrieNode* node, u16 label) { 
    TrieChild* children = node->children;
    u16 left = 0;
    u16 right = node->childCount;

    while (left < right) {
        u16 mid = left + (right - left) / 2;
        if(children[mid].label == label) {
            return children[mid].child;
        }
        if(children[mid].label < label) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return NULL;
}

void insertChildSorted(PageArena* arena, TrieNode* node, u16 label, TrieNode* child) {
    usize newSize = sizeof(TrieChild) * (node->childCount + 1);
    TrieChild* newChildren = arenaPageAlloc(arena, newSize, ALIGN_8);

    u16 i = 0, j = 0;
    while (i < node->childCount && node->children[i].label < label) {
        newChildren[j++] = node->children[i++];
    }

    newChildren[j++] = (TrieChild){ .label = label, .child = child };

    while (i < node->childCount) {
        newChildren[j++] = node->children[i++];
    }

    node->children = newChildren;
    node->childCount += 1;
}

TrieNode* insertTrie(PageArena* arena, TrieNode* root, u16* path, u16 pathLen, memptr lutPtr) {
    TrieNode* node = root;

    for (u16 i = 0; i < pathLen; ++i) {
        u16 label = path[i];

        TrieNode* next = searchChildByLabel(node, label);
        if(!next) {
            next = arenaPageAlloc(arena, sizeof(TrieNode), ALIGN_8);
            *next = (TrieNode) { 
                .label = label,
                .isTerminal = 0,
                .childCount = 0,
                .children = NULL,
                .lutPtr = NULL
            };
            insertChildSorted(arena, node, label, next);
        }
        node = next;
    }
    node->isTerminal = 1;
    node->lutPtr = lutPtr;
    return node;
}


TrieNode* searchTrie(TrieNode* root, u16* path, u16 pathLen) {
    TrieNode* node = root;

    for(u16 i = 0; i < pathLen; ++i) {
        node = searchChildByLabel(node, path[i]);
        if (!node) return NULL;
    }
    return (node->isTerminal) ? node : NULL;
}
