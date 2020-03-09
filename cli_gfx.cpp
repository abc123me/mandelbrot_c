#include "cli_gfx.h"

#include "stdlib.h"
#include "stdio.h"
#include "util.h"

#ifdef __unix__

#include "sys/ioctl.h"
#include "unistd.h"

CliGfxCtx::CliGfxCtx() {
	type_len = 64;
	type_str = malloc_str(type_len, 0);
}
CliGfxCtx::~CliGfxCtx() {
	free(type_str);
}
int8_t CliGfxCtx::init() {
	char* term = getenv("TERM");
	if(str_startswith(term, "xterm")) {
		color_bits = 24;
		double_h = 1;
	}
	snprintf(type_str, type_len, "Linux \"%s\", %d bit color", term, color_bits);
	int8_t out = ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	w = size.ws_col; h = size.ws_row;
	if(double_h) h *= 2;
	if(out != 0)
		puts("CliGfxCtx: ioctl failed");
	return out;
};
uint16_t CliGfxCtx::width() { return w; };
uint16_t CliGfxCtx::height() { return h; };
char* CliGfxCtx::name() { return "CLI graphics"; }
char* CliGfxCtx::type() { return type_str; }
void CliGfxCtx::set(uint16_t x, uint16_t y, Color color) {
	uint8_t sty = 4;
	char* str = " ";
	if(double_h) {
		str = "▀";
		sty = (y % 2 == 0) ? 3 : 4;
		y /= 2;
	}
	printf("\x1b[%d;%df", y, x);
	if(color_bits == 24) printf("\x1b[%d8;2;%d;%d;%dm%s", sty, color.r, color.g, color.b, str);
	else printf("\x1b[1;%d%dm%s", sty, color.to3bit(), str);
}
void CliGfxCtx::clear() {
	printf("\x1b[0;\x1b[2J\x1b[0;0f");
}
void CliGfxCtx::terminate() {}
void CliGfxCtx::show() {}
int8_t CliGfxCtx::try_resize(uint16_t w, uint16_t h) { return 1; }
#else
#error Windows not yet supported!
#endif
