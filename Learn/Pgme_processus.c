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

/* Déclaration des structures et des variables */

/* structure qui contiet les infos d'un processus sur l'accés à la variable v1 */

typedef struct info
{
    /* les données */
    int pid ;
    int rang ;
    int val ;
}info;

/* Préparation des opérations P et V */

 struct sembuf Pmutex={0,-1,0}; 
 struct sembuf Vmutex={0,1,0};

 struct sembuf Ptj={0,-1,0};
 struct sembuf Vtj={0,1,0};


int sema1,sema2,memPart2,memPart1;

int	main() {
    /* Déclaration des variavles */

    int i,j;
    info element;
    key_t cle;

    /* Les sémaphores et Les segments partagés */
    
    cle=ftok("main.c",1);
    sema1=semget(cle,1,0666);
    if(sema1==-1) {printf("\n Erreur de recuperation du sémaphore"); exit(1);}

    cle=ftok("main.c",2);
    sema2=semget(cle,N,0666);
    if(sema2==-1) {printf("\n Erreur de recuperation des sémaphores"); exit(1);}

    cle=ftok("main.c",3);
    memPart1=shmget(cle,2*sizeof(int),0666);
    if(memPart1==-1) {printf("\n Erreur de recuperation de la mémoire partage"); exit(2);}

    cle=ftok("main.c",4);
    memPart2=shmget(cle,N*sizeof(info),0666);
    if(memPart2==-1) {printf("\n Erreur de recuperation de la mémoire partage"); exit(2);}

    /* Attachement des segments partagés */

    int *seg_var= shmat(memPart1,0,0);
    info *Acces_V1= shmat(memPart2,0,0);

    /*Debut du programme*/
  
    semop(sema1,&Pmutex,1);  ///P(mutex)
        j=seg_var[0];
        seg_var[0]=seg_var[0]+1;
        printf("\nProcessus N: %d de PID = %d\n",j,getpid());
    semop(sema1,&Vmutex,1); ///V(mutex)

    Ptj.sem_num=j;
    Vtj.sem_num=((j+N-1)%N);

    semop(sema2,&Ptj,1);   /// P(t[j])
    
        seg_var[1]=seg_var[1]+j;
        element.pid=getpid();
        element.rang=j;
        element.val=seg_var[1];
        Acces_V1[j]=element;

    semop(sema2,&Vtj,1);  /// V(t[(j+N-1)%N])

    if(j==0){
        printf("\n Les resultats d'acces a la variable V1 :\n");
        for (int i = N-1; i >=0; i--){
            printf("\nAcces N : %d , par processus de PID : %d , de rang : %d, v1 = %d :\n",N-i,Acces_V1[i].pid,Acces_V1[i].rang,Acces_V1[i].val);
        }

    }

    /* Détachement des segements */
    shmdt(Acces_V1); 
    shmdt(seg_var);

    return 0;
}




