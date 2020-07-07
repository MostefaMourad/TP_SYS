#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define NUMBER_OF_DISKS 10
#define MAX_STRING_SIZE 256

char* disks[NUMBER_OF_DISKS];
FILE *disk;
int disksNum = 0;

FILE *file;
//------------------------------------------------------
unsigned int sectorSize;
unsigned int sectorPerCluster;
unsigned int reservedSectors;
unsigned int fatTablesNumber;
unsigned int fatSize;
unsigned int rootCluster;
unsigned int rootAdr;
//------------------------------------------------------

struct tfile{
    char file_name[256];
    char long_file_name[256];
    int size;
    int first_cluster;
    char type;
};


void getParameters(){
    unsigned char buffer[512];
    fread(buffer, 1, 512, disk);
    sectorSize = buffer[11] | buffer[12]<<8;
    sectorPerCluster = buffer[13];
    reservedSectors = buffer[14] | buffer[15]<<8;
    fatTablesNumber = buffer[16];
    fatSize = buffer[36] | buffer[37]<<8 | buffer[38]<<16 | buffer[39]<<24;
    rootCluster = buffer[44] | buffer[45]<<8 | buffer[46]<<16 | buffer[47]<<24;
    rootAdr = (reservedSectors + fatTablesNumber * fatSize) * sectorSize;
    printf("--------------------------------------------------\n");
    printf("SECTOR SIZE:%d\nSECTORPERCLUSTER:%d\nRESERVEDSECTORS:%d\n",sectorSize,sectorPerCluster,reservedSectors);
    printf("FATNUMBER:%d\nFATSIZE:%d\nROOTCLUSTER:%u\nROOTADR:%u\n",fatTablesNumber,fatSize,rootCluster,rootAdr);
    printf("--------------------------------------------------\n");
}

int readFromFile(unsigned char *buffer, unsigned int *pcluster, int *poffset){
    unsigned int fatAdr  = reservedSectors * sectorSize;
    if (*poffset == sectorSize * sectorPerCluster){
        fseek(disk, fatAdr + (*pcluster) * 4, SEEK_SET);
        fread(pcluster,1, 4, disk);
        if (*pcluster >= 0x0FFFFFF8 && *pcluster <= 0x0FFFFFFF) return 0;
        fseek(disk, rootAdr + (*pcluster - rootCluster) * sectorPerCluster * sectorSize, SEEK_SET);
        (*poffset) = 0;
    }
    fread(buffer, 1, 32, disk);
    *poffset = *poffset + 32;
    return 1;
}

void myGetFileInfo(char* buffer,struct tfile* fs, int *pcluster, int *poffset, int count, int LFN){
    if(LFN){
        while(count > 0){
            fs->long_file_name[(count - 1)*13 + 0] = buffer[1];
            fs->long_file_name[(count - 1)*13 + 1] = buffer[3];
            fs->long_file_name[(count - 1)*13 + 2] = buffer[5];
            fs->long_file_name[(count - 1)*13 + 3] = buffer[7];
            fs->long_file_name[(count - 1)*13 + 4] = buffer[9];
            fs->long_file_name[(count - 1)*13 + 5] = buffer[14];
            fs->long_file_name[(count - 1)*13 + 6] = buffer[16];
            fs->long_file_name[(count - 1)*13 + 7] = buffer[18];
            fs->long_file_name[(count - 1)*13 + 8] = buffer[20];
            fs->long_file_name[(count - 1)*13 + 9] = buffer[22];
            fs->long_file_name[(count - 1)*13 + 10] = buffer[24];
            fs->long_file_name[(count - 1)*13 + 11] = buffer[28];
            fs->long_file_name[(count - 1)*13 + 12] = buffer[30];
            count--;
        }

        if(!readFromFile(buffer, pcluster, poffset)){
            printf("Error in getFileInfo!");
            exit(1);
        }
    }
    fs->size = buffer[28] | buffer[29]<<8 | buffer[30]<<16 | buffer[31]<<24;
    fs->first_cluster = buffer[26] | buffer[27] <<8 | buffer[20]<<16 | buffer[21]<<24;
    fs->file_name[11] = 0x00;
    int i = 10;
    while(buffer[i] == 0x20) fs->file_name[i--] = 0x00;
    while(i >= 0) {fs->file_name[i] = buffer[i]; i--;}
}


