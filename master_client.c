//clementine guillot & Louis forestier

#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#include "myassert.h"

#include "master_client.h"


//==========================================================================
//création tubes nommés

void create_tube1()
{
  int tube1 = mkfifo(TUBE_CLIENT_MASTER, 0641);
  myassert(tube1 != -1, "pk ca plante");
  printf("le tube client master vient d'etre crée\n");
}

void create_tube2(){
  int tube2 = mkfifo(TUBE_MASTER_CLIENT, 0641);
  assert(tube2 != -1);
  printf("le tube master client vient d'etre crée\n");
}

//==========================================================================
//ouverture tubes nommés

int open_tube_lecture(char * file)
{
  int tube1 = open(file, O_RDONLY);
  assert(tube1 != -1);
  printf("le tube vient d'etre ouvert en lecture\n");
  return tube1;
}

int open_tube_ecriture(char * file)
{
  int tube2 = open(file, O_WRONLY);
  assert(tube2 != -1);
  printf("le tube vient d'etre ouvert en ecriture\n");
  return tube2;
}

//========================================================================
//fermeture tube nommés

void closetube(int tube)
{
  int g = close(tube);
  assert(g == 0);
  printf("le tube est fermer\n");
}

//========================================================================
//écriture dans un tube

void write_tube(int tube, int * result)
{
  int w = write(tube,result,sizeof(int));
  assert(w != -1);
}

//========================================================================
//lecture dans un tube

void read_tube(int tube, int * result)
{
  int r = read(tube,result,sizeof(int));
  assert(r >= 0);
}

//===========================================================================
//attendre pour un semaphore

void prendre(int semId)
{
  struct sembuf operation = {0, -1, 0};
  int ret = semop(semId, &operation, 1);
  assert(ret != -1);
}

//===========================================================================
//vendre pour un semaphore

void vendre(int semId)
{
  struct sembuf operation = {0, +1, 0};
  int ret = semop(semId, &operation, 1);
  assert(ret != -1);
}
