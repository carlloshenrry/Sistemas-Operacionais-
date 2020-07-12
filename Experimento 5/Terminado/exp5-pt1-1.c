/*
 * INCLUDES
 */
#include <stdlib.h>   /* for general purposes */
#include <errno.h>    /* errno and error codes */
#include <sys/time.h> /* for gettimeofday() */
#include <time.h>     /* for time() */
#include <stdio.h>    /* for printf() */
#include <unistd.h>   /* for fork() */
#include <sys/wait.h> /* for wait() */
#include <signal.h>   /* for kill(), sigsuspend(), others */
#include <sys/ipc.h>  /* for all IPC function calls */
#include <sys/shm.h>  /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>  /* for semget(), semop(), semctl() */
#include <sys/msg.h>  /* for msgget(), msgctl() */
#include <string.h>   /* for string manipulation */

/* 
 * DEFINES
 */
#define TRUE 1
#define FALSE 0
#define PROTECT
#define NO_OF_CHILDREN 22
#define NO_OF_CHAIRS 7
#define NO_OF_COSTUMER 20
#define NO_OF_BARBERS 2
#define SIZE_OF_ARRAY 1024
#define SEM_KEY 0x1243
#define SEM_KEY_2 0x1244
#define SHM_KEY 0x1432
#define MESSAGE_QUEUE_ID 550

/* 
 * STRUCTS
 */
typedef struct
{
    unsigned char arr_pre_ord[SIZE_OF_ARRAY + 6];
    unsigned char arr_ord[SIZE_OF_ARRAY + 6];
    unsigned char decrypt_arr[SIZE_OF_ARRAY + 6]; /// Guardara as chaves decriptadoras do array
    int id_costumer;                              /// ID do cliente
    int id_barber;                                /// ID do barbeiro
    int arr_n;                                    /// Quantidade de casas que serao percorridas no vetor
    struct timeval send_time;
} data_t;

typedef struct
{
    long int mtype;
    char mtext[sizeof(data_t)];
} msgbuf_t;

/* 
 * VARIAVELS GLOBAIS
 */
/// Para implementar o segmento de memoria compartilhada & semaforo
int g_sem_id, g_sem2_id;
int g_shm_id;
int *g_shm_addr;

/// Para controlar o semaforo
struct sembuf g_sem_op1[1];
struct sembuf g_sem_op2[1];

/// Para a fila de mensagens
key_t key = MESSAGE_QUEUE_ID;
int msg_queue_id;

/* 
 * PROTOTIPOS
 */
void barber(int);
void costumer(int);
void appreciate_hair(int);
void sort(unsigned char *, unsigned char *, int);
void populate_array(unsigned char *, unsigned char *, int *);
int rand_num(int, int, int *);

/*
 * LOGICA DO PROGRAMA
 * 
 * Se a barbearia não possui clientes, o barbeiro dorme;
 * Se a cadeira do barbeiro estiver livre, um cliente pode ser atendido;
 * O cliente aguarda o barbeiro em caso de uma cadeira de espera livre;
 * Se não houver onde sentar, o cliente vai embora.
 */

void sort(unsigned char *array, unsigned char *decrypt_arr, int max)
{
    int *arr_aux;
    arr_aux = (int *)malloc(max * sizeof(int));

    /// Descriptografa cada numero do array: num = resto + (256 * quociente) e coloca no auxiliar
    for (int i = 0; i < max; i++)
        arr_aux[i] = (int)array[i] + (256 * (int)decrypt_arr[i]);

    int i, j, min, aux;
    for (i = 0; i < (max - 1); i++)
    {
        /// O minimo é o primeiro número não ordenado ainda
        min = i;
        for (j = i + 1; j < max; j++)
        {
            /// Caso tenha algum numero maior ele faz a troca do minimo
            if (arr_aux[j] > arr_aux[min])
            {
                min = j;
            }
        }
        /// Se o minimo for diferente do primeiro numero não ordenado ele faz a troca para ordena-los
        if (i != min)
        {
            aux = arr_aux[i];
            arr_aux[i] = arr_aux[min];
            arr_aux[min] = aux;
        }
    }

    /// Salva os numeros ordenados e criptografados no vetor original
    for (int i = 0; i < max; i++)
    {
        decrypt_arr[i] = arr_aux[i] / 256; /// Gera o quociente (chave decriptadora)
        array[i] = arr_aux[i] % 256;       /// Gera e guarda o resto no vetor de chars
    }
    free(arr_aux);
}

/* 
 * Funcao gera numero randomicos de acordo com os parametros
 * 
 * @range determina o range dos numeros gerados
 * @mod determina a partir de qual numero sera gerado
 * @k modificador da seed
 */
int rand_num(int range, int mod, int *k)
{
    srand(time(NULL) * getpid() + *k);
    return ((rand() % range) + mod);
}

/* 
 * Funcao popula N casas de um vetor de caracteres
 * com N valores inteiros pseudo-aleatorios. Tambem
 * compacta os numeros gerados para armazena-los em
 * um unico byte do vetor de char
 * 
 * @array vetor de caracteres
 */
