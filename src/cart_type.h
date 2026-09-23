#pragma once

enum CART_TYPE {
	NO_MBC,
	MBC1,
	MBC2,
	MBC3,
	MBC5,
	MBC6,
	MBC7,
	MMM01,
	POCKET_CAMERA,
	BANDAI_TAMA5,
	HUC3,
	HUC1,
	UNUSED
};

struct cart_type_info {
	CART_TYPE type;
	bool has_ram;
	bool has_battery;
	bool has_timer;
	bool has_rumble;
	bool has_accelerometer;
	const char* readable_name;
};

const cart_type_info cart_type_data[256] = {
	// CART_TYPE::TYPE,			RAM,	Bat,	Timer,	Rumble,	Accel,	"Readable Name"
	{CART_TYPE::NO_MBC, false, false, false, false, false, "No MBC, ROM Only"},			// 0x00
	{CART_TYPE::MBC1, false, false, false, false, false, "MBC1"},						// 0x01
	{CART_TYPE::MBC1, true, false, false, false, false, "MBC1 + RAM"},					// 0x02
	{CART_TYPE::MBC1, true, true, false, false, false, "MBC1 + RAM + Battery"},			// 0x03
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x04
	{CART_TYPE::MBC2, false, false, false, false, false, "MBC2"},						// 0x05
	{CART_TYPE::MBC2, true, true, false, false, false, "MBC2 + RAM + Battery"},			// 0x06
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x07
	{CART_TYPE::NO_MBC, true, false, false, false, false, "ROM + RAM"},					// 0x08
	{CART_TYPE::NO_MBC, true, true, false, false, false, "ROM + RAM + Battery"},		// 0x09
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x0A
	{CART_TYPE::MMM01, false, false, false, false, false, "MMM01"},						// 0x0B
	{CART_TYPE::MMM01, true, false, false, false, false, "MMM01 + RAM"},				// 0x0C
	{CART_TYPE::MMM01, true, true, false, false, false, "MMM01 + RAM + Battery"},		// 0x0D
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x0E
	{CART_TYPE::MBC3, false, true, true, false, false, "MBC3 + Timer + Battery"},		// 0x0F
	{CART_TYPE::MBC3, true, true, true, false, false, "MBC3 + RAM + Timer + Battery"},	// 0x10
	{CART_TYPE::MBC3, false, false, false, false, false, "MBC3"},						// 0x11
	{CART_TYPE::MBC3, true, false, false, false, false, "MBC3 + RAM"},					// 0x12
	{CART_TYPE::MBC3, true, true, false, false, false, "MBC3 + RAM + Battery"},			// 0x13
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x14
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x15
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x16
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x17
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x18
	{CART_TYPE::MBC5, false, false, false, false, false, "MBC5"},						// 0x19
	{CART_TYPE::MBC5, true, false, false, false, false, "MBC5 + RAM"},					// 0x1A
	{CART_TYPE::MBC5, true, true, false, false, false, "MBC5 + RAM + Battery"},			// 0x1B
	{CART_TYPE::MBC5, false, false, false, true, false, "MBC5 + Rumble"},				// 0x1C
	{CART_TYPE::MBC5, true, false, false, true, false, "MBC5 + RAM + Rumble"},			// 0x1D
	{CART_TYPE::MBC5, true, true, false, true, false, "MBC5 + RAM + Battery + Rumble"}, // 0x1E
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x1F
	{CART_TYPE::MBC6, true, true, false, false, false, "MBC6 + RAM + Battery"},			// 0x20
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x21
	{CART_TYPE::MBC7, true, true, false, false, true, "MBC7 + RAM + Battery + Accel"},	// 0x22
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x23
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x24
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x25
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x26
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x27
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x28
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x29
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x2A
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x2B
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x2C
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x2D
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x2E
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x2F
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x30
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x31
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x32
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x33
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x34
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x35
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x36
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x37
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x38
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x39
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x3A
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x3B
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x3C
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x3D
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x3E
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x3F
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x40
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x41
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x42
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x43
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x44
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x45
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x46
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x47
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x48
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x49
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x4A
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x4B
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x4C
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x4D
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x4E
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x4F
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x50
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x51
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x52
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x53
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x54
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x55
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x56
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x57
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x58
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x59
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x5A
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x5B
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x5C
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x5D
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x5E
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x5F
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x60
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x61
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x62
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x63
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x64
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x65
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x66
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x67
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x68
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x69
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x6A
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x6B
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x6C
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x6D
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x6E
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x6F
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x70
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x71
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x72
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x73
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x74
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x75
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x76
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x77
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x78
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x79
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x7A
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x7B
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x7C
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x7D
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x7E
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x7F
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x80
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x81
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x82
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x83
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x84
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x85
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x86
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x87
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x88
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x89
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x8A
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x8B
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x8C
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x8D
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x8E
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x8F
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x90
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x91
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x92
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x93
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x94
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x95
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x96
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x97
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x98
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x99
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x9A
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x9B
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x9C
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x9D
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x9E
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0x9F
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xA0
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xA1
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xA2
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xA3
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xA4
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xA5
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xA6
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xA7
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xA8
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xA9
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xAA
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xAB
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xAC
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xAD
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xAE
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xAF
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xB0
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xB1
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xB2
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xB3
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xB4
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xB5
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xB6
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xB7
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xB8
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xB9
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xBA
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xBB
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xBC
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xBD
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xBE
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xBF
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xC0
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xC1
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xC2
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xC3
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xC4
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xC5
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xC6
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xC7
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xC8
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xC9
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xCA
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xCB
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xCC
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xCD
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xCE
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xCF
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xD0
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xD1
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xD2
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xD3
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xD4
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xD5
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xD6
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xD7
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xD8
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xD9
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xDA
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xDB
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xDC
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xDD
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xDE
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xDF
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xE0
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xE1
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xE2
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xE3
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xE4
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xE5
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xE6
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xE7
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xE8
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xE9
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xEA
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xEB
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xEC
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xED
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xEE
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xEF
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xF0
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xF1
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xF2
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xF3
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xF4
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xF5
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xF6
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xF7
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xF8
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xF9
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xFA
	{CART_TYPE::UNUSED, false, false, false, false, false, "Unused"},					// 0xFB
	{CART_TYPE::POCKET_CAMERA, false, false, false, false, false, "Pocket Camera"},		// 0xFC
	{CART_TYPE::BANDAI_TAMA5, false, false, false, false, false, "Bandai TAMA5"},		// 0xFD
	{CART_TYPE::HUC3, false, false, false, false, false, "HuC-3"},						// 0xFE
	{CART_TYPE::HUC1, true, true, false, false, false, "HuC-1 + RAM + Battery"},		// 0xFF
};