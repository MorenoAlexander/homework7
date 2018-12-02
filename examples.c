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

int main(int argc, char **argv)
{
  /* How to record how long it takes to do something using clock_gettime */
  struct timespec before;
  struct timespec after;
  clock_gettime(CLOCK_REALTIME,&before);
  sleep(3);
  clock_gettime(CLOCK_REALTIME,&after);
  /* How to do math on struct timespecs */
  long difference = (after.tv_sec * NANOSECONDS_PER_SECOND + after.tv_nsec) - (before.tv_sec * NANOSECONDS_PER_SECOND + before.tv_nsec);
  printf("sleeping for three seconds actually took %ld nanoseconds.\n",difference);

  /* How to keep track of lots of statistics for repetitions of doing the same thing */
  StatObject ctr = NewStatObject();
  for (int i  = 0; i < N_REPS; i++){
    clock_gettime(CLOCK_REALTIME, &before);
    sched_yield();
    clock_gettime(CLOCK_REALTIME, &after);
    double difference = (after.tv_sec * NANOSECONDS_PER_SECOND + after.tv_nsec) - (before.tv_sec * NANOSECONDS_PER_SECOND + before.tv_nsec);
    stat_object_add(ctr, difference);
  }
  /* How to print the statistics collected */
  StatResult r = (StatResult)malloc(sizeof(sStatResult));
  stat_obj_value(ctr,r);
  printf("time to yield: avg: %10.4f min: %10.4f max: %10.4f stddev: %10.4f\n",
         r->mean, r->min, r->max, r->stddev);

  return EXIT_SUCCESS;
}
