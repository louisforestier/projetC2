#ifndef MASTER_WORKER_H
#define MASTER_WORKER_H


//Clementine Guillot & Louis Forestier


#define IS_PRIME 1
#define IS_NOT_PRIME 0
#define STOP_SIGNAL -1


void execWorker(int tube_precedent, int tube_w_m, int p);

void ourread(int fd, int * result);

void ourwrite(int fd, int * result);

void ourclose(int fd);

#endif
