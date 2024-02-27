#include <stdio.h>
#include <stdlib.h>
#include "ac97.h"
#include <malloc.h>
#include <ports.h>
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC
#define FILESIZE 64*1024
#define currentdevicevendor = ICH_DID
#define FILENAME = "rickroll.wav"
int main(void){
    void* DCM_OUT = malloc(BDL_SIZE/16);
    void* WAV_BUFFER1 = malloc(FILESIZE/16);
    void* WAV_BUFFER2 = malloc(FILESIZE/16);
    // on part du principe qu il y a deja un periph pci ac97 (flemme de verif pour l instant ) de type 82801AA AC'97 Audio Controller (ICH)
    port_long_in(0x0);
return 0;

}