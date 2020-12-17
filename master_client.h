#ifndef CLIENT_CRIBLE
#define CLIENT_CRIBLE


//Clementine Guillot & Louis Forestier


#define FICHIER_SEMAPHORE_CLIENT "master_client.h"
#define PROJ_ID_SYNCRO 5
#define PROJ_ID_SEC_CRITIQUE 22

#define TUBE_CLIENT_MASTER "pipe_cl2ma"
#define TUBE_MASTER_CLIENT "pipe_ma2cl"



// ordres possibles pour le master
#define ORDER_NONE                0
#define ORDER_STOP               -1
#define ORDER_COMPUTE_PRIME       1
#define ORDER_HOW_MANY_PRIME      2
#define ORDER_HIGHEST_PRIME       3
#define ORDER_COMPUTE_PRIME_LOCAL 4   // ne concerne pas le master

void create_tube1();

void create_tube2();

int open_tube_lecture();

int open_tube_ecriture();

void write_tube(int tube, int * result);

void read_tube(int tube, int * result);

void closetube(int tube);

void prendre(int semId);

void vendre(int semId);

#endif
