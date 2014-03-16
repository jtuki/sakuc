/* Multi-pattern string match.
    (prefix-tree method adapted from Aho-Corasick algorithm)
    
    Reference:
    pythonic - http://papercruncher.com/2012/02/26/string-searching-using-aho-corasick/
    wikipedia - http://goo.gl/DtwB2
    
    History:
        2014-3-15 - created by jtuki@foxmail.com
 */

#include "common_memory_management_defs.h"
#include "multi_pattern_match.h"

/* If initialize without ch then use _new_trie_node(0). */
inline static struct trie_node * _new_trie_node(const char c) 
{
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
    *child = *last_child = nullptr;
    if (!node || !last_child)
        return -1;
    
    char found = FALSE;
    struct trie_node *curr_node = node->first_child;
    while (TRUE) {
        if (curr_node->ch == c) {
            found = TRUE;
            break;
        }
        if (curr_node->next_sibling)
            curr_node = curr_node->next_sibling;
        else
            break;
    }
    *last_child = found ? nullptr : curr_node;
    *child = found ? curr_node : nullptr;
    
    return 0;
}

/*  Build the search tree, and return the root node.
    @keywords - keyword list.
    @num - num of keywords in @keywords list.
    @deque_sub_queue_size - 
        In order to perform breath-first-search, the reserved deque (used as fifo) sub-queue size.
 */
int sakuc_build_multi_pattern_search_automaton
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
                new_node = _new_trie_node(keyword[j]);
                if (new_node)
                    new_node->failover = *root;
                else
                    goto sakuc_build_search_automaton_failed;
                
                if (last_child == nullptr)
                    current_node->first_child = new_node;
                else
                    last_child->next_sibling = new_node;
            }
            
            current_node = new_node;
        }
        
        current_node->is_keyword = TRUE;
        current_node->keyword = keyword; // const char *keyword within param @keywords
    }
    
    // build the failover relationship based on the tree built just now, using BFS.
    
    return 0;
sakuc_build_search_automaton_failed:
    // #mark# if failed due to memory problem, remember to destroy root to prevent memory leak.
    // #todo# ...
    *root = nullptr;
    return -1;
}
