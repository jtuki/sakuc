// 2013-7-13, jtuki@foxmail.com

#include "ringbuffer.h"
#include "common_memory_management_defs.h"

/*  New a struct rbufStruct (FIFO).
    @param capacity - capacity of the ringbuffer.
    @param elem_size - size of a single element.
 */
ringbuffer_t* rbuf_new(size_t capacity, size_t elem_size)
{
    ringbuffer_t *rb = (ringbuffer_t *) osal_mem_alloc(sizeof(ringbuffer_t));
    if (!rb)
        return nullptr;
        
    rb->buffer = osal_mem_alloc(capacity * elem_size);
    if (!rb->buffer) {
        osal_mem_free(rb);
        return nullptr;
    }

    rb->start = 0; rb->end = 0;
    rb->block_size = elem_size;
    rb->capacity = capacity; rb->length = 0;
    
    return rb;
}

#define AdvanceIndex(index, capacity)                       \
    do {                                                    \
        index += 1;                                         \
        if ((index) == (capacity))                          \
            (index) = 0;                                    \
    } while(__LINE__ == -1)

int rbuf_push_back(ringbuffer_t *rb, void *data, size_t len)
{
    if (!rb || !data
        || len != rb->block_size)
        return -1;
    
    char *rbuf = rb->buffer;
    if (!rbuf)
      return -1;

    osal_memcpy(rbuf + rb->end * rb->block_size, data, len);
    AdvanceIndex(rb->end, rb->capacity);

    if (rb->length == rb->capacity) {
        AdvanceIndex(rb->start, rb->capacity);
    }
    else {
        rb->length += 1;
    }
    return 0;
}

void *rbuf_pop_front(ringbuffer_t *rb, void *pop_value, size_t len)
{
    if (!rb || !pop_value
        || len != rb->block_size)
        return 0;
    
    char *rbuf = rb->buffer;
    if (!rbuf)
      return 0;

    // empty FIFO, nothing pop out.
    if (rb->length > 0) {
        osal_memcpy(pop_value, rbuf + rb->start * len, len);
        AdvanceIndex(rb->start, rb->capacity);

        rb->length -= 1;
        return pop_value;
    }
    
    return nullptr;
}

int rbuf_destroy(ringbuffer_t *rb)
{
    if (!rb || !(rb->buffer))
      return -1;
    
    osal_mem_free(rb->buffer);
    osal_mem_free(rb);
    return 0;
}
