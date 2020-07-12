#define PROTECT

/*
 * Includes Necessarios 
 */
#include <stdlib.h>
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
#include <sys/msg.h>	/* for msgget(), msgctl() */
#include <string.h>
#include <time.h>           /* for gettimeofday() */

//-------------------------------------------------------Defines
#define MSG "Mensagem escrita na memoria comum"


#define CHAIRS 7   //quantidade de cadeiras para espera
#define CLIENTES 20 //quantidades de clientes
#define BARBEIROS 2 //quantidades de barbeiros
#define NUMFILHOS 22
#define NO_OF_ITERATIONS 0 //PREICSA PEGAR O NUM DE ITERAÇOES
#define SHM_KEY		0x2549
//-------------------------------------------------------
int	*Espera;
int	*g_shm_addr;
int	g_shm_id;
int queue_id;
key_t key = 550;
//-------------------------------------------------------Structs

typedef struct
{
    char vetorPre[1030];
    char vetorPos[1030];
    int num_vet;
    long int pidCli;
    long int pidBar;
    struct timeval send_time;
} data_t;

typedef struct
{
    long int mtype;
    char mtext[sizeof(data_t)];
} msgbuf_t;


void cliente(int pid)
{
    // Variaveis locais

    char vetorRand[1030];
    int count;
    int randOmico = ((rand()%1020)+2);
    int fila = * g_shm_addr;
    struct timeval receive_time;

    if(fila<=7)
    {
        *g_shm_addr=fila+1;
        msgbuf_t message_buffer;
        data_t *data_ptr = (data_t *)(message_buffer.mtext);
        for(int i=0; i<randOmico; i++)
        {
            vetorRand[i]=((rand()+pid)%9+1)+'0';
        }

        strcpy(data_ptr->vetorPre,vetorRand);
        message_buffer.mtype = 55;
        data_ptr->num_vet=randOmico;
        data_ptr->pidCli=pid;
     
       gettimeofday(&receive_time,NULL);
       data_ptr->send_time = receive_time;
        /*
		for(int count=0;count<data_ptr->vetorPre;count++){
			printf("%d   \n", data_ptr->vetorPre[count]);
		}
        *
        if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 )
        {
            fprintf(stderr, "Impossivel enviar mensagem!\n");
            exit(1);
        }
        apreciate_hair(pid); 
    }
    else
    {
        printf("\n cliente %d  não tinha acesso a algum assento, então saiu revoltado voiceferando por aí, foi comprar sorvete \n", pid);
    }
    return;
}

void cut_hair(char vet[], int tam)/*////pode ser int*/
{
    //convercao em inteiros
    SelectSort (vet, tam);
    return;
}

void apreciate_hair(int pid)
{
    float delta;
    msgbuf_t message_buffer;
    struct timeval send_time;
    data_t *data_ptr = (data_t *)(message_buffer.mtext);


    if(msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),pid,0) == -1 )
    {
        fprintf(stderr, "Impossivel receber mensagem!\n");
        exit(1);
    }

    gettimeofday(&send_time,NULL);
    delta = (send_time.tv_usec - data_ptr->send_time.tv_usec)/(float)1000000;
     printf("O barbeiro: %d, cortou o cabelo do cliente: %d! E levou %.20fs para cortar!\n", data_ptr->pidBar, data_ptr->pidCli,delta);

    //Para imprimir o vetor nao ordenado (Pre Barbeiro)
    printf("\nVetor PRE ordenado: \n");
    for(int count=0; count<data_ptr->num_vet; count++)
    {
        printf(" %c |", data_ptr->vetorPre[count]);
    }
    printf("\n");

    //Para imprimir o vetor ordenado (Pos Barbeiro)
    printf("\nVetor POS ordenado:\n");

    for(int count=0; count<data_ptr->num_vet-1; count++)
    {
        printf(" %c |", data_ptr->vetorPos[count]);
    }

    printf("\n");
    return;
}

void barbeiro(int pid)
{
    char * vetOrdenado; 
    msgbuf_t message_buffer;
    data_t *data_ptr = (data_t *)(message_buffer.mtext);
    char  aux[1024];

    while(1)
    {
        //Recebe mensagem do cliente
        if( msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),55,0) == -1 )
        {
            exit(1);
        }

        int fila = *g_shm_addr;
        *g_shm_addr=fila-1;

     strcpy(data_ptr->vetorPos, data_ptr->vetorPre);

    SelectSort (data_ptr->vetorPos,data_ptr->num_vet);


        message_buffer.mtype = data_ptr->pidCli;
        data_ptr->pidBar=pid;
      
        if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 )
        {
            fprintf(stderr, "Impossivel enviar mensagem!\n");
            exit(1);
        }     
    }
    return;
}

void SelectSort(char Vetor[], int Tamanho)
{
    int Cont1=0, Cont2=0, Auxiliar=0, Posicao=0;

    for(Cont1=0; Cont1<(Tamanho-1); Cont1++)
    {
        Posicao=Cont1;

        for(Cont2=(Cont1+1); Cont2<=(Tamanho-1); Cont2++)
        {
            if((int)Vetor[Cont2] > (int)Vetor[Posicao])
            {
                Posicao=Cont2;
            }
        }

        if(Cont1!=Posicao)
        {
            Auxiliar = Vetor[Cont1];
            Vetor[Cont1] = Vetor[Posicao];
            Vetor[Posicao]=Auxiliar;
        }
    }
    return;
}


int main( int argc, char *argv[] )
{
   /*   Variaveis necessarias */
    int count=0;
	int rtn = 1;
    int id_barbeiro = 0;
    int id_cliente = 0;

     int pid[3];

        	
	if( (queue_id = msgget(key, IPC_CREAT | 0666)) == -1 )
    {
        fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
        exit(1);
    }
	

	if( (g_shm_id = shmget( SHM_KEY, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	if( (g_shm_addr = (int *)shmat(g_shm_id, NULL, 0)) == (int *)-1 ) {//recber o endereço 
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	*g_shm_addr = 0;


for(count = 0; count < NUMFILHOS; count++ )
    {
        if( rtn != 0 )
        {
            if(count==1 || count==2)
            {
            pid[count] = rtn = fork();
            }
            else
            {
                rtn = fork();
            }
        }
        else
        {
            break;
        }
    }

 if(rtn==0)
    {
//Sou o barbeiro

        if(count==1 )
        {
            
           barbeiro(count);
        } else {
			if(count==2){	
            barbeiro(count);		
			}
        else
        {
//Sou o clientes
            cliente((count-2));
        }
		}
    }

 else
    {

        // Sou o pai aguardando meus filhos terminarem
        //printf("\n o pai (dono) da barbearia acabou por falir durante o periodo da quaratena, este pretende fechar o estabelicimento,\nculpa disso se da devido seus filhos dorminhocos");
        for( int count = 0; count < 20; count++ )
        {
            wait(NULL);
        }
                if (msgctl(queue_id, IPC_RMID, NULL) == -1)
        {
            fprintf(stderr, "Impossivel remover a fila!\n");
            exit(1);
        }

        //pra fechar a memoria compartilhada
        if( shmctl(g_shm_id,IPC_RMID,NULL) != 0 ) {
            fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
            exit(1);
        }

        kill(pid[1], SIGKILL);
         kill(pid[2], SIGKILL);
                     fprintf(stderr, "salao fechou\n");
         
        //pra fechar a fila de mensagem 
    }
	
 exit(0);
}
