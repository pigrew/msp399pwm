/*
 * ring_buffer.h
 *
 *  Created on: 05 но€б. 2016 г.
 *      Author: Rostik
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdlib.h>
#include <stdint.h>

typedef uint16_t RB_ELM_TYPE;

struct ring_buffer
{
    size_t n_bits; // number of bits in count, e.g. 4 would be 16 items
    volatile size_t head;
    volatile size_t tail;
    volatile RB_ELM_TYPE *buf;
};

#define RB_NELEM(rb) (1<<(rb->n_bits))

RB_ELM_TYPE rb_put(struct ring_buffer *rb, RB_ELM_TYPE d);
RB_ELM_TYPE rb_get(struct ring_buffer *rb, RB_ELM_TYPE *d);
bool rb_empty(struct ring_buffer *rb);

#endif /* RING_BUFFER_H_ */
