#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


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
u_int32_t crc32b(u_int8_t *message) {
   int i, j;
   u_int32_t byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (message[i] != 0) {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   return ~crc;
}


void printGPTHeader(const union GPT_Header * h){
    printf("Signature: %08lX\n", h->signature);
    printf("Revision: %08X\n",h->revision);
    printf("HeaderSize: %d Bytes\n",h->headersize);
    printf("Number of Partitions: %d\n",h->numberofPartitionEntrys);
}

void printGPTEntry(const union GPT_Entry * e){
    printf("Partition\n");
    printf("\tStart of Partition: %ld\n", e->firstBlock);
    printf("\tLast Block: %ld\n", e->lastBlock);
    printf("\tName: ");
    for(int i = 0; i<36;i++){
        printf("%c",(char)e->name[i]);
    }
    printf("\n");
}

bool isGPTHeaderVaild(const union GPT_Header * h){
    return h->signature == 0x5452415020494645ULL && h->reserved == 0;
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
        //ToDo
    }
    return header;
}

int main(){
    const int sectorsize = 512;
    FILE * disk;
    u_int8_t block[sectorsize];
    int err;
    disk = fopen("diskimage.img","rb");
    if(disk == NULL) exit(EXIT_FAILURE);
    union GPT_Header header = readTableHeader(disk,0,sectorsize);
    union GPT_Entry* entrys = malloc(sizeof(union GPT_Entry) * header.numberofPartitionEntrys);
    readTableEntrys(disk,&header,entrys,sectorsize);
    printGPTHeader(&header);
    for(int i = 0;i<header.numberofPartitionEntrys;i++){
        printGPTEntry(&entrys[i]);
    }
    union GPT_Header headerCRCzero = header;
    headerCRCzero.CRC32_header = 0;
    headerCRCzero.CRC32_partitionarray = 0;
    if(crc32b(headerCRCzero.buffer) == header.CRC32_header){
        printf("CRC32 Valid\n");
    }
    free(entrys);
    return 0;
}