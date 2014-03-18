/* Multi-pattern string match.
    (prefix-tree method adapted from Aho-Corasick algorithm)
    
    Reference:
    pythonic - http://papercruncher.com/2012/02/26/string-searching-using-aho-corasick/
    wikipedia - http://goo.gl/DtwB2
    
    History:
        2014-3-15 - created by jtuki@foxmail.com
 */

#include "common_memory_management_defs.h"
#include "deque.h"
#include "multi_pattern_match.h"

/* If initialize without ch then use _new_trie_node(0). */
inline static struct trie_node * _new_trie_node(const char c)
{
    // malloc with all zeros - calloc.
    trie_node_t *node = (trie_node_t *) osal_mem_calloc(1, sizeof(trie_node_t));
    if (!node)
        return nullptr;
    if (c != 0)
        node->ch = c;
    return node;
}

/*  Find the child with @trie_node_t.ch as @c.

    If not found, @last_child is set to the last child (or set to nullptr if
    there is no any children, or found).
 */
static int _find_child(const struct trie_node *node, char c, 
                       struct trie_node **child, struct trie_node **last_child)
{
    if (!node)
        return -1;
    
    *child = nullptr;
    if (last_child)
        *last_child = nullptr;
    
    char found = FALSE;
    struct trie_node *curr_node = node->first_child;
    while (curr_node != nullptr) {
        if (curr_node->ch == c) {
            found = TRUE;
            break;
        }
        if (curr_node->next_sibling)
            curr_node = curr_node->next_sibling;
        else
            break;
    }
    if (last_child)
        *last_child = found ? nullptr : curr_node;
    *child = found ? curr_node : nullptr;
    
    return 0;
}

// #define SAKUC_DEBUG_ONLY

#ifdef SAKUC_DEBUG_ONLY
// deque contains pointers to trie_node.
static int _print_deque_for_debug_only(struct sakuc_deque *dq)
{
    if (!dq || dq->elem_size != sizeof (struct trie_node *))
        return -1;
        
    struct sakuc_sub_deque_ *sub = dq->dq_head;
    struct trie_node *p = nullptr;
    
    printf("deque size %d - ", sakuc_deque_size(dq));
    while (sub != nullptr) {
        for (size_t i=sub->start; i < sub->end; i++) {
            p = ((struct trie_node **)sub->data_buffer)[i];
            printf ("%c ", p->ch == 0 ? '@' : p->ch); // '@' denote the root node.
        }
        sub = sub->next;
    }
    printf ("\n");
    
    return 0;
}
#else
#define _print_deque_for_debug_only(whatever)
#endif // SAKUC_DEBUG_ONLY

#define build_assert(condition) do {         \
    if (!(condition))                        \
        goto sakuc_build_automaton_failed;   \
} while (__LINE__ == -1)

/*  Build the search tree, and return the root node.
    @keywords - keyword list.
    @num - num of keywords in @keywords list.
    @fifo_init_size - 
        In order to perform breath-first-search, the reserved fifo's initial size.
 */
