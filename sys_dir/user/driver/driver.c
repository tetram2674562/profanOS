#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <type.h>

void outportl(uint16_t port, uint32_t data) {
    asm volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}
uint32_t inportl(uint16_t port) {
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
// stack overflow my friend :D


int main(void){

    // FAIRE LES TRUCS POUR LA SOUND BLASTER 16 GENRE OUTB, INB tsais genre les machins pour initalisé sachant que tu as l'adresse de début de mémoire plus haut
    return 0;
}


