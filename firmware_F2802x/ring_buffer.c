#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ring_buffer.h"

// Ring for single producer and consumer!
// also, size MUST be a power of two!

// Get element, returns 1 on failure, 0 on success.
//__attribute__((ramfunc))
RB_ELM_TYPE rb_get(struct ring_buffer *rb, RB_ELM_TYPE *d) {
    if(rb->head == rb->tail) { // no data
        // Empty buffer
        return 1;
    }
    *d = rb->buf[rb->tail & (RB_NELEM(rb)-1)];
    rb->tail = rb->tail + 1;
    return 0;
}

//__attribute__((ramfunc))
RB_ELM_TYPE rb_put(struct ring_buffer *rb, RB_ELM_TYPE d) {
    if((rb->head - rb->tail) == RB_NELEM(rb)) // full
        return 1;
    rb->buf[rb->head & (RB_NELEM(rb)-1)] = d;
    rb->head++;
    return 0;
}

__attribute__((ramfunc))
bool rb_empty(struct ring_buffer *rb) {
    return rb->tail == rb->head;
}
