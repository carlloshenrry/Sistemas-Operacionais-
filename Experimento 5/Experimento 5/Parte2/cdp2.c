#include "pthread.h"
#include "semaphore.h"
#include "time.h"

#define CHAIRS 7 /* número de cadeiras para os clientes à espera */
#define TRUE 1
#define NUM 27

#include "hair.h"

sem_t customers;       /* número de cliente à espera de atendimento */
sem_t barbers;
sem_t print;         /* número de barbeiros à espera de clientes */
sem_t cli[NUM];

pthread_mutex_t mutex; /* para exclusão mútua */
pthread_mutex_t mutex1;

int waiting = 0;       /* clientes que estão esperando (não estão cortando) */


    
/* protótipos */
void *barber(void *arg);
void *customer(void *arg);

data_t buffer[NUM];

typedef struct{
    int ordem_barb;
    int ordem_client;
}gerencia;

gerencia gerente[NUM];

int J=0, K=0;


typedef struct
{
    pthread_t thread;
    int id;
} data_thread;

data_thread b, b2, b3, c[NUM];


//main
int main()
{
    
    b.id=1;
    b2.id=2;
    b3.id=3;

   
    sem_init(&customers, 0, 0);
    sem_init(&barbers, 0, 0);
    sem_init(&print, 0, 1);
    //sem_init(&B2print, 0, 1);

    for (int i = 0; i < NUM; i++)
         sem_init(&cli[i], 0, 0);

    pthread_mutex_init(&(mutex), NULL);
    pthread_mutex_init(&(mutex1), NULL);
    

    /* criando 3 barbeiros */
    pthread_create(&b.thread, NULL, (void *)barber, (void *)&b.id);
    pthread_create(&b2.thread, NULL, (void *)barber, (void *)&b2.id);
    pthread_create(&b3.thread, NULL, (void *) barber,(void *) &b3.id);

    

   
    /* criação de 27 clientes */
    
    for(int i=0;i< NUM; i++){
        c[i].id=i;
        pthread_create(&(c[i].thread), NULL,(void *)customer,(void *)&c[i].id );
    }
    int stat;
    //Loop para aguardar a finalizacao das threads filhas.
    for (int i = 0; i < NUM; i++)
    {
        stat = pthread_join(c[i].thread, NULL);
    }

     for (int i = 0; i < NUM; i++)
        printf(" %d-%d ",gerente[i].ordem_barb , gerente[i].ordem_client);

        printf("\n");
  

    return 0;
}

//-----------------------------------------------------------Iniciando Barbeiro---------------------------------------------------

void *barber(void *arg)
{
    int *id = arg;
    int k;
    while (TRUE)
    {
        
        sem_wait(&customers);       /* vai dormir se o número de clientes for*/
        pthread_mutex_lock(&mutex); /* obtém acesso a 'waiting' */
        k =K;
      // buffer.barber_number=*id;
        
        waiting = waiting - 1;      /*descresce de um o contador de clientes à espera */
      
        gerente[k].ordem_barb =*id;
        buffer[gerente[k].ordem_client].barber_number=*id;
       
        K++;
        pthread_mutex_unlock(&mutex); /* libera 'waiting' */
        
       // sem_wait(&print);
        cut_hair(&buffer[gerente[k].ordem_client]);
       // sem_post(&print);

        sem_post(&cli[gerente[k].ordem_client]);  
      
        sem_post(&barbers);  
         
        
    }
    pthread_exit(NULL);
}

//---------------------------------------------------------------Iniciando Cliente----------------------------------------------
void *customer(void *arg)
{

    int *idCliente = arg;
    int lok = 1;
        while(lok==1){
        pthread_mutex_lock(&mutex); /* entra na região crítica */
        if (waiting < CHAIRS)
        { /* se não houver cadeirquias vazias, saia */
            
         //  usleep(100);
            buffer[*idCliente].customer_number=*idCliente;
            customer_arrived(*idCliente,&(buffer[*idCliente]));
            generate_hair(&(buffer[*idCliente]));
            gerente[J].ordem_client =*idCliente;
            J++;
            waiting = waiting + 1;        /* incrementa o contador de clientes à espera */
            sem_post(&customers);         /* acorda o barbeiro se necessário */
            pthread_mutex_unlock(&mutex); /* libera o acesso a 'waiting' */
            sem_wait(&barbers);           /* vai dormir se o número de barbeiros livres for 0 */
         
             sem_wait(&cli[*idCliente]);
             sem_wait(&print);
             apreciate_hair(*idCliente,&(buffer[*idCliente])); 
             sem_post(&print);
            
           
            lok = 0;
        }
        else
        {
           // printf("ENTROU");
            pthread_mutex_unlock(&mutex); /* a barbearia está cheia; não espera */
            //giveup_haircut(*idCliente);
            sleep(5);
        }
        }
    pthread_exit(NULL);
}

