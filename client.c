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

// chaines possibles pour le premier paramètre de la ligne de commande
#define TK_STOP      "stop"
#define TK_COMPUTE   "compute"
#define TK_HOW_MANY  "howmany"
#define TK_HIGHEST   "highest"
#define TK_LOCAL     "local"

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <ordre> [<number>]\n", exeName);
    fprintf(stderr, "   ordre \"" TK_STOP  "\" : arrêt master\n");
    fprintf(stderr, "   ordre \"" TK_COMPUTE  "\" : calcul de nombre premier\n");
    fprintf(stderr, "                       <nombre> doit être fourni\n");
    fprintf(stderr, "   ordre \"" TK_HOW_MANY "\" : combien de nombres premiers calculés\n");
    fprintf(stderr, "   ordre \"" TK_HIGHEST "\" : quel est le plus grand nombre premier calculé\n");
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
             usage(argv[0], "le nombre doit être >= 2");
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

  //On va de 2 jusqu'� racine de n comprise pour que les nombres
  //inf�rieurs � 10 soit trait�s correctement.

  //On cr�e les donn�es pour les threads.
  for(int i = 2; i <= sqrt_n ; i++){
    datas[i].n = n;
    datas[i].numero = i;
    datas[i].tab = tab+(i-2);
  }

  //On lance les threads.
  for (int i = 2; i <= sqrt_n ; i++){
    pthread_create(&(tabId[i-2]), NULL, codeThread, &(datas[i-2]));
  }

  //On attend qu'ils se soient tous finis.
  for (int i = 2; i <= sqrt_n ; i++){
    pthread_join(tabId[i], NULL);
  }

  //On v�rifie les valeurs du tableau de bool.
  for (int i = 2; i <= sqrt_n ; i++){
    is_prime = is_prime && tab[i-2];
  }

  //On lib�re ce qui a �t� allou�.
  free(tabId);
  free(tab);
  free(datas);

  if (is_prime)
    printf("Le nombre %d est premier.\n", n);
  else
    printf("Le nombre %d n'est pas premier.\n", n);
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


    // order peut valoir 5 valeurs (cf. master_client.h) :
    //      - ORDER_COMPUTE_PRIME_LOCAL
    //      - ORDER_STOP
    //      - ORDER_COMPUTE_PRIME
    //      - ORDER_HOW_MANY_PRIME
    //      - ORDER_HIGHEST_PRIME
    //
    // si c'est ORDER_COMPUTE_PRIME_LOCAL
    //    alors c'est un code complètement à part multi-thread
    // sinon
    //    - entrer en section critique :
    //           . pour empêcher que 2 clients communiquent simultanément
    //           . le mutex est déjà créé par le master
    //    - ouvrir les tubes nommés (ils sont déjà créés par le master)
    //           . les ouvertures sont bloquantes, il faut s'assurer que
    //             le master ouvre les tubes dans le même ordre
    //    - envoyer l'ordre et les données éventuelles au master
    //    - attendre la réponse sur le second tube
    //    - sortir de la section critique
    //    - libérer les ressources (fermeture des tubes, ...)
    //    - débloquer le master grâce à un second sémaphore (cf. ci-dessous)
    // 
    // Une fois que le master a envoyé la réponse au client, il se bloque
    // sur un sémaphore ; le dernier point permet donc au master de continuer
    //
    // N'hésitez pas à faire des fonctions annexes ; si la fonction main
    // ne dépassait pas une trentaine de lignes, ce serait bien.

    if (order ==  ORDER_COMPUTE_PRIME_LOCAL) {
      localCompute(number);
    } else {
      int answer;
      prendre(section_critique_id);
      printf("debut section critique\n");

      int tube_c_m = open_tube_ecriture(TUBE_CLIENT_MASTER);
      printf("ouverture tube1\n");

      int tube_m_c = open_tube_lecture(TUBE_MASTER_CLIENT);
      printf("ouverture tube2\n");
      printf("%d\n", order);
      write_tube(tube_c_m,&order);
      if (order == ORDER_COMPUTE_PRIME)
	write_tube(tube_c_m, &number);
      printf("avant read\n");

      read_tube(tube_m_c, &answer);
      vendre(section_critique_id);
      printf("fin section critique\n");

      closetube(tube_c_m);
      closetube(tube_m_c);
      vendre(synchro_id);
    }
    
    return EXIT_SUCCESS;
}


//=============================================================================
//fonction interprete r�ponse

/* void interprete(int order, int reponse){ */
 
/* } */
