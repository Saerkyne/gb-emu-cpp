#pragma once
#include <stdint.h>

const uint32_t LCD_PIXELS_WIDTH = 160;
const uint32_t LCD_PIXELS_HEIGHT = 144;
const uint32_t PPU_RENDER_SCALE = 3;

const uint8_t LCDS_MODE_0_H_BLANK = 0;
const uint8_t LCDS_MODE_1_V_BLANK = 1;
const uint8_t LCDS_MODE_2_OAM_SEARCH = 2;
const uint8_t LCDS_MODE_3_DRAWING = 3;

const uint16_t OAM_ATTRIBUTE_ADDRESS = 0xFE00;

struct lcd_registers {
	uint8_t control;
	uint8_t status;
	uint8_t scy;
	uint8_t scx;
	uint8_t ly;
	uint8_t lyc;
	uint8_t oam_dma;
	uint8_t bgp;
	uint8_t obp0;
	uint8_t obp1;
	uint8_t wy;
	uint8_t wx;
};

struct object_attribute_data {
	uint8_t pos_y;
	uint8_t pos_x;
	uint8_t tile_id;
	uint8_t flags;
};

struct oam_buffer_data {
	uint8_t oam_id;
	bool drawn;
};

struct pixel_fifo_data {
	uint8_t color;
	bool from_sprite;
};

// LCD Control Flags
#define LCDC_ENABLE_LCD CHECK_BIT(ppu_lcd_registers->control, 7)
#define LCDC_WINDOW_TILE_MAP_SELECT CHECK_BIT(ppu_lcd_registers->control, 6)
#define LCDC_WINDOW_ENABLE CHECK_BIT(ppu_lcd_registers->control, 5)
#define LCDC_TILE_DATA_SELECT CHECK_BIT(ppu_lcd_registers->control, 4)
#define LCDC_BG_TILE_MAP_SELECT CHECK_BIT(ppu_lcd_registers->control, 3)
#define LCDC_SPRITE_SIZE CHECK_BIT(ppu_lcd_registers->control, 2)
#define LCDC_SPRITE_ENABLE CHECK_BIT(ppu_lcd_registers->control, 1)
#define LCDC_BG_WINDOW_ENABLE CHECK_BIT(ppu_lcd_registers->control, 0)

// LCD Status Flags
#define LCDS_LYC_EQUALS_LY_INTERRUPT_ENABLE CHECK_BIT(ppu_lcd_registers->status, 6)
#define LCDS_MODE_2_STAT_INTERRUPT_ENABLE CHECK_BIT(ppu_lcd_registers->status, 5)
#define LCDS_MODE_1_STAT_INTERRUPT_ENABLE CHECK_BIT(ppu_lcd_registers->status, 4)
#define LCDS_MODE_0_STAT_INTERRUPT_ENABLE CHECK_BIT(ppu_lcd_registers->status, 3)
#define LCDS_COINCIDENCE_FLAG CHECK_BIT(ppu_lcd_registers->status, 2)
#define LCDS_PPU_MODE (ppu_lcd_registers->status & 0b00000011)

void ppu_init();
void ppu_reset();
void ppu_advance_clocks(const uint8_t clocks);

void ppu_check_ly_compare();
void ppu_check_stat_signal();

void ppu_on_lyc_write(const uint8_t value);
void ppu_on_lcd_control_write(const uint8_t value);
void ppu_on_lcd_status_write(const uint8_t value);
void ppu_on_palette_write(const uint16_t addr);

void render_frame();
void render_debug();