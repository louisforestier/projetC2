#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "myassert.h"

#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le worker
// a besoin : le nombre premier dont il a la charge, ...


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

static void parseArgs(int argc, char * argv[] /*, structure à remplir*/)
{
    if (argc != 4)
        usage(argv[0], "Nombre d'arguments incorrect");

    // remplir la structure
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(/* paramètres */)
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
    read_tube(tube_ordre, &n);
    if (n == -1){
      if (tube_suivant != -1){
  	int status;
  	write_tube(tube_suivant, &n);
  	int wait_return = wait(&status);
  	myassert(wait_return != -1, "pas de fils à attendre");
      }
      end = true;
      
    } else {
      bool result;
      if (n == p){
  	result = true;
  	write_tube(tube_w_m, &result);
      } else if (n % p == 0){
  	result = false;
  	write_tube(tube_w_m, &result);
      } else if (fils != -1){
  	write_tube(tube_suivant, &n);
      } else {
  	int s_pid = fork();
  	myassert(s_pid != -1, "création de worker échouée");
  	if (s_pid == 0){
  	    char s_tube_precedent[12];
  	    char s_tube_w_m[12];
  	    char s_p[12];
  	    sprintf(s_tube_precedent, "%d", tube_precedent);
  	    sprintf(s_tube_w_m, "%d", tube_w_m);
  	    sprintf(s_n, "%d", n);
  	    char * argv[] = {"worker" , s_tube_precedent , s_tube_w_m, s_n, NULL } ;
  	    int execv_return = execv("somme", argv);
  	    assert(execv_return == 0);
  	}
      }
    } 
  }
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
    parseArgs(argc, argv /*, structure à remplir*/);
    
    // Si on est créé c'est qu'on est un nombre premier
    // Envoyer au master un message positif pour dire
    // que le nombre testé est bien premier

    loop(/* paramètres */);

    // libérer les ressources : fermeture des files descriptors par exemple

    return EXIT_SUCCESS;
}
