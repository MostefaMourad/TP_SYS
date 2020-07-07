 #include <stdio.h>
 #include <stdlib.h>
 #include <sys/types.h>
 #include <string.h>
 #include <dirent.h> //l'untulisation des dossiers



 unsigned int nbr_SectReserve; //nombre de secteurs de la Reserved Region; 2 octets
 unsigned int nbr_FATs; //nombre de FATs; 1 octet
 unsigned int nbr_SectFAT;  //nombre de secteurs occup�s par une FAT; 4 octets
 unsigned int nbr_SectParCluster; //nombre de secteurs par cluster;
 unsigned int taille_Cluster; //taille d'un cluster en octets;
 unsigned int num_ClusterRacine; //numero du cluster racine = 2;
 unsigned int num_Cluster; //numero d'un cluster;
 unsigned int taille_Sect; //taille d'un secteur;
 int k,n,m,s, s1, s2, s3, s4, i, j, i_rep;
 int num=0; /*On r�cup�re le MBR*/
 int offset = 0;
 int count = 0;
  unsigned char buffer[512]; /*bloc = 512 octets*/
 unsigned char entree[32]; /*entr�e de 32 octets*/
 unsigned short type_Partition;
 char fichier_nom_long[256];
 trep *tab_rep = NULL;

 

typedef struct trep{
    char nom_court[256];
    char nom_long[256];
    char nom_repertoire_parent[256];
    int taille;
    int premier_cluster;
    //char type;
    trep *adr;
}trep;
/**********************************************/

 unsigned int adr_DebutPartition; //adresse de 4 octets;
 unsigned int adr_ClusterRacine; //adresse de d�but du premier cluster du r�pertoire racine;
 unsigned int adr_FAT; //adresse de d�but de la FAT;
 unsigned int LFN; //Attribut d'une entr�e longue;
 unsigned int numCluster;

 /**********************************************/

//tfile tab[];

trep* Suivant(trep* p)
{
    trep* point = (*p).adr;
    return(point);
}

void Aff_adr(trep* p, trep*q)
{
    (*p).adr = q;

}


/*int LireFichier(unsigned char *entree, unsigned int *premcluster, int *premoffset){
    unsigned int adr_FAT  = nbr_SectReserve * taille_Sect;
    if (*premoffset == taille_Sect * nbr_SectParCluster){ //On est arriv� � la fin du cluster
        fseek(disk, adr_FAT + (*premcluster) * 4, SEEK_SET); //On se positionne sur le cluster correspondant dans la FAT;
        fread(premcluster, 1, 4, disk); //On r�cup�re le cluster du FAT;
        if (*premcluster >= 0x0FFFFFF8 && *premcluster <= 0x0FFFFFFF) return 0; //
        if (*premcluster == 0x0FFFFFF7) return 2;
        fseek(disk, adr_ClusterRacine + (*premcluster - 2) * nbr_SectParCluster * taille_Sect, SEEK_SET); //On se re-positionne au le d�but de l'entr�e dans laquelle on �tait
        (*premoffset) = 0; //On reinitialise l'offset � zero, d�but de l'entr�e
    }
    fread(entree, 1, 32, disk); //On lit l'entr�e
    *premoffset = *premoffset + 32; //On se positionne au d�but de la prochaine entr�e
    return 1;
}*/

