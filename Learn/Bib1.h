#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define NOMBRE_DE_DISQUES 10
#define NOMBRE_DE_PARTITIONS 512
#define TAILLE_DE_STRING_MAX 256

char* disques[NOMBRE_DE_DISQUES]; // variable globale pour sauvgarder les disques
int disquesNum = 0;
/*  Déclaration des entêtes des fonctions *******/
void Recuperer_Parametre_Partition(FILE *disque,int disque_physique,int partition);
int Liste_Disques();
char* Lire_secteur (FILE *disque,int offset);
void Afficher_secteur (FILE *disque, int Num_sect);
void Afficher_Fdel(int disque_physique,int partition);
int readFromFile(unsigned char *buffer, unsigned int *pcluster, int *poffset);
void myGetfichierInfo(char* buffer,int *pcluster, int *poffset, int count, int LFN);
void search_files_in_partition(int cluster);
void search_files_in_directory(char* directoryName);
//******************************************************************//

FILE *partit;
//------------------------------------------------------
unsigned int tailleSecteur;
unsigned int secteurParCluster;
unsigned int secteursReserves;
unsigned int NombreTablesFAT;
unsigned int tailleFat;
unsigned int clusterRacine;
unsigned int racineAdr;
unsigned int taillePartition;
unsigned int FatAdr;
unsigned int PartitionAdr;
//------------------------------------------------------

typedef struct tfichier{
    char nom_fichier[256];
    char longue_nom_fichier[256];
    int taille;
    int premier_cluster;
    char rep_pere[256];
    char type;
}tfichier;

struct tfichier *fichier;

/************************** Afficher la liste des disques physiques connectés.************************/
int Liste_Disques(){
    FILE *disque=NULL;
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

char* Lire_secteur (FILE *disque,int offset){
    int n,s;
    long int sectLBA = offset;
    unsigned char* buffer;
    buffer = malloc(sizeof(char)*512);
    /* bloc = 512 octets*/
    if(disque == NULL) printf("\nErreur le disque physique n'est pas ouvert\n" );
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
                printf("\nLecture du secteur %d; Nombre d’éléments lus = %d\n",offset/512,n);
            }
        }
    }
    fclose(disque);
    return buffer;
}
/************** lire le secteur Num_sect et affiche son contenu, en hexadécimal ********/

void Afficher_secteur (FILE *disque, int offset){
    unsigned char* buffer;int i;
    int j = 0;
    buffer = Lire_secteur(disque,offset);
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
    FILE *disque=NULL;
    disque = fopen(disques[disque_physique], "rb"); // ouvrir le disque
    if(disque == NULL) {
        printf("\n Erreur le disque physique %d n'est pas ouvert\n",disque_physique );
    }
    else{
    FILE *part=NULL;
    char chemin[1035];
    memset(chemin,'\0',1035);
    char snum[5];memset(snum,'\0',5) ;sprintf(snum, "%d",partition); // construire le chemain
    strcat(chemin,disques[disque_physique]);
    strcat(chemin,snum);
    part = fopen(chemin, "rb"); // ouvrir la partition  
    if(part == NULL) {
        printf("\n Erreur la partition %d n'est pas montée \n",partition );
    }
    else{
        Recuperer_Parametre_Partition(part,disque_physique,partition);
        partit=part;
        if (partit == NULL){
            printf("Error while opening file!\n");
        }
        search_files_in_directory("/");
    }
    fclose(part);
    }
    fclose(disque);
}

// -------------------- Des Fonctions supplémentaire mais necessaires------------------------------//

/**************** Recuperer les Parametres d'une partition    **************************************/

void Recuperer_Parametre_Partition(FILE *disque,int disque_physique,int partition){
    if(disque == NULL) printf("\nErreur le disque physique n'est pas ouvert\n");
    else{  
    unsigned char buffer[512];
    fread(buffer, 1, 512, disque); //lire le premier buffer (boot sector) de la partition
    /* le boot sector contient des données sur la partition donc on recupere ses données */
    tailleSecteur = buffer[11] | buffer[12]<<8;
    secteurParCluster = buffer[13];
    secteursReserves = buffer[14] | buffer[15]<<8;
    NombreTablesFAT = buffer[16];
    tailleFat = buffer[36] | buffer[37]<<8 | buffer[38]<<16 | buffer[39]<<24;
    clusterRacine = buffer[44] | buffer[45]<<8 | buffer[46]<<16 | buffer[47]<<24;
    taillePartition = buffer[32] | buffer[33]<<8 | buffer[34]<<16 | buffer[35]<<24;
    FILE *dp;
    dp = fopen(disques[disque_physique], "rb"); // ouvrir le disque
    if(dp == NULL) {
        printf("\n Erreur le disque physique %d n'est pas ouvert\n",disque_physique );
        FatAdr=0;
        PartitionAdr=0;
    }
    else{
        fread(buffer, 1, 512, dp);
        int i = 454 + (partition-1)*16;
        PartitionAdr = buffer[i] | buffer[i+1]<<8 | buffer[i+2]<<16 | buffer[i+3]<<24;
        FatAdr = (int)PartitionAdr/tailleSecteur + secteursReserves;
        racineAdr = (secteursReserves+NombreTablesFAT*tailleFat)*tailleSecteur;
        fclose(dp);
    }
    printf("----------------- Propriétés de la Partition -----------------------\n");
    printf("Le nombre d'octets par secteur                  :%d\n",tailleSecteur);
    printf("Le nombre de secteurs par cluster               :%d\n",secteurParCluster);
    printf("La taille de la Reserved Region                 :%d\n",secteursReserves);
    printf("La taille de la partition                       :%d\n",taillePartition);
    printf("Adresse début partition                         :0x%x\n",PartitionAdr);
    printf("Le numéro du 1er cluster du répertoire racine   :%d\n",clusterRacine);
    printf("Le nombre de FAT                                :%d\n",NombreTablesFAT);
    printf("L'adresse de la FAT                             :0x%x\n",FatAdr);
    printf("Secteurs occupés par une FAT                    :%d\n",tailleFat);  
    printf("L'Adresse(Racine) de début de la Data Region    :0x%x\n",racineAdr);
    printf("---------------------------------------------------------------------\n");
    }
}