void populate_array(unsigned char *array, unsigned char *decrypt_arr, int *n)
{
    int k = 0;
    *n = rand_num(SIZE_OF_ARRAY, 2, &k); /// Determina a qtd de casas que serao populadas

    for (int i = 0; i < *n; i++)
    {
        k++;
        int temp_num = rand_num(SIZE_OF_ARRAY, 1, &k); /// Recebe um numero pseudo aleatorio
        decrypt_arr[i] = temp_num / 256;               /// Gera o quociente (chave decriptadora)
        array[i] = temp_num % 256;                     /// Gera e guarda o resto no vetor de chars
    }
}

void barber(int id)
{
    msgbuf_t message_buffer;
    data_t *data_ptr = (data_t *)(message_buffer.mtext);
	
    while (TRUE)
    {
        //printf("\n----- Barbeiro #%d aguardando uma mensagem -----\n", id);
        /// Recebe a mensagem do cliente
        if (msgrcv(msg_queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), 55, 0) == -1)
            exit(1);
        //printf("\n----- Barbeiro #%d recebeu uma mensagem do cliente #%d -----\n", id, data_ptr->id_costumer);

        /// Recebe a quantidade de cadeiras ocupadas do segmento de memoria compartilhada
        int chairs = *g_shm_addr;

        /// Fecha o semaforo para entrar na regiao critica
        if (semop(g_sem_id, g_sem_op2, 1) == -1)
        {
            fprintf(stderr, "Chamada semop() falhou, impossivel fechar o recurso!");
            exit(1);
        }

        /// Atualiza a quantidade de cadeiras livres no segmento de memoria compartilhada
        *g_shm_addr = chairs - 1;

        /// Destrava o semaforo, fim da regiao critica
        if (semop(g_sem_id, g_sem_op1, 1) == -1)
        {
            fprintf(stderr, "Chamada semop() falhou, impossivel liberar o recurso!");
            exit(1);
        }

        strcpy(data_ptr->arr_ord, data_ptr->arr_pre_ord);
        sort(data_ptr->arr_ord, data_ptr->decrypt_arr, data_ptr->arr_n);

        message_buffer.mtype = data_ptr->id_costumer; /// Pega o ID do cliente
        data_ptr->id_barber = id;                     /// Salva o ID do barbeiro

        //printf("Barbeiro #%d ira enviar uma mensagem para o cliente #%d", id, data_ptr->id_costumer);
        /// Envia mensagem de volta para o cliente que esta aguardando em "appreciate_hair()"
        if (msgsnd(msg_queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), 0) == -1)
        {
            fprintf(stderr, "Impossivel enviar mensagem!\n");
            exit(1);
        }
        //printf("\n----- Barbeiro #%d enviou uma mensagem para o cliente #%d -----\n", id, message_buffer.mtype);
    }
}

void costumer(int id)
{
    int chairs = *g_shm_addr; /// Recebe a quantidade de cadeiras ocupadas do segmento de memoria compartilhada
    struct timeval receive_time;
	usleep(250);
    if (chairs < NO_OF_CHAIRS && chairs >= 0)
    {
        /// Fecha o semaforo para entrar na regiao critica
        if (semop(g_sem_id, g_sem_op2, 1) == -1)
        {
            fprintf(stderr, "Chamada semop() falhou, impossivel fechar o recurso!");
            exit(1);
        }

        *g_shm_addr = chairs + 1;

        /// Destrava o semaforo, fim da regiao critica
        if (semop(g_sem_id, g_sem_op1, 1) == -1)
        {
            fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
            exit(1);
        }

        msgbuf_t message_buffer;
        data_t *data_ptr = (data_t *)(message_buffer.mtext);

        populate_array(data_ptr->arr_pre_ord, data_ptr->decrypt_arr, &data_ptr->arr_n); /// Obtem o N e popula o vetor

        message_buffer.mtype = 55;  /// Define o tipo da mensagem
        data_ptr->id_costumer = id; /// Salva o ID do cliente na mensagem
        /// Medicao de tempo
        gettimeofday(&receive_time, NULL);
        data_ptr->send_time = receive_time;

        /// Envia uma mensagem para o barbeiro
        if (msgsnd(msg_queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), 0) == -1)
        {
            fprintf(stderr, "Impossivel enviar mensagem!\n");
            exit(1);
        }
        appreciate_hair(id);
    }
    else
    {
        printf("----- Cliente #%d foi embora, nenhum assento disponivel -----\n", id);
    }
    return;
}