int sakuc_multi_pattern_build_search_automaton
        (struct trie_node **root, const char *keywords[], size_t num, size_t fifo_init_size)
{
    build_assert(*root = _new_trie_node(0));
    (*root)->failover = *root;
    
    const char *keyword = nullptr;
    struct trie_node *current_node = nullptr;
    
    // build the tree based on prefix.
    for (size_t i=0; i < num; i++) {
        keyword = keywords[i]; current_node = *root;
        
        struct trie_node *new_node = nullptr;
        struct trie_node *last_child = nullptr;
        
        // handle the @keyword, "merge" it into the search tree
        for (size_t j=0; j < strlen(keyword); j++) {
            _find_child(current_node, keyword[j], &new_node, &last_child);
        
            if (new_node == nullptr) {
                build_assert(new_node = _new_trie_node(keyword[j]));
                // notice: all the newly created node's @failover is set to root by default.
                new_node->failover = *root;
                
                if (last_child == nullptr)
                    current_node->first_child = new_node;
                else
                    last_child->next_sibling = new_node;
            }
            current_node = new_node;
        }
        
        current_node->num_keywords = 1;  // at most 1 currently, when building failover relationship,
                                         // @num_keywords might be increased.
        current_node->keyword = keyword; // const char *keyword within param @keywords
    }
    
    // build the failover relationship based on the tree built just now, using BFS.
    
    struct sakuc_deque *dq = nullptr;
    build_assert(dq = sakuc_deque_new(fifo_init_size, sizeof(pointer_trie_node_t), 
                                      SAKUC_DEQUE_SWEEP_MANUALLY));
    build_assert(sakuc_deque_push_back(dq, root, sizeof (pointer_trie_node_t)) == 0);
    _print_deque_for_debug_only(dq);
    
    while (sakuc_deque_size(dq)) {
        build_assert(sakuc_deque_pop_front(dq, &current_node, sizeof(pointer_trie_node_t)) == 0);
        _print_deque_for_debug_only(dq);
        
        struct trie_node *child = current_node->first_child;
        struct trie_node *curr_failover = nullptr; // cannot be nullptr, default to be root node.
        
        for (; child != nullptr; child = child->next_sibling) {
            // push_back into the queue, then process the child node.
            build_assert(sakuc_deque_push_back(dq, &child, sizeof(pointer_trie_node_t)) == 0);
            _print_deque_for_debug_only(dq);
            
            struct trie_node *x = nullptr;
            do {
                curr_failover = (curr_failover == nullptr) ? 
                                current_node->failover : curr_failover->failover;
            
                build_assert(_find_child(curr_failover, child->ch, &x, nullptr) == 0);
                if (x && x != child) {
                    child->failover = x; // @failover changed from the default root node, to x node.
                    if (x->num_keywords > 0) {
                        child->num_keywords += x->num_keywords;
                    }
                    break;
                }
            } while (curr_failover != *root);
            
            curr_failover = nullptr;
        }
    }
    
    sakuc_deque_destroy(dq);
    
    return 0;
sakuc_build_automaton_failed:
    if (dq)
        sakuc_deque_destroy(dq);
    if (*root)
        sakuc_multi_pattern_destroy_search_automaton(*root, fifo_init_size);
    *root = nullptr;
    return -1;
}

#undef build_assert

/* find node corresponding to @keyword, store the found node into @matched.
 */
int sakuc_multi_pattern_find_node(struct trie_node *root, const char *keyword, 
                                  struct trie_node **matched)
{
    if (!root || !keyword || !matched)
        return -1;
    *matched = nullptr;
    
    struct trie_node *curr_node = root;
    struct trie_node *child = nullptr;
    
    size_t i = 0;
    for (; i < strlen(keyword); i++) {
        _find_child(curr_node, keyword[i], &child, nullptr);
        if (child == nullptr)
            break;
        else {
            curr_node = child;
        }
    }
    
    if (i == strlen(keyword))
        *matched = curr_node;
    return 0;
}

// only used in function @sakuc_multi_pattern_search
#define BEGIN_KEYWORD_ITERATE() static int _continue = FALSE

#define CONTINUE_LAST_ITER() do {         \
    _continue = TRUE;                     \
    goto sakuc_mpm_search_iter_continue;  \
} while (__LINE__ == -1)

#define NOT_CONTINUE_LAST_ITER() _continue = FALSE

#define ITER_TO_BE_CONTINUED() do {    \
sakuc_mpm_search_iter_continue:        \
    if (!_continue)                    \
        return 1;                      \
} while (__LINE__ == -1)

/* iterative search:
    # If @search_mode is SAKUC_MPM_SEARCH_MODE_START, begin a totally new search
    from @input stream (with length @len).
    # Else if @search_mode is SAKUC_MPM_SEARCH_MODE_CONTINUE, start from the position
    performed last time.
    
    Return value:
    #  1 - "matched @keyword at position @matched_pos_suffix" and have not yet gone through 
    the @input stream (continue next function invoke).
    #  0 - "nothing matched" and have gone through the @input stream.
    # -1 - some error occured. eg. @search_mode is "continue", however, @input is not
    the same with previous @input stream or @len is not the same with previous @len etc.
 */
