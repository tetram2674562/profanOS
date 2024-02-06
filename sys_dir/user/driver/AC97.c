#include <stdio.h>
#include <ktype.h>


int16_t BASESNDADRESS = 0;

uint8_t port_byte_in(uint16_t port) {
    uint8_t result;
    asm("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void port_byte_out(uint16_t port, uint8_t data) {
    asm volatile("out %%al, %%dx" : : "a" (data), "d" (port));
}

uint16_t port_word_in(uint16_t port) {
    uint16_t result;
    asm("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void port_word_out(uint16_t port, uint16_t data) {
    asm volatile("out %%ax, %%dx" : : "a" (data), "d" (port));
}

uint32_t port_long_in(uint32_t port) {
    uint32_t result;
    asm volatile("inl %%dx,%%eax":"=a" (result):"d"(port));
    return result;
}

void port_long_out(uint32_t port, uint32_t value) {
    asm volatile("outl %%eax,%%dx"::"d" (port), "a" (value));
}

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;
 
    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    // Write out the address
    port_long_out(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t)((port_long_in(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}
uint16_t getac97(){
    port_word_out( (0 << 16) | (5 << 11) | (0 << 8) | (0 & 0xfc) | ((uint32_t) 0x80000000)+0x2C,0x2);//on reset la carte?
    port_word_out( (0 << 16) | (5 << 11) | (0 << 8) | (0 & 0xfc) | ((uint32_t) 0x80000000)+0x00 ,0x00 ); 
    return (uint16_t)(port_long_in ((0 << 16) | (5 << 11) | (0 << 8) | (0 & 0xfc) | ((uint32_t) 0x80000000)+0x30 ));
} // A REFAIRE.
int main(void){
    printf("%x",getac97());
    //printf("%x", (0 << 16) | (5 << 11) | (0 << 8) | (0 & 0xfc) | ((uint32_t) 0x80000000)); du coup ça c'est l'adresse de la carte pci AC97? je crois bien que oui.
    //printf("%x",pciConfigReadWord(0,3,0,0));
    return 0;


}