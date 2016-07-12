#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

extern int clock_get_monotonic_time(struct timespec *ts);

#ifdef __cplusplus
extern "C" { /* for inclusion from C++ */
#endif

#define NUM_OF_RING                     (2)
#define NUM_OF_CELL                     (8192) //must power of 2
#define FILE_NAME                       "heap_log.csv"
#define GET_RINGBUFF_CELL_IDX(idx)      ((idx) & (NUM_OF_CELL - 1))
#define IDX_METHOD_ENABLE               0                      


struct ringbuff_cell {
        struct timespec         timestamp;
        uint32_t                curr_heap_size;
};

struct ringbuff_body {
        struct ringbuff_cell    cell[NUM_OF_CELL];
        uint64_t                writer_idx;
};

struct ring_ll {
    struct ring_ll              *next;
    struct ringbuff_body        *ring;
};

void ring_buffer_init(void);
void ring_buffer_done(void);

void enqueue(void *value);
void *reader(void *ptr);
void clock_get_hw_time(struct timespec *ts);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #define _RINGBUFFER_H_ */
