#ifndef SAKUC_MULTI_PATTERN_STRING_MATCH_H_
#define SAKUC_MULTI_PATTERN_STRING_MATCH_H_

// Aho-Corasick multi-pattern string matching.

#include "common_defs.h"

// a kind of adapted trie node.
typedef struct trie_node {
    struct trie_node *next_sibling; /* rightmost sibling node */
    struct trie_node *first_child;  /* all the children are linked with @next_sibling */
    struct trie_node *failover;     /* introduced @failover in order to avoid backtrace */
    char ch;
    char is_keyword;                /* boolean value: TRUE or FALSE */
    const char *keyword;
} trie_node_t;

#endif // SAKUC_MULTI_PATTERN_STRING_MATCH_H_
