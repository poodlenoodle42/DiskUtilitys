#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

union GPT_Entry{
    struct{
        __uint128_t type;
        __uint128_t GUID;
        u_int64_t firstBlock;
        u_int64_t lastBlock; //Inclusive
        u_int64_t attributes;
        u_int16_t name[36]; //In UTF16LE
    }__attribute__ ((__packed__));
    u_int8_t buffer[128];
};
union GPT_Header{
    struct{
        u_int64_t signature;
        u_int32_t revision;
        u_int32_t headersize;
        u_int32_t CRC32_header;
        u_int32_t reserved;
        u_int64_t thisGPTlocation;
        u_int64_t otherGPTlocation;
        u_int64_t firstAvailableSector;
        u_int64_t lastAvailableSector;
        __uint128_t diskGUID; //mixedEndian
        u_int64_t startOfTable; //In Sectors not in Address
        u_int32_t numberofPartitionEntrys;
        u_int32_t sizeofEntry;
        u_int32_t CRC32_partitionarray;
    }__attribute__ ((__packed__));
    u_int8_t buffer[92];
    //Zeros for rest of sector
};

//From https://stackoverflow.com/questions/21001659/crc32-algorithm-implementation-in-c-without-a-look-up-table-and-with-a-public-li
u_int32_t crc32b(u_int8_t *message, uint size) {
   int i, j;
   u_int32_t byte, crc, mask;
   crc = 0xFFFFFFFF;
   for(i = 0; i<size;i++) {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
   }
   return ~crc;
}

bool isGPTHeaderVaild(const union GPT_Header * h){
    union GPT_Header headerCRCzero = *h;
    headerCRCzero.CRC32_header = 0;
    return h->signature == 0x5452415020494645ULL && h->reserved == 0 && crc32b(headerCRCzero.buffer,h->headersize) == h->CRC32_header;
}

void printGPTHeader(const union GPT_Header * h){
    printf("Signature: %08lX\n", h->signature);
    printf("Revision: %08X\n",h->revision);
    printf("HeaderSize: %d Bytes\n",h->headersize);
    printf("Number of Partitions: %d\n",h->numberofPartitionEntrys);
    if(isGPTHeaderVaild(h)){
        printf("Header is valid\n");
    } else {
        printf("Header is not valid");
    }
}

void printGPTEntry(const union GPT_Entry * e){
    if(e->firstBlock != 0){ //Otherwise Partition can not be valid
        printf("Partition\n");
        printf("\tStart of Partition: %ld\n", e->firstBlock);
        printf("\tLast Block: %ld\n", e->lastBlock);
        printf("\tSize: %d Sectors\n", (e->lastBlock - e->firstBlock));
        printf("\tName: ");
        for(int i = 0; i<36;i++){
            printf("%c",(char)e->name[i]);
        }
        printf("\n");
    }
}



void readTableEntrys(FILE * disk, const union GPT_Header * header, union GPT_Entry * tableBuffer, int sectorsize){
    fseek(disk,header->startOfTable*sectorsize,SEEK_SET);
    u_int8_t* buffer = malloc(header->sizeofEntry);
    for(u_int32_t i = 0; i<header->numberofPartitionEntrys;i++){
        fread(buffer,header->sizeofEntry,1,disk);
        for(int a = 0; a<header->sizeofEntry;a++){
            tableBuffer[i].buffer[a] = buffer[a];
        }
    }
    free(buffer);
}


union GPT_Header readTableHeader(FILE * disk, const int headerNumber, const int sectorSize){ //0 - Primary, 1 - Backup
    union GPT_Header header;
    if(headerNumber == 0){
        fseek(disk,sectorSize*1,SEEK_SET);
        fread(header.buffer,92,1,disk);
    } else if (headerNumber == 1){
        fseek(disk,-sectorSize*1,SEEK_END);
        fread(header.buffer,92,1,disk);
    }
    return header;
}

void writeGPT(FILE * disk,const union GPT_Header * header, const union GPT_Entry * entrys, int sectorsize,int headerNumber){ // 0 - Rewrites Primary Sector, 1 - Rewrites Backup Sector
    if(headerNumber == 0){
        fseek(disk,sectorsize*1,SEEK_SET);
        fwrite(header->buffer,92,1,disk); //Writes Header
        fseek(disk,header->startOfTable*sectorsize,SEEK_SET);
        fwrite(entrys->buffer,header->numberofPartitionEntrys*header->sizeofEntry,1,disk);
    } else if(headerNumber == 1){
        fseek(disk,-sectorsize*1,SEEK_END);
        fwrite(header->buffer,92,1,disk); //Writes Header
        fseek(disk,header->startOfTable*sectorsize,SEEK_SET);
        fwrite(entrys->buffer,header->numberofPartitionEntrys*header->sizeofEntry,1,disk);
    }
}

void convertBackupGPTtoPrimary(union GPT_Header* header){
    u_int64_t primaryPos = header->otherGPTlocation;
    header->otherGPTlocation = header->thisGPTlocation;
    header->thisGPTlocation = primaryPos;
    header->startOfTable = 2;
    header->CRC32_header = 0;
    header->CRC32_header = crc32b(header->buffer,header->headersize);
}

int main(){
    int sectorsize = 512;
    char diskPathBuffer[512] = {0};
    printf("Path to Disk: \n");
    scanf("%s", diskPathBuffer);
    char * diskPath = malloc(strlen(diskPathBuffer));
    strcpy(diskPath,diskPathBuffer);
    printf("Sector size?\n");
    scanf("%d",&sectorsize);
    FILE * disk;
    int err;
    disk = fopen(diskPath,"rb+");
    if(disk == NULL){
        printf("File not found\n");
        exit(EXIT_FAILURE);
    }
    printf("Read Backup table...\n");
    union GPT_Header header = readTableHeader(disk,1,sectorsize);
    union GPT_Entry* entrys = malloc(sizeof(union GPT_Entry) * header.numberofPartitionEntrys);
    readTableEntrys(disk,&header,entrys,sectorsize);
    printGPTHeader(&header);
    for(int i = 0;i<header.numberofPartitionEntrys;i++){
        printGPTEntry(&entrys[i]);
    }
    printf("Do you want to overwrite the primary GPT with this one? Continue on your own risk! [y|n]\n");
    char an;
    scanf(" %c",&an);
    if(an == 'y'){
        convertBackupGPTtoPrimary(&header);
        writeGPT(disk,&header,entrys,sectorsize,0);
        printf("Wrote PartitionTable successfully");
    } else if (an == 'n'){
        printf("Aborting\n");
    } else {
        printf("Invalid answer");
    }
    free(entrys);
    return 0;
}