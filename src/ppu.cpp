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

uint8_t  ppu_oam_buffer_count = 0;
uint8_t ppu_oam_search_index = 0;
oam_buffer_data ppu_oam_buffer[10] = {
	{ 0, false }, { 0, false }, { 0, false }, { 0, false }, { 0, false },
	{ 0, false }, { 0, false }, { 0, false }, { 0, false }, { 0, false }
}

void ppu_oam_search_tick();
void ppu_fetcher_tick();
void ppu_shifter_tick();

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
			ppu_oam_search_tick();
			if (ppu_scanline_dot == 79)
			{
				ppu_set_lcd_mode(LCDS_MODE_3_DRAWING);
			}
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

void ppu_set_lcd_mode(uint8_t new_mode)
{
	const uint8_t ppu_mode = LCDS_PPU_MODE;
	const uint8_t prev_lcd_status = ppu_lcd_registers->status;
	const uint8_t new_lcd_status = (prev_lcd_status & 0b11111100) | new_mode;
	ppu_lcd_registers->status = new_lcd_status;

	if (new_mode != ppu_mode)
	{
		ppu_check_stat_signal();
		if (new_mode == LCDS_MODE_1_V_BLANK)
		{
			// ppu fetcher reset
		}
		else if (new_mode == LCDS_MODE_2_OAM_SEARCH)
		{
			// ppu_oam_search_index reset
		}
		else if (new_mode == LCDS_MODE_3_DRAWING)
		{
			// memset(ppu_pixel_fifo, 0, sizeof(ppu_pixel_fifo));
		}
	}
}

void ppu_on_lcd_control_write(const uint8_t value)
{
	const bool prev_lcd_enabled = LCDC_ENABLE_LCD;
	ppu_lcd_registers->control = value;
	const bool new_lcd_enabled = LCDC_ENABLE_LCD;
	if (new_lcd_enabled != prev_lcd_enabled)
	{
		ppu_scanline_dot = 0;
		ppu_lcd_registers->ly = 0;
		ppu_set_lcd_mode(LCDS_MODE_0_H_BLANK);

		if (new_lcd_enabled)
		{
			ppu_first_frame_after_lcd_enable = true;
		}
		ppu_check_stat_signal();
	}
}

void ppu_on_lcd_status_write(const uint8_t value)
{
	uint8_t prev_value = ppu_lcd_registers->status;
	uint8_t new_value = (prev_value & 0b10000111) | (value & 0b01111000);
	ppu_lcd_registers->status = new_value;
	ppu_check_stat_signal();
}

void ppu_on_palette_write(const uint16_t addr)
{
	if (addr == 0xFF47) extract_palette(addr, background_palette);
	if (addr == 0xFF48) extract_palette(addr, background_palette);
	if (addr == 0xFF49) extract_palette(addr, background_palette);
}

void render_frame()
{

}

void render_debug()
{

}

void extract_palette(uint16_t address, uint8_t* palette)
{
	const uint8_t pal = memory[address];
	palette[0] = (pal & 0b00000011);
	palette[1] = (pal & 0b00001100) >> 2;
	palette[2] = (pal & 0b00110000) >> 4;
	palette[3] = (pal & 0b11000000) >> 6;
}

void ppu_oam_search_tick()
{
	if (ppu_scanline_dot % 2 == 0 || ppu_oam_buffer_count == 10)
	{
		return;
	}
	const uint8_t* tile_data = memory + 0x8000;
	const uint8_t* oam_sprite_data = memory + OBJECT_ATTRIBUTE_ADDRESS;
	const uint8_t sprite_height = LCDC_SPRITE_SIZE ? 16 : 8;
	const uint8_t attribute_data_index = ppu_oam_search_index * 4;
	const object_attribute_data* sprite = (object_attribute_data*)(oam_sprite_data + attribute_data_index);
	const uint16_t pos_y = sprite->pos_y;
	const uint8_t ly = ppu_lcd_registers->ly + 16;
	if (sprite->pos_x > 0 && ly >= pos_y && ly < pos_y + sprite_height)
	{
		ppu_oam_buffer[ppu_oam_buffer_count].oam_id = attribute_data_index;
		ppu_oam_buffer[ppu_oam_buffer_count].drawn = false;
		ppu_oam_buffer_count++;
	}
	ppu_oam_search_index++;
}

