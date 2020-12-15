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
//création tubes nommés

void create_tube1()
{
  int tube1 = mkfifo(TUBE_CLIENT_MASTER, 0666);
  assert(tube1 != -1);
  printf("le tube client master vient d'etre crée\n");
}

void create_tube2(){
  int tube2 = mkfifo(TUBE_MASTER_CLIENT, 0666);
  assert(tube2 != -1);
  printf("le tube master client vient d'etre crée");
}

//==========================================================================
//ouverture tubes nommés

int open_tube1()
{
  int tube1 = open(TUBE_CLIENT_MASTER, O_RDONLY);
  assert(tube1 != -1);
  printf("le tube vient d'etre ouvert en lecture\n");
  return tube1;
}

int open_tube2()
{
  int tube2 = open(TUBE_MASTER_CLIENT, O_WRONLY);
  assert(tube1 != -1);
  printf("le tube vient d'etre ouvert en ecriture\n");
  return tube2;
}

//========================================================================
//fermeture tube nommés

void closetube(int tube)
{
  int g = close(tube);
  assert(g == 0);
  printf("le tube est fermer");
}

//========================================================================
//écriture dans un tube

void write_tube(int tube, int order)
{
  w = write(tube,&order,1);
  assert(w != -1);
}

//========================================================================
//lecture dans un tube

void read_tube(int tube, int result)
{
  r = read(tube,&result,1);
  assert(r >= 0);
}
