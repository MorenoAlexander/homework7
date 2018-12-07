#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUMTASKS 25000
#define NANOSECONDS_PER_SECOND 1E9
#define FIBNUM 26




 struct timespec beforeTotal, afterTotal;
 

int fib(int x) {
  if (x == 0) 
    return 0;
  else if (x == 1) 
    return 1;
  return fib(x - 1) + fib(x - 2);
}

void * workerthread(void * tid){

  fib(FIBNUM);
  return NULL;
}



int main(){
  pthread_t thr[NUMTASKS];
  double difference;
  
  printf("running fib(%d) %d times\n",FIBNUM,NUMTASKS);
  clock_gettime(CLOCK_REALTIME, &beforeTotal);
  for (int i = 0; i < NUMTASKS; i++) {
    int rcode;
    if ((rcode = pthread_create(&thr[i], NULL, workerthread, NULL))) {
      if (rcode == EAGAIN){
        // insufficient resources, just keep slamming it until we finish our work
        i--;
        continue;
      }
      fprintf(stderr, "Error in pthread_create at thread num %d: %d\n", i,rcode);
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < NUMTASKS; i++) {
    pthread_join(thr[i], NULL);
  }
  clock_gettime(CLOCK_REALTIME, &afterTotal);
  difference = (afterTotal.tv_sec * NANOSECONDS_PER_SECOND + afterTotal.tv_nsec) - (beforeTotal.tv_sec * NANOSECONDS_PER_SECOND + beforeTotal.tv_nsec);
  printf("Total Time elapsed: %10.4f\n",difference);

  
  return 0;
}
