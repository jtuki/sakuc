// 2013-7-13, jtuki@foxmail.com

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include "common_defs.h"

typedef struct ringbuffer {
    // size: capacity of ringbuffer.
    size_t capacity;
    // the block size of each data block. `capacity * block_size == total ring buffer size'
    size_t block_size;
    // length: actual length (1~capacity) of current buffer.
    size_t length;
    // @start and @end - 0~(capacity-1)
    int start; int end;
    void *buffer;
} ringbuffer_t;

/*  New a struct ringbuffer_t (FIFO). nullptr returned if failed to allocate memory.
 */
extern ringbuffer_t* rbuf_new(size_t capacity, size_t elem_size);

/*  Push @data (with length @len) to @rb. -1 returned if failed.
 */
extern int rbuf_push(ringbuffer_t *rb, void *data, size_t len);

/*  popleft value to @pop_value (buffer with length @len which equals to @rb->block_size).
    nullptr returned if @rb is empty.
 */
extern void *rbuf_popleft(ringbuffer_t *rb, void *pop_value, size_t len);

/* -1 returned if failed. */
extern int rbuf_destroy(ringbuffer_t *rb);

#define rbuf_length(rb) ((rb)->length)

#endif
