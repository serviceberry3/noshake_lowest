#ifndef CIRC_BUFFER_H
#define CIRC_BUFFER_H

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>


//struct representing circ buffer
typedef struct circ_buf {
    //data
    float* buf;

    //head and tail of the buffer
    int head, tail;

    //buffer capacity
    int max;

    //whether buffer is full or not
    bool full;
} circ_buf;


//construct an instance of a circular buffer using a design
circ_buf* circ_buf_create(size_t sz);

//destroy the circular buffer instance
void circ_buf_destroy();

//reset the circular buffer to empty, head == tail
void circ_buf_reset(circ_buf* buf);

void circ_buf_retreat_ptr(circ_buf* buf);

void circ_buf_advance_ptr(circ_buf* buf);

//put version 1 continues to add data if the buffer is full
//old data is overwritten
int circ_buf_put(circ_buf* buf, float datum);

//retrieve a value from the buffer
//returns 0 on success, -1 if the buffer is empty
float circ_buf_get(circ_buf* buf);

//returns true if the buffer is empty
bool circ_buf_empty(circ_buf* buf);

//returns true if the buffer is full
bool circ_buf_full(circ_buf* buf);

//returns the maximum capacity of the buffer
int circ_buf_capacity(circ_buf* buf);

//returns the current number of elements in the buffer
int circ_buf_size(circ_buf* buf);

//give an average of the last n entries in the buffer
float circ_buf_aggregate_last_n_entries(circ_buf* buf, int n);

//retrieve the head position of the circular buffer
int circ_buf_get_head(circ_buf* buf);


#endif