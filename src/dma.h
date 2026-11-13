#pragma once
#include <stdint.h>

struct dma_transfer_info {
	bool dma_transfer_request;
	uint8_t dma_counter;
	uint16_t dma_source; // Source information for DMA writes. 0xXX here becomes 0xXX00-0xXX9F for the source of the DMA transfer. The value written to 0xFF46 is multiplied by 0x100 to get this value.
};

extern dma_transfer_info dma_current_info; // Struct to hold the current state of the DMA transfer
extern dma_transfer_info dma_pending_info; // Struct to hold the pending state of the DMA transfer


void dma_transfer_start(const uint8_t value);
void dma_advance_clocks();
void dma_reset();