#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "statistics.h"

#define NUMTASKS 25000
#define NANOSECONDS_PER_SECOND 1E9
#define FIBNUM 26

int fib(int x) {
  if (x == 0) 
    return 0;
  else if (x == 1) 
    return 1;
  return fib(x - 1) + fib(x - 2);
}

int main(){

  StatObject procSpawn = NewStatObject();


  struct timespec spawnStart;
  struct timespec spawnAfter;
  int wpid, status, retval;
  


  
  for (int i = 0; i < NUMTASKS; i++) {
    clock_gettime(CLOCK_REALTIME,&spawnStart);
    retval = fork();
    clock_gettime(CLOCK_REALTIME,&spawnAfter);
    double difference = (spawnAfter.tv_sec * NANOSECONDS_PER_SECOND + spawnAfter.tv_nsec) - (spawnStart.tv_sec * NANOSECONDS_PER_SECOND + spawnStart.tv_nsec);
    stat_object_add(procSpawn,difference);
    if (retval == -1){
      // over our process limit: wait for at least one process to finish and try again
      //difference = 0;
      wpid = wait(&status);
      if (wpid < 0){
        printf("waited when no children to wait for\n");
        exit(1);
      }
      // try this iteration of the loop over again
      i--;
      continue;
    }
    if (retval == 0){

      fib(FIBNUM);
      exit(0);
    }
    else{
      continue;
    }
  }
  // wait for all worker processes to finish
  while ((wpid = wait(&status)) > 0);

  StatResult r = (StatResult)malloc(sizeof(sStatResult));
  stat_obj_value(procSpawn, r);
  printf("time to spawn a process: avg: %10.4f min: %10.4f max: %10.4f stddev: %10.4f\n",
         r->mean, r->min, r->max, r->stddev);
  return 0;
}