#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>




#ifdef __cplusplus
extern "C" { /* for inclusion from C++ */
#endif

#define NUM_OF_DATA                     (1024) //must power of 2
#define NUM_OF_CELL                     (8192) //must power of 2
#define FILE_NAME                       "heap.log"
#define GET_RINGBUFF_CELL_IDX(idx)      ((idx) & (NUM_OF_CELL - 1))
#define GET_CELL_DATA_IDX(idx)          ((idx) & (NUM_OF_DATA - 1))

#define BILLION 1000000000L

#define BINARY_FORMAT 1
#define handle_error_en(en, msg) \
       do { errno = en; perror(msg); } while (0)

struct ringbuff_data {
    double   timestamp;
    uint32_t curr_heap_size;
};

struct ringbuff_cell {
    struct ringbuff_data data[NUM_OF_DATA];
};

struct ringbuff_body {
        struct ringbuff_cell cell[NUM_OF_CELL];
        uint64_t writer_idx;
        uint64_t reader_idx;
        uint32_t wdata_index;
};

extern double current_time;
extern bool end_flag;

void enqueue(void *value);
void ring_buffer_init(void);
void *reader(void *ptr);
void set_thread_to_CPU(const char* function_name, unsigned int n);
void *get_curr_time_thread_loop(void*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #define _RINGBUFFER_H_ */