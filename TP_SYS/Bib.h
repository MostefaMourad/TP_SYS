#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define NOMBRE_DE_DISQUES 10
#define NOMBRE_DE_PARTITIONS 512
#define TAILLE_DE_STRING_MAX 256
#define NOMBRE_MAX_FICHIER 26800000

char* disques[NOMBRE_DE_DISQUES]; // variable globale pour sauvgarder les disques
int disquesNum = 0;
/*  Déclaration des entêtes des fonctions *******/
void Recuperer_Parametre_Partition(FILE *disque,int disque_physique,int partition);
int Liste_Disques();
char* Lire_secteur (FILE *disque,int offset);
void Afficher_secteur (FILE *disque, int Num_sect);
void Afficher_Fdel(int disque_physique,int partition);
void Rechercher_Fichiers_Partition(FILE *partition);
void Afficher_fichiers_Reps(char* buffer);
void get_Fichier_Infos(char* buffer);
struct tfichier *Allouer_F ();
void Afficher_Clusters();
void Parcourir_Cluster(int cluster,FILE *partition);
int est_vide(char *buffer);
int cluster_suivant(FILE *partition,int cluster_courant);
int cluster_suivant_FAT(FILE *partition,int cluster_courant);
int existe(int cluster);
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

typedef struct tfichier{
    char nom_fichier[256];
    int taille;
    int premier_cluster;
    char rep_pere[256];
    char type;
}tfichier;

typedef struct Tcluster
{
    int cluster;
    char *rep_parent;
    char *rep_cour;
}Tcluster;

Tcluster Clusters[NOMBRE_MAX_FICHIER];

int num_cluster;
int indice_courant_cluster;
char *cluster_parent;
char *cluster_courant;
int cluster_actuel;
int stop;
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
    num_cluster=-1;
    for(int i=0;i<26800000;i++){
        (Clusters[i]).cluster = -1;
    }
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
    return buffer;
}
/************** lire le secteur Num_sect et affiche son contenu, en hexadécimal ********/

