#include <stdlib.h>
#include <stdio.h>
#include "Bib.h" // toutes les fonctions sont dans la bibliothèque (respect des règles de clean Code)

int main(int argc, char const *argv[])
{
    /**************************** TEST DU FONCTIONNALITE 1 ********************************/
    Liste_Disques();
    /**************************** TEST DU FONCTIONNALITE 2 ********************************/
    FILE *disque;
    for (int i=0; i < disquesNum; i++)
    {
        disque = NULL;
        disque = fopen(disques[i],"rb");
        if(disque!=NULL){
           printf("\n**********************************************************************\n");
           printf("********* Affichage de secteurs 0 de disque num=°%d ********************\n",i);
           printf("**********************************************************************\n");  
           Afficher_secteur(disque,0); // lectire et affichage du secteur 0 du disque
        }
    }
    /**************************** TEST DU FONCTIONNALITE 3 ********************************/
    Afficher_Fdel(1,1); // afficher les fichiers et répértoire , du Flash disque
    return 0;
}


