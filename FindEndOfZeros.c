#include <stdio.h>
#include <stdlib.h>

int main(){
    const int sectorsize = 512; //Alter for different sector size
    const int startsector = 34; //Alter for different start sector
    const char * diskpath = "/dev/XXX"; //Path to disk
    FILE * disk = fopen(diskpath,"rb");
    fseek(disk, startsector*sectorsize,SEEK_SET);
    int test;
    for(int i = startsector;;i++){
        fread(&test,4,1,disk);
        fseek(disk,sectorsize-4,SEEK_CUR);
        if(test != 0){
            printf("Found not zero int at sector %d\n", i);
            printf("%d MB of your drive is overwritten with zeros",(i*sectorsize)/(1024*1024));
            break;
        } 
    }
    return 0;
}