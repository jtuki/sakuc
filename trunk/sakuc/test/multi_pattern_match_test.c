#include "../multi_pattern_match.h"
#include "common_test_defs.h"
#include "multi_pattern_match_test.h"

const char *match_keywords[] = {
    "hello", "world", "orld", "helloworld"
};

const size_t num_match_keywords = sizeof (match_keywords) / sizeof (match_keywords[0]);

int test_multi_pattern_match(void)
{
    // ## test part 1
    struct trie_node *search_db = nullptr;
    
    sakuc_multi_pattern_build_search_automaton(&search_db, match_keywords, num_match_keywords, 10);
    sakuc_assert(search_db && search_db->first_child->ch == 'h');
    
    // ## test part 2 - the tree built process it ok?
    struct trie_node *matched = nullptr;
    struct trie_node *failover = nullptr;
    
    sakuc_multi_pattern_find_node(search_db, "h", &matched);
    sakuc_assert(matched && matched->ch == 'h' && matched->next_sibling
                 && matched->next_sibling->ch == 'w'
                 && matched->next_sibling->next_sibling
                 && matched->next_sibling->next_sibling->ch == 'o');
    
    sakuc_multi_pattern_find_node(search_db, "he", &matched);
    sakuc_assert(matched && matched->ch == 'e' && matched->next_sibling == nullptr
                 && matched->num_keywords == 0);
    
    sakuc_multi_pattern_find_node(search_db, "hel", &matched);
    sakuc_assert(matched && matched->ch == 'l' && matched->next_sibling == nullptr);
    
    sakuc_multi_pattern_find_node(search_db, "hello", &matched);
    sakuc_assert(matched && matched->ch == 'o' && matched->num_keywords == 1
                 && matched->next_sibling == nullptr);
                 
    sakuc_multi_pattern_find_node(search_db, "hellow", &matched);
    sakuc_multi_pattern_find_node(search_db, "w", &failover);
    sakuc_assert(matched && matched->ch == 'w' && matched->num_keywords == 0
                 && matched->next_sibling == nullptr
                 && matched->failover == failover);
    
    sakuc_multi_pattern_find_node(search_db, "w", &matched);
    sakuc_assert(matched && matched->ch == 'w' && matched->next_sibling
                 && matched->next_sibling->ch == 'o');
    
    sakuc_multi_pattern_find_node(search_db, "wo", &matched);
    sakuc_multi_pattern_find_node(search_db, "o", &failover);
    sakuc_assert(matched && matched->ch == 'o' && matched->next_sibling == nullptr
                 && matched->failover == failover);
    
    sakuc_multi_pattern_find_node(search_db, "orl", &matched);
    sakuc_assert(matched && matched->ch == 'l' && matched->next_sibling == nullptr);
    
    sakuc_multi_pattern_find_node(search_db, "orld", &matched);
    sakuc_assert(matched && matched->ch == 'd' && matched->next_sibling == nullptr
                 && matched->num_keywords == 1);
                 
    sakuc_multi_pattern_find_node(search_db, "world", &matched);
    sakuc_assert(matched && matched->ch == 'd' && matched->next_sibling == nullptr
                 && matched->num_keywords == 2);
                 
    sakuc_multi_pattern_find_node(search_db, "helloworld", &matched);
    sakuc_assert(matched && matched->ch == 'd' && matched->next_sibling == nullptr
                 && matched->num_keywords == 3);
    
    return 0;
sakuc_assert_failed:
    return -1;
}
