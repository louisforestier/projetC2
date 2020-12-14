#ifndef CLIENT_CRIBLE
#define CLIENT_CRIBLE

// On peut mettre ici des éléments propres au couple master/client :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (création tubes, écriture dans un tube,
//      manipulation de sémaphores, ...)

#define FICHIER_SEMAPHORE_CLIENT "master_client.h"
#define PROJ_ID 5

#define TUBE_CLIENT_MASTER "tube_client_master"
#define TUBE_MASTER_CLIENT "tube_master_client"



// ordres possibles pour le master
#define ORDER_NONE                0
#define ORDER_STOP               -1
#define ORDER_COMPUTE_PRIME       1
#define ORDER_HOW_MANY_PRIME      2
#define ORDER_HIGHEST_PRIME       3
#define ORDER_COMPUTE_PRIME_LOCAL 4   // ne concerne pas le master

// bref n'hésitez à mettre nombre de fonctions avec des noms explicites
// pour masquer l'implémentation



//ceci est un test pour git
//est ce que ça marche ? oui je pense
#endif