void appreciate_hair(int id)
{
    float delta;
    struct timeval send_time;
    msgbuf_t message_buffer;
    data_t *data_ptr = (data_t *)(message_buffer.mtext);

    //printf("\n----- Cliente #%d aguardando uma mensagem -----\n", id);
    if (msgrcv(msg_queue_id, (struct msgbuf *)&message_buffer, sizeof(data_t), id, 0) == -1)
    {
        fprintf(stderr, "Impossivel receber mensagem!\n");
        exit(1);
    }
    //printf("\n----- Cliente #%d recebeu uma mensagem do barbeiro #%d -----\n", id, data_ptr->id_barber);

    /// Fecha o semaforo para entrar na regiao critica
    if (semop(g_sem2_id, g_sem_op2, 1) == -1)
    {
        fprintf(stderr, "chamada semop() falhou, impossivel fechar o recurso!");
        exit(1);
    }

    gettimeofday(&send_time, NULL);
    delta = (send_time.tv_usec - data_ptr->send_time.tv_usec) / (float)1000000;
    printf("\n\nCliente #%d teve seu cabelo cortado pelo barbeiro #%d! Duracao do corte: %.2fms ---\n", data_ptr->id_costumer, data_ptr->id_barber, delta * 1000.0);

    printf("\nVetor nao ordenado enviado pelo cliente para o barbeiro:\n");
    for (int i = 0; i < data_ptr->arr_n; i++)
        printf("[%d] ", (int)data_ptr->arr_pre_ord[i] + (256 * (int)data_ptr->decrypt_arr[i]));

    printf("\n\nVetor recebido do cliente e ordenado pelo barbeiro:\n");
    for (int i = 0; i < data_ptr->arr_n; i++)
        printf("[%d] ", (int)data_ptr->arr_ord[i] + (256 * (int)data_ptr->decrypt_arr[i]));

    if (semop(g_sem2_id, g_sem_op1, 1) == -1)
    {
        fprintf(stderr, "chamada semop() falhou, impossivel liberar o recurso!");
        exit(1);
    }
    return;
}

int main(int argc, char *argv[])
{
    /*
     * Variaveis necessarias
     */
    int rtn = 1;
    int count = 0;

    /*
     * Para armazenar os pids dos processos filhos,
     * permitindo o posterior uso do comando kill
     */
    int pid[NO_OF_BARBERS];

    /*
     * Construindo a estrutura de controle do semaforo
     */
    g_sem_op1[0].sem_num = 0;
    g_sem_op1[0].sem_op = 1; /// g_sem_op1 com sem_op = 1 destranca o semaforo
    g_sem_op1[0].sem_flg = 0;

    g_sem_op2[0].sem_num = 0;
    g_sem_op2[0].sem_op = -1; /// g_sem_op2 com sem_op = -1 tranca o semaforo
    g_sem_op2[0].sem_flg = 0;

    /* 
     * Criando a fila de mensagens
     */
    if ((msg_queue_id = msgget(key, IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
        exit(1);
    }

    /*
     * Criando o semaforo
     */
    if ((g_sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
        exit(1);
    }

    if ((g_sem2_id = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
        exit(1);
    }

    if (semop(g_sem_id, g_sem_op1, 1) == -1)
    { /// Inicializa com semafaro destrancado -> op1
        fprintf(stderr, "Chamada semop() falhou, impossivel inicializar o semaforo!");
        exit(1);
    }

    if (semop(g_sem2_id, g_sem_op1, 1) == -1)
    { /// Inicializa com semafaro destrancado -> op1
        fprintf(stderr, "Chamada semop() falhou, impossivel inicializar o semaforo!");
        exit(1);
    }

    /*
     * Criando o segmento de memoria compartilhada com
     * o tamanho de um inteiro para as cadeiras livres/ocupadas
     */
    if ((g_shm_id = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "Impossivel criar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    if ((g_shm_addr = (int *)shmat(g_shm_id, NULL, 0)) == (int *)-1)
    {
        fprintf(stderr, "Impossivel associar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    *g_shm_addr = 0;

    /* 
     * Criando processos filhos
     */
    int aux = 0;
    for (count = 0; count < NO_OF_CHILDREN; count++)
    {
        if (rtn != 0)
        {
            if (count == 1 || count == 2)
            {
                pid[aux] = rtn = fork();
                aux += 1;
            }
            else
                rtn = fork();
        }
        else
            break;
    }

    if (rtn == 0)
    {
        if (count == 1 || count == 2)
            barber(count);
        else
            costumer(count - 2);
    }
    else
    { /// Sou pai e aguardo os filhos clientes terminarem
        for (int i = 0; i < NO_OF_COSTUMER; i++)
            wait(NULL);

        printf("Teste");

        /// Fechar memoria compartilhada
        if (shmctl(g_shm_id, IPC_RMID, NULL) != 0)
        {
            fprintf(stderr, "Impossivel remover o segmento de memoria compartilhada!\n");
            exit(1);
        }
        /// Remover fila
        if (msgctl(msg_queue_id, IPC_RMID, NULL) == -1)
        {
            fprintf(stderr, "Impossivel remover a fila!\n");
            exit(1);
        }
        // Fechar semaforos
        if (semctl(g_sem_id, 0, IPC_RMID, 0) != 0)
        {
            fprintf(stderr, "Impossivel remover o conjunto de semaforos!\n");
            exit(1);
        }
        if (semctl(g_sem2_id, 0, IPC_RMID, 0) != 0)
        {
            fprintf(stderr, "Impossivel remover o conjunto de semaforos!\n");
            exit(1);
        }

        /// Matando o processo filho dos barbeiros
        kill(pid[0], SIGKILL);
        kill(pid[1], SIGKILL);

        fprintf(stderr, "Fora do horario de atendimento, o salao fechou!");
    }

    exit(0);
}
