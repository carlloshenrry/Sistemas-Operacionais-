#include<stdio.h> 
#include<unistd.h> 
#include <stdio.h>     /* for printf() */
#include <sys/time.h>  /* for gettimeofday() */
#include <sys/types.h> /* for wait() */
#include <sys/wait.h>  /* for wait() */
#include <unistd.h>    /* for gettimeofday() and fork() */

#define NO_OF_ITERATIONS 1000
#define NO_OF_CHILDREN 5
#define MICRO_PER_SECOND 1000000


int main(int argc, char *argv[])
{ 
  
  struct timeval start_time;
  struct timeval stop_time;
  
  float drift;
  int count;
  int child_no;
  int rtn;
	
	int SLEEP_TIME=atoi(argv[0]);
  
		

	/*
     * Primeiro, obtenho o tempo inicial.
     */
    gettimeofday(&start_time, NULL);

    /*
     * Este loop ocasiona a minha dormencia, de acordo com
     * SLEEP_TIME, tantas vezes quanto NO_OF_ITERATIONS
     */
    for (count = 0; count < NO_OF_ITERATIONS; count++) {
      usleep(SLEEP_TIME);
    }

    /*
     * Para obter o tempo final
     */
    gettimeofday(&stop_time, NULL);

    /*
     * Calcula-se o desvio
     */
    drift = (float)(stop_time.tv_sec - start_time.tv_sec);
    drift += (stop_time.tv_usec - start_time.tv_usec) / (float)MICRO_PER_SECOND;
    int numFilho = getpid() - getppid();
    
    /*
     * Exibe os resultados
     */
    printf("\nFilho #%d -- desvio total: %.8f -- desvio medio: %.8f\n",
           numFilho, drift - NO_OF_ITERATIONS * SLEEP_TIME / MICRO_PER_SECOND,
           (drift - NO_OF_ITERATIONS * SLEEP_TIME / MICRO_PER_SECOND) /
               NO_OF_ITERATIONS);

	return 0; 
} 
