#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdint.h>
#include "ringbuffer.h"


/*
As the mutex lock is stored in global (static) memory it can be 
    initialized with PTHREAD_MUTEX_INITIALIZER.
    If we had allocated space for the mutex on the heap, 
    then we would have used pthread_mutex_init(ptr, NULL)
*/
pthread_mutex_t ring_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;
static struct ringbuff_body g_rings[NUM_OF_RING]; 

int ring_body_idx = 0;
FILE *log_file;

void clock_get_hw_time(struct timespec *ts){
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
  clock_gettime(CLOCK_MONOTONIC_COARSE, ts);
#endif
}


void ring_buffer_init(void)
{
        log_file = fopen(FILE_NAME,"w");
        setvbuf(log_file, NULL, _IONBF, 0);
}

void enqueue(void *value)
{
        struct ringbuff_body *r; 
        pthread_mutex_lock(&ring_lock);
        
        r = &g_rings[ring_body_idx];

        if (r->writer_idx == r->reader_idx - 1) //ring buffer is full
        {
            pthread_cond_wait(&cond, &ring_lock);
        }
        r->cell[GET_RINGBUFF_CELL_IDX(r->writer_idx)] = *(struct ringbuff_cell *)value;
        r->writer_idx++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&ring_lock);
}

void* dequeue(void)
{
        struct ringbuff_body *r;
        pthread_mutex_lock(&ring_lock);
        r = &g_rings[ring_body_idx];
        if (r->writer_idx == r->reader_idx) //ring buffer is empty
        {
            pthread_cond_wait(&cond, &ring_lock);
        }

            fprintf(log_file,"%ld.%9ld, %d\n", (long)r->cell[GET_RINGBUFF_CELL_IDX(r->reader_idx)].timestamp.tv_sec 
                                         , r->cell[GET_RINGBUFF_CELL_IDX(r->reader_idx)].timestamp.tv_nsec
                                         , r->cell[GET_RINGBUFF_CELL_IDX(r->reader_idx)].curr_heap_size);
        r->reader_idx++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&ring_lock);

        return NULL;
}

void *writer(void *ptr)
{
        int in = 0;
        int count = *(int *)ptr;
        struct ringbuff_cell temp;
        for (in = 0; in < count; ++in)
        {
            clock_get_hw_time(&temp.timestamp);
            temp.curr_heap_size = in;
            enqueue(&temp);
        }
        return NULL;
}

void *reader(void *ptr)
{
        
        while (1)
        {
            dequeue();
        }
        fflush(log_file);
        fclose(log_file);
        return NULL;
}