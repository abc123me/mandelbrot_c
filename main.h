#ifndef _MAIN_H
#define _MAIN_H

#include "stdint.h"

#include "util.h"
#include "colors.h"

#define MODE_NONE 0
#define MODE_CLI  1
#define MODE_GUI  2

struct Context {
	Context(uint16_t c, char** v) : cnt(c), args(v), threads(get_threads()) {}
	char** args;
	uint16_t cnt;
	uint8_t mode = MODE_NONE;
	uint8_t upscale = 1;
	uint16_t iter = 255;
	uint8_t julia = 0;
	uint16_t threads = 0;
	uint16_t w = 0, h = 0;
	cmplx_f julia_pos = cmplx_f(0.0f, 0.0f);
	cmplx_f min = cmplx_f(-1.0f, -1.0f);
	cmplx_f max = cmplx_f(1.0f, 1.0f);
	Gradient* grad = Gradients::get_gradient(0);

	void parse();
};
//main.cpp
void print_help();
//main_gui.cpp
int8_t main_gui(Context* ctx);
void print_gui_help();
void gui_parse(uint16_t i, uint16_t cnt, char** args);
//main_cli.cpp
int8_t main_cli(Context* ctx);
void print_cli_help();
void cli_parse(uint16_t i, uint16_t cnt, char** args);

#define CHECK_ARG(args, i, lhand, shand, code) if(compare_arg(args[i], lhand, shand)) { code; }
#define CHECK_ARG_AND_SET(args, i, lhand, shand, val, func, code) if(compare_arg(args[i], lhand, shand)) { val = func(args[++i]); code; }

#endif
