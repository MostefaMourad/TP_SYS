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
char* Lire_secteur (FILE *disque,int Num_sect);
void Afficher_secteur (FILE *disque, int Num_sect);
void Afficher_Fdel(int disque_physique,int partition);
//******************************************************************//
FILE *fichier;
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

struct tfichier{
    char nom_fichier[256];
    char longue_nom_fichier[256];
    int taile;
    int premier_cluster;
    char type;
};

struct buffer {
	unsigned char tab[512]; 		
};
typedef struct buffer buffer;

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

char* Lire_secteur (FILE *disque,int Num_sect){
    int n,s;
    long int sectLBA = 512*Num_sect;
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
                printf("\nLecture du secteur %d; Nombre d’éléments lus = %d\n",Num_sect,n);
            }
        }
    }
    fclose(disque);
    return buffer;
}
/************** lire le secteur Num_sect et affiche son contenu, en hexadécimal ********/

void Afficher_secteur (FILE *disque, int Num_sect){
    unsigned char* buffer;int i;
    int j = 0;
    buffer = Lire_secteur(disque,Num_sect);
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

    }
    fclose(part);
    }
    fclose(disque);
}

// -------------------- Des Fonctions supplémentaire mais necessaires------------------------------//

/**************** Recuperer les Parametre d'une partition    **************************************/

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
        FatAdr = PartitionAdr + secteursReserves;
        racineAdr = FatAdr + (NombreTablesFAT*tailleFat);
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