uint8_t ppu_fetcher_internal_clock = 0;
bool ppu_fetcher_sprite_active = false;
uint8_t ppu_oam_sprite_fetcher_index = 0;
uint16_t ppu_fetcher_tile_no = 0;
bool ppu_fetcher_window_pixels = false;
uint8_t ppu_fetcher_internal_x_pos = 0;
uint8_t ppu_fetcher_window_line_counter = 0;
uint8_t ppu_fetcher_tile_data[2] = { 0, 0 };
uint8_t ppu_pixel_fifo_cursor = 0;
pixel_fifo_data ppu_pixel_fifo[16] = {
	{ 0, false }, { 0, false }, { 0, false }, { 0, false },
	{ 0, false }, { 0, false }, { 0, false }, { 0, false },
	{ 0, false }, { 0, false }, { 0, false }, { 0, false },
	{ 0, false }, { 0, false }, { 0, false }, { 0, false }
};
uint8_t ppu_pixel_fifo_occupancy = 0;
bool ppu_fetcher_first_scanline_steps = false;

void ppu_fetcher_tick()
{
	ppu_fetcher_internal_clock++;
	// Each fetcher stage takes 2 clock cycles to complete
	if (ppu_fetcher_internal_clock == 2) // Step 1: Fetch Tile Address
	{
		if (ppu_fetcher_sprite_active)
		{
			const uint8_t* oam_sprite_data = memory + OBJECT_ATTRIBUTE_ADDRESS;
			const object_attribute_data* sprite = (object_attribute_data*)(oam_sprite_data + ppu_oam_buffer[ppu_oam_sprite_fetcher_index].oam_id);
			ppu_fetcher_tile_no = sprite->tile_id;
		}
		else
		{
			if (!ppu_fetcher_window_pixels || !LCDC_WINDOW_ENABLE)
			{
				const uint8_t* tile_map_data = memory + (LCDC_BG_TILE_MAP_SELECT ? 0x9C00 : 0x9800);
				const uint16_t tile_x_offset = (ppu_fetcher_internal_x_pos + (ppu_lcd_registers->scx / 8)) & 0x1F;
				const uint16_t tile_y_offset = 32 * (((ppu_lcd_registers->ly + ppu_lcd_registers->scy) & 0xFF) / 8);
				const uint16_t tile_address = (tile_x_offset + tile_y_offset) & 0x3FF;
				ppu_fetcher_tile_no = tile_map_data[tile_address];
			}
			else
			{
				const uint8_t* tile_map_data = memory + (LCDC_WINDOW_TILE_MAP_SELECT ? 0x9C00 : 0x9800);
				const uint16_t tile_x_offset = ppu_fetcher_internal_x_pos;
				const uint16_t tile_y_offset = 32 * (ppu_fetcher_window_line_counter / 8);
				const uint16_t tile_address = (tile_x_offset + tile_y_offset) & 0x3FF;
				ppu_fetcher_tile_no = tile_map_data[tile_address];
			}
		}
	}
	else if (ppu_fetcher_internal_clock == 4 || ppu_fetcher_internal_clock == 6) // Steps 2 and 3: Fetch Tile Data
	{
		const bool high_byte = ppu_fetcher_internal_clock == 6;
		if (ppu_fetcher_sprite_active)
		{
			const uint8_t* oam_sprite_data = memory + OBJECT_ATTRIBUTE_ADDRESS;
			const object_attribute_data* sprite = (object_attribute_data*)(oam_sprite_data + ppu_oam_buffer[ppu_oam_sprite_fetcher_index].oam_id);
			const uint8_t* tile_data = memory + 0x8000;
			const int32_t tile_data_offset = ppu_fetcher_tile_no * 16;
			const bool flip_y = CHECK_BIT(sprite->flags, 6);
			const uint8_t flip_offset = LCDC_SPRITE_SIZE ? 30 : 14;
			const uint8_t vertical_offset = 2 * (ppu_lcd_registers->ly - (sprite->pos_y - 16));
			const uint16_t offset = flip_y ? flip_offset - vertical_offset : vertical_offset;
			const int32_t pixel_data_address = tile_data_offset + offset + (high_byte ? 1 : 0);
			ppu_fetcher_tile_data[high_byte] = tile_data[pixel_data_address];
		}
		else
		{
			const uint8_t* tile_data = memory + (LCDC_TILE_DATA_SELECT ? 0x8000 : 0x9000);
			const int32_t tile_index = LCDC_TILE_DATA_SELECT ? ppu_fetcher_tile_no : (int8_t)ppu_fetcher_tile_no;
			const int32_t tile_data_offset = tile_index * 16;
			if (!ppu_fetcher_window_pixels)
			{
				const uint16_t offset = 2 * ((ppu_lcd_registers->ly + ppu_lcd_registers->scy) % 8);
				const int32_t pixel_data_address = tile_data_offset + offset + (high_byte ? 1 : 0);
				ppu_fetcher_tile_data[high_byte] = tile_data[pixel_data_address];
			}
			else
			{
				const uint16_t offset = 2 * (ppu_fetcher_window_line_counter % 8);
				const int32_t pixel_data_address = tile_data_offset + offset + (high_byte ? 1 : 0);
				ppu_fetcher_tile_data[high_byte] = tile_data[pixel_data_address];
			}
		}
	}
	if (ppu_fetcher_internal_clock == 6) // Step 4: Push to pixel FIFO
	{
		if (ppu_fetcher_sprite_active)
		{
			const uint8_t* oam_sprite_data = memory + OBJECT_ATTRIBUTE_ADDRESS;
			const object_attribute_data* sprite = (object_attribute_data*)(oam_sprite_data + ppu_oam_buffer[ppu_oam_sprite_fetcher_index].oam_id);
			const bool sprite_uses_palette_1 = CHECK_BIT(sprite->flags, 4);
			const bool sprite_flip_x = CHECK_BIT(sprite->flags, 5);
			const bool sprite_bg_priority = CHECK_BIT(sprite->flags, 7);
			for (uint8_t column = 0; column < 8; column++)
			{
				const uint8_t right_bit = (ppu_fetcher_tile_data[0] & (0b10000000 >> column)) >> (7 - column);
				const uint8_t left_bit = (ppu_fetcher_tile_data[1] & (0b10000000 >> column)) >> (7 - column);
				const uint8_t pixel_value_sprite = ((left_bit << 1) | right_bit);
				const uint8_t index = (ppu_pixel_fifo_cursor + (sprite_flip_x ? 7 - column : column)) % 16;
				if (pixel_value_sprite > 0 && !ppu_pixel_fifo[index].from_sprite &&
					!(sprite_bg_priority && ppu_pixel_fifo[index].color > 0)) // Blend sprite pixels with pixel FIFO
				{
					const uint8_t paletted_pixel_value_sprite = sprite_uses_palette_1 ? sprite_palette_1[pixel_value_sprite] : sprite_palette_0[pixel_value_sprite];
					ppu_pixel_fifo[index].color = paletted_pixel_value_sprite;
					ppu_pixel_fifo[index].from_sprite = true;
				}
			}
			ppu_oam_buffer[ppu_oam_sprite_fetcher_index].drawn = true;
			ppu_fetcher_internal_clock = 0; // Successful push to FIFO
			ppu_fetcher_sprite_active = false;
		}
		else if (ppu_pixel_fifo_occupancy <= 8)
		{
			for (uint8_t column = 0; column < 8; column++)
			{
				const uint8_t index = (ppu_pixel_fifo_cursor + ppu_pixel_fifo_occupancy) % 16;
				const uint8_t right_bit = (ppu_fetcher_tile_data[0] & (0b10000000 >> column)) >> (7 - column);
				const uint8_t left_bit = (ppu_fetcher_tile_data[1] & (0b10000000 >> column)) >> (7 - column);
				const uint8_t pixel_value_bg = ((left_bit << 1) | right_bit);
				ppu_pixel_fifo[index].color = LCDC_BG_WINDOW_ENABLE ? pixel_value_bg : 0;
				ppu_pixel_fifo[index].from_sprite = false;
				ppu_pixel_fifo_occupancy++;
			}
			ppu_fetcher_internal_clock = 0; // Successful push to FIFO
			if (!ppu_fetcher_first_scanline_steps)
			{
				ppu_fetcher_internal_x_pos++;
			}
			else
			{
				ppu_fetcher_first_scanline_steps = false;
			}
		}
	}
}

int16_t ppu_shifter_pixel_x = 0;

void ppu_shifter_tick()
{
	if (ppu_fetcher_sprite_active)
	{
		return; // Can't shift pixels whilst sprite fetching is active
	}
	if (ppu_pixel_fifo_occupancy <= 8)
	{
		return; // Can't shift pixels unless there are at least 9 in the FIFO
	}
	if (LCDC_SPRITE_ENABLE && ppu_oam_buffer_count > 0)
	{
		
	}
}