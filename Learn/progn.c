 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#define NUMBER_OF_DIQUES 10
#define MAX_TAILLE_CHAINE 256
#define TAILLE_SECTEUR 512
#define ERR while(1){printf("Error : %s\n",strerror(errno));exit(1);}
#define drive FILE
#define RED(str) "\e[0;91;49m"str"\e[0m"
#define GREEN(str) "\e[0;92;49m"str"\e[0m"
/***partie declaration des variables et structures globales***/
void Lire_secteur(drive* disque, unsigned int num_secteur, char* buffer);
unsigned char buffer[512];
drive* disque=NULL;
FILE *disque1=NULL;
int i=0,n=0,k=0;
long int j=0;
int choix,num;
char* disques[NUMBER_OF_DIQUES];
char temp[TAILLE_SECTEUR];
char nom_pare[] ="vide"; 
int disquesNum = 0;
FILE *file;
/***********************************************************/
unsigned int TailleSecteur;
unsigned int SectParCluster;
unsigned int SecteursReserve;
unsigned int NombreTablesFat;
unsigned int TailleFat;
unsigned int rootCluster;
unsigned int rootAdr;
/***********************************************************/

struct tfichier{
    char nom_fich[256];
    char long_nom_fich[256];
    int taille;
    int premier_cluster;
    char type;
};
typedef struct Repertoire_Entree {
	unsigned char nom_fich[8];
	unsigned char extension[3];
	int8_t attrib;
	int8_t reserve;
	int8_t millisecondes_creation;
	int16_t heure_creation;
	int16_t date_creation;
	int16_t date_dernier_acces;
	int16_t numero_premier_cluster_fort;
	int16_t heure_derniere_modification;
	int16_t date_derniere_modification;
	int16_t numero_premier_cluster_faible;
	int32_t Taille_fich;
} Repertoire_Entree ;

/***Partie des prototypes des fonctions/procedures***/
void afficherDisquesConnectes();
void Liste_Disques();
//void follow_linked_list(FILE* disque, int16_t secteurs_reserves, int8_t secteurs_par_cluster, int32_t rootcluster_offset, int32_t cluster_du_root, int32_t parent_cluster);
//void Lire_secteur(drive* disque,int Num_Sect,char* buffer);
//void Afficher_Secteur(FILE *disque,int Num_Sect);
/***Partie implementation des fonctions/procedures***/
void afficherDisquesConnectes() /* fonction qui affiche les disques connecte*/
{
	//On essaie d'ouvrir chaque disque, et si l'ouverture fonctionne, c'est que le disque est connecté
	//Ici c'est juste un essai pour quelques disques
	disque1= fopen("/dev/sda","rb");
	if (disque1 != NULL)
	{
		printf("disque sda connecte\n");
		fclose(disque1);
	} 	
	disque1 = fopen("/dev/sdb","rb");
	if (disque1 != NULL)
	{
		printf("disque sdb connecte\n");
		fclose(disque1);
	} 
	disque1 = fopen("/dev/sdc","rb");
	if (disque1!= NULL) 
	{
		printf("disque sdc connecte\n");
		fclose(disque1);
	}
	disque1 = fopen("/dev/sdd","rb");
	if (disque1!= NULL)
	{ 
		printf("disque sdd connecte\n");
		fclose(disque1);
	}
}
void Liste_Disques(){
struct dirent *rep;
DIR *d=opendir("/dev"); /* Ouvrir le fichier /dev*/
char *nomDisk=NULL,*nom=NULL;


if (d){
   while((rep=readdir(d))!= NULL) /*recuperer chaque fichier de disque physique existe */
  {				   /* dans repertoire /dev*/
     printf("Nom Fichier %s \n",rep->d_name);/* afficher le nom de 							fichier*/
   }
  closedir(d);
}
}

/*char* Lire_Secteur(drive* disque,int Num_Sect){
int s=0;

long int a=512*Num_Sect;

if (fseek(disque, Num_Sect*TAILLE_SECTEUR, SEEK_SET) == -1)
		ERR;
if (fread(buffer, TAILLE_SECTEUR, 1, disque) == -1)
		ERR;

s=fseek(disque,a,SEEK_SET);

if(s!= 0){printf("Erreur de fseek \n");}
else{
s=fread(buffer,512,Num_Sect+1,disque);

}
return buffer;

}*/


