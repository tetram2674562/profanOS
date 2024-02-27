#include <stdlib.h>
#include <stdio.h>
#include "ac97.h"

#define NOT_BIT31 0x7FFFFFFF
#define MAX_PCI_DEVICES 0x80fff800

uint32_t port_long_in(uint32_t port) {
    uint32_t result;
    asm volatile("inl %%dx,%%eax":"=a" (result):"d"(port));
    return result;
}
uint32_t pciFindDevice(uint32_t vendorDeviceID) {
    uint32_t esi = vendorDeviceID;  // save off vend+device ID
    uint32_t edi = 0x80000000 - 0x100;  // start with bus 0, dev 0 func 0

    while (1) {
        edi += 0x100;

        if (edi >= MAX_PCI_DEVICES)  // scanned all devices?
            return NOT_BIT31;  // not found

        uint32_t pciConfigData = port_long_in(edi);

        if (pciConfigData == esi)  // found device?
            return edi & NOT_BIT31;  // return only bus/dev/fn #
    }
}

int main(void){

    printf("%x",pciFindDevice(0x2415));
    return 0;
}