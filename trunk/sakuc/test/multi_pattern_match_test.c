#include "../multi_pattern_match.h"
#include "common_test_defs.h"
#include "multi_pattern_match_test.h"

const char *keywords_list[] = {
    "hello", "world", "orld", "orl", "helloworld"
};
const size_t num_match_keywords = sizeof (keywords_list) / sizeof (keywords_list[0]);

struct match_idx_keyword {
    size_t keyword_idx;
    size_t idx;
};

// ============================================================

// @helloworld@or#^% etc. as input stream for pattern matching.
//  6(hello) 10(orl) 11(helloworld) 11(world) 11(orld) 15(orl)
const char input_stream_simple[] = "@helloworld@orl#^%";
const size_t input_stream_simple_len = sizeof(input_stream_simple) - 1;

struct match_idx_keyword expected_match_simple[] = {
    {.keyword_idx = 0, .idx = 6-1}   ,
    {.keyword_idx = 3, .idx = 10-1}  ,
    {.keyword_idx = 4, .idx = 11-1}  ,
    {.keyword_idx = 1, .idx = 11-1}  ,
    {.keyword_idx = 2, .idx = 11-1}  ,
    {.keyword_idx = 3, .idx = 15-1}  ,
};
const size_t num_expected_match_simple = 
    sizeof(expected_match_simple) / sizeof(expected_match_simple[0]);

const char input_stream[] =
    // 36(hello) - 52 chars in each line.
    "Usually we would love to write hello to the people  "
    // 19(hello) 23(orl) 24(helloworld) 24(world) 24(orld)
    "who can write helloworld program, instead of broken "
    // 11(orl) 18(orl) 19(orld) 39(orl) 40(world) 40(orld)
    "program orl or orld all around the world."
    ;
const size_t input_stream_len = sizeof(input_stream) - 1;

struct match_idx_keyword expected_match[] = {
    {.keyword_idx = 0, .idx = 36-1}       ,
    {.keyword_idx = 0, .idx = 52+19-1}    ,
    {.keyword_idx = 3, .idx = 52+23-1}    ,
    {.keyword_idx = 4, .idx = 52+24-1}    ,
    {.keyword_idx = 1, .idx = 52+24-1}    ,
    {.keyword_idx = 2, .idx = 52+24-1}    ,
    {.keyword_idx = 3, .idx = 52*2+11-1}   ,
    {.keyword_idx = 3, .idx = 52*2+18-1}  ,
    {.keyword_idx = 2, .idx = 52*2+19-1}  ,
    {.keyword_idx = 3, .idx = 52*2+39-1}  ,
    {.keyword_idx = 1, .idx = 52*2+40-1}  ,
    {.keyword_idx = 2, .idx = 52*2+40-1}  ,
};
const size_t num_expected_match = sizeof(expected_match) / sizeof(expected_match[0]);
// ============================================================

int test_multi_pattern_match(void)
{
    // ## test part 1
    struct trie_node *search_db = nullptr;
    
    sakuc_multi_pattern_build_search_automaton(&search_db, keywords_list, num_match_keywords, 10);
    sakuc_assert(search_db && search_db->first_child->ch == 'h');
    
    // ## test part 2 - the tree built process it ok?
    struct trie_node *matched = nullptr;
    struct trie_node *failover = nullptr;
    
    sakuc_multi_pattern_find_node(search_db, "h", &matched);
    sakuc_assert(matched && matched->ch == 'h'
                 && matched->next_sibling
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
                 && matched->keyword == keywords_list[0]
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
    sakuc_assert(matched && matched->ch == 'l' && matched->next_sibling == nullptr
                 && matched->num_keywords == 1
                 && matched->keyword == keywords_list[3]);
    
    sakuc_multi_pattern_find_node(search_db, "orld", &matched);
    sakuc_assert(matched && matched->ch == 'd' && matched->next_sibling == nullptr
                 && matched->num_keywords == 1
                 && matched->keyword == keywords_list[2]);
                 
    sakuc_multi_pattern_find_node(search_db, "world", &matched);
    sakuc_assert(matched && matched->ch == 'd' && matched->next_sibling == nullptr
                 && matched->num_keywords == 2
                 && matched->keyword == keywords_list[1]);
                 
    sakuc_multi_pattern_find_node(search_db, "helloworld", &matched);
    sakuc_assert(matched && matched->ch == 'd' && matched->next_sibling == nullptr
                 && matched->num_keywords == 3
                 && matched->keyword == keywords_list[4]);
                 
    sakuc_multi_pattern_find_node(search_db, "helloworl", &matched);
    sakuc_assert(matched && matched->ch == 'l' && matched->next_sibling == nullptr
                 && matched->num_keywords == 1
                 && matched->keyword == nullptr);
    
    // ## test part 3 - pattern match process ok? - shorter input stream
    size_t pos = 0;
    const char *matched_keyword = nullptr;
    size_t i = 0;
    for (; i < num_expected_match_simple; i++) {
        enum sakuc_mpm_search_mode mode = (i == 0) ?
            SAKUC_MPM_SEARCH_MODE_START : SAKUC_MPM_SEARCH_MODE_CONTINUE;
            
        sakuc_assert(
            sakuc_multi_pattern_search(search_db, mode,
                input_stream_simple, input_stream_simple_len, &pos, &matched_keyword) == 1
            && pos == expected_match_simple[i].idx
            && matched_keyword == keywords_list[expected_match_simple[i].keyword_idx]
        );
    }
    sakuc_assert(i == num_expected_match_simple);
    sakuc_assert(
        sakuc_multi_pattern_search(search_db, SAKUC_MPM_SEARCH_MODE_CONTINUE,
            input_stream_simple, input_stream_simple_len, &pos, &matched_keyword) == 0
    );
    
    // ## test part 3 - pattern match process ok? - longer input stream
    pos = 0;
    matched_keyword = nullptr;
    i = 0;
    for (; i < num_expected_match; i++) {
        enum sakuc_mpm_search_mode mode = (i == 0) ?
            SAKUC_MPM_SEARCH_MODE_START : SAKUC_MPM_SEARCH_MODE_CONTINUE;
            
        sakuc_assert(
            sakuc_multi_pattern_search(search_db, mode,
                input_stream, input_stream_len, &pos, &matched_keyword) == 1
            && pos == expected_match[i].idx
            && matched_keyword == keywords_list[expected_match[i].keyword_idx]
        );
    }
    sakuc_assert(i == num_expected_match);
    sakuc_assert(
        sakuc_multi_pattern_search(search_db, SAKUC_MPM_SEARCH_MODE_CONTINUE,
            input_stream, input_stream_len, &pos, &matched_keyword) == 0
    );
    
    return 0;
sakuc_assert_failed:
    return -1;
}
