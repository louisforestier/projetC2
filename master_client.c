#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_client.h"

// fonctions Ã©ventuelles proposÃ©es dans le .h



//==========================================================================
//création et ouverture tubes nommés

int create_tube1()
{
  mkfifo(TUBE_CLIENT_MASTER, 0666);
  int tube1 = open(TUBE_CLIENT_MASTER, O_RDONLY);
  assert(tube1 != -1);
  printf("le tube vient d'etre ouvert en lecture\n");

  return tube1;
}

int create_tube2(){
  mkfifo(TUBE_MASTER_CLIENT, 0666);
  int tube2 = open(TUBE_MASTER_CLIENT, O_WRONLY);
  assert(tube1 != -1);
  printf("le tube vient d'etre ouvert en ecriture\n");
  return tube2;
}

//========================================================================
//fermeture tube nommés

void closetube(int tube){
  int g = close(tube);
  assert(g == 0);
  printf("le tube est fermer");
}
