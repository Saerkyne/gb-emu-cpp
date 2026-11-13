#pragma once
#include <stdint.h>

#define TO_FLOAT_SAMPLE(value) (((float)value / 15.f) * -2.f) + 1.f

#define APU_VOLUME_LEFT ((audio_registers->nr5[0] & 0b01110000) >> 4)
#define APU_VOLUME_RIGHT (audio_registers->nr5[0] & 0b00000111)

#define APU_STATUS_SQ1 (audio_registers->nr5[2] & 0b00000001)
#define APU_ENABLESQ1_LEFT ((audio_registers->nr5[1] & 0b00010000) >> 4)
#define APU_ENABLESQ1_RIGHT (audio_registers->nr5[1] & 0b00000001)

#define APU_POWER_STATUS (audio_registers->nr5[2] >> 7)

#define SET_APU_STATUS_SQ1(value) audio_registers->nr5[2] = ((audio_registers->nr5[2] & 0b11111110) | (value))

#define SQ1_DAC_POWER ((audio_registers->nr1[2] >> 4) > 0)
#define SQ1_SWEEP_PERIOD ((audio_registers->nr1[0] & 0b01110000) >> 4)
#define SQ1_SWEEP_NEGATE ((audio_registers->nr1[0] & 0b00001000) >> 3)
#define SQ1_SWEEP_SHIFT (audio_registers->nr1[0] & 0b00000111)
#define SQ1_DUTY ((audio_registers->nr1[0] & 0b11000000) >> 6)
#define SQ1_LENGTH_LOAD ((audio_registers->nr1[1] & 0b00111111))
#define SQ1_ENV_VOLUME (audio_registers->nr1[2] >> 4)
#define SQ1_ENV_ADD ((audio_registers->nr1[2] & 0b00001000) >> 3)
#define SQ1_ENV_PERIOD (audio_registers->nr1[2] & 0b00000111)
#define SQ1_FREQUENCY_LSB (audio_registers->nr1[3])
#define SQ1_TRIGGER ((audio_registers->nr1[4] & 0b10000000) >> 7)
#define SQ1_LENGTH_ENABLE ((audio_registers->nr1[4] & 0b01000000) >> 6)
#define SQ1_FREQUENCY_MSB (audio_registers->nr1[4] & 0b00000111)
#define SQ1_FREQUENCY ((SQ1_FREQUENCY_MSB << 8) | SQ1_FREQUENCY_LSB)
#define SET_SQ1_TRIGGER(value) audio_registers->nr1[4] = ((audio_registers->nr1[4] & ~(1 << 7)) | (value << 7))
#define SET_SQ1_LENGTH_LOAD(value) audio_registers->nr1[1] = ((audio_registers->nr1[1] & 0b11000000) | value)

struct audio_register_set {
	uint8_t nr1[5]; // NR10, NR11, NR12, NR13, NR14
	uint8_t nr2[5]; // NR21, NR22, NR23, NR24
	uint8_t nr3[5]; // NR30, NR31, NR32, NR33, NR34
	uint8_t nr4[5]; // NR41, NR42, NR43, NR44
	uint8_t nr5[3]; // NR50, NR51, NR52
};

void apu_init();
void apu_reset();
void apu_tick(uint8_t clocks);
void apu_shutdown();

void apu_on_frame_end();
void apu_counter_tick();

uint8_t apu_read(uint16_t addr);
void apu_write(uint16_t addr, uint8_t value);