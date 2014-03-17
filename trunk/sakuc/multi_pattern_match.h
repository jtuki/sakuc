#ifndef SAKUC_MULTI_PATTERN_STRING_MATCH_H_
#define SAKUC_MULTI_PATTERN_STRING_MATCH_H_

// Aho-Corasick multi-pattern string matching.

#include "common_defs.h"

// #define SAKUC_DEBUG_ONLY
#ifdef SAKUC_DEBUG_ONLY
#include <stdio.h>
#endif

// a kind of adapted trie node.
typedef struct trie_node {
    struct trie_node *failover;     /* introduced @failover in order to avoid backtrace */
    struct trie_node *next_sibling; /* rightmost sibling node */
    struct trie_node *first_child;  /* all the children are linked with @next_sibling */
    /* @num_keywords:
        eg.
            keywords: "hello" "helloworld" "world" "orld";
            input stream: "here is helloworld";
            expected match: "hello", "world", "orld", "helloworld".
            
        What is worthwhile noticing:
            1. when we reach `d', there are 3 matches!
            2. when we reach first `o', "hello" become our first match, while `o' is not
               the end of "helloworld" keyword.
               
        All the keywords are stored and linked by @failover pointer.
     */
    size_t num_keywords;
    char ch;
    const char *keyword;
} trie_node_t;

typedef struct trie_node *pointer_trie_node_t;

int sakuc_multi_pattern_build_search_automaton
        (struct trie_node **root, const char *keywords[], size_t num, size_t deque_sub_queue_size);
        
int sakuc_multi_pattern_find_node(struct trie_node *root, const char *keyword, struct trie_node **matched);

#endif // SAKUC_MULTI_PATTERN_STRING_MATCH_H_
