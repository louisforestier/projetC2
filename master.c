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

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le master
// a besoin


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


/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(int mutex)
{
  // boucle infinie :
  // - ouverture des tubes (cf. rq client.c)
  // - attente d'un ordre du client (via le tube nommé)
  // - si ORDER_STOP
  //       . envoyer ordre de fin au premier worker et attendre sa fin
  //       . envoyer un accusé de réception au client
  // - si ORDER_COMPUTE_PRIME
  //       . récupérer le nombre N à tester provenant du client
  //       . construire le pipeline jusqu'au nombre N-1 (si non encore fait) :
  //             il faut connaître le plus nombre (M) déjà enovoyé aux workers
  //             on leur envoie tous les nombres entre M+1 et N-1
  //             note : chaque envoie déclenche une réponse des workers
  //       . envoyer N dans le pipeline
  //       . récupérer la réponse
  //       . la transmettre au client
  // - si ORDER_HOW_MANY_PRIME
  //       . transmettre la réponse au client
  // - si ORDER_HIGHEST_PRIME
  //       . transmettre la réponse au client
  // - fermer les tubes nommés
  // - attendre ordre du client avant de continuer (sémaphore : précédence)
  // - revenir en début de boucle
  //
  // il est important d'ouvrir et fermer les tubes nommés à chaque itération
  // voyez-vous pourquoi ?
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
      case ORDER_NONE :
	printf("il ne se passe rien\n");
	write_tube(tube_m_c, &reponse);
	break;
      case ORDER_STOP :
	printf("envoie ordre de fin au premier worker\n");
	end = true;
	write_tube(tube_m_c, &reponse);
	break;
      case ORDER_COMPUTE_PRIME :
	read_tube(tube_c_m, &nb_test);
	printf("construit pipeline\n");
	reponse = 42;
	write_tube(tube_m_c, &reponse);
	break;
      case ORDER_HOW_MANY_PRIME :
	printf("il y a pas encore de nombre premier calculer\n");
	write_tube(tube_m_c, &reponse);
	break;
      case ORDER_HIGHEST_PRIME :
	printf("le plus grand est 2\n");
	write_tube(tube_m_c, &reponse);
	break;
      default : printf("cette commande ne correspond � rien\n");
      }
    closetube(tube_c_m);
    closetube(tube_m_c);
    prendre(mutex);
  }
}






//===========================================================================
//cr�ation s�maphore pour les clients

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

  // - création des sémaphores
  int mutex_syn = create_mutex_syncronisation();
  int mutex_secc = create_mutex_section_critique();
  
  // - création des tubes nommés
  create_tube1();
  create_tube2();
  
  // - création du premier worker

  // boucle infinie
  loop(mutex_syn);

  // destruction des tubes nommés, des sémaphores, ...
  int d1 = semctl(mutex_syn, -1, IPC_RMID);
  assert(d1 != -1);
  int d2 = semctl(mutex_secc, -1, IPC_RMID);
  assert(d2 != -1);
  printf("les semaphores sont d�truis\n");

  int d3 = remove(TUBE_CLIENT_MASTER);
  assert(d3 == 0);
  int d4 = remove(TUBE_MASTER_CLIENT);
  assert(d4 == 0);
  printf("les tubes nomm�es ont �t� d�truis\n");

  return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
