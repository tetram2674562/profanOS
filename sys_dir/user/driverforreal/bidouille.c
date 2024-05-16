//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// MODIFIED BY TETRAM26
#include "main.h"
#include "malloc.h"
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


dword_t pci_read(dword_t bus, dword_t device, dword_t function, dword_t offset) {
 port_long_out(0xCF8, (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (offset)));
 return port_long_in(0xCFC);
}

void pci_write(dword_t bus, dword_t device, dword_t function, dword_t offset, dword_t value) {
 port_long_out(0xCF8, (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (offset)));
 port_long_out(0xCFC, value);
}

dword_t pci_read_bar_type(dword_t bus, dword_t device, dword_t function, dword_t bar) {
 port_long_out(0xCF8, (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (bar)));
 return (word_t) (port_long_in(0xCFC) & 0x1);
}

word_t pci_read_io_bar(dword_t bus, dword_t device, dword_t function, dword_t bar) {
 port_long_out(0xCF8, (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (bar)));
 return (word_t) (port_long_in(0xCFC) & 0xFFFC);
}

dword_t pci_read_mmio_bar(dword_t bus, dword_t device, dword_t function, dword_t bar) {
 port_long_out(0xCF8, (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (bar)));
 return (port_long_in(0xCFC) & 0xFFFFFFF0);
}

void pci_enable_io_busmastering(dword_t bus, dword_t device, dword_t function) {
 pci_write(bus, device, function, 0x04, ((pci_read(bus, device, function, 0x04) & ~(1<<10)) | (1<<2) | (1<<0))); //enable interrupts, enable bus mastering, enable IO space
}

void pci_enable_mmio_busmastering(dword_t bus, dword_t device, dword_t function) {
 pci_write(bus, device, function, 0x04, ((pci_read(bus, device, function, 0x04) & ~(1<<10)) | (1<<2) | (1<<1))); //enable interrupts, enable bus mastering, enable MMIO space
}

void pci_disable_interrupts(dword_t bus, dword_t device, dword_t function) {
 pci_write(bus, device, function, 0x04, (pci_read(bus, device, function, 0x04) | (1<<10))); //disable interrupts
}
void scan_pci_device(dword_t bus, dword_t device, dword_t function) {
 dword_t full_device_id, vendor_id, device_id, type_of_device, class, subclass, progif, mmio_port_base;
 word_t io_port_base;

 //read base informations about device
 vendor_id = (pci_read(bus, device, function, 0) & 0xFFFF);
 device_id = (pci_read(bus, device, function, 0) >> 16);
 full_device_id = pci_read(bus, device, function, 0);
 if(full_device_id==0xFFFFFFFF) {
  return; //no device
 }
 type_of_device = (pci_read(bus, device, function, 0x08) >> 8);
 class = (type_of_device >> 16);
 subclass = ((type_of_device >> 8) & 0xFF);
 progif = (type_of_device & 0xFF);



 //AC97 sound card
 if(type_of_device==0x040100 && number_of_sound_cards<MAX_NUMBER_OF_SOUND_CARDS) {
  
  pci_enable_io_busmastering(bus, device, function);

  sound_cards_info[number_of_sound_cards].driver = SOUND_CARD_DRIVER_AC97;
  sound_cards_info[number_of_sound_cards].vendor_id = vendor_id;
  sound_cards_info[number_of_sound_cards].device_id = device_id;
  sound_cards_info[number_of_sound_cards].io_base = pci_read_io_bar(bus, device, function, PCI_BAR0);
  sound_cards_info[number_of_sound_cards].io_base_2 = pci_read_io_bar(bus, device, function, PCI_BAR1);
  number_of_sound_cards++;

  return;
 }
 
 //HD Audio sound card
 if(type_of_device==0x040300 && number_of_sound_cards<MAX_NUMBER_OF_SOUND_CARDS) {
  
  pci_enable_mmio_busmastering(bus, device, function);

  sound_cards_info[number_of_sound_cards].driver = SOUND_CARD_DRIVER_HDA;
  sound_cards_info[number_of_sound_cards].vendor_id = vendor_id;
  sound_cards_info[number_of_sound_cards].device_id = device_id;
  sound_cards_info[number_of_sound_cards].mmio_base = pci_read_mmio_bar(bus, device, function, PCI_BAR0);
  number_of_sound_cards++;

  return;
 }
}


byte_t *get_pci_vendor_string(dword_t vendor_id) {
 extern dword_t pci_vendor_id_string_array[256];

 for(dword_t i=0; i<256; i+=2) {
  if(pci_vendor_id_string_array[i]==0) {
   break;
  }
  else if(pci_vendor_id_string_array[i]==vendor_id) {
   return (byte_t *)(pci_vendor_id_string_array[i+1]);
  }
 }

 return ""; //this vendor id is not in list
}


void scan_pci(void) {
 //initalize values that are used to determine presence of devices
 
 number_of_sound_cards = 0;

 //this array is used in System board
 pci_devices_array_mem = malloc(12*1000);
 pci_num_of_devices = 0;
 

 for(int bus=0; bus<256; bus++) {
  for(int device=0; device<32; device++) {
   scan_pci_device(bus, device, 0);
   
   //multifunctional device
   if( (pci_read(bus, device, 0, 0x0C) & 0x00800000)==0x00800000) {
    for(int function=1; function<8; function++) {
     scan_pci_device(bus, device, function);
    }
   }
  }
 }
}






