#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"


//Clementine Guillot & Louis Forestier


/************************************************************************
 * Donnees persistantes d'un master
 ************************************************************************/

static int nbCalcul = 1;
static int highestPrime = 2;


/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
  fprintf(stderr, "usage : %s\n", exeName);
  if (message != NULL)
    fprintf(stderr, "message : %s\n", message);
  exit(EXIT_FAILURE);
}


//==========================================================================
//sous fonctions du switch case

void order_stop(int tube_m_w, int tube_m_c, bool *end)
{
  int send = STOP_SIGNAL;
  int reponse = 0;
  printf("Envoie signal d'arret aux workers.\n");
  ourwrite(tube_m_w, &send);
  write_tube(tube_m_c, &reponse);
  *end = true;
  int w = wait(NULL);
  assert(w != -1);
}

void order_compute_prime(int tube_c_m, int * nb_test, int tube_m_w, int tube_w_m, int * reponse, int tube_m_c)
{
  read_tube(tube_c_m, nb_test);
  
  if(*nb_test > highestPrime)
    {
      for(int i = highestPrime + 1 ; (i <= *nb_test)  ; i++){
	ourwrite(tube_m_w, &i);
	ourread(tube_w_m, reponse);
	
	if(*reponse == 1)
	  {
	    nbCalcul += 1;
	    highestPrime = i;
	  }
      }
    }
  else
    {
      ourwrite(tube_m_w, nb_test);
      ourread(tube_w_m, reponse);
    }
  write_tube(tube_m_c, reponse);
	
}


/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(int mutex, int tube_m_w, int tube_w_m)
{
  bool end = false;
  while(!end) {
    int tube_c_m = open_tube_lecture(TUBE_CLIENT_MASTER);
    int tube_m_c = open_tube_ecriture(TUBE_MASTER_CLIENT);

    int order;
    int nb_test;
    int reponse = -1;
    
    read_tube(tube_c_m, &order);
    switch(order)
      {	
	case ORDER_STOP :
	  order_stop(tube_m_w, tube_m_c, &end);
	  break;
	
	case ORDER_COMPUTE_PRIME :
	  printf("Calcul d'un nombre premier.\n");
	  order_compute_prime(tube_c_m, &nb_test, tube_m_w, tube_w_m, &reponse, tube_m_c);
	  break;
	
	case ORDER_HOW_MANY_PRIME :
	  write_tube(tube_m_c, &nbCalcul);
	  break;
	
	case ORDER_HIGHEST_PRIME :
	  write_tube(tube_m_c, &highestPrime);
	  break;
      }
    closetube(tube_c_m);
    closetube(tube_m_c);
    prendre(mutex);
  }
}



//===========================================================================
//creation semaphore pour les clients

static int create_mutex_syncronisation()
{
  key_t key1 = ftok(FICHIER_SEMAPHORE_CLIENT,PROJ_ID_SYNCRO);
  assert(key1 != -1);
  int s = semget(key1, 1, 0641 | IPC_CREAT | IPC_EXCL);
  assert(s != -1);
  int init = semctl(s, 0, SETVAL, 0);
  assert(init != -1);
  return s;
}

static int create_mutex_section_critique()
{
  key_t key2 = ftok(FICHIER_SEMAPHORE_CLIENT,PROJ_ID_SEC_CRITIQUE);
  assert(key2 != -1);
  int s = semget(key2, 1, 0641 | IPC_CREAT | IPC_EXCL);
  assert(s != -1);
  int init = semctl(s, 0, SETVAL, 1);
  assert(init != -1);
  return s;
}

/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
  if (argc != 1)
    usage(argv[0], NULL);

  // - creation des semaphores
  int mutex_syn = create_mutex_syncronisation();
  int mutex_secc = create_mutex_section_critique();
  
  // - creation des tubes nommes
  create_tube1();
  create_tube2();
  
  // - creation du premier worker et des tubes anonymes
  int tube_m_w[2];
  int tube_w_m[2];
  int p1 = pipe(tube_m_w);
  assert(p1 != -1);
  int p2 = pipe(tube_w_m);
  assert(p2 != -1);

  int pid = fork();
  assert(pid != -1);

  if(pid == 0){
    ourclose(tube_m_w[1]);
    ourclose(tube_w_m[0]);
    execWorker(tube_m_w[0], tube_w_m[1], 2);
  } else {
    ourclose(tube_w_m[1]);
    ourclose(tube_m_w[0]);
  }
  
  printf("Lancement du master et du worker 2 effectue.\n");

  // boucle infinie
  loop(mutex_syn, tube_m_w[1], tube_w_m[0]);

  printf("Arret du master.\n");
  // destruction des tubes nommees et des semaphores
  int d1 = semctl(mutex_syn, -1, IPC_RMID);
  assert(d1 != -1);
  int d2 = semctl(mutex_secc, -1, IPC_RMID);
  assert(d2 != -1);

  int d3 = remove(TUBE_CLIENT_MASTER);
  assert(d3 == 0);
  int d4 = remove(TUBE_MASTER_CLIENT);
  assert(d4 == 0);

  return EXIT_SUCCESS;
}

