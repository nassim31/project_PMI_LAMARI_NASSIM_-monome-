#include "pmi.h"

/* Partie commune aux clients et au serveur */

#define REQUEST_KEY         0x00012345
#define RESPONSE_KEY        0x00012346 
#define WAIT_KEY	    0x00012347
#define LG_MAX              512

typedef struct { 
    long mtype;
    char mtext[ LG_MAX ]; 
} MESSAGE; 

typedef struct {
    int wait;
    int value;
} COUNTER;

/* Fin de la partie commune aux clients et au serveur */

int requests, responses;
MESSAGE msg;
int res;

COUNTER* counter;
int memid;

char command[PMI_STRING_LEN] = "";
char key[PMI_STRING_LEN] = "";
char value[PMI_STRING_LEN] = "";

int size=0;
int rank=0;



int PMI_Init() {
	

	if( PMI_Get_size(&size) != PMI_SUCCESS )
	{
		fprintf(stderr, "Could not get PMI size\n");
		return 1;
	}

	if( PMI_Get_rank(&rank) != PMI_SUCCESS )
	{
		fprintf(stderr, "Could not get PMI rank\n");
		return 1;
	}

	/* se connecter aux IPC de requête et de réponse */
	requests = msgget(REQUEST_KEY, 0700 | IPC_CREAT); 
	if (requests == -1) { perror("msgget"); return (EXIT_FAILURE); } 

	responses = msgget(RESPONSE_KEY, 0700 | IPC_CREAT); 
	if (responses == -1) { perror("msgget"); return (EXIT_FAILURE); }

	/* création ou lien avec la zone partagée */
    	memid = shmget(WAIT_KEY, sizeof(COUNTER), 0700 | IPC_CREAT); 
    	if (memid == -1) { perror("shmget"); return (EXIT_FAILURE); }
	/* montage en mémoire */
    	counter = shmat(memid, NULL, 0);

	return PMI_SUCCESS;
}

/* Libère la bibliothèque client PMI */
int PMI_Finalize(void) {
	sprintf(command,"end");	
	sprintf(msg.mtext, "%s", command);
	msg.mtype = getpid();
	res = msgsnd(requests, & msg, strlen(msg.mtext) + 1, 0); 
	if (res == -1) { perror("msgsnd"); return (EXIT_FAILURE); } 

	/* récupérer sa réponse signée par son numéro de processus */
	res = msgrcv(responses, & msg, LG_MAX, getpid(), 0); 
	if (res == -1) { perror("msgrcv"); return (EXIT_FAILURE); } 

	return PMI_SUCCESS;
}


/* Donne le nombre de processus faisant partie du JOB */
int PMI_Get_size(int *size) {
	char* pmi_process_count = getenv("PMI_PROCESS_COUNT");
	if (pmi_process_count == NULL) {
		return PMI_ERROR;
	}
	*size = atoi(pmi_process_count);
	if (*size <= 0) {
		return PMI_ERROR;
	}

	return PMI_SUCCESS;
}

/* Donne le rang du processus courant */
int PMI_Get_rank(int *rank) {
	char* pmi_rank = getenv("PMI_RANK");
	if (pmi_rank == NULL) {
		return PMI_ERROR;
	}

	*rank = atoi(pmi_rank);
	if (*rank < 0) {
		return PMI_ERROR;
	}
	return PMI_SUCCESS;
}

/* Donne un ID unique pour le job courant */
int PMI_Get_job(int *jobid) {

	return PMI_SUCCESS;
}

/* Effectue une barrière synchronisante entre les processus */
int PMI_Barrier(void) {
	
	sprintf(command,"wait");	
	sprintf(msg.mtext, "%s", command);
	msg.mtype = getpid();
	res = msgsnd(requests, & msg, strlen(msg.mtext) + 1, 0); 
	if (res == -1) { perror("msgsnd"); return (EXIT_FAILURE); } 

	/* récupérer sa réponse signée par son numéro de processus */
	res = msgrcv(responses, & msg, LG_MAX, getpid(), 0); 
	if (res == -1) { perror("msgrcv"); return (EXIT_FAILURE); } 

	int i=0;
	while(counter->wait==1){
	  usleep(1);
	}
	
	return PMI_SUCCESS;
}

int PMI_KVS_Put(char key[], char value[]) {

	sprintf(command,"put");
	sprintf(key,"%s",key);
	sprintf(value,"%s",value);	
	sprintf(msg.mtext, "%s,%s,%s", command,key, value);
	
	/* envoyer la requête signée par son numéro de processus */
	msg.mtype = getpid();
	res = msgsnd(requests, & msg, strlen(msg.mtext) + 1, 0); 
	if (res == -1) { perror("msgsnd"); return (EXIT_FAILURE); } 

	/* récupérer sa réponse signée par son numéro de processus */
	res = msgrcv(responses, & msg, LG_MAX, getpid(), 0); 
	if (res == -1) { perror("msgrcv"); return (EXIT_FAILURE); } 
	
	return PMI_SUCCESS;
}

int PMI_KVS_Get(char key[], char value[], int length) {
    
	sprintf(command,"get");
	sprintf(key,"%s",key);	
	sprintf(msg.mtext, "%s,%s", command,key); 
   
	/* envoyer la requête signée par son numéro de processus */
	msg.mtype = getpid();
	res = msgsnd(requests, & msg, strlen(msg.mtext) + 1, 0); 
	if (res == -1) { perror("msgsnd"); return (EXIT_FAILURE); } 

	/* récupérer sa réponse signée par son numéro de processus */
	res = msgrcv(responses, & msg, LG_MAX, getpid(), 0); 
	if (res == -1) { perror("msgrcv"); return (EXIT_FAILURE); } 
	sprintf(key, "%s", strtok(msg.mtext, ","));
	sprintf(value, "%s", strtok(NULL, ","));
	
	return PMI_SUCCESS;
}

