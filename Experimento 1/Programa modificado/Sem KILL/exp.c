
#include <signal.h>
#include <stdio.h>     /* for printf() */
#include <sys/time.h>  /* for gettimeofday() */
#include <sys/types.h> /* for wait() */
#include <sys/wait.h>  /* for wait() */
#include <unistd.h>    /* for gettimeofday() and fork() */
/*
 * Programa Principal. Contem apenas o codigo do pai
 */

#define NO_OF_CHILDREN 5

int main(int argc, char *argv[]) {
  
  /*
   * start_time e stop_time conterao o valor de tempo antes e depois
   * que as trocas de contexto comecem
   */

  struct timeval start_time;
  struct timeval stop_time;

  /*
   * Outras variaveis importantes
   */
  pid_t CHILDREN_PID;
  float drift;
  int count;
  int child_no;
  char *args[]={"400",NULL};

  /*
   * Criacao dos processos filhos
   */
  int rtn = 1;
  for (count = 0; count < NO_OF_CHILDREN; count++) {
    if (rtn != 0) {
      rtn = fork();
    } else {
      break;
    }
  }


  /*
   * Verifica-se rtn para determinar se o processo eh pai ou filho
   */
		
		
  if (rtn == 0) {
    /*
     * Portanto, sou filho. Faco coisas de filho.
     */
        
        child_no = count;	
	execvp("./filho",args);	// chama o arquivo filho 
   
  }
	else {
    /*
     * Sou pai, aguardo o termino dos filhos
     */

    for (count = 0; count < NO_OF_CHILDREN; count++) {     
	  wait(NULL);
    }
  }
  exit(0);
}