void Afficher_Secteur(drive* disque,int Num_Sect){

if(disque ==NULL) {printf("error");}
else{
   
   Lire_secteur(disque,Num_Sect,buffer);
    if(buffer!=NULL){
          printf("\nAdresse\t Contenu(octet de 1 à 16)");
    for (i = 0; i < 512; i++)
    {
        if (i > 0) printf(" ");
        if (j % 16 == 0) {
            printf("\n");
            printf("%04ld\t",j);
        };
        printf("%02X", buffer[i]);
        j++;
    }
    } 
    printf("\n");
}}
void Recuperer_Parametres(){ /*recuperer les pramettres de FAT32*/
    unsigned char buffer[512];
    fread(buffer, 1, 512, disque1); /*lecture de premier secteur*/
    TailleSecteur= buffer[11] | buffer[12]<<8; /*recuperer la taille de secteur*/
    SectParCluster = buffer[13];/*recuperer nombre de secteur par cluster*/
    SecteursReserve = buffer[14] | buffer[15]<<8;/** nombre de secteurs reserve*/
    NombreTablesFat = buffer[16];/* nombre de tables FAT*/
    TailleFat = buffer[36] | buffer[37]<<8 | buffer[38]<<16 | buffer[39]<<24;/* TAILLE de FAT*/
    rootCluster = buffer[44] | buffer[45]<<8 | buffer[46]<<16 | buffer[47]<<24;/* recuperer la racine du cluster*/
    rootAdr = (SecteursReserve + NombreTablesFat * TailleFat) *TailleSecteur;/* recuperer l'adresse de la racine du cluster*/
    printf("--------------------------------------------------\n");
    printf("TAILLE DE SECTEUR:%d\nSECTEUR PAR CLUSTER:%d\nSECTEURS RESERVE:%d\n",TailleSecteur,SectParCluster,SecteursReserve);
    printf("NOMBRE DE FAT:%d\nTAILLE DE FAT:%d\nROOTCLUSTER:%u\nROOTADR:%u\n",NombreTablesFat,TailleFat,rootCluster,rootAdr);
    printf("--------------------------------------------------\n");
}

int Lire_Depuis_Fichier(unsigned char *buffer, unsigned int *pcluster, int *poffset){
    unsigned int fatAdr  = SecteursReserve * TailleSecteur;/*recuperer l'adress de FAT*/
    if (*poffset == TailleSecteur* SectParCluster){
        fseek(disque1, fatAdr + (*pcluster) * 4, SEEK_SET);
        fread(pcluster,1, 4, disque1);
        if (*pcluster >= 0x0FFFFFF8 && *pcluster <= 0x0FFFFFFF) return 0;
        fseek(disque1, rootAdr + (*pcluster - rootCluster) * SectParCluster * TailleSecteur, SEEK_SET);
        (*poffset) = 0;
    }
    fread(buffer, 1, 32, disque1);
    *poffset = *poffset + 32;
    return 1;
}

