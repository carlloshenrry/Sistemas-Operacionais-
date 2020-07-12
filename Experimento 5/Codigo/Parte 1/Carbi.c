#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"
#include "semaphore.h"
#include <sys/time.h>  /* for gettimeofday() */
#include <sys/types.h> /* for wait(), msgget(), msgctl() */
#include <wait.h>      /* for wait() */
#include <sys/ipc.h>   /* for msgget(), msgctl() */
#include <sys/msg.h>   /* for msgget(), msgctl() */

//DEFINICOES

#define CHAIRS 7 /* número de cadeiras para os clientes à espera */

//Quantidade de clientes.
#define Processes 22

#define BARBEIROS 2
/*MICRO_PER_SECOND define o numero de microsegundos em um segundo*/
#define MICRO_PER_SECOND 1000000

/*
  MESSAGE_QUEUE_ID eh uma chave para acesso a mensagem.
 */
#define MESSAGE_QUEUE_ID 2020

#define SENDER_DELAY_TIME 10 // Tempo quando se envia uma msg

#define MESSAGE_MTYPE 1 // Hora que o cliente chegou e string enviada pelos clientes (cliente envia, barbeiro recebe)
#define MESSAGE_MTYPE2 2 // Tempo para ser atendido e string enviada pelo barbeiro (barbeiro envia, cliente recebe)
#define MESSAGE_MTYPE3 3 //numero de clientes na fila de espera.




/* Clientes */
void cliente(int queue_id);
void cut_hair();
void customer_arrived();
void get_haircut();
void giveup_haircut();
int test_wait();
int queue_id;
int generate_hair(char hair[]);
void appreciate_hair(int num_cliente, int num_barbeiro, char initial_H[],int final_H[],struct timeval elapsed_time,int number_of_random_numbers);

