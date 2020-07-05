#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define NOMBRE_DE_DISQUES 10
#define TAILLE_DE_STRING_MAX 256

char* disques[NOMBRE_DE_DISQUES]; // variable globale pour sauvgarder les disques
FILE *disque;
int disquesNum = 0;

FILE *fichier;
//------------------------------------------------------
unsigned int tailleSecteur;
unsigned int secteurParCluster;
unsigned int secteursReserves;
unsigned int NombreTablesFAT;
unsigned int tailleFat;
unsigned int clusterRacine;
unsigned int racineAdr;
//------------------------------------------------------

struct tfichier{
    char nom_fichier[256];
    char longue_nom_fichier[256];
    int taile;
    int premier_cluster;
    char type;
};

/************************** Afficher la liste des disques physiques connectés.************************/
int Liste_Disques(){
    int i = 0;
    char* alph = "abcdefghijklmnopqrstuvwxyz";
    char chemin[1035];
    for(int j=0;j<26;j++){ // Parcourir l'alphabet
        memset(chemin,'\0',1035) ;
        strcpy(chemin,"/dev/sd");
        chemin[strlen(chemin)] = alph[j]; // construire le chemain
        disque = fopen(chemin,"rb"); // ouvrir le repertoire assicié au chemin
        if(disque!= NULL){ // si le disque existe sauvegarder et afficher le chemain
          printf("disque %s connecte\n",chemin);
          disques[i] = malloc(sizeof(char)*TAILLE_DE_STRING_MAX);
          memset(disques[i],'\0',TAILLE_DE_STRING_MAX) ;
          strncpy(disques[i],chemin,strlen(chemin)) ;
          i++;
          fclose(disque);
        }
    }
    disquesNum = i;
    printf("%d disque(s) est (sont) connecte(s)\n",i);
}

/************** Lire le secteur ayant le numéro Num_sect du disque physique disque_physique ********/

char* Lire_secteur (int disque_physique,int Num_sect){
    int n,s;
    long int sectLBA = 512*Num_sect;
    unsigned char* buffer;
    buffer = malloc(sizeof(char)*512);
    /* bloc = 512 octets*/
    FILE *disque=NULL;
    disque = fopen(disques[disque_physique], "rb");
    if(disque == NULL) printf("\nErreur le disque physique %d n'est pas ouvert\n",disque_physique );
    else{ s=fseek(disque, sectLBA, SEEK_SET); /*seek par rapport au début du fichier : 0 */
        if(s!=0) {
            printf("\nErreur de fseek : s= %d",s);
            buffer=NULL;
        }
        else{ n=fread(buffer,512, 1, disque);
            if(n<=0){
                 printf("\nErreur de fread = %d ",n);
                 buffer=NULL;
            }
            else {
                printf("\nLecture du secteur %d; Nombre d’éléments lus = %d\n",Num_sect,n);
            }
        }
    }
    fclose(disque);
    return buffer;
}

/************** lire le secteur Num_sect et affiche son contenu, en hexadécimal ********/

void Afficher_secteur (int disque_physique, int Num_sect){
    unsigned char* buffer;int i;
    int j = 0;
    buffer = Lire_secteur(disque_physique,Num_sect);
    if(buffer!=NULL){
          printf("Adresse\tContenu(octet de 1 à 16)");
    for (i = 0; i < 512; i++)
    {
        if (i > 0) printf(" ");
        if (j % 16 == 0) {
            printf("\n");
            printf("%d\t",j);
        };
        printf("%02X", buffer[i]);
        j++;
    }
    } 
    printf("\n");
}

/****** Afficher les fichiers/répertoires de la partition spécifiée en entrée de type FAT32 ********/

void Afficher_Fdel(int disque_physique,int partition){
    
}

// -------------------- Des Fonctions supplémentaire mais necessaires------------------------------//

/**************** Recuperer les Parametre d'un disque phyisique **************************************/

void Recuperer_Parametre_Disque(int num_disque){

    disque = fopen(disques[num_disque], "rb");
    if(disque == NULL) printf("\nErreur le disque physique %d n'est pas ouvert\n",num_disque );
    else{  
    unsigned char buffer[512];
    fread(buffer, 1, 512, disque);
    tailleSecteur = buffer[11] | buffer[12]<<8;
    secteurParCluster = buffer[13];
    secteursReserves = buffer[14] | buffer[15]<<8;
    NombreTablesFAT = buffer[16];
    tailleFat = buffer[36] | buffer[37]<<8 | buffer[38]<<16 | buffer[39]<<24;
    clusterRacine = buffer[44] | buffer[45]<<8 | buffer[46]<<16 | buffer[47]<<24;
    racineAdr = (secteursReserves + NombreTablesFAT * tailleFat) * tailleSecteur;
    printf("--------------------------------------------------\n");
    printf("Taille du scteur :%d\nsecteur Par Cluster:%d\n secteurs Reserves:%d\n",tailleSecteur,secteurParCluster,secteursReserves);
    printf("Nombre de tables FAT:%d\ntaille Fat:%d\ncluster Racine:%u\nracine Adresses:%u\n",NombreTablesFAT,tailleFat,clusterRacine,racineAdr);
    printf("--------------------------------------------------\n");
    }
}