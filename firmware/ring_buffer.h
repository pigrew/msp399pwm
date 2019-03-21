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

#define RING_BUFFER_MAX 1
typedef struct {
    size_t s_elem;
    size_t n_elem;
    void *buffer;
} rb_attr_t;

typedef unsigned int rbd_t;

struct ring_buffer
{
    size_t s_elem;
    size_t n_elem;
    uint8_t *buf;
    volatile size_t head;
    volatile size_t tail;
};

static struct ring_buffer _rb[RING_BUFFER_MAX];

int ring_buffer_init(rbd_t *rbd, rb_attr_t *attr);

int ring_buffer_put(rbd_t rbd, const void *data);

int ring_buffer_get(rbd_t rbd, void *data);

#endif /* RING_BUFFER_H_ */
