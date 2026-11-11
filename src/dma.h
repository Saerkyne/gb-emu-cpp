#pragma once
#include <stdint.h>

extern bool dma_transfer_request; // Flag to indicate if a DMA transfer is requested
extern bool dma_transfer_restart; // Flag to indicate a DMA transfer restart request mid-transfer
extern uint8_t dma_counter; // Counter for the number of bytes transferred during a DMA transfer

void dma_transfer_start(const uint8_t value);
void dma_advance_clocks();
void dma_reset();