int main(int argc, char *argv[])
{

    //Variaveis para uso na criacao de processos.
    //clientes
    int nprocess = 0;
    int atendimento = 1;
    int loop = 1;
    int dormindo = 0;
    int w;
    int pedido =2;
    int tipo;

    /* Variaveis relativas a fila, id e key*/

    key_t key = MESSAGE_QUEUE_ID;

    /* Criando a fila de mensagens*/
    if ((queue_id = msgget(key, IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
        exit(1);
    }

    test_wait();

    // Iniciando a chegada dos clientes
    for (nprocess = 0; nprocess < Processes; nprocess++)
    {
        if (atendimento != 0)
        {
            atendimento = fork();
        }

        else
        {
            break;
        }
    }

    if (nprocess < 22)
    {
        if (nprocess < 20)
        {
            customer(nprocess);
        }
        else
        {
            barber(nprocess - 19);
        }
    }

    for (int w = 0; w < 22; w++)
    {
        wait(NULL);
    }
    
    exit 0;
}

void barber(int i)
{
    while (1)
    {
        //sem_wait(&customers);  /* vai dormir se o número de clientes for 0 */
        //sem_wait(&mutex);      /* obtém acesso a 'waiting' */
        //waiting = waiting - 1; /*descresce de um o contador de clientes à espera */
        //sem_post(&barbers);    /* um barbeiro está agora pronto para cortar cabelo */
        //sem_post(&mutex);      /* libera 'waiting' */
        //cut_hair();            /* corta o cabelo (fora da região crítica) */
        Barber_Receiver(queue_id); // recebendo pedido 
         if(dormindo==1){
            dormindo= 0;
            }else if(waiting>0){
                    Sender_Barber(queue_id);// enviando estado do barbeiro
            }else Sender_Barber(queue_id);// enviando estado do barbeiro
    }

    pthread_exit(NULL);
}

void customer(int i)
{
    if(test_wait() == 1){
        char hair[1023];
        int n;
        m = generate_hair(hair);
        Sender_Custumer(i,hair,n);
        data_t2* data;
        data = Custumer_Receiver();
        appreciate_hair(i,data->barber_number,hair,data->hair,data->send_time,data->number_of_numbers);
    }
    exit(1);
}

void cut_hair()
{
    printf("Barbeiro estah cortando o cabelo de alguem!\n");
    sleep(3);
}

void customer_arrived()
{
    printf("Cliente chegou para cortar cabelo!\n");
}
void get_haircut()
{
    printf("Cliente estah tendo o cabelo cortado!\n");
}

void giveup_haircut()
{
    printf("Cliente desistiu! (O salao estah muito cheio!)\n");
}





/*
 * O tipo de dados seguinte pode ser usado para declarar uma estrutura que
 * contera os dados que serao transferidos pela fila.  A estrutura vai conter 
 * um numero de mensagem (msg_no), o numero do cliente a ser atendido, a string com um numero aleatório de numeros,
 * o numero de numeros aleatórios gerados e o instante em que o cliente entrou na fila de espera, para o calculo do tempo
 * de espera.
 */
typedef struct
{
	//unsigned int msg_no;
    int customer_number;
    int number_of_numbers;
    char hair [1023];
	struct timeval send_time;
} data_t; 
/*
 * O tipo de dados seguinte pode ser usado para declarar uma estrutura que
 * contera os dados que serao transferidos pela fila.  A estrutura vai conter 
 * um numero de mensagem (msg_no), o resultado do trabalho e o numero de numeros aleatórios que foram trabalhados.
 */
typedef struct
{
	//unsigned int msg_no;
     //int barber_number;
    int hair [1023];
    int number_of_numbers;
	struct timeval send_time;
} data_t2;


/*
 * O tipo de dados seguinte pode ser usado para declarar uma estrutura que
 * contera os dados que serao transferidos pela fila.  A estrutura vai conter 
 * um numero de mensagem (msg_no) e o numero de pessoas na fila de espera, para
 * que esta mensagem fique disponivel para qualquer processo que quiser acessá-la.
 */
typedef struct 
{
    unsigned int msg_no;
    int waiting;
}data_t3;

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
	char mtext[sizeof(data_t2)];
} msgbuf_t;

/*
 *Testa se a fila de espera está vazia e atualiza ela se necessário  
 */
int test_wait(){
    msgbuf_t message_buffer;
    data_t3 *ptr = (data_t3*)(message_buffer.mtext);
    if( msgrcv(queue_id,(msgbuf_t*)&message_buffer,sizeof(data_t3),21,0) == -1){
			fprintf(stderr, "Impossivel receber mensagem 1!\n");
			exit(1);
		}
    message_buffer.mtype = 21;
    if(ptr->waiting == 7){
            if(msgsnd(queue_id,(msgbuf_t*)&message_buffer,sizeof(data_t3),0) == -1){
                fprintf(stderr, "Impossivel receber mensagem 1!\n");
            }
            exit(1);
    }
    else
    {
        ptr->waiting++;
        if(msgsnd(queue_id,(msgbuf_t*)&message_buffer,sizeof(data_t3),0) == -1){
                fprintf(stderr, "Impossivel receber mensagem 1!\n");
                exit(1);
        }
        return 1;
    }
}
/*
 *Recebe pela fila de mensagens o proximo cliente (quem ele é, o horario que ele chegou e a string representando o cabelo dele),
 *bem como o numero de numeros aleatórios gerados.
 */
data_t* Barber_Receiver()
{
    /*
	 * Variaveis locais
	 */
    struct timeval receive_time;
//mytepe=20
    /*
	 * Este eh o buffer para receber a mensagem
	 */
    msgbuf_t message_buffer;

    /*
	 * Este e o ponteiro para os dados no buffer.  Note
	 * como e setado para apontar para o mtext no buffer
	 */
    data_t *data_ptr = (data_t *)(message_buffer.mtext);

                        /*
                        * Recebe qualquer mensagem do tipo MESSAGE_MTYPE
                        */
                        if (msgrcv(queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), 20, 0) == -1)
                        {
                            fprintf(stderr, "Impossivel receber mensagem!\n");
                            exit(1);
                        }
                        /*
                        * Chama gettimeofday()
                        */
                        gettimeofday(&receive_time, NULL);
                        data_ptr->send_time = receive_time ;
            
                            return data_ptr;
       
}

/*
 * Esta funcao envia mensagens
 */
