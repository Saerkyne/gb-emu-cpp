#include "debug_log.h"
#include "dma.h"
#include "emulator_core.h"
#include "memory_bus.h"
#include "ppu.h"

bool dma_transfer_request = false; // Flag to indicate if a DMA transfer is requested
bool dma_transfer_restart = false; // Flag to indicate a DMA transfer restart request mid-transfer
uint8_t dma_counter = 0; // Counter for the number of bytes transferred during a DMA transfer
uint16_t dma_source = 0; // Source information for DMA writes. 0xXX here becomes 0xXX00-0xXX9F for the source of the DMA transfer. The value written to 0xFF46 is multiplied by 0x100 to get this value.

void dma_transfer_start(const uint8_t value)
{
	dma_transfer_request = true;
	dma_source = value; // Store the source address for the DMA transfer
	dma_counter = 0;
}

void dma_advance_clocks()
{
	const uint16_t addr = 0xFF46; // OAM DMA register address, doesn't change
	
	while (dma_transfer_request)
	{
		if (dma_counter == 0)
		{
			dma_counter++;
			return; // The first cycle is the write to the DMA register, which is handled in memory_bus_write
		}
		if (dma_counter == 1)
		{
			dma_counter++;
			return;
		}
		if (dma_counter >= 2 && dma_counter <= 161)
		{
			if (dma_transfer_restart)
			{
				dma_reset();
				dma_transfer_request = true;
			}
			else
			{
				uint16_t source_addr = (dma_source * 0x100); // OAM source is the high byte of the value written to 0xFF46, multiplied by 0x100 to equal 0xXX00-0xXX9F
				if (source_addr >= 0xE000 && source_addr <= 0xFFFF)
				{
					debug_log("Warning: DMA transfer source address is in the restricted range 0xE000-0xFFFF. Preventing copy until specific case logic written.");
					dma_transfer_request = false; // Prevent the copy from happening until specific case logic is written to handle this
					return;
				}
				const uint16_t dest = 0xFE00;
				memory[dest + dma_counter - 2] = memory_bus_read_inner(source_addr + dma_counter - 2); // Subtracting 2 to allow for the initial writes to the DMA register and the first cycles of the transfer window
				dma_counter++;
				return;
			}
		}
		if (dma_counter == 162)
		{
			dma_transfer_request = false; // DMA transfer is complete
			return;
		}
	}
}

void dma_reset()
{
	dma_counter = 0;
	dma_transfer_request = false;
	dma_transfer_restart = false;
	dma_source = 0;
}