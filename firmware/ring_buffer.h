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
    size_t n_elem;
    volatile size_t head;
    volatile size_t tail;
    uint8_t *buf;
};

uint8_t rb_put(struct ring_buffer *rb, uint8_t d);
uint8_t rb_get(struct ring_buffer *rb, uint8_t *d);
bool rb_empty(struct ring_buffer *rb);
#endif /* RING_BUFFER_H_ */
