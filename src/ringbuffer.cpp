#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdint.h>
#include "ringbuffer.h"
#include <errno.h>
 #include <unistd.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/kern_return.h>
#endif

/*
As the mutex lock is stored in global (static) memory it can be 
    initialized with PTHREAD_MUTEX_INITIALIZER.
    If we had allocated space for the mutex on the heap, 
    then we would have used pthread_mutex_init(ptr, NULL)
*/
pthread_mutex_t     ring_lock   = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      cond_space  = PTHREAD_COND_INITIALIZER;
pthread_cond_t      cond_count  = PTHREAD_COND_INITIALIZER;
double current_time;
bool end_flag = false;

static struct ringbuff_body ring;
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
#if BINARY_FORMAT
        log_file = fopen(FILE_NAME,"wb");
#else
        log_file = fopen(FILE_NAME,"w");
#endif   
        setvbuf(log_file, NULL, _IONBF, 0);
        ring.wdata_index = 0;
}

void enqueue(void *value)
{

        pthread_mutex_lock(&ring_lock);
        while (ring.writer_idx == ring.reader_idx - 1) //ring buffer is full
        {
            pthread_cond_wait(&cond_space, &ring_lock);
        }
        ring.cell[GET_RINGBUFF_CELL_IDX(ring.writer_idx)].data[GET_CELL_DATA_IDX(ring.wdata_index++)] = *(struct ringbuff_data *)value;
        if(GET_CELL_DATA_IDX(ring.wdata_index) ==  NUM_OF_DATA - 1)
        {
                ring.wdata_index = 0;
                __sync_fetch_and_add(&ring.writer_idx, 1);
        }
       
        pthread_mutex_unlock(&ring_lock);
        pthread_cond_signal(&cond_count);
}

void* dequeue(void)
{
        pthread_mutex_lock(&ring_lock);
        while (ring.writer_idx == ring.reader_idx) //ring buffer is empty
        {
            pthread_cond_wait(&cond_count, &ring_lock);
        }
#if BINARY_FORMAT
            fwrite(&ring.cell[GET_RINGBUFF_CELL_IDX(ring.reader_idx)],sizeof(struct ringbuff_cell),1,log_file);
#else
            fprintf(log_file,"%ld, %d\n", (long)ring.cell[GET_RINGBUFF_CELL_IDX(ring.reader_idx)].timestamp 
                                         , ring.cell[GET_RINGBUFF_CELL_IDX(ring->reader_idx)].curr_heap_size);

#endif
        __sync_fetch_and_add(&ring.reader_idx,1);
        pthread_mutex_unlock(&ring_lock);
        pthread_cond_signal(&cond_space);
        
        return NULL;
}

void *reader(void *ptr)
{
        /* TODO: stop the writer */
        set_thread_to_CPU(__FUNCTION__,2); 
        while (!end_flag)
        {
            dequeue();
        }
        fflush(log_file);
        fclose(log_file);
        return NULL;
}

#if __MACH__
#define SYSCTL_CORE_COUNT   "machdep.cpu.core_count"

typedef struct cpu_set {
  uint32_t    count;
} cpu_set_t;

static inline void
CPU_ZERO(cpu_set_t *cs) { cs->count = 0; }

static inline void
CPU_SET(int num, cpu_set_t *cs) { cs->count |= (1 << num); }

static inline int
CPU_ISSET(int num, cpu_set_t *cs) { return (cs->count & (1 << num)); }

int pthread_setaffinity_np(pthread_t thread, size_t cpu_size,
                           cpu_set_t *cpu_set)
{
  thread_port_t mach_thread;
  int core = 0;

  for (core = 0; core < 8 * cpu_size; core++) {
    if (CPU_ISSET(core, cpu_set)) break;
  }
  printf("binding to core %d\n", core);
  thread_affinity_policy_data_t policy = { core };
  mach_thread = pthread_mach_thread_np(thread);
  thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY,
                    (thread_policy_t)&policy, 1);
  return 0;
}
#endif /*__MACH__*/

void set_thread_to_CPU(const char* function_name, unsigned int n) {
    cpu_set_t cpuset;
    pthread_t thread = pthread_self();

    CPU_ZERO(&cpuset);
    CPU_SET(n, &cpuset);
    int s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0) {
       handle_error_en(s, "pthread_setaffinity_np");
    }
    else {
        printf("Set %s to CPU %d success!!\n", function_name, n);
    }
}

static inline double get_curr_time() {
    struct timespec ts;
    clock_get_hw_time(&ts);
    return ts.tv_sec + (double)ts.tv_nsec/(double)BILLION;
}
/* Periodic update the timer*/
void *get_curr_time_thread_loop(void*) 
{
    set_thread_to_CPU(__FUNCTION__, 3);
    while(!end_flag) {
        current_time = get_curr_time();
        usleep(100);  
    }
    return NULL;
}