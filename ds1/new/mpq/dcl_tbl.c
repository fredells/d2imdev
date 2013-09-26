/* DCL decompression table */

#include "dclib.h"

const UInt8 dcl_table[] =
{	0x02, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x03, 0x0D, 0x05, 0x19, 0x09, 0x11, 0x01, 0x3E,
	0x1E, 0x2E, 0x0E, 0x36, 0x16, 0x26, 0x06, 0x3A,
	0x1A, 0x2A, 0x0A, 0x32, 0x12, 0x22, 0x42, 0x02,
	0x7C, 0x3C, 0x5C, 0x1C, 0x6C, 0x2C, 0x4C, 0x0C,
	0x74, 0x34, 0x54, 0x14, 0x64, 0x24, 0x44, 0x04,
	0x78, 0x38, 0x58, 0x18, 0x68, 0x28, 0x48, 0x08,
	0xF0, 0x70, 0xB0, 0x30, 0xD0, 0x50, 0x90, 0x10,
	0xE0, 0x60, 0xA0, 0x20, 0xC0, 0x40, 0x80, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00,
	0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00,
	0x08, 0x00, 0x0A, 0x00, 0x0E, 0x00, 0x16, 0x00,
	0x26, 0x00, 0x46, 0x00, 0x86, 0x00, 0x06, 0x01,
	0x03, 0x02, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05,
	0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x07,
	0x05, 0x03, 0x01, 0x06, 0x0A, 0x02, 0x0C, 0x14,
	0x04, 0x18, 0x08, 0x30, 0x10, 0x20, 0x40, 0x00,
	0x0B, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x08, 0x07, 0x0C, 0x0C, 0x07, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0D, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x04, 0x0A, 0x08, 0x0C, 0x0A, 0x0C, 0x0A, 0x08,
	0x07, 0x07, 0x08, 0x09, 0x07, 0x06, 0x07, 0x08,
	0x07, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07,
	0x07, 0x08, 0x08, 0x0C, 0x0B, 0x07, 0x09, 0x0B,
	0x0C, 0x06, 0x07, 0x06, 0x06, 0x05, 0x07, 0x08,
	0x08, 0x06, 0x0B, 0x09, 0x06, 0x07, 0x06, 0x06,
	0x07, 0x0B, 0x06, 0x06, 0x06, 0x07, 0x09, 0x08,
	0x09, 0x09, 0x0B, 0x08, 0x0B, 0x09, 0x0C, 0x08,
	0x0C, 0x05, 0x06, 0x06, 0x06, 0x05, 0x06, 0x06,
	0x06, 0x05, 0x0B, 0x07, 0x05, 0x06, 0x05, 0x05,
	0x06, 0x0A, 0x05, 0x05, 0x05, 0x05, 0x08, 0x07,
	0x08, 0x08, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D, 0x0D,
	0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D,
	0x0D, 0x0D, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x90, 0x04, 0xE0, 0x0F, 0xE0, 0x07, 0xE0, 0x0B,
	0xE0, 0x03, 0xE0, 0x0D, 0xE0, 0x05, 0xE0, 0x09,
	0xE0, 0x01, 0xB8, 0x00, 0x62, 0x00, 0xE0, 0x0E,
	0xE0, 0x06, 0x22, 0x00, 0xE0, 0x0A, 0xE0, 0x02,
	0xE0, 0x0C, 0xE0, 0x04, 0xE0, 0x08, 0xE0, 0x00,
	0x60, 0x0F, 0x60, 0x07, 0x60, 0x0B, 0x60, 0x03,
	0x60, 0x0D, 0x60, 0x05, 0x40, 0x12, 0x60, 0x09,
	0x60, 0x01, 0x60, 0x0E, 0x60, 0x06, 0x60, 0x0A,
	0x0F, 0x00, 0x50, 0x02, 0x38, 0x00, 0x60, 0x02,
	0x50, 0x00, 0x60, 0x0C, 0x90, 0x03, 0xD8, 0x00,
	0x42, 0x00, 0x02, 0x00, 0x58, 0x00, 0xB0, 0x01,
	0x7C, 0x00, 0x29, 0x00, 0x3C, 0x00, 0x98, 0x00,
	0x5C, 0x00, 0x09, 0x00, 0x1C, 0x00, 0x6C, 0x00,
	0x2C, 0x00, 0x4C, 0x00, 0x18, 0x00, 0x0C, 0x00,
	0x74, 0x00, 0xE8, 0x00, 0x68, 0x00, 0x60, 0x04,
	0x90, 0x00, 0x34, 0x00, 0xB0, 0x00, 0x10, 0x07,
	0x60, 0x08, 0x31, 0x00, 0x54, 0x00, 0x11, 0x00,
	0x21, 0x00, 0x17, 0x00, 0x14, 0x00, 0xA8, 0x00,
	0x28, 0x00, 0x01, 0x00, 0x10, 0x03, 0x30, 0x01,
	0x3E, 0x00, 0x64, 0x00, 0x1E, 0x00, 0x2E, 0x00,
	0x24, 0x00, 0x10, 0x05, 0x0E, 0x00, 0x36, 0x00,
	0x16, 0x00, 0x44, 0x00, 0x30, 0x00, 0xC8, 0x00,
	0xD0, 0x01, 0xD0, 0x00, 0x10, 0x01, 0x48, 0x00,
	0x10, 0x06, 0x50, 0x01, 0x60, 0x00, 0x88, 0x00,
	0xA0, 0x0F, 0x07, 0x00, 0x26, 0x00, 0x06, 0x00,
	0x3A, 0x00, 0x1B, 0x00, 0x1A, 0x00, 0x2A, 0x00,
	0x0A, 0x00, 0x0B, 0x00, 0x10, 0x02, 0x04, 0x00,
	0x13, 0x00, 0x32, 0x00, 0x03, 0x00, 0x1D, 0x00,
	0x12, 0x00, 0x90, 0x01, 0x0D, 0x00, 0x15, 0x00,
	0x05, 0x00, 0x19, 0x00, 0x08, 0x00, 0x78, 0x00,
	0xF0, 0x00, 0x70, 0x00, 0x90, 0x02, 0x10, 0x04,
	0x10, 0x00, 0xA0, 0x07, 0xA0, 0x0B, 0xA0, 0x03,
	0x40, 0x02, 0x40, 0x1C, 0x40, 0x0C, 0x40, 0x14,
	0x40, 0x04, 0x40, 0x18, 0x40, 0x08, 0x40, 0x10,
	0x40, 0x00, 0x80, 0x1F, 0x80, 0x0F, 0x80, 0x17,
	0x80, 0x07, 0x80, 0x1B, 0x80, 0x0B, 0x80, 0x13,
	0x80, 0x03, 0x80, 0x1D, 0x80, 0x0D, 0x80, 0x15,
	0x80, 0x05, 0x80, 0x19, 0x80, 0x09, 0x80, 0x11,
	0x80, 0x01, 0x80, 0x1E, 0x80, 0x0E, 0x80, 0x16,
	0x80, 0x06, 0x80, 0x1A, 0x80, 0x0A, 0x80, 0x12,
	0x80, 0x02, 0x80, 0x1C, 0x80, 0x0C, 0x80, 0x14,
	0x80, 0x04, 0x80, 0x18, 0x80, 0x08, 0x80, 0x10,
	0x80, 0x00, 0x00, 0x1F, 0x00, 0x0F, 0x00, 0x17,
	0x00, 0x07, 0x00, 0x1B, 0x00, 0x0B, 0x00, 0x13,
	0xA0, 0x0D, 0xA0, 0x05, 0xA0, 0x09, 0xA0, 0x01,
	0xA0, 0x0E, 0xA0, 0x06, 0xA0, 0x0A, 0xA0, 0x02,
	0xA0, 0x0C, 0xA0, 0x04, 0xA0, 0x08, 0xA0, 0x00,
	0x20, 0x0F, 0x20, 0x07, 0x20, 0x0B, 0x20, 0x03,
	0x20, 0x0D, 0x20, 0x05, 0x20, 0x09, 0x20, 0x01,
	0x20, 0x0E, 0x20, 0x06, 0x20, 0x0A, 0x20, 0x02,
	0x20, 0x0C, 0x20, 0x04, 0x20, 0x08, 0x20, 0x00,
	0xC0, 0x0F, 0xC0, 0x07, 0xC0, 0x0B, 0xC0, 0x03,
	0xC0, 0x0D, 0xC0, 0x05, 0xC0, 0x09, 0xC0, 0x01,
	0xC0, 0x0E, 0xC0, 0x06, 0xC0, 0x0A, 0xC0, 0x02,
	0xC0, 0x0C, 0xC0, 0x04, 0xC0, 0x08, 0xC0, 0x00,
	0x40, 0x0F, 0x40, 0x07, 0x40, 0x0B, 0x40, 0x03,
	0x00, 0x03, 0x40, 0x0D, 0x00, 0x1D, 0x00, 0x0D,
	0x00, 0x15, 0x40, 0x05, 0x00, 0x05, 0x00, 0x19,
	0x00, 0x09, 0x40, 0x09, 0x00, 0x11, 0x00, 0x01,
	0x00, 0x1E, 0x00, 0x0E, 0x40, 0x01, 0x00, 0x16,
	0x00, 0x06, 0x00, 0x1A, 0x40, 0x0E, 0x40, 0x06,
	0x40, 0x0A, 0x00, 0x0A, 0x00, 0x12, 0x00, 0x02,
	0x00, 0x1C, 0x00, 0x0C, 0x00, 0x14, 0x00, 0x04,
	0x00, 0x18, 0x00, 0x08, 0x00, 0x10, 0x00, 0x00
};
