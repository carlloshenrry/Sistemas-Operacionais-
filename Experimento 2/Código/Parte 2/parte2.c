/*******************************************************************************
*
* Este programa faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
*
* Seguem os comentarios originais:
*
* Experiment #3: Shared Resources, Measureing Message Queue Transfer Time
*
*    Programmer: Eric Sorton
*          Date: 2/11/97
*           For: MSE599, Special Topics Class
*
*       Purpose: The purpose of this program is to measure the time it takes
*                a message to be transfered across a message queue.  The
*                total time will include the time to make the call to msgsnd(),
*                the time for the system to transfer the message, the time
*                for the context switch, and finally, the time for the other
*                end to call msgrcv().
*
*                The algorithm for this program is very simple:
*
*                   o The parent creates the message queue
*                   o The parents starts two children
*                   o The first child will:
*                         - Receive a message on the queue
*                         - Call gettimeofday() to get the current time
*                         - Using the time in the message, calculate
*                              the difference and store it in an array
*                         - Loop (X number of times)
*    - Display the results
*                   o The second child will:
*                         - Call gettimeofday() to get the current time
*                         - Place the time in a message
*                         - Place the message on the queue
*                         - Pause to allow the other child to run
*                         - Loop (X number of times)
*                   o The parent waits for the children to finish
*
* Traduzindo:
*
*     Prop�sito: O prop�sito deste programa � a medicao do tempo que leva
*                uma mensagem para ser transferida por uma fila de mensagens.
*                O tempo total incluira o tempo para realizar a chamada
*                msgsnd(), o tempo para o sistema transferir a mensagem, o
*                tempo para troca de contexto e, finalmente, o tempo para,
*                na outra ponta, ocorrer a chamada msgrcv().
*
*                O algoritmo para este programa e bastante simples:
*
*                   o O pai cria a fila de mensagens
*                   o O pai inicializa dois filhos
*                   o O primeiro filho:
*                         - Recebe uma mensagem pela fila
*                         - Chama gettimeofday() para obter o tempo atual
*                         - Usando o tempo existente na mensagem, calcula
*                              a diferenca
*                         - Repete (numero X de vezes)
*  - Exibe os resultados
*                   o O segundo filho:
*                         - Chama gettimeofday() para obter o tempo atual
*                         - Coloca o tempo em uma mensagem
*                         - Coloca a mensagem na fila
*                         - Realiza uma pausa para permitir a execucao do irmao
*                         - Repete (numero X de vezes)
*                   o O pai espera os filhos terminarem
*
*******************************************************************************/

/*
 * Includes Necessarios
 */
#include <sys/time.h>  /* for gettimeofday() */
#include <stdio.h>	 /* for printf() */
#include <unistd.h>	/* for fork() */
#include <sys/types.h> /* for wait(), msgget(), msgctl() */
#include <wait.h>	  /* for wait() */
#include <sys/ipc.h>   /* for msgget(), msgctl() */
#include <sys/msg.h>   /* for msgget(), msgctl() */
#include <stdlib.h>

/*
 * NO_OF_ITERATIONS corresponde ao numero de mensagens que serao enviadas.
 * Se este numero cresce, o tempo de execucao tambem deve crescer.
 */
#define NO_OF_ITERATIONS 500

/*
 * MICRO_PER_SECOND define o numero de microsegundos em um segundo
 */
#define MICRO_PER_SECOND 1000000

/*
 * MESSAGE_QUEUE_ID eh uma chave arbitraria, foi escolhido um numero qualquer,
 * que deve ser unico. Se outra pessoa estiver executando este mesmo programa
 * ao mesmo tempo, o numero pode ter que ser mudado!
 */
#define MESSAGE_QUEUE_ID 3102
#define MESSAGE_QUEUE_ID_2 3104

/*
 * Constantes
 */
#define SENDER_DELAY_TIME 10
#define MESSAGE_MTYPE 1

#define NO_OF_CHILDREN 3
int tam = 0;

typedef struct
{
	float time_min;
	float time_max;
	float time_total;
	float time_medio;
} times;

void Receiver(int, int, times *);

void Sender(int, times, int);
/*
 * Pergunta 1: O que eh um prot�tipo? Por qual motivo eh usado?
 */

/*
 * Programa principal
 */

