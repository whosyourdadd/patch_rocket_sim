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
double current_time;
bool end_flag = false;

static struct ringbuff_body g_bodies;
int ring_body_idx;
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
}

void enqueue(void *value)
{
    while(__sync_bool_compare_and_swap(&g_bodies.writer_idx, g_bodies.reader_idx - 1, g_bodies.writer_idx));
    //while(__sync_bool_compare_and_swap(&g_bodies.len, NUM_OF_CELL, NUM_OF_CELL));
    //uint64_t tmp = __sync_fetch_and_add(&g_bodies.writer_idx, 1);
    g_bodies.cell[GET_RINGBUFF_CELL_IDX(g_bodies.writer_idx)] = *(struct ringbuff_cell *)value;
    __sync_fetch_and_add(&g_bodies.writer_idx, 1);
    //__sync_add_and_fetch(&g_bodies.len, 1);
}

void* dequeue(void)
{
        while(__sync_bool_compare_and_swap(&g_bodies.reader_idx, g_bodies.writer_idx, g_bodies.reader_idx));
        //while(__sync_bool_compare_and_swap(&g_bodies.len, 0, 0));
        //uint64_t tmp = __sync_fetch_and_add(&g_bodies.reader_idx, 1);
#if BINARY_FORMAT
        fwrite(&g_bodies.cell[GET_RINGBUFF_CELL_IDX(g_bodies.reader_idx)],sizeof(struct ringbuff_cell),1,log_file);
        __sync_fetch_and_add(&g_bodies.reader_idx, 1);
#else
         fprintf(log_file,"%ld.%ld %d\n", (long)g_bodies.cell[GET_RINGBUFF_CELL_IDX(tmp)].timestamp.tv_sec 
                                             , g_bodies.cell[GET_RINGBUFF_CELL_IDX(tmp)].timestamp.tv_nsec
                                             , g_bodies.cell[GET_RINGBUFF_CELL_IDX(tmp)].curr_heap_size);
#endif
         //__sync_sub_and_fetch(&g_bodies.len, 1);
        return NULL;
}

void *reader(void *ptr)
{
        /* TODO: stop the writer */
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
    while(!end_flag) {
        current_time = get_curr_time();
        usleep(100);  
    }
    return NULL;
}