void Recuperer_Info_Fichier(char* buffer,struct tfichier* fs, int *pcluster, int *poffset, int cpt, int LFN){
    if(LFN){
        while( cpt > 0){
            fs->long_nom_fich[( cpt- 1)*13 + 0] = buffer[1];
            fs->long_nom_fich[(cpt - 1)*13 + 1] = buffer[3];
            fs->long_nom_fich[(cpt - 1)*13 + 2] = buffer[5];
            fs->long_nom_fich[(cpt - 1)*13 + 3] = buffer[7];
            fs->long_nom_fich[(cpt - 1)*13 + 4] = buffer[9];
            fs->long_nom_fich[(cpt - 1)*13 + 5] = buffer[14];
            fs->long_nom_fich[(cpt - 1)*13 + 6] = buffer[16];
            fs->long_nom_fich[(cpt - 1)*13 + 7] = buffer[18];
            fs->long_nom_fich[(cpt - 1)*13 + 8] = buffer[20];
            fs->long_nom_fich[(cpt - 1)*13 + 9] = buffer[22];
            fs->long_nom_fich[(cpt - 1)*13 + 10] = buffer[24];
            fs->long_nom_fich[(cpt - 1)*13 + 11] = buffer[28];
            fs->long_nom_fich[(cpt - 1)*13 + 12] = buffer[30];
            cpt--;
        }

        if(!Lire_Depuis_Fichier(buffer, pcluster, poffset)){
            printf("Erreur dans la recuperation des informtions de fichier !.\n");
            exit(1);
        }
    }
    fs->taille = buffer[28] | buffer[29]<<8 | buffer[30]<<16 | buffer[31]<<24;
    fs->premier_cluster = buffer[26] | buffer[27] <<8 | buffer[20]<<16 | buffer[21]<<24;
    fs->nom_fich[11] = 0x00;
    int i = 10;
    while(buffer[i] == 0x20) fs->nom_fich[i--] = 0x00;
    while(i >= 0) {fs->nom_fich[i] = buffer[i]; i--;}
}


void Cherch_fichiers_Dans_partition(struct tfichier* fs, int cluster){
    unsigned int address = rootAdr + (cluster - rootCluster) * SectParCluster* TailleSecteur;
    unsigned char buffer[32];
    unsigned int offset = 0;
    fseek(disque1, address, SEEK_SET);
    while(Lire_Depuis_Fichier(buffer,&cluster,&offset)){
        if(buffer[0] == 0x00) break;
        if(buffer[0] == 0xE5){
            printf("Entree Supprimee\n");
        }
//            fseek(disque, -32, SEEK_CUR); offset -= 32;
        int  cpt = 0; int LFN = 0;
        if(buffer[11] == 0X0F) {LFN = 1; cpt = buffer[0] & 0X0F;}
        Recuperer_Info_Fichier(buffer,fs, &cluster, &offset,  cpt, LFN);
        printf("Nom Court: %s\n",fs->nom_fich);
        if(LFN) printf("Nom Long: %s\n",fs->long_nom_fich);
        printf("Taille in Bytes: %d\n",fs->taille);
        printf("Number of the first Cluster: %d\n\n",fs->premier_cluster);
    };
}
void Cherch_del_fichiers_Dans_partition(struct tfichier* fs, int cluster){
    unsigned int address = rootAdr + (cluster - rootCluster) * SectParCluster * TailleSecteur;
    unsigned char buffer[32];
    unsigned int offset = 0;
    fseek(disque1, address, SEEK_SET);
    while(Lire_Depuis_Fichier(buffer,&cluster,&offset)){
        if(buffer[0] == 0xE5){
            int cpt = 0; int LFN = 0;
            if(buffer[11] == 0X0F) {LFN = 1; cpt = buffer[0] & 0X0F;}
            Recuperer_Info_Fichier(buffer,fs, &cluster, &offset,  cpt, LFN);
            printf("Short Name: %s\n",fs->nom_fich);
            if(LFN) printf("Long Name: %s\n",fs->long_nom_fich);
            printf("Taille en octets : %d\n",fs->taille);
            printf("Number of the first Cluster: %d\n\n",fs->premier_cluster);
        }
    };
}

void Cherch_Fichiers_Dans_Rep(struct tfichier* fs,char* nom_repertoire){
    int cluster = 2;
    unsigned int address = rootAdr + (cluster - rootCluster) * SectParCluster * TailleSecteur;
    unsigned char buffer[32];
    unsigned int offset = 0;
    fseek(disque1, address, SEEK_SET);
    
    while(Lire_Depuis_Fichier(buffer,&cluster,&offset)){
        if(buffer[0] == 0x00) break;
        int  cpt = 0; int LFN = 0;
        if(buffer[11] == 0X0F) {LFN = 1;  cpt= buffer[0] & 0X0F;}
        Recuperer_Info_Fichier(buffer,fs, &cluster, &offset,  cpt, LFN);
        if(strcmp(fs->long_nom_fich, nom_repertoire) == 0) {
            Cherch_fichiers_Dans_partition(&fs,fs->premier_cluster);
            return;
        };
    };
}