/*void SauvegardeInfoFichier(char* entree, struct tfile* fichier, int *pcluster, int *poffset, int compteur, int LFN, char *tabRep[]){
//Proc�dure qui permet de sauvegarder les informations d'un fichier (nom, taille, numero du premier cluster)
//d'une entr�e "entree" dans la structure "fichier" selon le num de son premier cluster, son premier offset
//Et LFN et compteur pour les entr�es longues.


    int i;
    int j = 0;
    int k = 10;
    int m = (compteur - 1)*13;
    if(LFN == 1){ //Dans le cas d'une entr�e longue
        while(compteur > 0){
            while(i<=30){
                if(i == 11){
                    i = 14;
                    j = 5;
                }
                fichier->nom_fichier_long[m + j] = entree[i];
                i = i + 2;
                j++;
            }

            compteur--;
        }

        if(LireFichier(entree, pcluster, poffset) == 0){
            printf("Erreur: Dernier cluster du fichier atteint\n");
            exit(1);
        }

        if(LireFichier(entree, pcluster, poffset) == 2){
            printf("Erreur: Bad cluster\n");
            exit(1);
        }
    }
    fichier->taille = entree[28] | entree[29]<<8 | entree[30]<<16 | entree[31]<<24;
    fichier->premier_cluster = entree[26] | entree[27] <<8 | entree[20]<<16 | entree[21]<<24;
    fichier->nom_court[11] = 0x00;

    while(entree[k] == 0x20) {
            fichier->nom_court[k--] = 0x00;
    }

    while(k >= 0) {
            fichier->nom_court[k] = entree[k];
            k--;
    }

    if(LFN == 1) printf("Nom long: %s\n", fichier->nom_long);
    printf("Nom court: %s\n", fichier->nom_court);
    printf("Nom du r�pertoire p�re: %s\n", fichier->nom_repertoire_parent);
    printf("Taille: %d\n", fichier->taille);
}*/


void afficherDisk(){

 struct dirent *lecture; //declaration du pointeur vers la structure dirent
 DIR *rep;
 rep = opendir("/dev" ); //ouvrire le repertoire dev
 while ((lecture = readdir(rep))) {
  printf("%s\n", lecture->d_name); //on utilise une fleche pour designer le name de la structure a partir du pointeur lecture
 } //lire et afficher les rep du dossier (les disque connecter)
 closedir(rep);//fermer le repertoire dev

}


char *lire_secteur(char disque_physique[25], int num_Sect){

 int n,s,j;
 char disque[25]="/dev/";
 strcat(disque,disque_physique); //le nom du repertoire a ouvrire /dev/disque_physique
 unsigned char buffer[512]; /* bloc = 512 octets*/
 long int a=512*num_Sect;
 FILE *disk=NULL;
 disk = fopen(disque, "rb"); // ouvrire le sique_physique

 if(disk == NULL) printf("\n le disque n'est pas ouvert\n");
  else{ s=fseek(disk, a, SEEK_SET);
   if(s!=0) printf("\n err de fseek: s=%d",s);
    else {n=fread(buffer, 512, 1, disk);
     if(n<=0) printf("\n Erreur de fread = %d ",n);
      else{
      printf("\n Secteur%d: Nombre d'elements lus = %d\n",num_Sect,n);
     }
    }
   }
  fclose(disk);

  return buffer;
 }


void afficher_secteur(char disque_physique[25], int num_Sect){
 int n,j;
 int i=0; //indice pour le parcourt du tableau
 int k=0; //indice pour le parcourt du tableau
 int tab[32][16]; //tableau pour l'affichage

 lire_secteur(disque_physique,num_Sect);

 unsigned char buffer[512]; /* bloc = 512 octets*/
 FILE *disk=NULL;
 char disque[25]="/dev/";
 strcat(disque,disque_physique); //le nom du repertoire a ouvrire /dev/disque_physique
 disk = fopen(disque, "rb");
 n=fread(buffer, 512, 1, disk);

 j=0;
  while(i<32){
  k=0;
  while(k<16){
   tab[i][k]=buffer[j];
   printf("%02x ",tab[i][k]);
   j++;
   k++;
  }
  printf("\n");
  i++;
 }
 fclose(disk);
}