int main(int argc, char *argv[])
{
	/*
         * Algumas variaveis necessarias
         */
	times struct_time; // Struct contendo os times (time_min, time_max, time_total)
	int rtn = 1;
	int count = 10;

	/*
         * Variaveis relativas a fila, id e key
         */
	int queue_id_1, queue_id_2;
	key_t key1 = MESSAGE_QUEUE_ID;
	key_t key2 = MESSAGE_QUEUE_ID_2;

	printf("\nDigite o tamanho da mensagem (de 1 a 10): ");
	scanf(" %d", &tam);

	// Calculando o tamanho da mensagem

	tam = tam * 512;

	/*
* Pergunta 2: O que significa cada um dos d�gitos 0666?
* Pergunta 3: Para que serve o arquivo stderr?
* Pergunta 4: Caso seja executada a chamada fprintf com o handler stderr, onde aparecer� o seu resultado?
* Pergunta 5: Onde stderr foi declarado?
*/

	/*
* Pergunta 6: Explicar o que s�o e para que servem stdin e stdout.
  */

	/*
* Inicializa dois filhos
*/

	for (count = 0; count < NO_OF_CHILDREN; count++)
	{
		if (rtn != 0)
		{
			rtn = fork();
		}
		else
		{
			break;
		}
	}

	/*
* Verifica o valor retornado para determinar se o processo eh pai ou filho
*
* OBS:  o valor de count no loop anterior indicara cada um dos filhos
*       count = 1 para o primeiro filho, 2 para o segundo, etc.
*/

	if (rtn == 0 && count == 1)
	{

		// Sou o filho 1
		// Cria a primeira fila de mensagens
		if ((queue_id_1 = msgget(key1, 0666 | IPC_CREAT)) == -1)
		{
			fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
			exit(1);
		}
			// Envia a fila de mensagens
		Sender(queue_id_1, struct_time, 1);

		exit(0);
	}

	else if (rtn == 0 && count == 2)
	{

		// Sou o filho 2
		// Receber a primeira fila criada pelo filho 1
		if ((queue_id_1 = msgget(key1, 0666 | IPC_CREAT)) == -1)
		{
			fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
			exit(1);
		}

		// Receber a primeira fila criada pelo filho 1
		Receiver(queue_id_1, 2, &struct_time);

		// Criar e enviar a fila pro filho 3

		// Cria a fila 2 de mensagens
		if ((queue_id_2 = msgget(key2, 0666 | IPC_CREAT)) == -1)
		{
			fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
			exit(1);
		}
			// Enviando a fila 2 pro filho 3
		Sender(queue_id_2, struct_time, 2);

		// Removendo a fila de mensagens
		if (msgctl(queue_id_1, IPC_RMID, NULL) == -1)
		{
			fprintf(stderr, "Impossivel remover a fila!\n");
			exit(1);
		}

		exit(0);
	}

	else if (rtn == 0 && count == 3)
	{
		// Sou o filho 3

		if ((queue_id_2 = msgget(key2, 0666 | IPC_CREAT)) == -1)
		{
			fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
			exit(1);
		}

		// Recebendo fila
		Receiver(queue_id_2, 3, &struct_time);

		// Removendo a fila de mensagens
		if (msgctl(queue_id_2, IPC_RMID, NULL) == -1)
		{
			fprintf(stderr, "Impossivel remover a fila!\n");
			exit(1);
		}
	}

	else
	{
		
	// Sou o pai aguardando meus filhos terminarem

		wait(NULL);
		wait(NULL);
		wait(NULL);

		/*
    * Pergunta 7: O que ocorre com a fila de mensagens, se ela n�o � removida e os
    * processos terminam?
      */
		exit(0);
	}
}

/*
 * O tipo de dados seguinte pode ser usado para declarar uma estrutura que
 * contera os dados que serao transferidos pela fila.  A estrutura vai conter
 * um numero de mensagem (msg_no) e o tempo de envio (send_time).  Para filas
 * de mensagens, o tipo da estrutura pode definir qualquer dado que seja necessario.
 */
typedef struct
{
	unsigned int msg_no;
	struct timeval send_time;

} data_t;

/*
 * O conteudo de uma estrutura com o seguinte tipo de dados sera enviado
 * atraves da fila de mensagens. O tipo define dois dados.  O primeiro eh
 * o tipo da mensagem (mtype) que sera como uma identificacao de mensagem.
 * Neste exemplo, o tipo eh sempre o mesmo. O segundo eh um vetor com tamanho
 * igual ao definido pelo tipo declarado anteriormente. Os dados a serem
 * transferidos sao colocados nesse vetor. Na maioria das circunstancias,
 * esta estrutura nao necessita mudar.
 */
