#ifndef MASTER_WORKER_H
#define MASTER_WORKER_H

// On peut mettre ici des éléments propres au couple master/worker :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (écriture dans un tube, ...)

void execWorker(int tube_precedent, int tube_w_m, int p);

void ourread(int fd, void * content, int size);

void ourwrite(int fd, void * content, int size);

void ourclose(int fd);

#endif
