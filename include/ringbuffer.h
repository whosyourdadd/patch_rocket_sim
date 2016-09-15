#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/kern_return.h>
#endif

#ifdef __cplusplus
extern "C" { /* for inclusion from C++ */
#endif

#define NUM_OF_CELL                     (0x1000000) //must power of 2
#define RB_CELL_IDX(idx)      ((idx) & (NUM_OF_CELL - 1)) //mod NUM_OF_CELL
#define FILE_NAME                       "heap.log"
#define BILLION 1000000000L
#define INLINE                        __attribute__((always_inline))
extern bool reader_end_flag;
struct ringbuff_cell {
        uint32_t curr_heap_size;
        double timestamp;
};

struct ringbuffer
{
        volatile uint32_t writer_idx;
        volatile uint32_t reader_idx;
        volatile uint32_t cell_nums;
        struct ringbuff_cell cell[NUM_OF_CELL]; 
};

void rb_init(struct ringbuffer *rb);
void rb_deinit(struct ringbuffer *rb);
void rb_put(struct ringbuffer *rb, void *value);
void rb_get(struct ringbuffer *rb, void *value);

void clock_get_hw_time(struct timespec *ts);
INLINE double get_curr_time(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* __RINGBUFFER_H__ */
