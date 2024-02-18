#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ac97.h"
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

// Sound card MMIO register offsets for BAR1 (replace with actual values)
#define NABM_PCM_IN_BASE          0x00
#define NABM_PCM_OUT_BASE         0x10
#define NABM_MICROPHONE_BASE      0x20
#define GLOBAL_CONTROL_REGISTER   0x2C
#define GLOBAL_STATUS_REGISTER    0x30

// NABM register box offsets (replace with actual values)
#define BDL_BASE_ADDRESS_OFFSET           0x00
#define CURRENT_PROCESSED_ENTRY_OFFSET    0x04
#define LAST_VALID_ENTRY_OFFSET           0x05
#define TRANSFER_STATUS_OFFSET            0x06
#define POSITION_IN_CURRENT_ENTRY_OFFSET  0x08
#define PREFETCHED_ENTRY_OFFSET           0x0A
#define TRANSFER_CONTROL_OFFSET           0x0B

// Function to perform an outl operation (platform-specific)
void outl(uint32_t value, uint16_t port) {
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Function to perform an inl operation (platform-specific)
uint32_t inl(uint16_t port) {
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




// Example function to set Global Control Register
void set_global_control_register(void* mmio_base, uint32_t value) {
    // Set Global Control Register using the corresponding MMIO register
    *(volatile uint32_t*)((uintptr_t)mmio_base + GLOBAL_CONTROL_REGISTER) = value;
}

// Example function to read Global Status Register
uint32_t read_global_status_register(void* mmio_base) {
    // Read Global Status Register using the corresponding MMIO register
    return *(volatile uint32_t*)((uintptr_t)mmio_base + GLOBAL_STATUS_REGISTER);
}

// Example function to get the Physical Address of Buffer Descriptor List for a specific NABM register box
uint32_t get_bdl_base_address(void* mmio_base, uint32_t nabm_register_box_offset) {
    // Read the Physical Address of Buffer Descriptor List using the corresponding MMIO register
    return *(volatile uint32_t*)((uintptr_t)mmio_base + nabm_register_box_offset + BDL_BASE_ADDRESS_OFFSET);
}

// Example function to configure NABM registers for data transfer
void configure_nabm_registers(void* mmio_base, uint32_t nabm_register_box_offset) {
    // Set reset bit of the output channel (Transfer Control register, offset 0x0B, bit 1)
    *(volatile uint8_t*)((uintptr_t)mmio_base + nabm_register_box_offset + TRANSFER_CONTROL_OFFSET) |= (1 << 1);

    // Wait for the card to clear the reset bit
    // You may need to implement a delay or a mechanism to check the status

    // Configure other NABM registers as needed
    // ...

    // Clear the reset bit of the output channel
    *(volatile uint8_t*)((uintptr_t)mmio_base + nabm_register_box_offset + TRANSFER_CONTROL_OFFSET) &= ~(1 << 1);
}

int main() {
    // Replace with the actual bus, device, function numbers of your sound card
    uint8_t bus = 0;
    uint8_t device = 1;
    uint8_t function = 0;

    // Set up the PCI configuration address for BAR1
    uint32_t pci_address = 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (NABM_PCM_IN_BASE & ~0x3);
    outl(pci_address, PCI_CONFIG_ADDRESS);

    // Read the base address from BAR1
    uint32_t base_address = inl(PCI_CONFIG_DATA);

    // Assuming the size of the MMIO region is encoded in the lower bits of BAR1
    size_t size = base_address & 0xFFFFFFF0;

    // Allocate memory for the MMIO region
    void* mmio_base = malloc(size);
    if (mmio_base == NULL) {
        perror("Error allocating MMIO region");
        exit(EXIT_FAILURE);
    }
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
    // Example: Set Global Control Register
    set_global_control_register(mmio_base, 0x5); // Replace with the desired value

    // Example: Read Global Status Register
    uint32_t global_status = read_global_status_register(mmio_base);
    printf("Global Status Register: 0x%x\n", global_status);

    // Example: Get the Physical Address of Buffer Descriptor List for NABM PCM IN
    uint32_t bdl_pcm_in = get_bdl_base_address(mmio_base, NABM_PCM_IN_BASE);
    printf("Buffer Descriptor List for PCM IN: 0x%x\n", bdl_pcm_in);

    // Example: Configure NABM registers for data transfer for NABM PCM IN
    configure_nabm_registers(mmio_base, NABM_PCM_IN_BASE);

    // Free the allocated memory
    free(mmio_base);

    return 0;
}