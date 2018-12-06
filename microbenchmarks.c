#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include "statistics.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NANOSECONDS_PER_SECOND 1E9
#define N_REPS 50000
#define NUM_SPECS 3

struct timespec before;
struct timespec after;
double difference;

void *childfunc(void *offset)
{
  return NULL;
}

void *childfunc2(void *offset)
{
  clock_gettime(CLOCK_REALTIME,&after);
  return NULL;
}

/* malloc_shared acts like malloc in that it will allocate and return size bytes of memory,
   but it will also ensure that this region of memory is shared with all child processes.
   NOTE: it will NOT be guarded by any synchronization primitives, so you need to make sure only one
   of the parent or child is accessing it at any given time. Pro-tip: you can use exit() in the child
   and wait() in the parent in a way similar to the barriers in homework 6 to ensure only one process
   is accessing it at any given time.
   */
void * malloc_shared(size_t size){
   return mmap(NULL,size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
}

void test_threads(){
  StatObject ctr = NewStatObject();
  StatObject ctr2 = NewStatObject();
  pthread_t child;
  for (long i = 0; i < N_REPS; i++)
  {
    int result;
    clock_gettime(CLOCK_REALTIME,&before);
    result = pthread_create(&child, NULL, childfunc, (void *)i);
    clock_gettime(CLOCK_REALTIME,&after);
    if(result == 0){
      difference = (after.tv_sec * NANOSECONDS_PER_SECOND + after.tv_nsec) - (before.tv_sec * NANOSECONDS_PER_SECOND + before.tv_nsec);
      stat_object_add(ctr, difference);
    }
    pthread_join(child, NULL);
  }
  
  for (long i = 0; i < N_REPS; i++)
  {
    int result;
    clock_gettime(CLOCK_REALTIME,&before);
    result = pthread_create(&child, NULL, childfunc2, (void *)i);
    if(result == 0){
    }
    pthread_join(child, NULL);
    difference = (after.tv_sec * NANOSECONDS_PER_SECOND + after.tv_nsec) - (before.tv_sec * NANOSECONDS_PER_SECOND + before.tv_nsec);
    stat_object_add(ctr2, difference);
  }
  
  StatResult r = (StatResult)malloc(sizeof(sStatResult));
  stat_obj_value(ctr,r);
  printf("time to spawn a new thread: avg: %10.4f min: %10.4f max: %10.4f stddev: %10.4f\n",
         r->mean, r->min, r->max, r->stddev);
  stat_obj_value(ctr2,r);
  printf("time to start running a new thread: avg: %10.4f min: %10.4f max: %10.4f stddev: %10.4f\n",
         r->mean, r->min, r->max, r->stddev);
  free(r);
}

void test_processes(){
  int wpid, status, retval;
  StatObject ctr = NewStatObject();
  StatObject ctr2 = NewStatObject();
  double difference2;
  struct timespec* after2 = malloc_shared(sizeof(struct timespec));

  for (int i = 0; i < N_REPS; i++) {
    
    clock_gettime(CLOCK_REALTIME,&before);
    retval = fork();
    clock_gettime(CLOCK_REALTIME,&after);
    if(retval == 0){
      exit(0);
    }
    else{
      difference = (after.tv_sec * NANOSECONDS_PER_SECOND + after.tv_nsec) - (before.tv_sec * NANOSECONDS_PER_SECOND + before.tv_nsec);
      stat_object_add(ctr, difference);
    }
    wpid = wait(&status);
  }
  
  for (int i = 0; i < N_REPS; i++) {
    
    clock_gettime(CLOCK_REALTIME,&before);
    retval = fork();
    clock_gettime(CLOCK_REALTIME,&after);
    if(retval == 0){
      *after2 = after;
      exit(0);
    }
    else{
      
    }
    wpid = wait(&status);
    difference2 = (after2->tv_sec * NANOSECONDS_PER_SECOND + after2->tv_nsec) - (before.tv_sec * NANOSECONDS_PER_SECOND + before.tv_nsec);
    stat_object_add(ctr2, difference2);
  }
  
  StatResult r = (StatResult)malloc(sizeof(StatResult));
  stat_obj_value(ctr,r);
  printf("time to spawn a new process: avg: %10.4f min: %10.4f max: %10.4f stddev: %10.4f\n",
         r->mean, r->min, r->max, r->stddev);
  stat_obj_value(ctr2,r);
  printf("time to start running a new process: avg: %10.4f min: %10.4f max: %10.4f stddev: %10.4f\n",
         r->mean, r->min, r->max, r->stddev);
  free(r);
  // wait for all worker processes to finish
  while ((wpid = wait(&status)) > 0);
}


int main(int argc, char **argv)
{
  printf("spawning a lot of threads.\n");
  test_threads();
  printf("spawning a lot of processes.\n");
  test_processes();
  return EXIT_SUCCESS;
}