void search_files_in_partition(struct tfile* fs, int cluster){
    unsigned int address = rootAdr + (cluster - rootCluster) * sectorPerCluster * sectorSize;
    unsigned char buffer[32];
    unsigned int offset = 0;
    fseek(disk, address, SEEK_SET);
    while(readFromFile(buffer,&cluster,&offset)){
        if(buffer[0] == 0x00) break;
        if(buffer[0] == 0xE5){
            printf("Entree Supprimee\n");
        }
//            fseek(disk, -32, SEEK_CUR); offset -= 32;
        int count = 0; int LFN = 0;
        if(buffer[11] == 0X0F) {LFN = 1; count = buffer[0] & 0X0F;}
        myGetFileInfo(buffer,fs, &cluster, &offset, count, LFN);
        printf("Short Name: %s\n",fs->file_name);
        if(LFN) printf("Long Name: %s\n",fs->long_file_name);
        printf("Size in Bytes: %d\n",fs->size);
        printf("Number of the first Cluster: %d\n\n",fs->first_cluster);
    };
}
void search_del_files_in_partition(struct tfile* fs, int cluster){
    unsigned int address = rootAdr + (cluster - rootCluster) * sectorPerCluster * sectorSize;
    unsigned char buffer[32];
    unsigned int offset = 0;
    fseek(disk, address, SEEK_SET);
    while(readFromFile(buffer,&cluster,&offset)){
        if(buffer[0] == 0xE5){
            int count = 0; int LFN = 0;
            if(buffer[11] == 0X0F) {LFN = 1; count = buffer[0] & 0X0F;}
            myGetFileInfo(buffer,fs, &cluster, &offset, count, LFN);
            printf("Short Name: %s\n",fs->file_name);
            if(LFN) printf("Long Name: %s\n",fs->long_file_name);
            printf("Size in Bytes: %d\n",fs->size);
            printf("Number of the first Cluster: %d\n\n",fs->first_cluster);
        }
    };
}

void search_files_in_directory(struct tfile* fs,char* directoryName){
    int cluster = 2;
    unsigned int address = rootAdr + (cluster - rootCluster) * sectorPerCluster * sectorSize;
    unsigned char buffer[32];
    unsigned int offset = 0;
    fseek(disk, address, SEEK_SET);
    while(readFromFile(buffer,&cluster,&offset)){
        if(buffer[0] == 0x00) break;
        int count = 0; int LFN = 0;
        if(buffer[11] == 0X0F) {LFN = 1; count = buffer[0] & 0X0F;}
        myGetFileInfo(buffer,fs, &cluster, &offset, count, LFN);
        if(strcmp(fs->long_file_name, directoryName) == 0) {
            search_files_in_partition(&fs,fs->first_cluster);
            return;
        };
    };
}

void search_del_files_in_directory(struct tfile* fs,char* directoryName){
    int cluster = 2;
    unsigned int address = rootAdr + (cluster - rootCluster) * sectorPerCluster * sectorSize;
    unsigned char buffer[32];
    unsigned int offset = 0;
    fseek(disk, address, SEEK_SET);
    while(readFromFile(buffer,&cluster,&offset)){
        if(buffer[0] == 0x00) break;
        int count = 0; int LFN = 0;
        if(buffer[11] == 0X0F) {LFN = 1; count = buffer[0] & 0X0F;}
        myGetFileInfo(buffer,fs, &cluster, &offset, count, LFN);
        if(strcmp(fs->long_file_name, directoryName) == 0) {
            search_del_files_in_partition(&fs,fs->first_cluster);
            return;
        };
    };
}

int Liste_Disques(){
    int i = 0;
//    FILE *fp;
    char path[1035];
//    char un = 1;
    /* Read the output a line at a time - output it. */
    for (char letter = 97; letter<123;letter = letter + (char)1){
        memset(path,'\0',1035) ;
        strcpy(path,"/dev/sd");
        path[strlen(path)] = letter;
        disk = fopen(path,"rb");
        if (disk != NULL)
        {
            printf("disque %s connecte\n",path);
            disks[i] = malloc(sizeof(char)*MAX_STRING_SIZE);
            memset(disks[i],'\0',MAX_STRING_SIZE) ;
            strncpy(disks[i],path,strlen(path)) ;
            i++;
            fclose(disk);
        }

    }
    disksNum = i;
    printf("%d disque(s) est (sont) connecte(s)",i);
}

void Lire_secteur (int disque_physique,int Num_sect){
    int n,s;
    int i;
    int j = 0;
    long int sectLBA = 512*Num_sect;
    unsigned char buffer[512];
/* bloc = 512 octets*/
    FILE *disk=NULL;
    disk = fopen(disks[disque_physique], "rb");
    if(disk == NULL) printf("\n Erreur le disque physique %d n'est pas ouvert\n",disque_physique );
    else{ s=fseek(disk, sectLBA, SEEK_SET); /*seek par rapport au début du fichier : 0 */
        if(s!=0) printf("\n Erreur de fseek : s= %d",s);
        else{ n=fread(buffer,512, 1, disk);
            if(n<=0) printf("\n Erreur de fread = %d ",n);
            else printf("\n Lecture du secteur %d; Nombre d’éléments lus = %d\n",Num_sect,n);
        }
    }
    fclose(disk);

    printf("Adresse\t (octet de 1 à 16)");

    for (i = 0; i < 512; i++)
    {
        if (i > 0) printf(":");
        if (j % 16 == 0) {
            printf("\n");
            printf("%d\t",j);
        };
        printf("%02X", buffer[i]);
        j++;
    }
    printf("\n");
}

int main() {
    Liste_Disques();
    disk = fopen("/dev/sdb1", "rb");
    struct tfile fs;
    if (disk == NULL){
        printf("Error while opening file!\n");
    }
    getParameters();
//    Lire_secteur(2,1);
    search_files_in_directory(&fs,"newFolder");
    return 0;
}
//134217728