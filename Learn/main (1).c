#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define N 6 
typedef struct info
{
    int pid;
    int rang;
    int val;
}info;


int mutex,t,mem2,mem1;
int main(){

    key_t cle;
    /* Création du sémaphore mutex */
    cle=ftok("main.c",1);

    mutex=semget(cle,1,IPC_CREAT|0666);
    if(mutex==-1) {printf("\n Erreur de creation des sémaphores"); exit(1);}
    /* Initialisation du mutex */
    semctl(mutex,0,SETVAL,1);

    /* Création du tableau de N sémaphores */

    cle=ftok("main.c",2);

    t=semget(cle,N,IPC_CREAT|0666);

    if(t==-1) {printf("\n Erreur de creation des sémaphores"); exit(1);}

    /* Initialisation des sémaphores */
    for(int i=0;i<N-1;i++){
        semctl(t,i,SETVAL,0);
    }
    semctl(t,N-1,SETVAL,1);

    /* Création du segment partagé pour cpt et v1 */
    cle=ftok("main.c",3);
    mem1=shmget(cle,2*sizeof(int),IPC_CREAT|0666);
    if(mem1==-1) {printf("\n Erreur de creation de la mémoire partagée"); exit(2);}
    /* Attachement */
    int *seg_var= shmat(mem1,0,0);
    /* Initialisation des variables */  
    seg_var[0]=0;
    seg_var[1]=0;
    /* Détachement du segement */
    shmdt(seg_var);


    /* Création du segment partagé pour la table Acces_V1 */
    cle=ftok("main.c",4);
    mem2=shmget(cle,N*sizeof(info),IPC_CREAT|0666);
    if(mem2==-1) {printf("\n Erreur de creation de la mémoire partagée"); exit(2);}
    



    for (int i = 0; i < N; i++)
    {
        int p=fork();
        if(p==-1) {printf("\n Erreur de creation du processus"); exit(3);}
         if(p==0) { 
              int err=execl("./processus","processus",NULL);
               printf("\n Erreur de chargement du programme processus N° erreur %d\n", err); 
               exit(4);  
             }
    }
    


   while(wait(NULL)!=-1);
    
/* Destrcution des sémaphores et des segments partagés */ 

    semctl(mutex,IPC_RMID,0);  
    semctl(t,IPC_RMID,0);  
    shmctl(mem1,IPC_RMID,0);
    shmctl(mem2,IPC_RMID,0);

return 0;
}