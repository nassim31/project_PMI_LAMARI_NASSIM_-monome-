#ifndef PMI_H
#define PMI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <ctype.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/time.h>

/* Valeur renvoyée en cas de succès */
#define PMI_SUCCESS 0
/* Valeur renvoyée en cas d’erreur */
#define PMI_ERROR 1
/* Valeur renvoyée si une clef n’est pas trouvée par PMI_KVS_Get */
#define PMI_NO_KEY 2

/* Initialise la bibliothèque client PMI */
int PMI_Init();
/* Libère la bibliothèque client PMI */
int PMI_Finalize(void);
/* Donne le nombre de processus faisant partie du JOB */
int PMI_Get_size(int *size);
/* Donne le rang du processus courant */
int PMI_Get_rank(int *rank);
/* Donne un ID unique pour le job courant */
int PMI_Get_job(int *jobid);

/* Effectue une barrière synchronisante entre les processus */
int PMI_Barrier(void);

/* Définit la longueur maximale d’une CLEF et d’une VALEUR */
#define PMI_STRING_LEN 256

/* Ajoute une clef et une valeur dans le stockage de la PMI */
int PMI_KVS_Put( char key[],  char value[]);

/* Lit une clef depuis le stockage de la PMI */
int PMI_KVS_Get( char key[], char value[], int length);

#endif /* PMI_H */
