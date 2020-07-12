
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

  int vet[5]={0};
  pid_t CHILDREN_PID;
  float drift;
  int count;
  int child_no;
  int rtn;
	char *args[]={"400",NULL};
  /*
   * Criacao dos processos filhos
   */

  rtn = 1;
  for (count = 0; count < NO_OF_CHILDREN; count++) {
    if (rtn != 0) {
      rtn = fork();
      vet[count]=rtn;	

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

	printf("\n PID DO PAI: %d \n", getpid());	// Printa o PID do PAI

    for (count = 0; count < NO_OF_CHILDREN; count++) {     
    
	printf("\n #PID DO filho%d: %d  -> PROCESSO ENCERRADO",count+1, vet[count]); 	// Printa o PID dos FILHOS
      printf("\n RETORNO KILL: %d \n",kill(vet[count],SIGKILL));	// Mata os processos dos filhos
	  wait(NULL);	

    }
  }

  exit(0);
}
