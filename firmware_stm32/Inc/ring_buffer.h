/*
 * ring_buffer.h
 *
 *  Created on: Apr 2, 2019
 *      Author: nconrad
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct ring_buffer
{
    size_t n_bits; // number of bits in count, e.g. 4 would be 16 items
    volatile size_t head;
    volatile size_t tail;
    volatile uint8_t *buf;
};

#define RB_NELEM(rb) (1<<(rb->n_bits))

uint8_t rb_put(struct ring_buffer *rb, uint8_t d);
uint8_t rb_get(struct ring_buffer *rb, uint8_t *d);
bool rb_empty(struct ring_buffer *rb);

#endif /* RING_BUFFER_H_ */
