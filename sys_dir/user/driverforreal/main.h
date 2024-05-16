//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define SOUND_CARD_DRIVER_AC97 0
#define SOUND_CARD_DRIVER_HDA 1

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned int dword_t;
typedef unsigned long long qword_t;
#define MAX_NUMBER_OF_SOUND_CARDS 10
struct sound_card_info {
 byte_t driver;
 word_t vendor_id;
 word_t device_id;
 word_t io_base;
 word_t io_base_2;
 dword_t mmio_base;
}__attribute__((packed));
struct sound_card_info sound_cards_info[MAX_NUMBER_OF_SOUND_CARDS];
byte_t number_of_sound_cards;
#define NO_SOUND_CARD 0xFF

#define PCI_BAR0 0x10
#define PCI_BAR1 0x14
#define PCI_BAR2 0x18
#define PCI_BAR3 0x1C
#define PCI_BAR4 0x20
#define PCI_BAR5 0x24

#define PCI_MMIO_BAR 0x0
#define PCI_IO_BAR 0x1

#define DEVICE_PRESENCE_IS_NOT_KNOWN 0xFF
#define DEVICE_NOT_PRESENT 0
#define DEVICE_PRESENT 1
#define DEVICE_PRESENT_BUT_ERROR_STATE 2

#define VENDOR_INTEL 0x8086
#define VENDOR_AMD_1 0x1022
#define VENDOR_AMD_2 0x1002
#define VENDOR_BROADCOM 0x14E4
#define VENDOR_REALTEK 0x10EC
#define VENDOR_QUALCOMM_ATHEROS_1 0x168C
#define VENDOR_QUALCOMM_ATHEROS_2 0x1969
#define VENDOR_NVIDIA 0x10DE
#define VENDOR_TEXAS_INSTUMENTS 0x104C
#define VENDOR_CONEXANT_SYSTEMS 0x14F1
#define VENDOR_SIGMATEL 0x8384
#define VENDOR_RED_HAT 0x1AF4

dword_t pci_devices_array_mem = 0, pci_num_of_devices = 0;

byte_t selected_sound_card;

byte_t sound_volume = 0;
dword_t sound_sample_rate = 0, sound_memory = 0, sound_length = 0, sound_position = 0;

void initalize_sound_card(void);
void sound_set_volume(byte_t volume);
byte_t is_supported_sound_format(byte_t channels, byte_t bits_per_channel, dword_t sample_rate);
void play_new_sound(dword_t sound_memory, dword_t channels, dword_t bits_per_sample, dword_t sample_rate, dword_t number_of_samples);
void pause_sound(void);
void play_sound(void);

