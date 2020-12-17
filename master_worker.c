//clementine guillot & Louis forestier

#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "myassert.h"

#include "master_worker.h"

//========================================================================
//exec d'un worker

void execWorker(int tube_precedent, int tube_w_m, int p)
{
  char s_tube_precedent[12];
  char s_tube_w_m[12];
  char s_p[12];
  sprintf(s_tube_precedent, "%d", tube_precedent);
  sprintf(s_tube_w_m, "%d", tube_w_m);
  sprintf(s_p, "%d", p);
  char * argv[] = {"worker", s_tube_precedent, s_tube_w_m, s_p, NULL } ;
  int execv_return = execv("worker", argv);
  myassert(execv_return == 0, "echec execv worker");
} 

//========================================================================
//écriture dans un tube

void ourread(int fd, int * result)
{
  int r = read(fd, result, sizeof(int));
  myassert(r != -1, "echec ecriture tube");
}

//========================================================================
//lecture dans un tube

void ourwrite(int fd, int * result)
{
  int w = write(fd, result, sizeof(int));
  myassert(w != -1, "echec lecture tube");
}


//========================================================================
//fermeture tube 

void ourclose(int fd)
{
  int g = close(fd);
  myassert(g == 0, "echec close");
}
