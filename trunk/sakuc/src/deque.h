// 2014-3-15 - created by jtuki@foxmail.com

#ifndef SAKUC_DEQUE_H_
#define SAKUC_DEQUE_H_

#include "common_defs.h"

struct sakuc_sub_deque_
{
    struct sakuc_sub_deque_ *next;
    struct sakuc_sub_deque_ *prev;
    size_t length;  // current length of queue
    
    // below are two invariants.
    size_t start;   // _always_ the next place to be #pop_front if (start < end).
                    // as to #push_front, decrement start first if start > 0, then copy the value there.
    size_t end;     // _always_ the next place to be #push_back if (end < sakuc_deque_t.sub_deque_size).
                    // as to #pop_back, decrement end first if end > 0, then pop the value there.
    
    // with length of (sakuc_deque.sub_deque_size * sakuc_deque.elem_size)
    void *data_buffer;
};

// behavior bit-vector, 0x0001 0x0080 etc.
#define SAKUC_DEQUE_SWEEP_IMMEDIATELY   0x0001
#define SAKUC_DEQUE_SWEEP_MANUALLY      0x0002
#define SAKUC_DEQUE_SWEEP_HEAD          0x0004
#define SAKUC_DEQUE_SWEEP_TAIL          0x0008

typedef struct sakuc_deque
{
    struct sakuc_sub_deque_ *dq_head;
    struct sakuc_sub_deque_ *dq_tail;

    int behavior;           // OR-ed behavior bit-vector.
    size_t sub_deque_size;  // how many elements are there within each sub-queue.
    size_t elem_size;       // sizeof each element.

    size_t length;
    size_t num_sub_deque;
} sakuc_deque_t;

/*
    behavior:
        SAKUC_DEQUE_SWEEP_IMMEDIATELY
        SAKUC_DEQUE_SWEEP_MANUALLY   
        SAKUC_DEQUE_SWEEP_HEAD       
        SAKUC_DEQUE_SWEEP_TAIL        
 */
extern struct sakuc_deque *
sakuc_deque_new(size_t sub_deque_size, size_t elem_size, int behavior);

extern int sakuc_deque_push_back(struct sakuc_deque *dq, const void *data, size_t len);

extern int sakuc_deque_push_front(struct sakuc_deque *dq, const void *data, size_t len);

extern int sakuc_deque_pop_back(struct sakuc_deque *dq, void *data, size_t len);

extern int sakuc_deque_pop_front(struct sakuc_deque *dq, void *data, size_t len);

extern int sakuc_deque_sweep(struct sakuc_deque *dq, int behavior);

extern int sakuc_deque_destroy(struct sakuc_deque *dq);

#define sakuc_deque_size(dq) ((dq)->length)

#endif // SAKUC_DEQUE_H_