void Afficher_secteur (FILE *disque, int offset){
    unsigned char* buffer;int i;
    int j = 0;
    buffer = Lire_secteur(disque,offset); //lecture de secteur
    if(buffer!=NULL){
          printf("Adresse\tContenu(octet de 1 à 16)");
    for (i = 0; i < 512; i++) //affichage de secteur
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
    if(part == NULL) { //tester si la partition est monée
        printf("\n Erreur la partition %d n'est pas montée \n",partition ); 
    }
    else{ 
        Recuperer_Parametre_Partition(part,disque_physique,partition); // pour recuperer les parametre de la partition
        Rechercher_Fichiers_Partition(part); // rechercher des fichiers dans la partition
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
    (Clusters[num_cluster+1]).cluster = clusterRacine;
    (Clusters[num_cluster+1]).rep_cour="/";
    (Clusters[num_cluster+1]).rep_parent="N'existe Pas";
    num_cluster+=1;
    indice_courant_cluster=0;
    cluster_parent="N'existe Pas";
    cluster_courant="/";
    cluster_actuel=clusterRacine;
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
        PartitionAdr = buffer[i] | buffer[i+1]<<8 | buffer[i+2]<<16 | buffer[i+3]<<24; // l'adresse de debut de partition
        FatAdr = (int)PartitionAdr/tailleSecteur + secteursReserves;
        racineAdr = (secteursReserves+NombreTablesFAT*tailleFat)*tailleSecteur; // adresse racine
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

/**************** Recuperer les fichiers/repertoires dans une partition   ***************************/

void Rechercher_Fichiers_Partition(FILE *partition){
     int clust = cluster_actuel;
     while (cluster_actuel!=-1)
     {
         Parcourir_Cluster(cluster_actuel,partition); // parcourir le cluster 
         clust = cluster_suivant(partition,cluster_actuel); // trouver le cluster suivant a parcourir
         cluster_actuel=clust;
     }
     
}

/**************** Afficher les fichiers repertoires d'un Secteur    ********************************/

void Afficher_fichiers_Reps(char* buffer){
    int i,j;
    unsigned char* buff;
    for(i=0;i<16;i++){
        buff = malloc(sizeof(char)*32); // allouer la taille d'une enree
        for(j=0;j<32;j++){
           buff[j] = buffer[j+i*32]; // copier l'entree dans un beffer de 32 Octets
        }
        if(est_vide(buff)==-1){ // tester si le buffer est vide pour arreter parcour du cluster et secteur
          stop=-1;
          break;
        }
        else{
          get_Fichier_Infos(buff); // recuperer les information d'un fichier
        }
        buff=NULL;
    }
}

/****************** Recuperer les propriétés d'un fichiers    ***************************************/

void get_Fichier_Infos(char* buffer){
    tfichier *f;
    f=Allouer_F();
    f->taille = buffer[28] | buffer[29]<<8 | buffer[30]<<16 | buffer[31]<<24; //recuperer la taille du fichier
    f->premier_cluster = buffer[26] | buffer[27] <<8 | buffer[20]<<16 | buffer[21]<<24; // recuperer le premier cluster
    f->type = buffer[11]; // recuperer le type de fichier ( fichier ou répertoire)
    f->nom_fichier[11] = 0x00;
    int i = 10;
    while(buffer[i] == 0x20) f->nom_fichier[i--] = 0x00;
    while(i >= 0) {f->nom_fichier[i] = buffer[i]; i--;} // copier le nom du fichier
    if((strcmp(f->nom_fichier,"")!=0) && (f->taille>=0) && (f->premier_cluster>=0)){ // afficher les fichier non defectuex
    printf("----------------- Propriétés du Fichier --------------------------\n");
    printf("Le Nom du Fichier                  :%s\n",f->nom_fichier);
    printf("La Taille du Fichier               :%d\n",f->taille);
    printf("Le Numero du Premier cluster       :%d\n",f->premier_cluster);
    printf("Répetoire Parent                   :%s\n",cluster_parent);
    char *type;
    if(f->type==0x10){ // tester si le fichier est un répertoire
       type = "Répértoire";
       if(existe(f->premier_cluster)==0){ // tester si le repertoire est déja sauvegarder pour ne pas dupliquer
            (Clusters[num_cluster+1]).cluster = f->premier_cluster; // sauvegarde du repertiore pour le parcourir ultirieurement
            (Clusters[num_cluster+1]).rep_cour = f->nom_fichier; // garder son nom
            (Clusters[num_cluster+1]).rep_parent = cluster_courant; // garder le nom son repertoire parent
            num_cluster+=1; 

       }
    }
    else{
       type = "Fichier";
    }
    printf("Type                               :%s\n",type);
    printf("-------------------------------------------------------------------\n");
    }
}

/****************** Allouer une structure de type tfichier    ***************************************/

struct tfichier *Allouer_F ()
{ return ((struct tfichier *) malloc( sizeof(struct tfichier))); } //allouer une structure de type fichier

/****************** Afficher le tableau des clusters disponible a lecture ***************************/

void Afficher_Clusters(){ //fonctions pour faire des test sur la table des repertoire a parcourir
    for(int i=0;i<=num_cluster;i++){
        char *chaine;
        chaine = (Clusters[i]).rep_parent ;
        printf("Le cluster N=°%d est disponible\tRepertoire Parent :%s\n",(Clusters[i]).cluster,chaine);
    }
}

/********************************* Parcourir un cluster *********************************************/

void Parcourir_Cluster(int cluster,FILE *partition){
    unsigned int address = racineAdr + (cluster - clusterRacine) * secteurParCluster * tailleSecteur;
    unsigned char* buffer;
    stop=1;
    for(int i=0;i<secteurParCluster && stop!=-1;i++){ // parcourir les secteurs de cluster
        buffer = NULL;
        buffer = Lire_secteur(partition,address+i*tailleSecteur); // lire le secteur
        Afficher_fichiers_Reps(buffer); //parcourir le secteur pour recuperer les fichiers
    }
}

/********************************* vérifier si l'entree' est vide *********************************/

int est_vide(char *buffer){ 
    for(int j=0;j<32;j++){
        if(buffer[j]!=0x00){ // tester si tout le octets de l'entree de fichier son des zeros donc entree vide
            return 1;
        }
    }
    return -1;
}

/********************************** Trouver le Prochin cluster a lire ******************************/

int cluster_suivant(FILE *partition,int cls){
    int clus = -1 ;
    clus = cluster_suivant_FAT(partition,cls); // chercher dans la table des FATs pour trouver le cluster prochain
    if(clus==-1){ //si le cluster est le dernier dans la chaine ou bien bad cluster , on change vers un autre repertoire
        clus = (Clusters[indice_courant_cluster+1]).cluster; // chercher un cluster dans la table des repertoire
        if(clus!=-1){
            char *chaine,*chaine2;
            chaine = (Clusters[indice_courant_cluster+1]).rep_parent ; //actualiser le repertoire parent
            chaine2 = (Clusters[indice_courant_cluster+1]).rep_cour; //actualiser le repertoire courant
            cluster_parent = chaine;
            cluster_courant = chaine2;
        }
        indice_courant_cluster++;
    }
    return clus;
}

/*********************************** Trouver le Prochain cluster dans la Table Fat ******************/

int cluster_suivant_FAT(FILE *partition,int cluster_act){
    char buffer[4];
    int s,n;
    unsigned int fatAdr  = secteursReserves * tailleSecteur;
    if(partition == NULL) printf("\nErreur la Partition n'est pas montée\n" );
    else{ s=fseek(partition,fatAdr+(cluster_act), SEEK_SET); /*seek par rapport au début du fichier : 0 */
        if(s!=0) {
            printf("\nErreur de fseek : s= %d",s);
            memset(buffer,'\0',4);
        }
        else{ n=fread(buffer,4, 1, partition);
            if(n<=0){
                 printf("\nErreur de fread = %d ",n);
                 memset(buffer,'\0',4);
            }
            else {
                unsigned int a = buffer[0] | buffer[1]<<8 | buffer[2]<<16 | buffer[3]<<24;
                if ( (a >= 0x0FFFFFF8) || (a == 0x00000000) || (a == 0x0FFFFFF7)) //tester si le cluster est le dernier dans la chaine ou bien BAD cluster
                {
                    return -1;
                }
                else
                { // si ce n'est pas le dernier dans la chaine chercher le cluster suivant
                   char little_india[4]; //les donnes son en little india ,
                   for(int i=0;i<4;i++){
                       little_india[i] = buffer[3-i]; //  donc on change vers l'hexadecimal
                   }
                   unsigned int b = little_india[0] | little_india[1]<<8 | little_india[2]<<16 | little_india[3]<<24;
                   return (int)b; //convertir l'hexa vers un entier
                }    
            }
        }
    }
    return -1;
}

/****************************** Tester si le cluster existe déja dans le tableau  ******************/

int existe(int cluster){ // tester si un cluster existe dans le tableau pour ne pas dupliquer
    for(int i=0;i<=num_cluster;i++){ //parcourir le tableau
        if((Clusters[i]).cluster==cluster){ // tester s'il existe
            return 1;
            printf("le cluster :%d\n",cluster);
        }
    }
    return 0;
}