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
static int _find_child(struct trie_node *node, char c, 
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
            osal_memcpy(&p, (char *)sub->data_buffer + dq->elem_size * i, sizeof(struct trie_node *));
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

#define build_assert(condition) do {                \
    if (!(condition))                               \
        goto sakuc_build_search_automaton_failed;   \
} while (__LINE__ == -1)

/*  Build the search tree, and return the root node.
    @keywords - keyword list.
    @num - num of keywords in @keywords list.
    @deque_sub_queue_size - 
        In order to perform breath-first-search, the reserved deque (used as fifo) sub-queue size.
 */
int sakuc_multi_pattern_build_search_automaton
        (struct trie_node **root, const char *keywords[], size_t num, size_t deque_sub_queue_size)
{
    *root = _new_trie_node(0);
    if (*root)
        (*root)->failover = *root;
    else
        return -1;
    
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
    build_assert(dq = sakuc_deque_new(deque_sub_queue_size, sizeof(pointer_trie_node_t), 
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
        }
    }
    
    sakuc_deque_destroy(dq);
    
    return 0;
sakuc_build_search_automaton_failed:
    // #mark# if failed due to memory problem, remember to destroy root to prevent memory leak.
    // #todo# ...
    *root = nullptr;
    return -1;
}

/* find node corresponding to @keyword, store the found node into @matched.
 */
int sakuc_multi_pattern_find_node(struct trie_node *root, const char *keyword, struct trie_node **matched)
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
