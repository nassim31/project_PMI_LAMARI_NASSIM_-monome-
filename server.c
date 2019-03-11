#include "pmi.h"

void server_listen();
void handle_request();

/* Partie commune aux clients et au serveur */

#define REQUEST_KEY         0x00012345
#define RESPONSE_KEY        0x00012346 
#define WAIT_KEY	    0x00012347
#define LG_MAX              512
#define KVS_MAX		    16384


typedef struct { 
    long mtype;
    char mtext[ LG_MAX ]; 
} MESSAGE; 

typedef struct {
    volatile int wait;
    volatile int value;
} COUNTER;

//Structure d'une pair de key,value
typedef struct node {
	char key[PMI_STRING_LEN];
	char value[PMI_STRING_LEN];
} node_t;

typedef struct table {
	node_t nodes[KVS_MAX];
	int size;
} table_t;

table_t* kvs;

/* Fin de la partie commune aux clients et au serveur */
int requests, responses;
int counter = 0;


MESSAGE msg;

int res, i; 

char command[PMI_STRING_LEN] = "";
char key[PMI_STRING_LEN] = "";
char value[PMI_STRING_LEN] = "";

//barrière
int end_count=0;
long ids[2];
COUNTER *cpt;
int memid;

int PMI_Init() {
	
    	kvs=malloc(sizeof(table_t));

    	/* Se connecter aux IPC de requête et de réponse */
    	requests = msgget(REQUEST_KEY, 0700 | IPC_CREAT); 
    	if (requests == -1) { 
		perror("msgget"); 
		return (EXIT_FAILURE); 
	} 

	responses = msgget(RESPONSE_KEY, 0700 | IPC_CREAT); 
	if (responses == -1) { 
		perror("msgget"); 
		return (EXIT_FAILURE); 
	} 

	/* création ou lien avec la zone partagée */
 	memid = shmget(WAIT_KEY, sizeof(COUNTER), 0700 | IPC_CREAT); 
    	if (memid == -1) { perror("shmget"); return (EXIT_FAILURE); }
	/* montage en mémoire */
    	cpt = shmat(memid, NULL, 0);
	cpt->wait=0;
	cpt->value=0;
}

/* Libère la bibliothèque client PMI */
int PMI_Finalize(void) {
	
	free(kvs);
	shmdt(cpt);
	if (msgctl(requests, IPC_RMID, NULL) == -1) {
		fprintf(stderr, "Message queue could not be deleted.\n");
		exit(PMI_ERROR);
	}
	if (msgctl(responses, IPC_RMID, NULL) == -1) {
		fprintf(stderr, "Message queue could not be deleted.\n");
		exit(PMI_ERROR);
	}
	return PMI_SUCCESS;
}

void server_listen(){

   while (1) {
	handle_request();
	if(end_count==2){
		break;
	}
    }
}

void handle_request(){
	
	//printf("Waiting a request (%d processed)...\n", counter);
        res = msgrcv(requests, & msg, LG_MAX, 0, 0);
        if (res == -1) {
		 perror("msgrcv");
		 return (EXIT_FAILURE); 
	} 

	
	sprintf(command, "%s", strtok(msg.mtext, ","));
	
	if (strcmp(command, "wait")==0) {
		PMI_Barrier();
		sprintf(msg.mtext, "OK");
	}

	if (strcmp(command, "end")==0) {
		end_count++;
		sprintf(msg.mtext, "OK");
	}

	if (strcmp(command, "put")==0) {
		sprintf(key, "%s", strtok(NULL, ","));
		sprintf(value, "%s", strtok(NULL, ","));
		if (PMI_KVS_Put(key, value) == PMI_SUCCESS) {
			sprintf(msg.mtext, "OK");
		} else {
			sprintf(msg.mtext, "KO");
		}
		
	}

	if (strcmp(command, "get")==0) {
		sprintf(key, "%s", strtok(NULL, ","));
		if (PMI_KVS_Get(key, value,PMI_STRING_LEN) == PMI_SUCCESS) {
			sprintf(msg.mtext, "%s,%s",key,value);
		} else {
			sprintf(msg.mtext, "KO");
		}
		
	}
	/* Envoyer la réponse (signée par le client) */
        res = msgsnd(responses, & msg, strlen(msg.mtext) + 1, 0); 
        if (res == -1) { 
		perror("msgsnd"); 
		return (EXIT_FAILURE); 
	}        
       	counter++;
	bzero(command, sizeof(command));
	bzero(key, sizeof(key));
	bzero(value, sizeof(value));

	

}


/* Lit une clef depuis le stockage de la PMI */
int PMI_KVS_Get(char key[], char value[], int length) {

	int found=0;
	if (kvs == NULL) {
		return PMI_ERROR;
	}

	for (int i = 0; i < kvs->size; i++) {
		if (strcmp((kvs->nodes[i].key), key) == 0) {
			found=1;
			strcpy(key,kvs->nodes[i].key);
			strcpy(value,kvs->nodes[i].value);
		}
	}
	if(found){
		return PMI_SUCCESS;
	}else{
		return PMI_ERROR;
	}
	

}

/* Donne un ID unique pour le job courant */
int PMI_Get_job(int *jobid) {

	return PMI_SUCCESS;
}

/* Effectue une barrière synchronisante entre les processus */
int PMI_Barrier(void) {
	cpt->wait=1;	
	cpt->value++;

	if(cpt->value==2){
		cpt->wait=0;
		cpt->value=0;
	        
	}

	return PMI_SUCCESS;
}

/* Ajoutte une clef, valeur dans le stockage de la PMI */
int PMI_KVS_Put(char key[], char value[]) {

	if (kvs == NULL) {
		return PMI_ERROR;
	}
	for (int i = 0; i < kvs->size; i++) {
		if (strcmp((kvs->nodes[i].key), key) == 0) {
			return PMI_ERROR;
		}
	}

	if (kvs->size < KVS_MAX) {
		strcpy(kvs->nodes[kvs->size].key, key);
		strcpy(kvs->nodes[kvs->size].value, value);
		kvs->size++;
		return PMI_SUCCESS;
	}
	return PMI_ERROR;
}


int main(void) { 
   
	PMI_Init();
	server_listen();
	PMI_Finalize();
	return (EXIT_SUCCESS); 
}





