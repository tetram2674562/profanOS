#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <type.h>
#include <stdint.h>
#include <malloc.h>
#include "ac97.h"
void outportl(uint16_t port, uint32_t data) {
    asm volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}
uint32_t inportl(uint16_t port) {
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Sound card MMIO register offsets (replace with actual values)
#define SOUND_CARD_MASTER_VOLUME      0x20
#define SOUND_CARD_PCM_VOLUME         0x24
#define SOUND_CARD_BDL_PHYSICAL_ADDR  0x30
#define SOUND_CARD_LAST_BUFFER_ENTRY  0x35
#define SOUND_CARD_NABM_CONTROL       0x1B
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

// Sound card MMIO register offsets (replace with actual values)
#define SOUND_CARD_GLOBAL_CONTROL     0x00
#define SOUND_CARD_NAM_RESET          0x04
#define SOUND_CARD_GLOBAL_STATUS      0x08
// Example function to set Master Volume
void set_master_volume(void* mmio_base, uint8_t volume) {
    // Set Master Volume using the corresponding MMIO register
    *(volatile uint8_t*)((uintptr_t)mmio_base + SOUND_CARD_MASTER_VOLUME) = volume;
}

// Example function to set PCM output volume
void set_pcm_volume(void* mmio_base, uint8_t volume) {
    // Set PCM output volume using the corresponding MMIO register
    *(volatile uint8_t*)((uintptr_t)mmio_base + SOUND_CARD_PCM_VOLUME) = volume;
}

// Example function to load sound data to memory and describe it in BDL
void load_sound_data(void* mmio_base, void* sound_data, size_t data_size) {
    // Replace this function with your actual implementation
    // This could involve copying sound data to a specific memory region and setting up the BDL.
    // For simplicity, this example assumes sound_data is a pre-allocated buffer.
}

// Example function to configure NABM registers for data transfer
void configure_nabm_registers(void* mmio_base, uint32_t bdl_physical_addr, uint8_t last_buffer_entry) {
    // Set reset bit of output channel (NABM transfer control register 0x1B, value 0x2)
    *(volatile uint8_t*)((uintptr_t)mmio_base + SOUND_CARD_NABM_CONTROL) = 0x2;

    // Wait for the card to clear the reset bit
    // You may need to implement a delay or a mechanism to check the status

    // Write physical position of BDL to NABM register 0x10 (output BDL address register)
    *(volatile uint32_t*)((uintptr_t)mmio_base + SOUND_CARD_BDL_PHYSICAL_ADDR) = bdl_physical_addr;

    // Write the number of the last valid buffer entry to NABM register 0x15
    *(volatile uint8_t*)((uintptr_t)mmio_base + SOUND_CARD_LAST_BUFFER_ENTRY) = last_buffer_entry;

    // Set bit for transferring data (NABM transfer control register 0x1B, value 0x1)
    *(volatile uint8_t*)((uintptr_t)mmio_base + SOUND_CARD_NABM_CONTROL) = 0x1;
}
// Function to initialize the sound card
void initialize_sound_card(void* mmio_base) {
    // Write value 0x2 to Global Control register to resume card from cold reset and set power
    *(volatile uint32_t*)((uintptr_t)mmio_base + SOUND_CARD_GLOBAL_CONTROL) = 0x2;

    // Write any value to NAM reset register to set all NAM registers to their defaults
    *(volatile uint32_t*)((uintptr_t)mmio_base + SOUND_CARD_NAM_RESET) = 0x1;

    // Read card capability info from Global Status register
    uint32_t global_status = *(volatile uint32_t*)((uintptr_t)mmio_base + SOUND_CARD_GLOBAL_STATUS);

    // Extract information from the global status register
    uint8_t channels_supported = (global_status >> 16) & 0xFF;
    uint8_t is_20bit_audio_supported = (global_status >> 24) & 0x01;

    // Print the obtained information
    printf("Channels Supported: %d\n", channels_supported);
    printf("20-bit Audio Supported: %s\n", is_20bit_audio_supported ? "Yes" : "No");
}
void load_sound_data(void* mmio_base, void* sound_data, size_t data_size) {
    // Replace this function with your actual implementation

    // Allocate memory for the Buffer Descriptor List (BDL)
    size_t bdl_size = sizeof(uint32_t) * 8;  // Assuming 8 entries in the BDL
    void* bdl_memory = malloc(bdl_size);
    if (bdl_memory == NULL) {
        perror("Error allocating BDL memory");
        exit(EXIT_FAILURE);
    }

    // Set up the BDL entries
    for (int i = 0; i < 8; ++i) {
        // Calculate the physical address of the buffer in sound_data
        uint32_t buffer_physical_address = /* Calculate the physical address */;

        // Set the buffer address in the BDL entry
        *(volatile uint32_t*)((uintptr_t)bdl_memory + i * sizeof(uint32_t)) = buffer_physical_address;

        // Set other BDL entry details as needed
        // ...
    }

    // Copy sound data to a specific memory region
    void* sound_data_memory = malloc(data_size);
    if (sound_data_memory == NULL) {
        perror("Error allocating sound data memory");
        free(bdl_memory);
        exit(EXIT_FAILURE);
    }

    memcpy(sound_data_memory, sound_data, data_size);

    // Configure NABM registers for data transfer (assuming NABM_PCM_OUT_BASE is used)
    configure_nabm_registers(mmio_base, NABM_PCM_OUT_BASE);

    // Free the allocated memory
    free(bdl_memory);
    free(sound_data_memory);
}
int main() {
    // Replace with the actual bus, device, function numbers of your sound card
    uint8_t bus = 0;
    uint8_t device = 5;
    uint8_t function = 0;

    // Set up the PCI configuration address for BAR0
    uint32_t pci_address = 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (SOUND_CARD_GLOBAL_CONTROL & ~0x3);
    outportl(pci_address, PCI_CONFIG_ADDRESS);

    // Read the base address from BAR0
    uint32_t base_address = inportl(PCI_CONFIG_DATA);

    // Assuming the size of the MMIO region is encoded in the lower bits of BAR0
    size_t size = base_address & 0xFFFFFFF0;

    // Allocate memory for the MMIO region
    void* mmio_base = malloc(size);
    if (mmio_base == NULL) {
        perror("Error allocating MMIO region");
        exit(EXIT_FAILURE);
    }

    // Map the MMIO region into the program's address space
    // Note: In a real scenario, you would typically use mmap here, but for simplicity, we are using malloc in this example.
    // mmap example: mmio_base = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, base_address);
    
    // Initialize the sound card
    initialize_sound_card(mmio_base);
    size_t data_size = 1024; // Replace with the actual size of your sound data
    void* sound_data = malloc(data_size);
    if (sound_data == NULL) {
        perror("Error allocating sound data");
        free(mmio_base);
        exit(EXIT_FAILURE);
    }

    // Call the function to load sound data and set up BDL
    load_sound_data(mmio_base, sound_data, data_size);

    // Example: Configure NABM registers for data transfer
    uint32_t bdl_physical_addr = /* Set the physical address of your Buffer Descriptor List */;
    uint8_t last_buffer_entry = /* Set the number of the last valid buffer entry */;
    configure_nabm_registers(mmio_base, bdl_physical_addr, last_buffer_entry);

    // Free the allocated memory
    free(mmio_base);
    free(sound_data);
    return 0;
}