void Cherch_del_fichiers_Dans_Rep(struct tfichier* fs,char* nom_repertoire){
    int cluster = 2;
    unsigned int address = rootAdr + (cluster - rootCluster) * SectParCluster * TailleSecteur;
    unsigned char buffer[32];
    unsigned int offset = 0;
    fseek(disque1, address, SEEK_SET);
    while(Lire_Depuis_Fichier(buffer,&cluster,&offset)){
        if(buffer[0] == 0x00) break;
        int  cpt= 0; int LFN = 0;
        if(buffer[11] == 0X0F) {LFN = 1;  cpt = buffer[0] & 0X0F;}
        Recuperer_Info_Fichier(buffer,fs, &cluster, &offset,  cpt, LFN);
        if(strcmp(fs->long_nom_fich, nom_repertoire) == 0) {
            Cherch_del_fichiers_Dans_partition(&fs,fs->premier_cluster);
            return;
        };
    };
}
void Lire_secteur(drive* disque, unsigned int num_secteur, char* buffer)
{
	if (fseek(disque, num_secteur*TAILLE_SECTEUR, SEEK_SET) == -1)
		ERR;
	if (fread(buffer, TAILLE_SECTEUR, 1, disque) == -1)
		ERR;
}
/*void Lire_secteur(drive* disque, unsigned int num_secteur, char* buffer)
{
	if (fseek(disque, num_secteur*TAILLE_SECTEUR, SEEK_SET) == -1)
		ERR;
	if (fread(buffer, TAILLE_SECTEUR, 1, disque) == -1)
		ERR;
}
*/
/*void Afficher_secteur(drive* disque, unsigned int num_secteur)
{
	char buffer[TAILLE_SECTEUR];
	Lire_secteur(disque, num_secteur, buffer);
	for (long int addr = 0; addr < TAILLE_SECTEUR/16; addr += 1)
	{
		printf("%04ld : ", addr);
		for (size_t i = 0; i < 16; i++)
			printf("%02X ", (unsigned char) buffer[addr*16+i]);
		printf("\n");
	}	
	
}*/

