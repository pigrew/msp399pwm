/*
 * ring_buffer.c
 *
 *  Created on: Apr 2, 2019
 *      Author: nconrad
 */


#include <stdlib.h>
#include <stdbool.h>
#include "ring_buffer.h"

// Ring for single producer and consumer!
// also, size MUST be a power of two!

// Get element, returns 1 on failure, 0 on success.
uint8_t rb_get(struct ring_buffer *rb, uint8_t *d) {
    if(rb->head == rb->tail) { // no data
        // Empty buffer
        return 1;
    }
    *d = rb->buf[rb->tail & (RB_NELEM(rb)-1)];
    rb->tail = rb->tail + 1;
    return 0;
}

uint8_t rb_put(struct ring_buffer *rb, uint8_t d) {
    if((rb->head - rb->tail) == RB_NELEM(rb)) // full
        return 1;
    rb->buf[rb->head & (RB_NELEM(rb)-1)] = d;
    rb->head++;
    return 0;
}

bool rb_empty(struct ring_buffer *rb) {
    return rb->tail == rb->head;
}
