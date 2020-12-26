#include <stdio.h>
#include <stdlib.h>

struct GPT_Entry{
    
};
struct GPT_Header{
    u_int64_t signature;
    u_int32_t revision;
    u_int32_t headersize;
    u_int32_t CRC32_header;
    u_int32_t reserved;
    u_int64_t thisGPTlocation;
    u_int64_t otherGPTlocation;
    u_int64_t primaryPartitionTable;
    u_int64_t secondaryPartitionTable;
    __uint128_t diskGUID;
    u_int64_t startOfTable; //In Sectors not in Address
    u_int32_t numberofPartitionEntrys;
    u_int32_t sizeofEntry;
    u_int32_t CRC32_partitionarray;
    //Zeros for rest of sector

};


int main(){
    FILE * disk;
    u_int8_t block[512];
    int err;

    disk = fopen("diskimage.img","rb");
    if(disk == NULL) exit(EXIT_FAILURE);
    fseek(disk,0x1C2,SEEK_SET);
    err = fread(block,512,1,disk);
    for(int i = 0; i<512;i+=1){
        printf("%02X",block[i]);
    }
    return 0;
}