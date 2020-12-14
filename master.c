#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
void loop(/* paramètres */)
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
  
}


//===========================================================================
//cr�ation s�maphore pour les clients

static int my_semget(int nbreWorkers)
{
  /* key_t key = ftok(MON_FICHIER,PROJ_ID); */
  /* assert(key != -1); */
  /* int s = semget(key, 1, 0641 | IPC_CREAT | IPC_EXCL); */
  /* assert(s != -1); */
  /* int init = semctl(s, 0, SETVAL, nbreWorkers); */
  /* assert(init != -1); */
  /* return s; */
}

//==========================================================================
//cr�ation et ouverture tubes nomm�s

static int create_tube1(){
  mkfifo(TUBE_CLIENT_MASTER, 0666);
  int tube1 = open(TUBE_CLIENT_MASTER, O_RDONLY);
  assert(tube1 != -1);
  printf("le tube vient d'etre ouvert en lecture\n");

  return tube1;
}

static int create_tube2(){
  mkfifo(TUBE_MASTER_CLIENT, 0666);
  int tube2 = open(TUBE_MASTER_CLIENT, O_WRONLY);
  assert(tube1 != -1);
  printf("le tube vient d'etre ouvert en ecriture\n");
  return tube2;
}

//========================================================================
//fermeture tube nomm�s

static void closetube(int tube){
  int g = close(tube);
  assert(g == 0);
  printf("le tube est fermer");
}

/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
  if (argc != 1)
    usage(argv[0], NULL);

  // - création des sémaphores
  // - création des tubes nommés
  // - création du premier worker

  // boucle infinie
  loop(/* paramètres */);

  // destruction des tubes nommés, des sémaphores, ...

  return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
