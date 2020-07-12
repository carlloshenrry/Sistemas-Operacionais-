/*******************************************************************************
 *
 * Este programa faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
 * 
 * Seguem os comentarios originais:
 *
 * Experiment #5: Semaphores
 *
 *    Programmer: Eric Sorton
 *          Date: 3/17/97
 *           For: MSE599, Special Topics Class
 *
 *       Purpose: The purpose of this program is to demonstrate how semaphores
 *		 can be used to protect a critical region.  Its sole purpose
 *		 is to print a character string (namely the alphabet) to the
 *		 screen.  Any number of processes can be used to cooperatively
 *		 (or non-cooperatively) print the string to the screen.  An
 *		 index is stored in shared memory, this index is the index into
 *		 the array that identifies which character within the string
 *		 should be printed next.  Without semaphores, all the processes
 *		 access this index simultaneously and conflicts occur.  With
 *		 semahpores, the character string is displayed neatly to the
 *		 screen.
 *
 *		 The optional semaphore protection can be compiled into the
 *		 program using the MACRO definition of PROTECT.  To compile
 *		 the semaphore protection into the program, uncomment the
 *		 #define below.
 *
 *
 *       Proposito: O proposito deste programa e o de demonstrar como semaforos
 *		podem ser usados para proteger uma regiao critica. O programa exibe
 *		um string de caracteres (na realidade um alfabeto). Um n�mero 
 *		qualquer de processos pode ser usado para exibir o string, seja
 *		de maneira cooperativa ou nao cooperativa. Um indice e armazenado
 *		em memoria compartilhada, este indice e aquele usado para 
 * 		identificar qual caractere deve ser exibido em seguida. Sem 
 *		semaforos, todos os processos acessam esse indice concorrentemente 
 *		causando conflitos. Com semaforos, o string de caracteres e exibido
 *		de maneira correta (caracteres do alfabeto na ordem correta e apenas
 *		um de cada caractere).
 *
 *		A protecao opcional com semaforo pode ser compilada no programa
 *		usando a definicao de MACRO denominada PROTECT. Para compilar a
 *		protecao com semaforo, retire o comentario do #define que segue.
 *
 *
 *******************************************************************************/

#define PROTECT

/*
 * Includes Necessarios 
 */
#include <errno.h>              /* errno and error codes */
#include <sys/time.h>           /* for gettimeofday() */
#include <stdio.h>              /* for printf() */
#include <unistd.h>             /* for fork() */
#include <sys/types.h>          /* for wait() */
#include <sys/wait.h>           /* for wait() */
#include <signal.h>             /* for kill(), sigsuspend(), others */
#include <sys/ipc.h>            /* for all IPC function calls */
#include <sys/shm.h>            /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>            /* for semget(), semop(), semctl() */
#include <stdlib.h>


/*
 * Constantes Necessarias 
 */
#define SEM_KEY 0x1243
#define SHM_KEY 0x1432
#define NO_OF_CHILDREN 8

/*
 * As seguintes variaveis globais contem informacao importante. A variavel
 * g_sem_id e g_shm_id contem as identificacoes IPC para o semaforo e para
 * o segmento de memoria compartilhada que sao usados pelo programa. A variavel
 * g_shm_addr e um ponteiro inteiro que aponta para o segmento de memoria
 * compartilhada que contera o indice inteiro da matriz de caracteres que contem
 * o alfabeto que sera exibido.
 */
int g_sem_id;
int g_shm_id;
int *g_shm_addr;

/*
 * As seguintes duas estruturas contem a informacao necessaria para controlar
 * semaforos em relacao a "fecharem", se nao permitem acesso, ou 
 * "abrirem", se permitirem acesso. As estruturas sao incializadas ao inicio
 * do programa principal e usadas na rotina PrintAlphabet(). Como elas sao
 * inicializadas no programa principal, antes da criacao dos processos filhos,
 * elas podem ser usadas nesses processos sem a necessidade de nova associacao
 * ou mudancas.
 */
