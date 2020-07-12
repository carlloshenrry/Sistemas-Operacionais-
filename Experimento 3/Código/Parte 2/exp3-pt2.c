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
void PutInBuffer(int);
void ConsumeBuffer(void);

/*
 * Programa Principal
 */
int main(int argc, char *argv[]) {
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
     * Criando o segmento de memoria compartilhada com o tamanho
     * do buffer + 4 inteiros para os indices e contadores
     */
    if ((g_shm_id = shmget(SHM_KEY, sizeof(g_letters_and_numbers)+(sizeof(int)*4), IPC_CREAT | 0666)) == -1) { // Alterado para 0666
        fprintf(stderr, "Impossivel criar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    if ((g_shm_addr = (int*) shmat(g_shm_id, NULL, 0)) == (int*) - 1) {
        fprintf(stderr, "Impossivel associar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    /* 
     * Usaremos o segmento de memoria compartilhada da maneira que:
     * [0-3]    = 4 bytes destinados ao indice global dos filhos produtores
     * [4-7]    = 4 bytes destinados ao indice global dos filhos consumidores
     * [8-11]   = 4 bytes destinados ao contador de quantos caracteres totais foram produzidos
     * [12-15]  = 4 bytes destinados ao contador de quantos caracteres totais foram consumidos
     * [16-80]  = 65 bytes destinados ao buffer de caracteres
     */
    *g_shm_addr = 0; // Inicializando indice do produtor
    *(g_shm_addr + sizeof(int)) = 0; // Inicializando indice do consumidor
    *(g_shm_addr + (sizeof(int) * 2)) = 0; // Inicializando contador de caracteres totais produzidos
    *(g_shm_addr + (sizeof(int) * 3)) = 0; // Inicializando contador de caracteres totais consumidos

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
        if(count < 5){
            PutInBuffer(count);
        } else { // Filhos consumidores entram aqui
            ConsumeBuffer();
        }
        


        
    } else {
		// Quanto mais tempo o pai demora para matar os filhos, mais printa
        usleep(32500);

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
void PutInBuffer(int num_filho) {
	struct timeval tv;
    int number;

    int tmp_index; // Recebe o indice do produtor (do segmento de memoria compartilhado)
    int tmp_num_prod; // Recebe o num total de produzidos (do segmento de memoria compartilhado)
    int tmp_num_cons; // Recebe o num total de consumidos (do segmento de memoria compartilhado)
    int i;

    /*
     * Este tempo permite que todos os filhos sejam inciados
     */
    usleep(450);

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

        tmp_index = *g_shm_addr; // Recebendo o indice do produtor
        tmp_num_prod = *(g_shm_addr + (sizeof(int) * 2)); // Recebendo o total ja produzido

        /*
         * O contador abaixo sera utilizado para auxiliar no calculo
         * de caracteres produzidos totais. Se usarmos "i" do "for" abaixo,
         * corremos risco de contarmos caracteres q nao foram inseridos
         * em razao do produtor ja ter percorrido todo o indice e "i"
         * continuar sendo incrementado mesmo assim
         */
        int aux = 0;

        /*
         * Se o indice for menor que o tamanho total do buffer de chars,
         * printa os caracteres que serao produzidos pelo filho produtor
         */
        if(tmp_index < sizeof(g_letters_and_numbers)-1)
            fprintf(stderr, "\nSou o filho Nº %d, PID %d e produzi os seguintes caracteres:" , num_filho, getpid());

        /*
         * Repita o numero especificado de vezes, esteja certo de nao
         * ultrapassar os limites do vetor, o comando if garante isso
         */
        for (i = 0; i < number; i++) {
            if (!(tmp_index + i > sizeof(g_letters_and_numbers))) {
                fprintf(stderr, " %c ", g_letters_and_numbers[tmp_index + i]); // Imprime caracteres produzidos
                *(g_shm_addr + (sizeof(int) * 4) + tmp_index + i) = g_letters_and_numbers[tmp_index + i]; // Salva no buffer os caracteres
                usleep(1);
                aux++; // Incrementa o contador auxiliar
            }
        }

        /*
         * Atualizando o indice e o total ja produzido na memoria compartilhada
         */
        *g_shm_addr = tmp_index + aux;
        *(g_shm_addr + (sizeof(int) * 2)) = *(g_shm_addr + (sizeof(int) * 2)) + aux;

        /*
         * Se o indice e maior que o tamanho do alfabeto, exibe um
         * caractere return para iniciar a linha seguinte e coloca
         * zero no indice e printa o buffer inteiro
         */
        if (tmp_index + aux > sizeof(g_letters_and_numbers)) {
            fprintf(stderr, "\n");
            *g_shm_addr = 0;
            for(i = 0; i < sizeof(g_letters_and_numbers); i++){
                fprintf(stderr, "%c", *(g_shm_addr + (sizeof(int) * 4) + i));
            }
            fprintf(stderr, " -> Produtor printou\n\n");
        }
        
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

void ConsumeBuffer() {
    struct timeval tv;
    int number;

    int tmp_index;
    int tmp_num_prod; // Recebe o num total de produzidos 
    int tmp_num_cons; // Recebe o num total de consumidos
    int i;

    /*
     * Este tempo permite que todos os filhos sejam inciados
     */
    usleep(450);

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
        tmp_index = *(g_shm_addr + sizeof(int));
        tmp_num_cons = *(g_shm_addr + (sizeof(int) * 3));
        tmp_num_prod = *(g_shm_addr + (sizeof(int) * 2));

        /*
         * O contador abaixo sera utilizado para auxiliar no calculo
         * de caracteres consumidos totais. Se usarmos "i" do "for" abaixo,
         * corremos risco de contarmos caracteres q nao foram inseridos
         * em razao do consumidor ja ter percorrido todo o indice e "i"
         * continuar sendo incrementado mesmo assim
         */
        int aux = 0;

        /*
         * Repita o numero especificado de vezes, esteja certo de nao
         * ultrapassar os limites do vetor, o comando if garante isso
         */
        if(tmp_num_cons+number <= tmp_num_prod){
            for (i = 0; i < number; i++) {
                if (!(tmp_index + i > sizeof(g_letters_and_numbers))) {
                    *(g_shm_addr + (sizeof(int) * 4) + tmp_index + i)= '#'; // Salva no buffer o "#"
                    usleep(1);
                    aux++; // Incrementa o contador auxiliar
                }
            }
        }

        /*
         * Atualizando o indice e o total ja consumido na memoria compartilhada
         */
        *(g_shm_addr + sizeof(int)) = tmp_index + aux;
        *(g_shm_addr + (sizeof(int) * 3)) = *(g_shm_addr +(sizeof(int) * 3)) + aux;
        

        /*
         * Se o indice e maior que o tamanho do alfabeto, exibe um
         * caractere return para iniciar a linha seguinte e coloca
         * zero no indice e printa o buffer inteiro
         */
        if (tmp_index + aux > sizeof(g_letters_and_numbers)) {
            fprintf(stderr, "\n");
            *(g_shm_addr + sizeof(int)) = 0;
            for(i = 0; i < sizeof(g_letters_and_numbers); i++){
                fprintf(stderr, "%c", *(g_shm_addr + (sizeof(int) * 4) + i));
            }
            fprintf(stderr, " -> Consumidor printou\n\n");
        }

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