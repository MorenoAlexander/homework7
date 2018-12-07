#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUMTASKS 25000
#define NANOSECONDS_PER_SECOND 1E9
#define FIBNUM 26

int numworkers = 2;
sem_t available;
pthread_mutex_t protect_state;
struct worker {
  pthread_t tid;
  long input; // if parent sees input = -1, worker available. if worker sees input = -1, work is done
  long output; // if output = -1, nothing done yet
  sem_t done;
};
struct worker * workers; 

int fib(int x) {
  if (x == 0) 
    return 0;
  else if (x == 1) 
    return 1;
  return fib(x - 1) + fib(x - 2);
}

void * workerthread(void * tid){
  int i = (long)tid;
  while(1){

    sem_wait(&workers[i].done);
    if(workers[i].input == -1)
      return NULL;
    long result = fib(workers[i].input);
    pthread_mutex_lock(&protect_state);
    workers[i].output = result;
    workers[i].input = -1;
    pthread_mutex_unlock(&protect_state);
    sem_post(&available);
  }

}


struct timespec beforeTotal;
struct timespec afterTotal;


int main(int argc, char ** argv){
  // If you want to use a different number of workers, specify it as the first
  // argument on the command line
  if(argc == 2){
    numworkers = atoi(argv[1]);
  }
  workers = malloc(numworkers * sizeof(struct worker));
  sem_init(&available,0,numworkers);
  pthread_mutex_init(&protect_state,NULL);
  for(long i = 0; i < numworkers; i++){
    sem_init(&workers[i].done,0,0);
    workers[i].output = -1;
    workers[i].input = -1;
    if(pthread_create(&workers[i].tid,NULL,workerthread,(void*)i) != 0){
      perror("pthread_create failed: ");
      exit(1);
    }
  }

  printf("Running fib(%d) %d times with %d worker threads\n",FIBNUM,NUMTASKS,numworkers);
  clock_gettime(CLOCK_REALTIME,&beforeTotal); //get time before we start
  for (int j=0; j<NUMTASKS; j++){
    sem_wait(&available);
    pthread_mutex_lock(&protect_state);
    for(int i = 0; i < numworkers; i++){
      if (workers[i].input == -1){
        if (workers[i].output != -1){
          // If you cared about output, you would do something with it here, e.g.:
          //printf("worker %d finished task with result %ld\n",i,workers[i].output);
        }
        workers[i].input = FIBNUM;
        sem_post(&workers[i].done);
        break;
      }
    }
    pthread_mutex_unlock(&protect_state);
  };
  // drain the final results
  for (int j = 0; j < numworkers; j++){
    sem_wait(&available);
    pthread_mutex_lock(&protect_state);
    for(int i = 0; i < numworkers; i++){
      if (workers[i].input == -1){
        if (workers[i].output != -1){
          // If you cared about output, you would do something with it here
        }
        workers[i].input = -1;
        sem_post(&workers[i].done);
      }
    }
    pthread_mutex_unlock(&protect_state);
  }
  clock_gettime(CLOCK_REALTIME,&afterTotal); //time after we're done

  double difference  = (afterTotal.tv_sec * NANOSECONDS_PER_SECOND + afterTotal.tv_nsec) - (beforeTotal.tv_sec * NANOSECONDS_PER_SECOND + beforeTotal.tv_nsec);
  printf("Total elapsed time for threadpool: %10.4f\n", difference);
  return 0;
}