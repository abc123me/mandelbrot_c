#include "main.h"

#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "cli_gfx.h"
#include "util.h"

uint8_t force_color_mode = 0; // 0 bit color means ignore
uint8_t force_2xh = 0;        // Bit 0 is mode, bit 1 is enable

void print_cli_help() {
        puts("\t--24bit/-e: Force use of 24 bit ANSI mode");
        puts("\t--3bit/-3: Force use of 3 bit ANSI mode (default)");
        puts("\t--no-color/-g: Disable ANSI colors");
        puts("\t--2h/-d: Use double height technique (needs unicode)");
        puts("\t--1h/-s: Don't use double height technique (needs unicode)");
}
void cli_parse(uint16_t i, uint16_t cnt, char** args) {
	puts("cli_parse");
	CHECK_ARG(i, args, "--24bit", "-e",    { force_color_mode = 24; return; });
	CHECK_ARG(i, args, "--np-color", "-g", { force_color_mode = 1; return; });
	CHECK_ARG(i, args, "--3bit", "-3",     { force_color_mode = 3; return; });
	CHECK_ARG(i, args, "--2h", "-d",       { force_2xh = 3; return; });
	CHECK_ARG(i, args, "--1h", "-s",       { force_2xh = 2; return; });
}
int8_t main_cli(Context* ctx) {
	mb_opts opts;
	CliGfxCtx gfx = CliGfxCtx();
	puts("Initializing ANSI graphics");
	gfx.init(force_color_mode, force_2xh);
	uint16_t w = gfx.width(), h = gfx.height();
	printf("Termianl size is: %dx%d\n", w, h);
	printf("%s\n%s\n", gfx.name(), gfx.type());
	uint16_t uw = ctx->upscale, uh = uw;
	uint16_t rw = uw * w, rh = uh * h;
	uint16_t* dat = (uint16_t*) malloc(sizeof(uint16_t) * rw * rh);

	puts("Render started");
	ctx->min = cmplx_f(-2.0f, -1.25f);
	ctx->max = cmplx_f(2.0f, 1.25f);
	if(ctx->julia) opts = new_julia_job(rw, rh, ctx->julia_pos, ctx->iter, ctx->min, ctx->max);
	else opts = new_mandelbrot_job(rw, rh, ctx->iter, ctx->min, ctx->max);
	render_mb_buf(opts, ctx->threads, dat);

	puts("Press enter to continue!");
	while(getchar() != '\n');

	gfx.clear();
	float max = uw * uh * opts.max_iter;
	for(uint16_t x = 0; x < w; x++) {
		for(uint16_t y = 0; y < h; y++) {
			uint16_t g = 0;
			uint16_t ox = x * uw, oy = y * uh;
			for(uint8_t ux = 0; ux < uw; ux++)
				for(uint8_t uy = 0; uy < uh; uy++)
					g += dat[ox + ux + (oy + uy) * rw];
			gfx.set(x, y, ctx->grad->at((float)g, 0.0f, max));
		}
	}
	gfx.show();
	gfx.terminate();

	free(dat);
	puts("Press enter to continue!");

	while(getchar() != '\n') gfx.show();
	return 0;
}

