/*
 * INCLUDES 
 */
#include <pthread.h> /* para poder manipular threads */
#include <stdio.h>	 /* para printf() */
#include <stdlib.h>
#include <unistd.h>

/*
 * CONSTANTES
 */
#define NUM_PHI 5								 /* Sera a quantidade de filosofas */
#define NO_OF_ITERATIONS 365					 /* Sera a quantidade de vezes que uma filosofa comera */
#define ID_LEFT (phi_id + NUM_PHI - 1) % NUM_PHI /* Calcula o id da filosofa a esquerda do atual */
#define ID_RIGHT (phi_id + 1) % NUM_PHI			 /* Calcula o id da filosofa a direita do atual */
#define THINKING 0
#define HUNGRY 1
#define EATING 2

/* Struct que sera usada para o status de cada filosofa */
typedef struct
{
	pthread_t phi; /* A filosofa */
	int eaten_no;  /* Guardara o numero de vezes q cada filosofa comeu */
	int status;	   /* Guardara o estado atual da filosofa */
} phi;

/*
 * VARIAVEIS GLOBAIS
 */
pthread_mutex_t mutex;			  /* Regiao critica */
pthread_mutex_t mux_phi[NUM_PHI]; /* Mutex por filosofa */
phi philosophers[NUM_PHI];		  /* Guardara as filosofas e seus status */
int num_phi[5] = {0, 1, 2, 3, 4}; /* Vetor que contem o id das filosofas */

/*
 * PROTOTIPOS
 */
void think(int);
void eat(int);
void status(int);
void take_fork(int);
void return_fork(int);

/* 
 * Rotina sequencial das acoes do filosofo 
 */
void *phi_actions(void *phi_id)
{
	int *id = phi_id; /* Recebe endereco do id da filosofa */
	philosophers[*(id)].eaten_no = 0;
	while (philosophers[*(id)].eaten_no < NO_OF_ITERATIONS)
	{
		think(*(id));
		take_fork(*(id));
		printf("---- A filosofa %d esta COMENDO\n", *(id) + 1, philosophers[*(id)].eaten_no + 1); /* Informa qual filosofa esta comendo */
		philosophers[*(id)].eaten_no++;
		return_fork(*(id));
	}
	printf("---------------------------------------- A filosofa %d TERMINOU\n", *(id) + 1);
	pthread_exit(NULL);
}

/* 
 * Funcao que faz a filosofa de ID x "pensar" por 25 microssegundos
 */
void think(int phi_id)
{
	printf("---- A filosofa %d esta PENSANDO\n", phi_id + 1); /* Informa qual filosofa esta pensando */
	usleep(25);												  /* Tempo em que permanece pensando */
}

/*
 * Funcao que faz a filosofa de ID x entrar na regiao critica,
 * mudar seu estado, verificar se pode comer e bloquear os garfos
 */
void take_fork(int phi_id)
{
	pthread_mutex_lock(&(mutex));							 /* Trava semaforo para entrar na regiсo crítica */
	printf("---- A filosofa %d esta FAMINTA\n", phi_id + 1); /* Informa qual filosofa esta faminta */
	philosophers[phi_id].status = HUNGRY;					 /* Altera o estado da filósofa */
	status(phi_id);											 /* Verifica intencao desta filosofa e o status das filosofas adjacentes */
	pthread_mutex_unlock(&(mutex));							 /* Destrava semaforo */
	pthread_mutex_lock(&(mux_phi[phi_id]));					 /* Bloqueia os garfos */
}

/*
 * Funcao que faz a filosofa de ID x entrar na regiao critica,
 * mudar seu estado, verificar se as filosofas adjacentes podem
 * comer para, entao, liberar os garfos
 */
void return_fork(int phi_id)
{
	pthread_mutex_lock(&(mutex));							  /* Trava semaforo para entrar na regiсo crítica */
	printf("---- A filosofa %d esta PENSANDO\n", phi_id + 1); /* Informa qual filosofa esta pensando */
	philosophers[phi_id].status = THINKING;					  /* Altera o estado da filosofa */
	status(ID_LEFT);										  /* Verifica intencao da filosofa a esquerda */
	status(ID_RIGHT);										  /* Verifica intencao da filosofa a direita */
	pthread_mutex_unlock(&(mutex));							  /* Destrava semaforo */
}

/*
 * Verifica se a filosofa de ID x pode comer, alterando seu
 * estado caso possa, e liberando o semaforo dos garfos para seu ID
 */
void status(int phi_id)
{
	/* Se a filosofa atual estiver com fome
	* e suas adjacentes nao estiverem comendo,
	* a filosofa atual ganha a vez de comer
	*/
	if ((philosophers[phi_id].status == HUNGRY) &&
		(philosophers[ID_LEFT].status != EATING) &&
		(philosophers[ID_RIGHT].status != EATING))
	{
		printf("---- A filosofa %d esta na sua VEZ DE COMER\n", phi_id + 1);
		philosophers[phi_id].status = EATING;	  /* Altera o estado da filosofa */
		pthread_mutex_unlock(&(mux_phi[phi_id])); /* Libera os garfos */
	}
}

/*
 * Rotina Principal (que tambem e a thread principal, quando executada) 
 */
int main(int argc, char *argv[])
{
	int stat; /* Guardar status da criacao da thread */
	int i;

	/* Inicializa os mutexes e as threads filosofas */
	pthread_mutex_init(&(mutex), NULL);
	for (i = 0; i < NUM_PHI; i++)
	{
		pthread_mutex_init(&(mux_phi[i]), NULL);
		/* Tenta criar uma thread filosofa */
		stat = pthread_create(&(philosophers[i].phi), NULL, phi_actions, (void *)&(num_phi[i]));
		if (stat)
		{
			printf("ERRO: Impossivel criar uma thread filosofa\n");
			exit(EXIT_FAILURE);
		}
	}
	/* Aguarda cada uma das threads filhas terminarem antes de prosseguir */
	for (i = 0; i < NUM_PHI; i++)
	{
		stat = pthread_join(philosophers[i].phi, NULL); /* Realiza a espera pelo termino das threads filhas */
		if (stat)
		{
			printf("Erro ao aguardar finalização da thread filosofa %d.\n", i + 1);
			exit(EXIT_FAILURE);
		}
	}

	/* Destroi os semaforos antes de sair */
	pthread_mutex_destroy(&(mutex));
	for (i = 0; i < NUM_PHI; i++)
	{
		pthread_mutex_destroy(&(mux_phi[i]));
	}

	printf("Terminando a thread main()\n");
	pthread_exit(NULL);
}