int sakuc_multi_pattern_search(const struct trie_node *search_db,
                               enum sakuc_mpm_search_mode search_mode,
                               const char *input, size_t len, 
                               size_t *matched_pos_suffix, const char **matched_keyword)
{
    static const struct trie_node *curr_automaton = nullptr;
    static const struct trie_node *curr_node = nullptr;
    static const char *curr_input = nullptr;
    static size_t curr_len = 0;
    static size_t curr_search_pos = 0;
    
    // during last search, how many keywords still remains.
    static size_t curr_remain_keywords = 0;
    static const struct trie_node *curr_keyword_node = nullptr;
    
    switch (search_mode) {
    case SAKUC_MPM_SEARCH_MODE_START:
        if (!input || len == 0)
            return -1;
        curr_automaton = search_db;
        curr_node = search_db;
        
        curr_input = input;
        curr_len = len; 
        curr_search_pos = 0;
        
        curr_remain_keywords = 0;
        curr_keyword_node = nullptr;
        break;
    case SAKUC_MPM_SEARCH_MODE_CONTINUE:
        // check if the same input stream && the same automaton @search_db.
        if (search_db != curr_automaton || input != curr_input || len != curr_len)
            return -1;
        if (curr_search_pos >= len)
            return 0;
        break;
    default:
        return -1;
    }

    BEGIN_KEYWORD_ITERATE();
    struct trie_node *transition = nullptr;
    if (curr_remain_keywords > 0)
        CONTINUE_LAST_ITER();
        
    while (curr_search_pos < curr_len) {
        _find_child(curr_node, curr_input[curr_search_pos], &transition, nullptr);
        if (transition) {
            curr_node = transition;
            if (curr_node->num_keywords > 0) {
                // return all the keywords ONE-BY-ONE (yield-continue paradigm).
                // refer to @trie_node_t.num_keywords definition.
                curr_keyword_node = curr_node;
                curr_remain_keywords = curr_keyword_node->num_keywords;
                
                for (; curr_remain_keywords > 0; --curr_remain_keywords) {
                    while (curr_keyword_node->keyword == nullptr
                           && curr_keyword_node != curr_automaton)
                        // refer to @trie_node_t.num_keywords definition.
                        curr_keyword_node = curr_keyword_node->failover;
                    
                    if (curr_keyword_node->keyword) {
                        // not continue last iter, but return 1 for next iteration.
                        NOT_CONTINUE_LAST_ITER();
                        
                        *matched_keyword = curr_keyword_node->keyword;
                        *matched_pos_suffix = curr_search_pos;
                        
                        ITER_TO_BE_CONTINUED();
                        curr_keyword_node = curr_keyword_node->failover;
                    }
                    else {
                        // curr_keyword_node == curr_automaton && curr_remain_keywords > 0
                        // It's a _impossible_ condition. Should never fall here.
                        return -1;
                    }
                }
            }
            ++ curr_search_pos;
        }
        else {
            curr_node = curr_node->failover;
            if (curr_node == curr_automaton)
                ++ curr_search_pos;
        }
    }
    
    return 0;
}

#undef CONTINUE_LAST_YIELD
#undef YIELD_TO_BE_CONTINUED

#define destroy_assert(condition) do {              \
    if (!(condition))                               \
        goto sakuc_destroy_automaton_failed; \
} while (__LINE__ == -1)

int sakuc_multi_pattern_destroy_search_automaton
    (struct trie_node *root, size_t fifo_init_size)
{
    if (!root || fifo_init_size == 0)
        return -1;
    
    struct sakuc_deque *dq = sakuc_deque_new(fifo_init_size, sizeof(struct trie_node *),
                                             SAKUC_DEQUE_SWEEP_MANUALLY);
    destroy_assert(dq);
    destroy_assert(sakuc_deque_push_back(dq, &root, sizeof(void *)) == 0);
    
    struct trie_node *curr_node = nullptr;
    struct trie_node *child = nullptr;
    
    while (sakuc_deque_size(dq)) {
        destroy_assert(sakuc_deque_pop_front(dq, &curr_node, sizeof(void *)) == 0);
        destroy_assert(curr_node);
        
        child = curr_node->first_child;
        while (child) {
            destroy_assert(sakuc_deque_push_back(dq, &child, sizeof(void *)) == 0);
            child = child->next_sibling;
        }
        
        osal_mem_free(curr_node);
    }
    
    sakuc_deque_destroy(dq);
    return 0;
    
sakuc_destroy_automaton_failed:
    if (dq)
        sakuc_deque_destroy(dq);
    return -1;
}

#undef destroy_assert