void Sender_Barber(int hair[1023])
{
    /*
	 * Variaveis locais
	 */
    int loop;
    struct timeval send_time;

    /*
	 * Este e o buffer para as mensagens enviadas
	 */
    msgbuf_t message_buffer;

    /*
	 * Este e o ponteiro para od dados no buffer.  Note
	 * como e setado para apontar para mtext no buffer
	 */
    data_t2 *data_ptr = (data_t2 *)(message_buffer.mtext);

        /*
		 * Chama gettimeofday()
		 */
        gettimeofday(&send_time, NULL);

        /*
		 * Apronta os dados
		 */
                    
                    message_buffer.mtype = 20;
                    data_ptr->send_time = send_time;
                    
                    for(loop=0;loop<1023;loop++){
                        data_ptr->hair[loop] = hair[loop];}

                        data_ptr->number_of_numbers=loop;       
                                /*
                    * Envia a mensagem... usa a identificacao da fila, um ponteiro
                    * para o buffer, e o tamanho dos dados enviados
                    */
                    if (msgsnd(queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), 0) == -1)
                    {
                        fprintf(stderr, "Impossivel enviar mensagem!\n");
                        exit(1);
                    }
                    
                    

          
}

void Sender_Custumer(int customer_number,char hair[1023],int number_of_random_numbers){

 /*
	 * Variaveis locais
	 */
    int loop;
    struct timeval send_time;

    /*
	 * Este e o buffer para as mensagens enviadas
	 */
    msgbuf_t message_buffer;


    /*
	 * Este e o ponteiro para od dados no buffer.  Note
	 * como e setado para apontar para mtext no buffer
	 */
    data_t *data_ptr = (data_t *)(message_buffer.mtext);

     /*
	 * Chama gettimeofday()
	 */
     gettimeofday(&send_time, NULL);

     message_buffer.mtype = customer_number;
     data_ptr->send_time = send_time;
     data_ptr->number_of_numbers= number_of_random_numbers;
            
            for(loop=0;loop<1023;loop++){
                    data_ptr->hair[loop] = hair[loop];}   
                     /*
                    * Envia a mensagem... usa a identificacao da fila, um ponteiro
                    * para o buffer, e o tamanho dos dados enviados
                    */
                    if (msgsnd(queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), 0) == -1)
                    {
                        fprintf(stderr, "Impossivel enviar mensagem!\n");
                        exit(1);
                    }

}

data_t2* Custumer_Receiver(int customer_number){
    
     /*
	 * Variaveis locais
	 */
    struct timeval receive_time;
//mytepe=20
    /*
	 * Este eh o buffer para receber a mensagem
	 */
    msgbuf_t message_buffer;

     /*
	 * Este e o ponteiro para os dados no buffer.  Note
	 * como e setado para apontar para o mtext no buffer
	 */
    data_t *data_ptr = (data_t *)(message_buffer.mtext);

                        /*
                        * Recebe qualquer mensagem do tipo MESSAGE_MTYPE
                        */
                        if (msgrcv(queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), customer_number , 0) == -1)
                        {
                            fprintf(stderr, "Impossivel receber mensagem!\n");
                            exit(1);
                        }
            
                            return data_ptr;

}

int generate_hair(char *hair){
    int i;
    int n;
        srand( (unsigned)time(NULL) );
    n = 2 + rand() % 1022 ;
    for(i=1 ; i <= n ; i++)
       hair[i]= ( rand() % 10 ) + '0';
    return n;
}


/*
 * Apresenta na tela o cliente, o barbeiro que cortou o cabelo do cliente, 
 * como estava o cabelo antes do corte, como ficou após o corte e o tempo
 * de espera na fila.
 */
void appreciate_hair(int customer, int barber, char initial_H[1023],int final_H[1023], struct timeval elapsed_time, int rn){
    printf("\n Numero do Cliente: %d\n Numero do Barbeiro:%d\n",customer,barber);
    int i;
    printf(" Cabelo inicial: ");
    for(i = 0;i<rn;i++){
        printf("%c ",initial_H[i]);
    }
    printf("\n Cabelo final: ");
    for(i = 0;i<rn;i++){
        printf("$d ",initial_H[i]);
    }
    printf("\n Tempo de espera: %.2f\n",elapsed_time.tv_sec + elapsed_time.tv_usec);
}


