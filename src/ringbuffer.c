#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ringbuffer.h"

/*
As the mutex lock is stored in global (static) memory it can be 
    initialized with PTHREAD_MUTEX_INITIALIZER.
    If we had allocated space for the mutex on the heap, 
    then we would have used pthread_mutex_init(ptr, NULL)
*/
FILE              *log_file;
bool reader_running_flag = true;

void rb_init(struct ringbuffer *rb)
{
    rb->writer_idx = 0;
    rb->reader_idx = 0;
    rb->cell_nums = 0;
    log_file = fopen(FILE_NAME,"w");
    setvbuf(log_file, NULL, _IONBF, 0);
    reader_running_flag = true;
}

void rb_deinit(struct ringbuffer *rb)
{
        fflush(log_file);
        fclose(log_file);
}

INLINE void rb_put(struct ringbuffer *rb, void *value)
{
        while(__sync_bool_compare_and_swap(&rb->cell_nums, NUM_OF_CELL, NUM_OF_CELL));
        __sync_fetch_and_add(&rb->writer_idx, 1);
        rb->cell[RB_CELL_IDX(rb->writer_idx)] = *(struct ringbuff_cell *)value;
        __sync_add_and_fetch(&rb->cell_nums, 1);
}

INLINE void rb_get(struct ringbuffer *rb, void *value)
{
        
        while(__sync_bool_compare_and_swap(&rb->cell_nums, 0, 0));
        __sync_fetch_and_add(&rb->reader_idx, 1);

#if 0 //Add your logging method
        struct ringbuff_cell *temp;
        temp = &rb->cell[RB_CELL_IDX(rb->reader_idx)];
        fprintf(log_file, "%ld.%ld, %d\n", temp->timestamp.tv_sec
                                         , temp->timestamp.tv_nsec
                                         , temp->curr_heap_size);
#else
        //fprintf(log_file, "%d\n", rb->cell[RB_CELL_IDX(rb->reader_idx)].curr_heap_size);
#endif
        __sync_sub_and_fetch(&rb->cell_nums, 1);
}


void clock_get_hw_time(struct timespec *ts)
{
#ifdef __MACH__
    clock_serv_t cclock;
    mach_timespec_t mts;
    kern_return_t ret_val;
    if ((ret_val = host_get_clock_service(mach_host_self(),
                                          SYSTEM_CLOCK, &cclock) != KERN_SUCCESS))
        goto ret;

    ret_val = clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
ret:
    return;
#else
  //clock_gettime(CLOCK_MONOTONIC_COARSE, ts);
  clock_gettime(CLOCK_MONOTONIC, ts);
#endif
}