void Afficher_Fdel(drive* disque)
{
	afficherDisquesConnectes() ;
	/* disque doit etre la partition FAT32*/
	unsigned char secteur_boot[TAILLE_SECTEUR];
	/********* Lire le secteur'0'=secteur boot ****************/
	Lire_secteur(disque, 0, secteur_boot);
	/**** recuperer les informations sur la structure de FAT32 ****/
	int32_t cluster_du_racine = *(int32_t*) (secteur_boot+44);
	int8_t secteurs_par_cluster = *(int8_t*) (secteur_boot+13);
	int16_t bytes_par_secteur = *(int16_t*) (secteur_boot+11);
	int16_t secteurs_reserves = *(int16_t*) (secteur_boot+14);
	int8_t nombre_de_fats = *(int8_t*) (secteur_boot+16);
	int32_t secteurs_par_fat = *(int32_t*) (secteur_boot+36);
	int32_t fat_offset = secteurs_reserves * bytes_par_secteur;
	int32_t racine_repertoire_offset = fat_offset + nombre_de_fats * secteurs_par_fat * bytes_par_secteur;
	/***** Affichage des deplacements de fat et rep racine ****/
	printf("REPERTOIRE RACINE DEVRAIT ETRE A : %d\n", racine_repertoire_offset);
	printf("FAT A L'OFFSET : %d\n", fat_offset);
	Lire_secteur(disque, secteurs_reserves, temp); // lire la FAT
	int32_t cluster = cluster_du_racine ;
	
		
		
		Lire_secteur(disque, secteurs_reserves+cluster*4/TAILLE_SECTEUR, temp);
		cluster = *(int32_t*) (temp+((cluster*sizeof(int32_t))%TAILLE_SECTEUR));
		printf("LIRE LE PROCHAIN CLUSTER ON : %d\n", fat_offset+cluster*4);
	
	printf("CLUSTER = %d\n", cluster);
	disque1=fopen("/dev/sda1","rb");
        struct tfichier fs;
        if (disque1 == NULL){
	printf("Erreur quant on a ouvrir le fichier!\n");
        }
        Recuperer_Parametres();
        /*follow_linked_list(disque, secteurs_reserves, secteurs_par_cluster, racine_repertoire_offset, cluster_du_racine, cluster_du_racine);*/
        fclose(disque1);
}
/*void follow_linked_list(FILE* disque, int16_t secteurs_reserves, int8_t secteurs_par_cluster, int32_t rootcluster_offset, int32_t cluster_du_root, int32_t parent_cluster)
{
	printf("\n--------------------------------------------------------------------------------\n\n") ; 
	int32_t cluster = cluster_du_root;
	while (cluster < 0x0FFFFFF8)
	{
		printf("\t==>Cluster = %d; offset = %d\n\n -------------------------------------------------------------------------------\n", cluster,  rootcluster_offset+ (cluster - cluster_du_root) * secteurs_par_cluster*TAILLE_SECTEUR);
		for (int i = 0; i < secteurs_par_cluster; i++)
		{
			Lire_secteur(disque,  rootcluster_offset/TAILLE_SECTEUR+ (cluster - cluster_du_root) * secteurs_par_cluster+i, temp);
			Repertoire_Entree *entree= (Repertoire_Entree*) temp;
			for (int j = 0; j < TAILLE_SECTEUR / sizeof(Repertoire_Entree) && entree[j].nom_fich[0] != '\0'; j++)
			{
				entree[j].nom_fich[strcspn(entree[j].nom_fich, "\x20")] = 0 ;
				entree[j].extension[strcspn(entree[j].extension, "\x20")] = 0 ;
				if (entree[j].nom_fich[0] != 0xE5 && entree[j].attrib != 0xf){
					if ( entree[j].Taille_fich == 0 && entree[j].nom_fich[0] != "."  ) { printf(yellow("\n   repertoire trouvé \n\tLe nom du repertoire : %s\n\tNom de repertoire pere : %s\n\tNumero de premier cluster dans la FAT: %d \n\n " ),entree[j].nom_fich , nom_pare , entree[j].numero_premier_cluster_faible);}
					else {
					printf(GREEN("\n   Fichier trouvé \n\tLe nom du fichier+extension: %s\n\tExtension: %s\n\tTaille du fichier : %d octets \n\tNom du repertoire pere :  %s \n\tNumero de premier cluster dans la FAT : %d\n\n"), entree[j].nom_fich, entree[j].extension, entree[j].Taille_fich ,  nom_pare , entree[j].numero_premier_cluster_faible);}}


				else if (entree[j].nom_fich[0] == 0xE5 && entree[j].attrib != 0xf)
					printf(RED("\n   Fichier supprimé trouvé \n\tLe nom du fichier+extension:%s\n\textension: %s\n\tTaille de fichier: %d octets \n\tNom du repertoire pere :  %s \n\tNumero de premier cluster dans la FAT : %d\n\n"), entree[j].nom_fich, entree[j].extension, entree[j].Taille_fich , nom_pare , entree[j].numero_premier_cluster_faible);
				if (entree[j].nom_fich[0] == 0xff)
				{
					printf("!! %d\n", rootcluster_offset + (cluster - cluster_du_root) * secteurs_par_cluster * TAILLE_SECTEUR + i * TAILLE_SECTEUR + j * 32);
					Afficher_Secteur(disque, rootcluster_offset/TAILLE_SECTEUR + (cluster - cluster_du_root) * secteurs_par_cluster+i);
				}
				if (entree[j].attrib & 16)
				{
					int32_t cluster_of_directory =
 entree[j].numero_premier_cluster_faible | (entree[j].numero_premier_cluster_fort << 16); 
					if (cluster_of_directory != cluster && cluster_of_directory != parent_cluster && cluster_of_directory != 0) // veut c'est pas . ou ..
					{
			
			
				strcpy( nom_pare, entree[j].nom_fich ) ; 
				follow_linked_list(disque, secteurs_reserves, secteurs_par_cluster, rootcluster_offset + (cluster_of_directory - cluster_du_root) * secteurs_par_cluster * TAILLE_SECTEUR, cluster_of_directory, cluster);
						Lire_secteur(disque, rootcluster_offset/TAILLE_SECTEUR + (cluster - cluster_du_root) * secteurs_par_cluster+i, temp);
					}
				}
			}
		}
		Lire_secteur(disque, secteurs_reserves+cluster*sizeof(int32_t)/TAILLE_SECTEUR, temp);
		cluster = *(int32_t*) (temp+((cluster*sizeof(int32_t))%TAILLE_SECTEUR));
	}
}*/
/************************ Programme Principale ************/
int main(){

/************************ Menu de TP SYC2 ************************/
menu: printf("==============TP SYC2 ==============\n");
/** verifier si on est un utilisateur administrateur 'root' **/
	setreuid(0,0);
	if (geteuid() != 0)
	{
		printf(" CE PROGRAMME N'EST PAS EXECUTE EN TANT ROOT \n EXECUTE LA CMD : sudo su APERES LANCER gcc ");
		exit(1);
	}
printf("1. AFFICHER LA LISTE DES DISQUES CONNECTES.\n");
printf("2. AFFICHER LA LISTE DES DISQUES DE REPERTOIRE /dev.\n");
printf("3. AFFICHER LE CONTENU D'UN SECTEUR D'UN DISQUE DONNEE.\n");
printf("4. AFFICHER LE CONTENU DE 10 SECTEURS D'UN DISQUE DONNEE.\n");
printf("5. AFFICHER LA LISTE DES FICHIOER/REPERTOIRES AUI SE TROUVE DANS  REPERTOIRE D'UN FAT32.\n");
printf("6. QUITTER LE PROGRAMME.\n");
printf("ENTRER VOTRE CHOIX :\n");
scanf("%d",&choix);
switch(choix){
case 1:{
printf("1. AFFICHER LA LISTE DES DISQUES CONNECTES.\n");
afficherDisquesConnectes() ;
goto menu;
}break;
case 2:{
printf("2. AFFICHER LA LISTE DES DISQUES DE REPERTOIRE /dev.\n");
Liste_Disques();
printf("\n \n \n");
goto menu;
}break;
case 3:{
printf("3. AFFICHER LE CONTENU D'UN SECTEUR D'UN DISQUE DONNEE.\n");
printf("Entrer le Numero de secteur:");
scanf("%d",&num);
disque1=fopen("/dev/sda","rb");
printf("\n");

    Afficher_Secteur(disque1,num);

goto menu;}break;
case 4:{
printf("4. AFFICHER LE CONTENU DE 10 SECTEURS D'UN DISQUE DONNEE.\n");
disque1=fopen("/dev/sda","rb");
printf("Entrer le Numero de secteur de debut:");
scanf("%d",&num);
printf("\n");
k=0;
	while((k<10)&(!feof(disque1))){
	printf("Secteur Numero=%d",num);
	    Afficher_Secteur(disque1,num);
	    k++;num++;
	    
	    }
goto menu;}break;
case 5:{
printf("5. AFFICHER LA LISTE DES FICHIOER/REPERTOIRES AUI SE TROUVE DANS  REPERTOIRE D'UN FAT32.\n");
    afficherDisquesConnectes() ;
	drive* f = fopen("/dev/sda1", "rb");
	if (f == NULL){
		printf("sda N'EST PAS CONNECTE\n");}
	else{	
	printf("POUR LA PARTITION DE DISQUE sda1\n");
	Afficher_Secteur(f, 0);
	Afficher_Fdel(f);
	fclose(f);   
	}
	 f = fopen("/dev/sdb", "rb");
	if (f == NULL)
	{
	printf("sdb N'EST PAS CONNECTE\n");	
	}
	else{
	printf("POUR LA PARTITION DE DISQUE sdb1\n");
	Afficher_Secteur(f, 0);
	Afficher_Fdel(f);
	fclose(f); 
	}
		     
    goto menu;}break;
case 6:{
printf(" QUITTER LE PROGRAMME AVEC SUCCES.\n");
}break;
default:{
printf(" CETTE OPTION N'EXISTE PAS .\n");
goto menu;}
}





return(0);

}