typedef struct
{
	long mtype;
	char mtext[5120];
} msgbuf_t;

/*
 * Esta funcao executa o recebimento das mensagens
 */
void Receiver(int queue_id, int num_filho, times *time)
{
	/*
* Variaveis locais
*/
	int count = 0;
	struct timeval receive_time;
	float delta = 0;
	float max = 0;
	float min = 10;
	float total = 0;

	/*
* Este eh o buffer para receber a mensagem
*/
	msgbuf_t message_buffer;

	if (num_filho == 2)
	{

		/*
* Este e o ponteiro para os dados no buffer.  Note
* como e setado para apontar para o mtext no buffer
*/
		data_t *data_ptr = (data_t *)(message_buffer.mtext);

		// Pergunta 8: Qual ser� o conte�do de data_ptr?

		/*
* Inicia o loop
*/
		for (count = 0; count < NO_OF_ITERATIONS; ++count)
		{
			/*
* Recebe qualquer mensagem do tipo MESSAGE_MTYPE
*/
			if (msgrcv(queue_id, (struct msgbuf *)&message_buffer, tam, MESSAGE_MTYPE, 0) == -1)
			{
				fprintf(stderr, "Impossivel receber mensagem!\n");
				exit(1);
			}

			/*
* Chama gettimeofday()
*/
			gettimeofday(&receive_time, NULL);

			/*
* Calcula a diferenca
*/

			delta = receive_time.tv_sec - data_ptr->send_time.tv_sec;
			delta += (receive_time.tv_usec - data_ptr->send_time.tv_usec) / (float)MICRO_PER_SECOND;
			total += delta;

			// Calculando o time maximo
			if (delta > max)
			{
				max = delta;
			}

			// Calculando o time minimo
			if (delta < min)
			{
				min = delta;
			}
		}

		// Calcular tempo medio
		time->time_max = max;
		time->time_min = min;
		time->time_total = total;
		time->time_medio = total/NO_OF_ITERATIONS;
	}

	else if (num_filho == 3)
	{

		times *data_ptr = (times *)(message_buffer.mtext);

		if (msgrcv(queue_id, (struct msgbuf *)&message_buffer, tam, MESSAGE_MTYPE, 0) == -1)
		{
			fprintf(stderr, "Impossivel receber mensagem!\n");
			exit(1);
		}

		printf("Tempo medio de transferencia: %.9f \n", data_ptr->time_medio);
		printf("Tempo maximo de transferencia: %.9f \n", data_ptr->time_max);
	}

	return;
}

/*
 * Esta funcao envia mensagens
 */
void Sender(int queue_id, times time, int num_filho)
{
	/*
* Variaveis locais
*/
	int count;
	struct timeval send_time;

	/*
* Este eh o buffer para receber a mensagem
*/

	msgbuf_t message_buffer;

	if (num_filho == 1)
	{

		/*
* Este e o ponteiro para od dados no buffer.  Note
* como e setado para apontar para mtext no buffer
*/

		data_t *data_ptr = (data_t *)(message_buffer.mtext);

		/*
* Inicia o loop
*/
		for (count = 0; count < NO_OF_ITERATIONS; count++)
		{
			/*
* Chama gettimeofday()
*/
			gettimeofday(&send_time, NULL);

			/*
* Apronta os dados
*/
			message_buffer.mtype = MESSAGE_MTYPE;
			data_ptr->msg_no = count;
			data_ptr->send_time = send_time;

			/*
* Envia a mensagem... usa a identificacao da fila, um ponteiro
* para o buffer, e o tamanho dos dados enviados
*/
			if (msgsnd(queue_id, (struct msgbuf *)&message_buffer, tam, 0) == -1)
			{
				fprintf(stderr, "Impossivel enviar mensagem!\n");
				exit(1);
			}

			/*
* Dorme por um curto espaco de tempo
          */
			usleep(SENDER_DELAY_TIME);
		}
	}
	else
	{

		times *data_ptr = (times *)(message_buffer.mtext);

		data_ptr->time_min = time.time_min;
		data_ptr->time_max = time.time_max;
		data_ptr->time_total = time.time_total;
		data_ptr->time_medio = time.time_medio;

		if (msgsnd(queue_id, (struct msgbuf *)&message_buffer, tam, 0) == -1)
		{
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}

		/*
* Dorme por um curto espaco de tempo
          */
		usleep(SENDER_DELAY_TIME);
	}

	return;
}
