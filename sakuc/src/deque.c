// 2014-3-15 - created by jtuki@foxmail.com

#include "deque.h"
#include "common_memory_management_defs.h"

struct sakuc_deque *
sakuc_deque_new(size_t sub_deque_size, size_t elem_size, int behavior)
{
    if (sub_deque_size == 0 || elem_size == 0)
        return nullptr;
    
    sakuc_deque_t *dq = (sakuc_deque_t *) osal_mem_alloc(sizeof (sakuc_deque_t));
    if (!dq)
        return nullptr;

    struct sakuc_sub_deque_ *sub_deque = osal_mem_alloc(sizeof (struct sakuc_sub_deque_));
    if (!sub_deque) {
        osal_mem_free(dq);
        return nullptr;
    }

    void *buf = osal_mem_alloc(elem_size * sub_deque_size);
    if (!buf) {
        osal_mem_free(sub_deque);
        osal_mem_free(dq);
        return nullptr;
    }

    sub_deque->length = sub_deque->start = sub_deque->end = 0;
    sub_deque->data_buffer = buf;
    sub_deque->next = sub_deque->prev = nullptr;

    dq->behavior = behavior;
    dq->sub_deque_size = sub_deque_size;
    dq->elem_size = elem_size;
    dq->dq_head = dq->dq_tail = sub_deque;
    dq->length = 0;
    dq->num_sub_deque = 1;

    return dq;
}

/* new a sub deque if current sakuc_deque.dq_head and dq_tail cannot satisfy */
static struct sakuc_sub_deque_ * sakuc_sub_dq_new_(size_t sub_deque_size, size_t elem_size)
{
    void *buf = osal_mem_alloc(elem_size * sub_deque_size);
    if (!buf) {
        return nullptr;
    }

    // calloc - clear malloc.
    struct sakuc_sub_deque_ *sub_deque = osal_mem_calloc(1, sizeof (struct sakuc_sub_deque_));
    if (!sub_deque) {
        osal_mem_free(buf);
        return nullptr;
    }

    sub_deque->data_buffer = buf;
    return sub_deque;
}

int sakuc_deque_push_back(struct sakuc_deque *dq, const void *data, size_t len)
{
    struct sakuc_sub_deque_ *tail = nullptr;

    // find the tail, if current tail is full, create a new sub_deque node as tail.
    if (dq->dq_tail->end < dq->sub_deque_size) {
        tail = dq->dq_tail;
    }
    else if (dq->dq_tail->next == nullptr) {
        struct sakuc_sub_deque_ *sub_dq = sakuc_sub_dq_new_(dq->sub_deque_size, dq->elem_size);
        if (sub_dq) {
            sub_dq->prev = dq->dq_tail;
            dq->dq_tail->next = sub_dq;
            dq->dq_tail = sub_dq;
            ++ dq->num_sub_deque;

            tail = sub_dq;
        }
    }
    else {
        // if not empty and valid sub_deque, some problem has occured, simply keep @tail as nullptr.
        struct sakuc_sub_deque_ *p = dq->dq_tail->next;
        if (p->length == 0 && p->start == 0 && p->end == 0 && p->data_buffer) {
            p->prev = dq->dq_tail;
            dq->dq_tail = p;
        
            tail = p;
        }
    }
    
    if (tail) {
        osal_memcpy((char *)tail->data_buffer + tail->end * dq->elem_size, data, len);
        ++ tail->end;
        ++ tail->length;
        ++ dq->length;
        return 0;
    }
    else
        return -1;
}

int sakuc_deque_push_front(struct sakuc_deque *dq, const void *data, size_t len)
{
    struct sakuc_sub_deque_ *head = nullptr;
    
    // find the head, if current head is full, create a new sub_deque node as head.
    if (dq->dq_head->start > 0) {
        head = dq->dq_head;
    }
    else if (dq->dq_head->prev == nullptr) {
        struct sakuc_sub_deque_ *sub_dq = sakuc_sub_dq_new_(dq->sub_deque_size, dq->elem_size);
        if (sub_dq) {
            // refer to @start and @end - two invariant member.
            sub_dq->length = 0;
            sub_dq->start = sub_dq->end = dq->sub_deque_size;
        
            sub_dq->next = dq->dq_head;
            dq->dq_head->prev = sub_dq;
            dq->dq_head = sub_dq;
            ++ dq->num_sub_deque;

            head = sub_dq;
        }
    }
    else {
        // if not empty and valid sub_deque, some problem has occured, simply keep @tail as nullptr.
        struct sakuc_sub_deque_ *p = dq->dq_head->prev;
        if (p->length == 0 && p->data_buffer 
            && p->end == dq->sub_deque_size && p->start == dq->sub_deque_size)
        {
            dq->dq_head->prev->next = dq->dq_head;
            dq->dq_head = dq->dq_head->prev;
        
            head = p;
        }
    }
    
    if (head) {
        -- head->start;
        osal_memcpy((char *)head->data_buffer + head->start * dq->elem_size, data, len);
        ++ head->length;
        ++ dq->length;
        return 0;
    }
    else
        return -1;
    return 0;
}