void AccesRepertoire(unsigned int clusterRacine, trep **tete, FILE *disque_physique){

    trep *p, *q;
    int i1 = 10;
    int offset = 0;
    trep *tmp = *tete;
    FILE *disk = disque_physique;

    unsigned char entree[32]; /*entr�e de 32 octets*/

    char nom_fichier_court[256];
    char nom_fichier_long[256];
    char nom_repertoire_parent[256];

    int taille;
    int premier_cluster;

    printf("Nom court du r�pertoire: %s\n", tmp->nom_court);
    if(LFN && tmp->nom_long != "") printf("Nom long du r�pertoire: %s\n",tmp->nom_long);
    printf("Nom du r�pertoire parent: %s\n", tmp->nom_repertoire_parent);
    //printf("Taille en octets: %d\n",tmp->taille);
    printf("Numero du premier cluster: %d\n\n",tmp->premier_cluster);

    unsigned int adresse = adr_ClusterRacine + (tmp->premier_cluster - 2) * nbr_SectParCluster * taille_Sect;
    fseek(disk, adresse*512, SEEK_SET);
    fread(entree, 1, 32, disk);
    offset += 32;

    while(entree[0] != 0x00 && offset <= taille_Cluster){
            if(entree[0] == 0xE5){printf("Entree Supprimee\n");}
            else if(entree[11] == 0X0F) {
                LFN = 1;
                while(LFN){
                        count = entree[0] & 0X0F;
                        while(count > 0){
                                nom_fichier_long[(count - 1)*13 + 0] = entree[1];
                                nom_fichier_long[(count - 1)*13 + 1] = entree[3];
                                nom_fichier_long[(count - 1)*13 + 2] = entree[5];
                                nom_fichier_long[(count - 1)*13 + 3] = entree[7];
                                nom_fichier_long[(count - 1)*13 + 4] = entree[9];
                                nom_fichier_long[(count - 1)*13 + 5] = entree[14];
                                nom_fichier_long[(count - 1)*13 + 6] = entree[16];
                                nom_fichier_long[(count - 1)*13 + 7] = entree[18];
                                nom_fichier_long[(count - 1)*13 + 8] = entree[20];
                                nom_fichier_long[(count - 1)*13 + 9] = entree[22];
                                nom_fichier_long[(count - 1)*13 + 10] = entree[24];
                                nom_fichier_long[(count - 1)*13 + 11] = entree[28];
                                nom_fichier_long[(count - 1)*13 + 12] = entree[30];
                                count--;
                        }

                        fseek(disk, (adresse + offset)*512, SEEK_SET);
                        fread(entree, 1, 32, disk);
                        offset = offset + 32;
                        LFN = (entree[11] == 0x0F);
                    }

                }
                if(entree[11] == 0x10){ //cas d'un r�pertoire
                q= malloc(sizeof(trep)); //creation du maillon.
                p = tmp;

                while(Suivant(p) != NULL){
                        p = Suivant(p);
                }

                Aff_adr(p,q);
                Aff_adr(q,NULL);

                q->nom_long = fichier_nom_long;
                q->nom_repertoire_parent = tmp->nom_court;
                q->premier_cluster = entree[26] | entree[27] <<8 | entree[20]<<16 | entree[21]<<24;
                q->nom_court[11] = 0x00;
                i1 = 10;
                while(entree[i1] == 0x20) q->nom_court[i1--] = 0x00;
                while(i1 >= 0) {q->nom_court[i1] = entree[i1]; i1--;}
                }

                else{
                        i1 = 10;
                        taille = entree[28] | entree[29]<<8 | entree[30]<<16 | entree[31]<<24;
                        premier_cluster = entree[26] | entree[27] <<8 | entree[20]<<16 | entree[21]<<24;
                        nom_fichier_court[11] = 0x00;
                        while(entree[i1] == 0x20) nom_fichier_court[i1--] = 0x00;
                        while(i1 >= 0) {nom_fichier_court[i1] = entree[i1]; i1--;}


                        printf("------------------------------------------------------------------------------------");
                        printf("Nom court du fichier: %s\n", nom_fichier_court);
                        if(LFN && tmp->nom_long != "") printf("Nom long du r�pertoire: %s\n",nom_fichier_long);
                        printf("Nom du r�pertoire parent: %s\n", tmp->nom_court);
                        printf("Taille en octets: %d\n",taille);
                        printf("Numero du premier cluster: %d\n\n",premier_cluster);
                        printf("------------------------------------------------------------------------------------");

                }

                fseek(disk,(adresse + offset)*512,SEEK_SET);
                fread(entree, 1, 32, disk);
                offset += 32;
            }
            if(entree[0] == 0){ //fin du r�pertoire
                //p = tmp;
                *tete = Suivant(*tete);
                free(tmp);
            }
            else{
                printf("Erreur.");
            }

}

