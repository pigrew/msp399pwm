/*
 * ring_buffer.h
 *
 *  Created on: 05 но€б. 2016 г.
 *      Author: Rostik
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <msp430.h>
#include <stdlib.h>
#include <stdint.h>

struct ring_buffer
{
    size_t n_bits; // number of bits in count, e.g. 4 would be 16 items
    volatile size_t head;
    volatile size_t tail;
    volatile uint8_t *buf;
};

#define RB_NELEM(rb) (1<<(rb->n_bits))

inline uint8_t rb_put(struct ring_buffer *rb, uint8_t d);
inline uint8_t rb_get(struct ring_buffer *rb, uint8_t *d);
bool rb_empty(struct ring_buffer *rb);

#endif /* RING_BUFFER_H_ */
