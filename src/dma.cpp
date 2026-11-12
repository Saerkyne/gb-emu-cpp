#include "debug_log.h"
#include "dma.h"
#include "emulator_core.h"
#include "memory_bus.h"
#include "ppu.h"

dma_transfer_info dma_current_info = { false, 0, 0 }; // Initialize the DMA transfer info struct with default values
dma_transfer_info dma_pending_info = { false, 0, 0 }; // Initialize the DMA pending info struct with default values

bool dma_transfer_request = false; // Flag to indicate if a DMA transfer is requested
bool dma_transfer_restart = false; // Flag to indicate a DMA transfer restart request mid-transfer
uint8_t dma_counter = 0; // Counter for the number of bytes transferred during a DMA transfer
uint16_t dma_source = 0; // Source information for DMA writes. 0xXX here becomes 0xXX00-0xXX9F for the source of the DMA transfer. The value written to 0xFF46 is multiplied by 0x100 to get this value.

void dma_transfer_start(const uint8_t value)
{
	if (dma_current_info.dma_transfer_request)
	{
		dma_pending_info.dma_transfer_request = true;
		dma_pending_info.dma_counter = 0;
		dma_pending_info.dma_source = value; // Store the source address for the pending DMA transfer
	}
	else
	{
		dma_current_info.dma_transfer_request = true;
		dma_current_info.dma_source = value; // Store the source address for the DMA transfer
		dma_current_info.dma_counter = 0;
	}
}

void dma_advance_clocks()
{
	if (dma_current_info.dma_counter == 0)
	{
		dma_current_info.dma_counter++;
		return; // The first cycle is the write to the DMA register, which is handled in memory_bus_write
	}
	if (dma_current_info.dma_counter == 1)
	{
		dma_current_info.dma_counter++;
		return;
	}
	if (dma_current_info.dma_counter >= 2 && dma_current_info.dma_counter <= 161)
	{
		if (dma_pending_info.dma_transfer_request)
		{
			dma_current_info.dma_transfer_request = true;
			dma_current_info.dma_counter = 2;
			dma_current_info.dma_source = dma_pending_info.dma_source; // Start the pending DMA transfer immediately
			dma_pending_info.dma_transfer_request = false; // Clear the pending request
			dma_pending_info.dma_counter = 0;
			dma_pending_info.dma_source = 0;
			return; // Restart the DMA transfer with the new source address
		}
		else
		{
			uint16_t source_addr = (dma_current_info.dma_source * 0x100); // OAM source is the high byte of the value written to 0xFF46, multiplied by 0x100 to equal 0xXX00-0xXX9F
			if (source_addr >= 0xE000 && source_addr <= 0xFFFF)
			{
				debug_log("Warning: DMA transfer source address is in the restricted range 0xE000-0xFFFF. Preventing copy until specific case logic written.");
				dma_current_info.dma_transfer_request = false; // Prevent the copy from happening until specific case logic is written to handle this
				return;
			}
			const uint16_t dest = 0xFE00;
			memory[dest + dma_current_info.dma_counter - 2] = memory_bus_read_inner(source_addr + dma_current_info.dma_counter - 2); // Subtracting 2 to allow for the initial writes to the DMA register and the first cycles of the transfer window
			dma_current_info.dma_counter++;
			return;
		}
	}
	if (dma_current_info.dma_counter == 162)
	{
		dma_current_info.dma_transfer_request = false; // DMA transfer is complete
		return;
	}
}

void dma_reset()
{
	dma_current_info.dma_counter = 0;
	dma_current_info.dma_transfer_request = false;
	dma_transfer_restart = false;
	dma_source = 0;
}