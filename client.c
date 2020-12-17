#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <assert.h>
#include <pthread.h>

#include "myassert.h"

#include "master_client.h"


//Clementine Guillot & Louis Forestier


// chaines possibles pour le premier paramÃ¨tre de la ligne de commande
#define TK_STOP      "stop"
#define TK_COMPUTE   "compute"
#define TK_HOW_MANY  "howmany"
#define TK_HIGHEST   "highest"
#define TK_LOCAL     "local"

/************************************************************************
 * Usage et analyse des arguments passÃ©s en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <ordre> [<number>]\n", exeName);
    fprintf(stderr, "   ordre \"" TK_STOP  "\" : arrÃªt master\n");
    fprintf(stderr, "   ordre \"" TK_COMPUTE  "\" : calcul de nombre premier\n");
    fprintf(stderr, "                       <nombre> doit Ãªtre fourni\n");
    fprintf(stderr, "   ordre \"" TK_HOW_MANY "\" : combien de nombres premiers calculÃ©s\n");
    fprintf(stderr, "   ordre \"" TK_HIGHEST "\" : quel est le plus grand nombre premier calculÃ©\n");
    fprintf(stderr, "   ordre \"" TK_LOCAL  "\" : calcul de nombre premier en local\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static int parseArgs(int argc, char * argv[], int *number)
{
    int order = ORDER_NONE;

    if ((argc != 2) && (argc != 3))
        usage(argv[0], "Nombre d'arguments incorrect");

    if (strcmp(argv[1], TK_STOP) == 0)
        order = ORDER_STOP;
    else if (strcmp(argv[1], TK_COMPUTE) == 0)
        order = ORDER_COMPUTE_PRIME;
    else if (strcmp(argv[1], TK_HOW_MANY) == 0)
        order = ORDER_HOW_MANY_PRIME;
    else if (strcmp(argv[1], TK_HIGHEST) == 0)
        order = ORDER_HIGHEST_PRIME;
    else if (strcmp(argv[1], TK_LOCAL) == 0)
        order = ORDER_COMPUTE_PRIME_LOCAL;
    
    if (order == ORDER_NONE)
        usage(argv[0], "ordre incorrect");
    if ((order == ORDER_STOP) && (argc != 2))
        usage(argv[0], TK_STOP" : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME) && (argc != 3))
        usage(argv[0], TK_COMPUTE " : il faut le second argument");
    if ((order == ORDER_HOW_MANY_PRIME) && (argc != 2))
        usage(argv[0], TK_HOW_MANY" : il ne faut pas de second argument");
    if ((order == ORDER_HIGHEST_PRIME) && (argc != 2))
        usage(argv[0], TK_HIGHEST " : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME_LOCAL) && (argc != 3))
        usage(argv[0], TK_LOCAL " : il faut le second argument");
    if ((order == ORDER_COMPUTE_PRIME) || (order == ORDER_COMPUTE_PRIME_LOCAL))
    {
        *number = strtol(argv[2], NULL, 10);
        if (*number < 2)
             usage(argv[0], "le nombre doit Ãªtre >= 2");
    }       
    
    return order;
}


static int my_semget(const int id)
{
  int key = ftok(FICHIER_SEMAPHORE_CLIENT, id);
  assert(key != -1); 
  int semid = semget(key, 1, IPC_EXCL);
  assert(semid >= 0);
  
  return semid;
}


/************************************************************************
 * Calcul en multithread
 ************************************************************************/
 
typedef struct
{
  int n;
  int numero;
  bool *tab;
} ThreadData;

//========================================================================

void *codeThread(void *arg)
{
  ThreadData *data = (ThreadData *)arg;
  *(data->tab) = ((data->n % data->numero) != 0);
  return NULL;
}

//========================================================================

 void localCompute(int n)
{
  bool is_prime = true;
  int sqrt_n = (int)round(sqrt((double)n)) ;
  
  ThreadData * datas = malloc(sizeof(ThreadData) * (sqrt_n - 1));
  pthread_t * tabId = malloc (sizeof(pthread_t) * (sqrt_n - 1));
  bool * tab = malloc (sizeof(bool) * (sqrt_n - 1));

  //On va de 2 jusqu'à racine de n comprise pour que les nombres
  //inférieurs à 10 soit traités correctement.

  //On crée les données pour les threads.
  for(int i = 0; i <= sqrt_n - 2 ; i++){
    datas[i].n = n;
    datas[i].numero = i+2;
    datas[i].tab = tab+i;
  }

  //On lance les threads.
  for (int i = 0; i <= sqrt_n - 2 ; i++){
    pthread_create(&(tabId[i]), NULL, codeThread, &(datas[i]));
  }

  //On attend qu'ils se soient tous finis.
  for (int i = 0 ; i <= sqrt_n - 2 ; i++){
    pthread_join(tabId[i], NULL);
  }

  //On vérifie les valeurs du tableau de bool.
  for (int i = 0 ; i <= sqrt_n - 2 ; i++){
    is_prime = is_prime && tab[i];
  }

  //On libère ce qui a été alloué.
  free(tabId);
  free(tab);
  free(datas);

  if (is_prime)
    printf("Le nombre %d est premier.\n", n);
  else
    printf("Le nombre %d n'est pas premier.\n", n);
}


//==========================================================================
//fonction interpret anwer

static void interpret(int order, int answer)
{ 
  switch(order){ 
    case ORDER_STOP :
      if(answer == 0)
	printf("Le master s'est arrete correctement.\n");
      else
	printf("Le master ne s'est pas arrete correctement.\n");
      break;
    case ORDER_COMPUTE_PRIME :
      if(answer == 0)
	printf("Ce nombre n'est pas premier.\n");
      else
	printf("Ce nomre est premier.\n");
      break;
      
    case ORDER_HOW_MANY_PRIME :
      printf("Le master a calculé %d nombre(s) premier(s).\n", answer);
      break;
      
    case ORDER_HIGHEST_PRIME :
      printf("Le plus grand nombre premier calculé par le master est %d.\n", answer);
      break;
  }
} 


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    int number = 0;
    int order = parseArgs(argc, argv, &number);
    int synchro_id = my_semget(PROJ_ID_SYNCRO);
    int section_critique_id = my_semget(PROJ_ID_SEC_CRITIQUE);


    if (order ==  ORDER_COMPUTE_PRIME_LOCAL)
      {
	localCompute(number);
      }
    else
      {
	int answer;
	prendre(section_critique_id);

	int tube_c_m = open_tube_ecriture(TUBE_CLIENT_MASTER);
	int tube_m_c = open_tube_lecture(TUBE_MASTER_CLIENT);
	
	write_tube(tube_c_m,&order);
	if (order == ORDER_COMPUTE_PRIME)
	  write_tube(tube_c_m, &number);

	read_tube(tube_m_c, &answer);
	
	vendre(section_critique_id);

	closetube(tube_c_m);
	closetube(tube_m_c);

	interpret(order,answer);
      
	vendre(synchro_id);
      }
    
    return EXIT_SUCCESS;
}



