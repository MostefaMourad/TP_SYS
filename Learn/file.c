#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

void Liste_Disques();
char* Lire_Secteur(FILE *disque,int Num_Sect);
void Afficher_Secteur(FILE *disque,int Num_Sect);
unsigned char buffer[512];
FILE *disque=NULL;
int i=0,n=0,j=0;
int main(){


disque=fopen("/dev/sda","rb");



if(disque ==NULL) {printf("error");}
else{
   printf("here  \n");
   strcpy(buffer,Lire_Secteur(disque,0));
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
 /*  while(!feof(disque)){
   	printf("here  \n");
	n=fread(buffer,sizeof(char),512,disque);
	printf("here n %d \n",n);
	if(n<0){ printf("\n  error %d \n",n);}
	else{
	   printf("\n secteur Num %d et  Nombre d'elements lus= %d \n",i,n);  		   
	   for(j=0;j<512;j++){
	     printf(" %02x ",buffer[j]);
	   }		   
	}  
	i++;
   }*/
}

fclose(disque);
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

char* Lire_Secteur(FILE *disque,int Num_Sect){
int s=0;

long int a=512*Num_Sect;


s=fseek(disque,a,SEEK_SET);

if(s!= 0){printf("Erreur de fseek \n");}
else{
s=fread(buffer,512,1,disque);

}
return buffer;

}


void Afficher_Secteur(FILE *disque,int Num_Sect){
unsigned char buffer[512];
int j=0;
strcpy(buffer,Lire_Secteur(disque,0));
   for(j=0;j<512;j++){
	     printf(" %02x ",buffer[j]);}
}

