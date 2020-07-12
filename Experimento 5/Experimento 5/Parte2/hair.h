

#include <stdlib.h>

#include <time.h>	 /* for gettimeofday() */

#include <stdio.h>	 /* para printf() */

#include <sys/time.h>
#include <unistd.h>

typedef struct
{
    int barber_number;
    int customer_number;
    int number_of_numbers;
    int hair[1023];
    unsigned char hair2[1023];
    struct timeval send_time;
} data_t;

void customer_arrived(int id, data_t *buffer);
void generate_hair(data_t *buffer);
void cut_hair(data_t *buffer);
void get_haircut(int nCliente);
void giveup_haircut();
void apreciate_hair(int Cliente,data_t *buffer);
void selection_sort(data_t *buffer);




//------------------------------------------cortando cabelo------------------------------------------------
void cut_hair(data_t *buffer)
{
    int loop, t;
    //printf("\nBarbeiro estah cortando o cabelo de alguem!\n");

    for (loop = 0; loop < buffer->number_of_numbers; loop++)
    {
        buffer->hair[loop] = (int)buffer->hair2[loop];
    }
    selection_sort(buffer);
/*
    printf("\n\nCabelo cortado");
      for (t = 0; t < buffer.number_of_numbers; t++){
      printf("%d   ", buffer.hair[t]);
    }*/
     
}
//--------------------------------------------cliente chegou-------------------------------------------
void customer_arrived(int id, data_t *buffer)

{
   // printf("\nCliente %d chegou para cortar cabelo!\n", id);
    struct timeval antes;
    //Calcular o tempo de chegada
    /** Chama gettimeofday()*/
    gettimeofday(&antes, NULL);
    buffer->send_time=antes;
}
//------------------------------------------------cliente tendo vabelo cortado---------------------------------------------
void get_haircut(int nCliente)
{
    printf("\nCliente %d estah tendo o cabelo cortado!\n", nCliente);
}
//--------------------------------------------cliente indo embora---------------------------------------------------
void giveup_haircut(int nCliente)
{
    printf("\nCliente %d desistiu! (O salao estah muito cheio! Voltara mais tarde)\n", nCliente);
    //printf("\nCliente esta voltando.\n");
}

//----------------------------------------gerando o cabelo-------------------------------------------------
void generate_hair(data_t *buffer)
{
   // printf("\n[%d]Gerando cabelo.\n", getpid());
    int i;
    int n;
    int t= buffer->customer_number;
    srand(time(NULL));
    
    n = 2 + (rand()+t*t) % 1022;
    buffer->number_of_numbers = n;
   // printf("Gerando cabelo do cliente\n");
    for (i = 0; i < n; i++)
        buffer->hair2[i] = ((rand()+t*t) % 255);

    return;
}
void selection_sort(data_t *buffer)
{
    int i, j, min, aux, max;
    max = buffer->number_of_numbers;

    for (i = 0; i < (max - 1); i++)
    {
        /*O minimo é o primeiro número não ordenado ainda */
        min = i;
        for (j = i + 1; j < max; j++)
        {
            /*Caso tenha algum numero maior ele faz a troca do minimo*/
            if (buffer->hair[j] < buffer->hair[min])
            {
                min = j;
            }
            
        }
        /* Se o minimo for diferente do primeiro numero não ordenado ele faz a troca para ordena-los*/
        if (i != min)
        {
            aux = buffer->hair[i];
            buffer->hair[i] = buffer->hair[min];
            buffer->hair[min] = aux;
        }
    }
    return;
}
//Saida do Cliente printando todos os dados precisos
void apreciate_hair(int Cliente,data_t *buffer)
{
    int t;
    float delta;
    struct timeval receive_time;          /*varivel para pegar o tempo atual*/
    gettimeofday(&receive_time, NULL); /*Recebendo tempo final do cliente*/
    struct timeval st = buffer->send_time; //Variavel para pular tempo total
    st.tv_sec = receive_time.tv_sec - st.tv_sec;
    st.tv_usec = receive_time.tv_usec - st.tv_usec;
    buffer->send_time = st; //recebendo tempo total gasto.
    delta = buffer->send_time.tv_usec;
printf("\n--------------------------RESULTADO FINAL---------------------------------------\n");
    printf("\nCliente atendido: %d\n", Cliente);
    printf("\nNumero do barbeiro: %d\n",buffer->barber_number);
    printf("\nTempo decorrido:%.2f\n",delta);
   
   printf("\n\nString inicial:\n\n");
   //Printando a string a ser ordenada.
      for (t = 0; t < buffer->number_of_numbers; t++){
         printf("  %d  ", buffer->hair2[t]);}

printf("\n\nString ordenada:\n\n");
   //Printando a string ordenada
    for (t = 0; t < buffer->number_of_numbers; t++){
         printf("  %d  ", buffer->hair[t]);}

 printf("\n\n");
}
