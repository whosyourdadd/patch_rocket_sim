#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdint.h>
#include "ringbuffer.h"


struct ringbuff_body ring;

/*
As the mutex lock is stored in global (static) memory it can be 
    initialized with PTHREAD_MUTEX_INITIALIZER.
    If we had allocated space for the mutex on the heap, 
    then we would have used pthread_mutex_init(ptr, NULL)
*/
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
sem_t countsem; 
sem_t spacesem;
FILE *log_file;

void ring_buffer_init(void)
{
        sem_init(&countsem, 0, 0);
        sem_init(&spacesem, 0, NUM_OF_CELL);
        
        log_file = fopen(FILE_NAME,"w");
        setvbuf(log_file, NULL, _IONBF, 0);
}

void enqueue(void *value)
{
        // wait if there is no space left:
        sem_wait( &spacesem );
        
        pthread_mutex_lock(&lock);
#if IDX_METHOD_ENABLE
        if (ring.writer_idx == ring.reader_idx - 1) //ring buffer is full
        {
            /* TODO Need to wait the consumer clean a cell*/
        } 
        else
        {
            ring.cell[GET_RINGBUFF_CELL_IDX(ring.writer_idx)] = *(struct ringbuff_cell *)value;
            ring.writer_idx ++;
        }
#else
       ring.cell[(ring.writer_idx++) & (NUM_OF_CELL - 1)] = *(struct ringbuff_cell *)value;
#endif /* IDX_METHOD_ENABLE */
        pthread_mutex_unlock(&lock);
        // increment the count of the number of items
        sem_post(&countsem);
}

void* dequeue(void)
{
        // Wait if there are no items in the buffer
        void *out_value = NULL;
        sem_wait(&countsem);

        pthread_mutex_lock(&lock);
#if IDX_METHOD_ENABLE       
        if (ring.writer_idx == ring.reader_idx) //ring buffer is empty
        {
            /* TODO Need to wait the producer enqueue the buffer*/
        } 
        else
        {
            fprintf(log_file,"%ld.%9ld %d\n", (long) ring.cell[GET_RINGBUFF_CELL_IDX(ring.reader_idx)].timestamp.tv_sec 
                                              , ring.cell[GET_RINGBUFF_CELL_IDX(ring.reader_idx)].timestamp.tv_nsec
                                              , ring.cell[GET_RINGBUFF_CELL_IDX(ring.reader_idx)].curr_heap_size);
            ring.reader_idx++;
        }
#else
        fprintf(log_file,"%ld.%9ld %d\n", (long) ring.cell[GET_RINGBUFF_CELL_IDX(ring.reader_idx)].timestamp.tv_sec 
                                          , ring.cell[GET_RINGBUFF_CELL_IDX(ring.reader_idx)].timestamp.tv_nsec
                                          , ring.cell[GET_RINGBUFF_CELL_IDX(ring.reader_idx)].curr_heap_size);
        ring.reader_idx++;
#endif /*IDX_METHOD_ENABLE*/
        pthread_mutex_unlock(&lock);

        // Increment the count of the number of spaces
        sem_post(&spacesem);
        return out_value;
}

void *writer(void *ptr)
{
        int i = 0;
        int count = *(int *)ptr;
        struct ringbuff_cell temp;
        for (i = 0; i < count; ++i)
        {
            clock_get_monotonic_time(&temp.timestamp);
            temp.curr_heap_size = i;
            enqueue(&temp);
        }
        return NULL;
}

void *reader(void *ptr)
{
        int spaceloop, countloop;
        while(1)
        {
            dequeue();
            sem_getvalue(&spacesem,&spaceloop);
            sem_getvalue(&countsem,&countloop);
            if (spaceloop == NUM_OF_CELL && countloop == 0)
                break;
        }
        fflush(log_file);
        fclose(log_file);
        return NULL;
}