int sakuc_deque_pop_back(struct sakuc_deque *dq, void *data, size_t len)
{
    struct sakuc_sub_deque_ *tail = nullptr;
    
    if (dq->dq_tail->start < dq->dq_tail->end) // not empty
        tail = dq->dq_tail;
    else if (dq->dq_tail->start == 0 && dq->dq_tail->end == 0
             && dq->dq_tail->prev)
    {
        struct sakuc_sub_deque_ *p = dq->dq_tail->prev;
        if (p->length > 0 && p->end == dq->sub_deque_size) {
            dq->dq_tail = p;
            tail = p;
            
            if (dq->behavior & SAKUC_DEQUE_SWEEP_IMMEDIATELY)
                sakuc_deque_sweep(dq, SAKUC_DEQUE_SWEEP_TAIL);
        }
    }

    if (tail) {
        -- tail->end;
        osal_memcpy(data, (char *)tail->data_buffer + tail->end * dq->elem_size, len);
        -- tail->length;
        -- dq->length;
        return 0;
    }
    else
        return -1;
}

int sakuc_deque_pop_front(struct sakuc_deque *dq, void *data, size_t len)
{
    struct sakuc_sub_deque_ *head = nullptr;
    
    if (dq->dq_head->start < dq->dq_head->end) // not empty
        head = dq->dq_head;
    else if (dq->dq_head->start == dq->sub_deque_size && dq->dq_head->end == dq->sub_deque_size
             && dq->dq_head->next)
    {
        struct sakuc_sub_deque_ *p = dq->dq_head->next;
        if (p->length > 0 && p->start == 0) {
            dq->dq_head = p;
            head = p;
            
            if (dq->behavior & SAKUC_DEQUE_SWEEP_IMMEDIATELY)
                sakuc_deque_sweep(dq, SAKUC_DEQUE_SWEEP_HEAD);
        }
    }
    
    if (head) {
        osal_memcpy(data, (char *)head->data_buffer + head->start * dq->elem_size, len);
        ++ head->start;
        -- head->length;
        -- dq->length;
        return 0;
    }
    else 
        return -1;
}

int sakuc_deque_sweep(struct sakuc_deque *dq, int behavior)
{
    if (!dq || !dq->dq_head || !dq->dq_tail)
        return -1;
    
    struct sakuc_sub_deque_ *sweep = nullptr;
    struct sakuc_sub_deque_ *next_sweep = nullptr;
    
    if (behavior & SAKUC_DEQUE_SWEEP_IMMEDIATELY
        || behavior & SAKUC_DEQUE_SWEEP_MANUALLY)
        behavior = SAKUC_DEQUE_SWEEP_HEAD | SAKUC_DEQUE_SWEEP_TAIL;
    
    // sweep the head's prev list.
    if (behavior & SAKUC_DEQUE_SWEEP_HEAD) {
        sweep = dq->dq_head->prev;
        dq->dq_head->prev = nullptr;
        
        while (sweep != nullptr) {
            next_sweep = sweep->prev;
            
            if (sweep->data_buffer)
                osal_mem_free(sweep->data_buffer);
            osal_mem_free(sweep);
            -- dq->num_sub_deque;
            
            sweep = next_sweep;
        }
    }
    
    // sweep the tail's next list.
    if (behavior & SAKUC_DEQUE_SWEEP_TAIL) {
        sweep = dq->dq_tail->next;
        dq->dq_tail->next = nullptr;
        
        while (sweep != nullptr) {
            next_sweep = sweep->next;
            if (sweep->data_buffer)
                osal_mem_free(sweep->data_buffer);
            osal_mem_free(sweep);
            -- dq->num_sub_deque;
            
            sweep = next_sweep;
        }
    }
    
    return 0;
}

int sakuc_deque_destroy(struct sakuc_deque *dq)
{
    if (!dq || !dq->dq_head || !dq->dq_tail)
        return -1;
    
    struct sakuc_sub_deque_ *p = dq->dq_head;
    while (p->prev)
        p = p->prev;
    
    struct sakuc_sub_deque_ *next_destroy = nullptr;
    do {
        next_destroy = p->next;

        // best-effort memory free process.
        if (p->data_buffer)
            osal_mem_free(p->data_buffer);
        osal_mem_free(p);
        
        p = next_destroy;
    } while (p != nullptr);
    
    osal_mem_free(dq);    
    return 0;
}
