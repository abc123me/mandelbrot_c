#include "main.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "util.h"

int main(int argc, char** argv) {
	for(uint16_t i = 1; i < argc; i++) {
		if(compare_arg(argv[i], "--help", "-h"))
			{ print_help(); return 0; }
		if(compare_arg(argv[i], "--gradients", NULL))
			{ Gradients::print_gradients(); return 0; }
	}
	Context ctx(argc, argv); ctx.parse();
	void (*func)(uint16_t, uint16_t, char**) = NULL;
	if(ctx.mode == MODE_NONE) {
		puts("No mode selected, testing best mode");
		char* dsp = getenv("DISPLAY");
		if(dsp) {
			printf("Displaying on %s!\n", dsp);
			ctx.mode = MODE_GUI;
		} else {
			puts("Defaulting to terminal!");
			ctx.mode = MODE_CLI;
		}
	}
	if(func) for(uint16_t i = 1; i < argc; i++)
		if(argv[i]) func(i, argc, argv);
	if(ctx.mode == MODE_GUI) return main_gui(&ctx);
	if(ctx.mode == MODE_CLI) return main_cli(&ctx);
}
void Context::parse() {
	uint8_t set_grad = 0;
	for(uint16_t i = 1; i < cnt; i++) {
		uint16_t j = i;
		if(!args[i]) continue;
		CHECK_ARG(args, i, "--cli", NULL, { mode = MODE_CLI; goto pop; });
		CHECK_ARG(args, i, "--gui", NULL, { mode = MODE_GUI; goto pop; });
		CHECK_ARG(args, i, "--sqrt-gradient", NULL, { set_grad |= GRADIENT_SQRT; });
		CHECK_ARG(args, i, "--invert-gradient", NULL, { set_grad |= GRADIENT_INVERT; });
		if(i >= cnt - 1) continue;
		CHECK_ARG_AND_SET(args, i, "--upscale", "-u", upscale, atoi, { printf("Set upscale to %d\n", upscale); goto pop; });
		CHECK_ARG_AND_SET(args, i, "--iterations", "-i", iter, atoi, { printf("Set iterations to %d\n", iter); goto pop; });
		CHECK_ARG_AND_SET(args, i, "--threads", "-t", threads, atoi, { printf("Set threads to %d\n", threads); goto pop; });
		CHECK_ARG_AND_SET(args, i, "--julia", "-j", julia_pos, atocmplx_f, { julia = 1; goto pop; });
		CHECK_ARG(args, i, "--gradient", "-g", {
			Gradient* g = Gradients::find_gradient(args[++i]);
			if(g) { grad = g;
				printf("Gradient set to %s\n", grad->name());
			} else printf("Failed to find gradient %s\n", args[i]);
			goto pop;
		});
		if(i >= cnt - 2) continue;
		CHECK_ARG(args, i, "--size", "-s", { w = atoi(args[++i]); h = atoi(args[++i]); goto pop; });
pop:		for(uint16_t k = j; k < i; k++)
			args[k] = NULL;
	}
	if(upscale == 0) { upscale = 1; printf("Upscale must be >0, it is now: %d\n", upscale); }
	if(threads == 0) { threads = 1; printf("Threads must be >0, it is now: %d\n", threads); }
	if(iter == 0) { iter = 1; printf("Iterations must be >0, it is now: %d\n", iter); }
	grad->setAction(set_grad);
}
void print_help() {
        puts("Mandelbrot/Julia set rendering tool");
        puts("-----------------------------------");
        puts("  --help/-h: Shows this help dialog");
        puts("  --cli/--gui: Force CLI/GUI (default) mode");
        puts("  --size/-s <w> <h>: Sets the output resolution");
        puts("  --upscale/-u <factor>: Set upscaling (default 1x)");
        puts("  --iterations/-i <iterations>: Set iterations (default 255)");
        puts("  --julia/-j <c>: Julia mode, Position should be in format a+/-bi");
	puts("  --gradient/-g <g>: Selects a gradient to use");
	puts("  --gradients: Shows a list of gradients");
	puts("  --sqrt-gradient: Square roots gradient curve");
	puts("  --invert-gradient: Inverts the gradient curve");
      printf("  --threads/-t <num>: Number of render threads used, defaults to %d\n", get_threads());
        puts("------------- CLI Mode ------------");
	print_cli_help();
        puts("------------- GUI Mode ------------");
        print_gui_help();
}
