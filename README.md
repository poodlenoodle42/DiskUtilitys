# DiskUtilitys
A repo for some experiments with C and disks.
## GPT-Recovery
This little C programm is meant to help people who accidentally deleted their primary GPT. It reads the backup GPT at the end of a drive and rewrites it to the position of the primary GPT.
USE THIS AT YOUR OWN RISK.
The path to disk is the path to the disk you want to recover the GPT on in the /dev directory, /dev/sda for example.
You can find out the sector size of your disk with 
```bash
fdisk -l
```
## FindEndOfZeros
Finds the end of a string of zeros on a disk. You have to alter the const variables at the beginning of the program according to your needs and then recompile the program with gcc:
```bash
gcc FindEndOfZeros.c -o FindEndOfZeros
```
and then run the result.
