#ifndef _CLI_GFX_H
#define _CLI_GFX_H
#include "stdint.h"
#include "colors.h"

#ifdef __unix__

#include "sys/ioctl.h"

#define GFX_CLI_GFX
class CliGfxCtx {
private:
	uint16_t w, h;
	uint16_t color_bits = 3;
	uint8_t double_h = 0;
	struct winsize size;
	char* type_str;
	uint8_t type_len;
public:
	CliGfxCtx();
	~CliGfxCtx();
	int8_t init();
	uint16_t width();
	uint16_t height();
	char* type();
	char* name();
	void set(uint16_t x, uint16_t y, Color c);
	void clear();
	void show();
	int8_t try_resize(uint16_t w, uint16_t h);
	void terminate();
};
#else
#error Windows not supported!
#endif
#endif
