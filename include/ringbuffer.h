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

#define NUM_OF_RING                     (1)
#define NUM_OF_CELL                     (16384) //must power of 2
#define FILE_NAME                       "heap_log.csv"
#define GET_RINGBUFF_CELL_IDX(idx)      ((idx) & (NUM_OF_CELL - 1))


struct ringbuff_cell {
        struct timespec timestamp;
        uint32_t curr_heap_size;
};

struct ringbuff_body {
        struct ringbuff_cell cell[NUM_OF_CELL];
        uint64_t writer_idx;
        uint64_t reader_idx;
};

void enqueue(void *value);
void ring_buffer_init(void);
void *reader(void *ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #define _RINGBUFFER_H_ */