int readFromFile(unsigned char *buffer, unsigned int *pcluster, int *poffset){
    unsigned int fatAdr  = secteursReserves * tailleSecteur;
    if (*poffset == tailleSecteur * secteurParCluster){
        fseek(partit, fatAdr + (*pcluster) * 4, SEEK_SET);
        fread(pcluster,1, 4, partit);
        if (*pcluster >= 0x0FFFFFF8 && *pcluster <= 0x0FFFFFFF) return 0;
        fseek(partit, racineAdr + (*pcluster - clusterRacine) * secteurParCluster * tailleSecteur, SEEK_SET);
        (*poffset) = 0;
    }
    fread(buffer, 1, 32, partit);
    *poffset = *poffset + 32;
    return 1;
}

void myGetfichierInfo(char* buffer,int *pcluster, int *poffset, int count, int LFN){
    if(LFN){
        while(count > 0){
            fichier->longue_nom_fichier[(count - 1)*13 + 0] = buffer[1];
            fichier->longue_nom_fichier[(count - 1)*13 + 1] = buffer[3];
            fichier->longue_nom_fichier[(count - 1)*13 + 2] = buffer[5];
            fichier->longue_nom_fichier[(count - 1)*13 + 3] = buffer[7];
            fichier->longue_nom_fichier[(count - 1)*13 + 4] = buffer[9];
            fichier->longue_nom_fichier[(count - 1)*13 + 5] = buffer[14];
            fichier->longue_nom_fichier[(count - 1)*13 + 6] = buffer[16];
            fichier->longue_nom_fichier[(count - 1)*13 + 7] = buffer[18];
            fichier->longue_nom_fichier[(count - 1)*13 + 8] = buffer[20];
            fichier->longue_nom_fichier[(count - 1)*13 + 9] = buffer[22];
            fichier->longue_nom_fichier[(count - 1)*13 + 10] = buffer[24];
            fichier->longue_nom_fichier[(count - 1)*13 + 11] = buffer[28];
            fichier->longue_nom_fichier[(count - 1)*13 + 12] = buffer[30];
            count--;
        }

        if(!readFromFile(buffer, pcluster, poffset)){
            printf("Error in getfichierInfo!");
            exit(1);
        }
    }
    fichier->taille = buffer[28] | buffer[29]<<8 | buffer[30]<<16 | buffer[31]<<24;
    fichier->premier_cluster = buffer[26] | buffer[27] <<8 | buffer[20]<<16 | buffer[21]<<24;
    fichier->nom_fichier[11] = 0x00;
    int i = 10;
    while(buffer[i] == 0x20) fichier->nom_fichier[i--] = 0x00;
    while(i >= 0) {fichier->nom_fichier[i] = buffer[i]; i--;}
}


void search_files_in_partition(int cluster){
    unsigned int address = racineAdr + (cluster - clusterRacine) * secteurParCluster * tailleSecteur;
    unsigned char buffer[32];
    unsigned int offset = 0;
    fseek(partit, address, SEEK_SET);
    while(readFromFile(buffer,&cluster,&offset)){
        if(buffer[0] == 0x00) break;
        if(buffer[0] == 0xE5){
            printf("Entree Supprimee\n");
        }
//            fseek(partit, -32, SEEK_CUR); offset -= 32;
        int count = 0; int LFN = 0;
        if(buffer[11] == 0X0F) {LFN = 1; count = buffer[0] & 0X0F;}
        myGetfichierInfo(buffer,&cluster, &offset, count, LFN);
        printf("Short Name: %s\n",fichier->nom_fichier);
        if(LFN) printf("Long Name: %s\n",fichier->longue_nom_fichier);
        printf("taille in Bytes: %d\n",fichier->taille);
        printf("Number of the first Cluster: %d\n\n",fichier->premier_cluster);
    };
}

void search_files_in_directory(char* directoryName){
    int cluster = 2;
    unsigned int address = racineAdr + (cluster - clusterRacine) * secteurParCluster * tailleSecteur;
    unsigned char buffer[32];
    unsigned int offset = 0;
    fseek(partit, address, SEEK_SET);
    while(readFromFile(buffer,&cluster,&offset)){
        if(buffer[0] == 0x00) break;
        int count = 0; int LFN = 0;
        if(buffer[11] == 0X0F) {LFN = 1; count = buffer[0] & 0X0F;}
        myGetfichierInfo(buffer, &cluster, &offset, count, LFN);
        if(strcmp(fichier->longue_nom_fichier, directoryName) == 0) {
            search_files_in_partition(fichier->premier_cluster);
            return;
        };
    };
}