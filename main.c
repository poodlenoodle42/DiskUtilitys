#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
struct GPT_Entry{
    __uint128_t type;
    __uint128_t GUID;
    u_int64_t firstBlock;
    u_int64_t lastBlock; //Inclusive
    u_int64_t attributes;
    u_int16_t name[36]; //In UTF16LE
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
        u_int64_t primaryPartitionTable;
        u_int64_t secondaryPartitionTable;
        __uint128_t diskGUID; //mixedEndian
        u_int64_t startOfTable; //In Sectors not in Address
        u_int32_t numberofPartitionEntrys;
        u_int32_t sizeofEntry;
        u_int32_t CRC32_partitionarray;
    };
    u_int8_t buffer[92];
    //Zeros for rest of sector
};

void printGPTHeader(const union GPT_Header * h){
    printf("Signature: %08lX\n", h->signature);
    printf("Revision: %08X\n",h->revision);
    printf("HeaderSize: %d Bytes\n",h->headersize);
}

bool isGPTHeaderVaild(const union GPT_Header * h){
    return h->signature == 0x5452415020494645ULL && h->reserved == 0;
}

int main(){
    const int sectorsize = 512;
    FILE * disk;
    u_int8_t block[sectorsize];
    int err;

    disk = fopen("diskimage.img","rb");
    if(disk == NULL) exit(EXIT_FAILURE);
    fseek(disk,sectorsize*1,SEEK_SET); //Go to first sector
    err = fread(block,512,1,disk);
    union GPT_Header header;
    for(int i = 0; i<92;i+=1){ //For all bytes in Header
       // printf("%02X",block[i]);
        header.buffer[i] = block[i];
    }
    printf("\n");
    printGPTHeader(&header);
    if(isGPTHeaderVaild(&header)){
        printf("Valid GPT-Header");
    }
    return 0;
}