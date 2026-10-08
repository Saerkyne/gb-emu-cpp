#include "ppu.h"
#include "emulator_core.h"
#include "interrupts.h"
#include "memory_bus.h"

lcd_registers* ppu_lcd_registers = (lcd_registers*)(memory + 0xFF40);
uint8_t frame_pixels[LCD_PIXELS_WIDTH * LCD_PIXELS_HEIGHT];
uint8_t sprite_palette_0[4] = { 0, 1, 2, 3 };
uint8_t sprite_palette_1[4] = { 0, 1, 2, 3 };
uint8_t background_palette[4] = { 0, 1, 2, 3 };
uint16_t ppu_scanline_dot = 0;
uint8_t ppu_pixel_buffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
bool ppu_first_frame_after_lcd_enable = false;

void ppu_init()
{
	memset(frame_pixels, 0, LCD_PIXELS_WIDTH * LCD_PIXELS_HEIGHT);
}

void ppu_reset() 
{
	ppu_lcd_registers->control = 0x91;
	ppu_lcd_registers->status = 0x85;
	ppu_lcd_registers->ly = 0;
	ppu_lcd_registers->lyc = 0;
	ppu_lcd_registers->oam_dma = 0xFF;
	ppu_lcd_registers->bgp = 0xFC;
	ppu_lcd_registers->obp0 = 0xFF;
	ppu_lcd_registers->obp1 = 0xFF;
	ppu_lcd_registers->wx = 0;
	ppu_lcd_registers->wy = 0;
	ppu_scanline_dot = 0;
	ppu_first_frame_after_lcd_enable = false;
}

void ppu_advance_clocks(const uint8_t clocks)
{
	if (!LCDC_ENABLE_LCD)
	{
		return;
	}
	for (uint8_t c = 0; c < clocks; c++)
	{
		uint8_t ppu_mode = LCDS_PPU_MODE;
		if (ppu_mode == LCDS_MODE_2_OAM_SEARCH)
		{
			// Perform OAM Search
		}
		else if (ppu_mode == LCDS_MODE_3_DRAWING)
		{
			// Tick pixel fetcher and pixel shifter
		}
		else if (ppu_mode == LCDS_MODE_0_HBLANK)
		{
			
		}
		else if (ppu_mode == LCDS_MODE_1_VBLANK)
		{
			
		}
		ppu_scanline_dot++;
	}
}

void ppu_check_ly_compare()
{
	if (LCDC_ENABLE_LCD && ppu_lcd_registers->ly == ppu_lcd_registers->lyc)
	{
		CHANGE_BIT(ppu_lcd_registers->status, 1, 2);
	}
	else
	{
		CHANGE_BIT(ppu_lcd_registers->status, 0, 2)
	}
}

void ppu_check_stat_signal()
{
	if (!LCDC_ENABLE_LCD)
	{
		return;
	}
	const uint8_t ppu_mode = LCDS_PPU_MODE;
	if (((ppu_lcd_registers->ly == ppu_lcd_registers->lyc) && LCDS_LYC_EQUALS_LY_INTERRUPT_ENABLE) ||
		((ppu_mode == LCDS_MODE_2_OAM_SEARCH) && LCDS_MODE_2_STAT_INTERRUPT_ENABLE) ||
		((ppu_mode == LCDS_MODE_1_VBLANK) && LCDS_MODE_1_STAT_INTERRUPT_ENABLE) ||
		((ppu_mode == LCDS_MODE_0_HBLANK) && LCDS_MODE_0_STAT_INTERRUPT_ENABLE))
	{
		interrupt_raise_flag(INTERRUPT_FLAG_STAT);
	}
}

void ppu_on_lyc_write(const uint8_t value)
{
	ppu_lcd_registers->lyc = value;
	if (LCDC_ENABLE_LCD)
	{
		ppu_check_ly_compare();
		ppu_check_stat_signal();
	}
}

void ppu_on_lcd_control_write(const uint8_t value);
void ppu_on_lcd_status_write(const uint8_t value);
void ppu_on_palette_write(const uint16_t addr);

void render_frame();
void render_debug();