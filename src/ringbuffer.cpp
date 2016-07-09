#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h> 
#include "ringbuffer.h"



/*
As the mutex lock is stored in global (static) memory it can be 
    initialized with PTHREAD_MUTEX_INITIALIZER.
    If we had allocated space for the mutex on the heap, 
    then we would have used pthread_mutex_init(ptr, NULL)
*/
pthread_mutex_t ring_lock = PTHREAD_MUTEX_INITIALIZER; 
static struct ringbuff_body *ring;
static struct ringbuff_body bodies[NUM_OF_RING];

int ring_body_idx;

sem_t *spacesem;
FILE *log_file;

void ring_buffer_init(void)
{
        sem_unlink("ring_protect");
        spacesem = sem_open("ring_protect", O_CREAT, 0700, 0);
        if (spacesem == SEM_FAILED) {
            printf("%s\n", strerror(errno));
            exit(-1);
        }
        
        log_file = fopen(FILE_NAME,"w");
        setvbuf(log_file, NULL, _IONBF, 0);
}

void enqueue(void *value)
{
        struct ringbuff_body *r; 
        pthread_mutex_lock(&ring_lock);
        
        r = &bodies[ring_body_idx];
        r->cell[r->writer_idx++] = *(struct ringbuff_cell *)value;

        if (r->writer_idx == NUM_OF_CELL) {
            ring = r;
            ring_body_idx = (++ring_body_idx) % NUM_OF_RING;
            sem_post(spacesem);
        }
        pthread_mutex_unlock(&ring_lock);

}

void* dequeue(void)
{
        uint32_t cell_idx;
        pthread_mutex_lock(&ring_lock);
        for (cell_idx = 0; cell_idx < ring->writer_idx; cell_idx++)
        {
            struct ringbuff_cell *cell = &ring->cell[cell_idx];

            fprintf(log_file,"%ld.%9ld %d\n", (long)cell->timestamp.tv_sec 
                                         , cell->timestamp.tv_nsec
                                         , cell->curr_heap_size);
        }

        memset(ring, 0, sizeof(*ring));
        pthread_mutex_unlock(&ring_lock);

        return NULL;
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
        while (1)
        {
            if (sem_wait(spacesem) < 0) {
            assert(errno == 0);
            }
            dequeue();
        }
        fflush(log_file);
        fclose(log_file);
        return NULL;
}