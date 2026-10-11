#include "emulator_core.h"
#include "cart.h"
#include "cpu.h"
#include "interrupts.h"
#include "ppu.h"
#include "timer.h"
#include "memory_bus.h"

gfx_context* gfx_debug_ctx = nullptr;
gfx_context* gfx_render_ctx = nullptr;

uint32_t core_clock_counter = 0; // Global clock counter for the emulator core
bool core_quit_requested = false; // Flag to indicate if the emulator core should quit

void core_init_gfx_contexts();

int core_init()
{

	if (!cart_open_file()) 
	{
		return -1;
	}

	cart_print_info();

	core_init_gfx_contexts();

	ppu_init();
	timer_init();

	return 0;
}

void core_run() 
{
	memset(memory, 0, MEMORY_SIZE);
	cpu_reset();
	ppu_reset();

	while (!core_quit_requested) 
	{
		cpu_tick();
	}
}

void core_shutdown() 
{
	
}

void core_advance_cpu_clocks(uint8_t clocks) 
{
	timer_advance_clocks(clocks);
	ppu_advance_clocks(clocks);
	
	core_clock_counter += clocks;

	if (interrupt_enable_ime_delay > 0) 
	{
		for (uint8_t c = 0; c < clocks; c++) 
		{
			interrupt_enable_ime_delay--;
			if (interrupt_enable_ime_delay == 0) 
			{
				interrupt_master_enable = true;
				break;
			}
		}
	}
}

void core_init_gfx_contexts()
{
	const SDL_Color COLOR_BLACK = { 0, 0, 0, 0 };

	gfx_debug_ctx = gfx_init(256, 256, "Debug");
	gfx_clear(gfx_debug_ctx, COLOR_BLACK);
	gfx_update(gfx_debug_ctx);

	gfx_render_ctx = gfx_init(LCD_PIXELS_WIDTH * PPU_RENDER_SCALE, LCD_PIXELS_HEIGHT * PPU_RENDER_SCALE, "GoodBoy");
	gfx_clear(gfx_render_ctx, COLOR_BLACK);
	gfx_update(gfx_render_ctx);
}