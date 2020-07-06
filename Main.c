#include <stdlib.h>
#include <stdio.h>
#include "Bib.h"

int main(int argc, char const *argv[])
{
    Liste_Disques();
    FILE *disk=NULL;
    disk = fopen("/dev/sdb1", "rb");
    Recuperer_Parametre_Partition(disk);
    return 0;
}


