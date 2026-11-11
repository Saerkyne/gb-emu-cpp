#include "dma.h"
#include "emulator_core.h"
#include "memory_bus.h"

bool dma_transfer_request = false; // Flag to indicate if a DMA transfer is requested
bool dma_transfer_restart = false; // Flag to indicate a DMA transfer restart request mid-transfer
uint8_t dma_counter = 0; // Counter for the number of bytes transferred during a DMA transfer
uint16_t dma_source = 0; //Source address for the DMA transfer

void dma_transfer_start(const uint8_t value)
{
	dma_transfer_request = true;
	dma_counter = 0;
	dma_source = value; // Store the source address for the DMA transfer
	while (dma_counter < 162)
	{
		core_advance_cpu_clocks(4); // Each byte transfer takes 4 CPU clocks
		dma_counter++;
	}
	dma_transfer_request = false; // Clear the DMA transfer request flag after completion
}

void dma_advance_clocks()
{
	const uint16_t addr = 0xFF46; // OAM DMA register address, doesn't change
	
	if (dma_counter == 0)
	{
		memory_bus_write(addr, dma_source);
	}
	if (dma_counter == 1)
	{
		dma_transfer_restart = false; // Clear the restart flag at the beginning of the transfer
	}
	if (dma_counter >= 3 && dma_counter <= 162)
	{
		if (dma_transfer_restart)
		{
			dma_counter = 0; // Reset the counter to restart the transfer
			dma_transfer_restart = false; // Clear the restart flag
			return; // Skip the rest of this iteration and start over
		}
		else
		{
			uint16_t source_addr = (dma_source * 0x100);
			const uint16_t dest = 0xFE00;
			memory[dest + dma_counter - 3] = memory_bus_read_inner(source_addr + dma_counter - 3); // Subtracting 3 to allow for the initial writes to the DMA register and the first cycles of the transfer window
		}
	}
}

void dma_reset()
{
	// Implementation of DMA reset
}