struct sembuf g_sem_op1[1];
struct sembuf g_sem_op2[1];

/*
 * O seguinte vetor de caracteres contem o alfabeto que constituira o string
 * que sera exibido.
 */
char g_letters_and_numbers[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 1234567890";
/*
 *	O vetor abaixo corresponde ao buffer que sera compartilhado entre os processos filhos
 */


/*
 * Funcoes
 */
void PutInBuffer(void);
void ConsumeBuffer(void);

/*
 * Programa Principal
 */
int main(int argc, char * argv[]) {
    /*
     * Variaveis necessarias
     */
    int rtn;
    int count;

    /*
     * Para armazenar os ids dos processos filhos, permitindo o posterior
     * uso do comando kill
     */
    int pid[NO_OF_CHILDREN];

    /*
     * Construindo a estrutura de controle do semaforo
     */
    g_sem_op1[0].sem_num = 0;
    g_sem_op1[0].sem_op = 1; // g_sem_op1 com sem_op = 1 destranca o semaforo
    g_sem_op1[0].sem_flg = 0;

    g_sem_op2[0].sem_num = 0;
    g_sem_op2[0].sem_op = -1; // g_sem_op2 com sem_op = -1 tranca o semaforo
    g_sem_op2[0].sem_flg = 0;

    /*
     * Criando o semaforo
     */
    if ((g_sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1) {
        fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
        exit(1);
    }

    if (semop(g_sem_id, g_sem_op1, 1) == -1) { // Inicializa com semafaro destrancado -> op1
        fprintf(stderr, "chamada semop() falhou, impossivel inicializar o semaforo!");
        exit(1);
    }

    /*
     * Criando o segmento de memoria compartilhada com o tamanho do buffer
     */
    if ((g_shm_id = shmget(SHM_KEY, (sizeof(g_letters_and_numbers)+4*sizeof(int)), IPC_CREAT | 0666)) == -1) { // Alterado para 0666
        fprintf(stderr, "Impossivel criar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    if ((g_shm_addr = (int*) shmat(g_shm_id, NULL, 0)) == (int*) - 1) {
        fprintf(stderr, "Impossivel associar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    *g_shm_addr = 0;
    *(g_shm_addr +sizeof(int))= 0;
    *(g_shm_addr +2*sizeof(int))= 0;
    *(g_shm_addr +3*sizeof(int))= 0;

    /*
     * Criando os filhos
     */
    rtn = 1;
    for (count = 0; count < NO_OF_CHILDREN; count++) {
        if (rtn != 0) {
            pid[count] = rtn = fork();
        } else {
            break; // Corrigido, exit() para break
        }
    }
    /*
     * Verificando o valor retornado para determinar se o processo e 
     * pai ou filho 
     */
    if (rtn == 0) {
        /*
         * Eu sou um filho
         */
        printf("Filho %i comecou ...\n", count);
        // Filhos produtores entram aqui
        usleep(100);
        if(count < 4){
             PutInBuffer();
        } else { // Filhos consumidores entram aqui
			ConsumeBuffer();
        }
        


        
    } else {
		// Quanto mais tempo o pai demora para matar os filhos, mais printa
        usleep(150000);

        /*
         * Matando os filhos 
         */
        kill(pid[0], SIGKILL);
        kill(pid[1], SIGKILL);
        kill(pid[2], SIGKILL);
        kill(pid[3], SIGKILL);
        kill(pid[4], SIGKILL);
        kill(pid[5], SIGKILL);
        kill(pid[6], SIGKILL);
        kill(pid[7], SIGKILL);

        /*
         * Removendo a memoria compartilhada
         */
        if (shmctl(g_shm_id, IPC_RMID, NULL) != 0) {
            fprintf(stderr, "Impossivel remover o segmento de memoria compartilhada!\n");
            exit(1);
        }

        /*
         * Removendo o semaforo
         */
        if (semctl(g_sem_id, 0, IPC_RMID, 0) != 0) {
            fprintf(stderr, "Impossivel remover o conjunto de semaforos!\n");
            exit(1);
        }

        exit(0);
    }
}

/*
 * Esta rotina realiza a exibicao de caracteres. Nela e calculado um numero
 * pseudo-randomico entre 1 e 5 para determinar o numero de caracteres a colocar no buffer.
 * Se a protecao esta estabelecida, a rotina entao consegue o recurso. Em
 * seguida, PutInBuffer() acessa o indice com seu valor corrente a partir da
 * memoria compartilhada. A rotina entra em loop, exibindo o numero aleatorio de
 * caracteres. Finalmente, a rotina incrementa o indice, conforme o necessario,
 * e libera o recurso, se for o caso.
 */
void PutInBuffer(void) { //Produtor
	struct timeval tv;
    int number;

    int tmp_index;
    int i;
    int cont1;
    /*
     * Este tempo permite que todos os filhos sejam inciados
     */
    usleep(400);

    /*
     * Entrando no loop principal
     */
    while (1) {
        /*
         * Conseguindo o tempo corrente, os microsegundos desse tempo
         * sao usados como um numero pseudo-randomico. Em seguida,
         * calcula o numero randomico atraves de um algoritmo simples
         */
        if (gettimeofday(&tv, NULL) == -1) {
            fprintf(stderr, "Impossivel conseguir o tempo atual, terminando.\n");
            exit(1);
        }
        number = ((tv.tv_usec / 47) % 5) + 1;

        /*
         * O #ifdef PROTECT inclui este pedaco de codigo se a macro
         * PROTECT estiver definida. Para sua definicao, retire o comentario
         * que a acompanha. semop() e chamada para fechar o semaforo.
         */

        #ifdef PROTECT
        if (semop(g_sem_id, g_sem_op2, 1) == -1) { // Realiza tranca com o (g_sem_op2)
            fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
            exit(1);
        }
		#endif

        /*
         * Lendo o indice do segmento de memoria compartilhada
         */
        tmp_index = *g_shm_addr;
        
        
        /*
         * Repita o numero especificado de vezes, esteja certo de nao
         * ultrapassar os limites do vetor, o t  if garante isso
         */
        int j=0;
        fprintf(stderr, "\nProdutor produziu:" );
        for (i = 0; i < number; i++) {

            if (!(tmp_index + i > sizeof(g_letters_and_numbers))) {
                fprintf(stderr, " %c ", g_letters_and_numbers[tmp_index + i]); //imprime caracteres produzidos
                *(g_shm_addr+4*sizeof(int)+tmp_index+i)=g_letters_and_numbers[tmp_index + i]; //salva caracteres produzidos no segmento de memoria compartlhada
                usleep(1);
                j++;
            }
        }
       // fprintf(stderr, "\n");

        /*
         * Atualizando o indice na memoria compartilhada
         */
        *g_shm_addr = tmp_index + j; 


        *(g_shm_addr+2*sizeof(int)) =*(g_shm_addr+2*sizeof(int))+ j; //atualiza contador de produção
        
        /*iniciar a linha seguinte e coloca
         * zero no indice
         */
        if (tmp_index + j > sizeof(g_letters_and_numbers)) {
            fprintf(stderr, "\nBuffer:\n&");
            for(int i=0;i<sizeof(g_letters_and_numbers);i++){
            fprintf(stderr, "%c", *(g_shm_addr+4*sizeof(int)+i)); //imprime caracteres do segmento
            
            }
            fprintf(stderr, "&->p \n");
            //fprintf(stderr, "&\n");
            *g_shm_addr = 0;
        }

		//printf("\n\nSou filho produtor e acabei de produzir %d caracteres", number);

        /*
         * Liberando o recurso se a macro PROTECT estiver definida
         */

        #ifdef PROTECT
        if (semop(g_sem_id, g_sem_op1, 1) == -1) { // Realiza destranca com o (g_sem_op1) 		 
            fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
            exit(1);
        }
		#endif
    }
}

void ConsumeBuffer(void) { //consumidor
struct timeval tv;
    int number;

    int tmp_index;
    int tmp_index2;
    int i;
    int cont1,cont2;

    /*
     * Este tempo permite que todos os filhos sejam inciados
     */
    usleep(400);

    /*
     * Entrando no loop principal
     */
    while (1) {
        /*
         * Conseguindo o tempo corrente, os microsegundos desse tempo
         * sao usados como um numero pseudo-randomico. Em seguida,
         * calcula o numero randomico atraves de um algoritmo simples
         */
        if (gettimeofday(&tv, NULL) == -1) {
            fprintf(stderr, "Impossivel conseguir o tempo atual, terminando.\n");
            exit(1);
        }
        number = ((tv.tv_usec / 47) % 5) + 1;

        /*
         * O #ifdef PROTECT inclui este pedaco de codigo se a macro
         * PROTECT estiver definida. Para sua definicao, retire o comentario
         * que a acompanha. semop() e chamada para fechar o semaforo.
         */

        #ifdef PROTECT
        if (semop(g_sem_id, g_sem_op2, 1) == -1) { // Realiza tranca com o (g_sem_op2)
            fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
            exit(1);
        }
        #endif

        /*
         * Lendo o indice do segmento de memoria compartilhada
         */
        tmp_index = *g_shm_addr;

        tmp_index2=*(g_shm_addr+sizeof(int));

//obetem contador de produção e consumo
        cont1= *(g_shm_addr+2*sizeof(int));
        cont2=*(g_shm_addr+3*sizeof(int));

        /*
         * Repita o numero especificado de vezes, esteja certo de nao
         * ultrapassar os limites do vetor, o comando if garante isso
         */
        int j=0;
        if(cont2+number<cont1){ //verifica se o produtor é maior que consumidor para evitar que haja consumo antes da produção
        for (i = 0; i < number; i++) {

            if (!(tmp_index2 + i > sizeof(g_letters_and_numbers))) {
                //fprintf(stderr, "%c", g_letters_and_numbers[tmp_index + i]);
                *(g_shm_addr+4*sizeof(int)+tmp_index2+i)='#'; //salva # no ssegmento
                usleep(1);
                j++;
            }
        }
        /*
         * Atualizando o indice na memoria compartilhada
         */
        *(g_shm_addr+sizeof(int)) = tmp_index2 + j;
        *(g_shm_addr+3*sizeof(int)) =*(g_shm_addr+3*sizeof(int))+ j;
    }
        /*
         * Se o indice e maior que o tamanho do alfabeto, exibe um
         * caractere return para iniciar a linha seguinte e coloca
         * zero no indice
         */
        if (tmp_index2 + j > sizeof(g_letters_and_numbers)) {
            fprintf(stderr, "\nBuffer:\n&");
            for(int i=0;i<sizeof(g_letters_and_numbers);i++){ 
            fprintf(stderr, "%c", *(g_shm_addr+4*sizeof(int)+i)); //faz impressão do segmento de memoria
            
            }
            fprintf(stderr, "&-> c \n");
           // fprintf(stderr, "&\n");
            *(g_shm_addr+sizeof(int)) = 0;
        }

        //printf("\n\nSou filho produtor e acabei de produzir %d caracteres", number);

        /*
         * Liberando o recurso se a macro PROTECT estiver definida
         */

        #ifdef PROTECT
        if (semop(g_sem_id, g_sem_op1, 1) == -1) { // Realiza destranca com o (g_sem_op1)        
            fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
            exit(1);
        }
        #endif
    }
}