void Afficher_Fdel(char *disque_physique, int partition){



 //unsigned int adresse = adr_ClusterRacine + (num_Cluster - 2) * nbr_SectParCluster * taille_Sect;


 FILE *disk=NULL;

 char disque[25]="/dev/";

 strcat(disque,disque_physique); //le nom du repertoire a ouvrire /dev/disque_physique

 disk = fopen(disque, "rb");

 if(disk == NULL) printf("\n le disque n'est pas ouvert\n");
 else{

            int adresse_type_partition = partition*16 + 450;
            s=fseek(disk, adresse_type_partition*512, SEEK_SET);
            if(s!=0) printf("\n err de fseek: s=%d",s);
            else {
                n=fread(type_Partition, 2, 1, disk);
                if(n<=0) printf("\n Erreur de fread = %d ",n);
                else{
                    if (type_Partition == 0x0C || type_Partition == 0x0B){
                        s = fseek(disk, (adresse_type_partition + 4)*512, SEEK_SET);
                        if(s!=0) printf("\n err de fseek: s=%d",s);
                        else{
                            fread(adr_DebutPartition, 4, 1, disk);
                            printf("\n Adresse de d�but de la partition = %02x\n", adr_DebutPartition);

                            fseek(disk, adr_DebutPartition*512, SEEK_SET);
                            fread(buffer, 1, 512, disk);

                            taille_Sect = buffer[11] | buffer[12]<<8;
                            nbr_SectParCluster = buffer[13];
                            nbr_SectReserve = buffer[14] | buffer[15]<<8;
                            nbr_FATs = buffer[16];
                            nbr_SectFAT = buffer[36] | buffer[37]<<8 | buffer[38]<<16 | buffer[39]<<24;
                            num_ClusterRacine = buffer[44] | buffer[45]<<8 | buffer[46]<<16 | buffer[47]<<24;


                            unsigned char cluster[nbr_SectParCluster*512];

                            adr_FAT = adr_DebutPartition + nbr_SectReserve;
                            adr_ClusterRacine = adr_DebutPartition + nbr_SectReserve + nbr_FATs*nbr_SectFAT;

                                s = fseek(disk, adr_ClusterRacine*512, SEEK_SET);
                                taille_Cluster = nbr_SectParCluster*taille_Sect;
                                fread(entree, 1, 32, disk);
                                offset += 32;
                                //m = fread(cluster, taille_Cluster, 1, disk);

                                tab_rep->nom_court = "root";
                                tab_rep->nom_long = "";
                                tab_rep->nom_repertoire_parent = "";
                                tab_rep->premier_cluster = num_ClusterRacine;
                                tab_rep->taille = taille_Cluster;
                                tab_rep->adr = NULL;

                                printf("Nom court du r�pertoire: %s\n", tab_rep->nom_court);
                                //if(LFN && nom_long != "") printf("Nom long du r�pertoire: %s\n",tab_rep->nom_long);
                                printf("Nom du r�pertoire parent: %s\n", tab_rep->nom_repertoire_parent);
                                printf("Taille en octets: %d\n",tab_rep->taille);
                                printf("Numero du premier cluster: %d\n\n",tab_rep->premier_cluster);
                                if (entree[0]= 0x00) printf("R�pertoire racine vide.");
                                while(entree[0] != 0x00 && offset <= taille_Cluster){
                                    if(entree[0] == 0xE5){printf("Entree Supprimee\n");}
                                    else if(entree[11] == 0X0F) {
                                        LFN = 1;
                                            while(LFN){
                                                count = entree[0] & 0X0F;
                                                while(count > 0){
                                                        fichier_nom_long[(count - 1)*13 + 0] = entree[1];
                                                        fichier_nom_long[(count - 1)*13 + 1] = entree[3];
                                                        fichier_nom_long[(count - 1)*13 + 2] = entree[5];
                                                        fichier_nom_long[(count - 1)*13 + 3] = entree[7];
                                                        fichier_nom_long[(count - 1)*13 + 4] = entree[9];
                                                        fichier_nom_long[(count - 1)*13 + 5] = entree[14];
                                                        fichier_nom_long[(count - 1)*13 + 6] = entree[16];
                                                        fichier_nom_long[(count - 1)*13 + 7] = entree[18];
                                                        fichier_nom_long[(count - 1)*13 + 8] = entree[20];
                                                        fichier_nom_long[(count - 1)*13 + 9] = entree[22];
                                                        fichier_nom_long[(count - 1)*13 + 10] = entree[24];
                                                        fichier_nom_long[(count - 1)*13 + 11] = entree[28];
                                                        fichier_nom_long[(count - 1)*13 + 12] = entree[30];
                                                        count--;
                                                }

                                                fseek(disk, (adr_ClusterRacine + offset)*512, SEEK_SET);
                                                fread(entree, 1, 32, disk);
                                                offset = offset + 32;
                                                LFN = (entree[11] == 0x0F);
                                    }

                                }
                                if(entree[11] == 0x10){ //cas d'un r�pertoire
                                        trep *q = malloc(sizeof(trep)); //creation du maillon.
                                        p = tab_rep;

                                        while(Suivant(p) != NULL){
                                                p = Suivant(p);
                                        }

                                        Aff_adr(p,q);
                                        Aff_adr(q,NULL);

                                        q->nom_long = fichier_nom_long;
                                        q->nom_repertoire_parent = tab_rep->nom_court;
                                        q->premier_cluster = entree[26] | entree[27] <<8 | entree[20]<<16 | entree[21]<<24;
                                        q->nom_court[11] = 0x00;
                                        int i1 = 10;
                                        while(entree[i1] == 0x20) q->nom_court[i1--] = 0x00;
                                        while(i1 >= 0) {q->nom_court[i1] = entree[i1]; i1--;}
                                        //q->taille = entree[28] | entree[29]<<8 | entree[30]<<16 | entree[31]<<24;
                                }

                                fseek(disk,(adr_ClusterRacine + offset)*512,SEEK_SET);
                                fread(entree, 1, 32, disk);
                                offset += 32;
                            }

                            if(entree[0] == 0){ //fin du r�pertoire
                                    p = tab_rep;
                                    tab_rep = Suivant(*tab_rep);
                                    free(p);
                            }
                            else{
                                    printf("Erreur.");
                            }


                            //fseek(disk, adresse, SEEK_SET);
                            //fseek(disk, -32, SEEK_CUR); offset -= 32;

                        if(tab_rep != NULL){
                                trep *tete = tab_rep;
                                    while(tete != NULL){
                                            AccesRepertoire(adr_ClusterRacine, &tete, disk);
                                    }
                        }

                    }



                }
                else{
                    printf("Partition n'est pas FAT32\n");
                }
            }
        }
        //}

 }


fclose(disk);

}


 int main()
 {
  int numSect=0;
  char disquePhysique[25];
  printf("donner le nom du disque physique: ");
  scanf("%s",disquePhysique);
  printf("donner le num du secteur: ");
  scanf("%d",&numSect);
  afficherDisk();
  lire_secteur(disquePhysique,numSect);
  afficher_secteur(disquePhysique,numSect);

  Afficher_Fdel("/dev/sdc", 0);

  return 0;
}
