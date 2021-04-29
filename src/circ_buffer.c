#include "circ_buffer.h"


//TODO: null checking and safety testing

circ_buf* circ_buf_create(size_t sz) {
    circ_buf* new_buffer = malloc(sizeof(circ_buf));
    new_buffer->buf = calloc(sizeof(float), sz);

    new_buffer->max = sz;
    circ_buf_reset(new_buffer);
    assert(circ_buf_empty(new_buffer));

    return new_buffer;
}

void circ_buf_destroy(circ_buf* buf) {
    //free actual float buffer
    free(buf->buf);

    //free mem that was occupied by the struct
    free(buf);
}


void circ_buf_reset(circ_buf* buf) {
    buf->head = buf->tail = 0;
    buf->full = false;
}


int circ_buf_put(circ_buf* buf, float datum) {
    buf->buf[buf->head] = datum;
    circ_buf_advance_ptr(buf);
    return buf->head;
}


//get one member from the queue--namely, the FIRST-IN data that's currently located at the tail position
float circ_buf_get(circ_buf* buf) {
    //error code of -10000 is safe for our purposes
    float r = -10000;

    if (!circ_buf_empty(buf)) {
        r = buf->buf[buf->tail];
        circ_buf_retreat_ptr(buf);
    }

    //return data at tail, otherwise return -10000 if the buffer is empty;
    return r;
}

void circ_buf_retreat_ptr(circ_buf* buf) {
    buf->full = false;
    buf->tail = (buf->tail + 1) % buf->max;
}

int circ_buf_get_head(circ_buf* buf) {
    return buf->head;
}

int circ_buf_capacity(circ_buf* buf) {
    return buf->max;
}


//advance the queue head (used after adding data), and possibly advance the tail if the buffer is already full
void circ_buf_advance_ptr(circ_buf* buf) {
    //if the buffer is full (head=tail), we need to throw OUT the the FIRST-IN data by advancing the tail as well (it's a FIFO queue)
    if (buf->full) {
        buf->tail = (buf->tail + 1) % buf->max;
    }

    //advance the head no matter what
    buf->head = (buf->head + 1) % buf->max;

    //check if the advancement made head equal to tail, which means the circular queue is now full
    buf->full = (buf->head == buf->tail); //if it was full before the advance it'll be full after too
}


bool circ_buf_empty(circ_buf* buf) {
    //boolean of negation of full anded with head=tail
    return (!buf->full && (buf->head == buf->tail));
}

//check if the circular buffer is full
bool circ_buf_full(circ_buf* buf) {
    return buf->full;
}


//take the average of the last n entries behind the queue head. Used to determine if the device is shaking.
float circ_buf_aggregate_last_n_entries(circ_buf* buf, int n) {
    //make sure the requested n is not greater than the current population of the queue
    size_t size = circ_buf_size(buf);
    if (n > size) {
        return -1;
    }

    //initialize an average
    float average = 0;

    //find the head of the queue as an integer
    int position = (int) buf->head - 1;

    int cutoff = position - n;
    if (cutoff < 0) {
        cutoff = (int) buf->max + cutoff;
    }

    //run back n spaces in the queue, adding all entries to the average
    for (int i = position; i != cutoff; i--) {
        //i could become negative if the head was at a low number, so need to correct that
        if (i < 0) {
            //change i to the correct index of the buffer
            i = (int) buf->max + i;
            if (i == cutoff) {
                return average / (float)n;
            }
        }

        //add absolute value acceleration reading the the average
        average += fabs(buf->buf[i]);
    }

    //divide average by number of elements read to get aggregate reading
    return average / (float)n;
}


int circ_buf_size(circ_buf* buf) {
    //if the buffer is full, our size is the max
    size_t size = buf->max;

    if (!buf->full)
    {
        //if circular buffer is not full and head is greater than tail, find difference to get current size
        if (buf->head >= buf->tail)
        {
            size = buf->head - buf->tail;
        }

        //otherwise we've taken out stuff past the head (which means the buffer was full, so the current size is the maximum minus
        // however much has been taken out (space between head and tail)
        else
        {
            size = (buf->max + buf->head - buf->tail);
        }
    }
    return size;
}




