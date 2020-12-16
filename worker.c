#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 

#include "myassert.h"

#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/


typedef struct {
  int tube_precedent, tube_suivant, tube_w_m, p;
} worker;


/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
  fprintf(stderr, "usage : %s <n> <fdIn> <fdToMaster>\n", exeName);
  fprintf(stderr, "   <n> : nombre premier géré par le worker\n");
  fprintf(stderr, "   <fdIn> : canal d'entrée pour tester un nombre\n");
  fprintf(stderr, "   <fdToMaster> : canal de sortie pour indiquer si un nombre est premier ou non\n");
  if (message != NULL)
    fprintf(stderr, "message : %s\n", message);
  exit(EXIT_FAILURE);
}

static void parseArgs(int argc, char * argv[], worker * data)
{
  if (argc != 4)
    usage(argv[0], "Nombre d'arguments incorrect");

  data->tube_precedent = (int)strtol(argv[1], NULL, 10);
  data->tube_suivant = -1;
  data->tube_w_m = (int)strtol(argv[2], NULL, 10);
  data->p = (int)strtol(argv[3], NULL, 10);
}

/************************************************************************
 * Fonction annexe pour la création du worker suivant
 ************************************************************************/

static void creerWorkerSuivant(worker * data, int n)
{
  int pipefd[2];
  int r_close;
  int pipe_return = pipe(pipefd);
  myassert(pipe_return >= 0, "echec création tube anonyme vers worker suivant");
  int s_pid = fork();
  myassert(s_pid != -1, "echec création de worker");
  if (s_pid == 0){

    ourclose(pipefd[1]);
    execWorker(pipefd[0], data->tube_w_m, n);
    
  } else {
    ourclose(pipefd[0]);
    data->tube_suivant = pipefd[1];
  }
}


/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(worker * data)
{
  // boucle infinie :
  //    attendre l'arrivée d'un nombre à tester
  //    si ordre d'arrêt
  //       si il y a un worker suivant, transmettre l'ordre et attendre sa fin
  //       sortir de la boucle
  //    sinon c'est un nombre à tester, 4 possibilités :
  //           - le nombre est premier
  //           - le nombre n'est pas premier
  //           - s'il y a un worker suivant lui transmettre le nombre
  //           - s'il n'y a pas de worker suivant, le créer
  bool end = false;
  int n;
  while(!end){
    ourread(data->tube_precedent, &n, sizeof(int));
    
    if (n == -1){
      if (data->tube_suivant != -1){
	
  	int status;
        ourwrite(data->tube_suivant, &n, sizeof(int));
  	int wait_return = wait(&status);
  	myassert(wait_return != -1, "pas de fils à attendre");
      }
      end = true;
      
    } else {
      
      bool result;
      if (n == data->p){
  	result = true;
  	ourwrite(data->tube_w_m, &result, sizeof(bool));
	
      } else if (n % data->p == 0){
  	result = false;
  	ourwrite(data->tube_w_m, &result, sizeof(bool));
	
      } else if (data->tube_suivant != -1){
  	ourwrite(data->tube_suivant, &n, sizeof(int));
	
      } else {
	creerWorkerSuivant(data, n);
      }
    } 
  }
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
  worker data;
  parseArgs(argc, argv, &data);
    
  // Si on est créé c'est qu'on est un nombre premier
  // Envoyer au master un message positif pour dire
  // que le nombre testé est bien premier

  bool is_prime = true;
  ourwrite(data.tube_w_m, &is_prime, sizeof(bool));


  loop(&data);
  
  // libérer les ressources : fermeture des files descriptors par exemple

  ourclose(data.tube_w_m);
  ourclose(data.tube_precedent);
  if (data.tube_suivant != -1)
    ourclose(data.tube_suivant);
  
  return EXIT_SUCCESS;
}
