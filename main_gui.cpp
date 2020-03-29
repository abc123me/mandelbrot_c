/*       ____           __          __
        /  _/___  _____/ /_  ______/ /__  _____
        / // __ \/ ___/ / / / / __  / _ \/ ___/
      _/ // / / / /__/ / /_/ / /_/ /  __(__  )
     /___/_/ /_/\___/_/\__,_/\__,_/\___/____/
================================================== */
// Standard
#include "stdio.h"
#include "stdlib.h"
#include "strings.h"
#include "time.h"
#include "math.h"
// OpenGL
#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
// GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
// My bullshit
#include "main.h"
#include "util.h"
#include "shader.h"
#include "debug.h"
#include "ft_text.h"

/*     __  __               __
      / / / /__  ____ _____/ /__  __________                                     *
     / /_/ / _ \/ __ `/ __  / _ \/ ___/ ___/
	/ __  /  __/ /_/ / /_/ /  __/ /  (__  )
   /_/ /_/\___/\__,_/\__,_/\___/_/  /____/
 ===============================================*/
//GLFW Callbacks
void init_glfw_callbacks(GLFWwindow* win);
void onWindowResize(GLFWwindow*, int w, int h);
void onMouseMove(GLFWwindow*, double x, double y);
void onMouseScroll(GLFWwindow*, double x, double y);
void onMouseButton(GLFWwindow*, int button, int action, int mods);
void onKeyCallback(GLFWwindow*, int key, int scancode, int action, int mods);

//Random
int8_t opengl_init();
int8_t go();
int8_t loop();
void draw();
void render();
void imm_render_buf(uint16_t* buf, uint16_t w, uint16_t h, uint8_t u, uint8_t s);

struct view_t {
	uint8_t julia = 0;
	cmplx_f min = cmplx_f(-1.0f, -1.0f);
	cmplx_f max = cmplx_f(1.0f, 1.0f);
	cmplx_f pos = cmplx_f(0.0f, 0.0f);
	uint16_t x = 0, y = 0, w = 0, h = 0;
	uint16_t* dat = NULL;
	uint8_t render = 0;
	float zoom = 1;
	
	view_t() {};
	void set_size(uint16_t w, uint16_t h);
};


/*      ________      __          __
       / ____/ /___  / /_  ____ _/ /____
      / / __/ / __ \/ __ \/ __ `/ / ___/
     / /_/ / / /_/ / /_/ / /_/ / (__  )
     \____/_/\____/_.___/\__,_/_/____/
==============================================*/
GLFWwindow* win;
Context* ctx;
FontRenderer font;
mb_opts cur_job;
uint8_t vsync = 0, core_mode = 0;
uint16_t winW = 0, winH = 0;
uint16_t newW = 0, newH = 0;
uint16_t mouseX = 0, mouseY = 0;
cmplx_f current_pos(0, 0);
cmplx_f mb_min(-1.0f, -1.0f);
cmplx_f mb_max(1.0f, 1.0f);
cmplx_f ju_min(-1.0f, -1.0f);
cmplx_f ju_max(1.0f, 1.0f);
uint16_t render_time_ms = 0;
float tiny_view_size = 0.25f;
double deltaTime = 0;

#define CMASK_MOUSE_MODE 1
#define CMASK_SWAP_VIEWS 2
uint16_t control_mask = CMASK_MOUSE_MODE;

//View pointers
view_t* mainView = NULL;
view_t* tinyView = NULL;
view_t* mandlView = NULL;
view_t* juliaView = NULL;
view_t* draw_mouse_onto = NULL;

// Update mask
uint8_t update = 0;
#define UPDATE_VP        1
#define UPDATE_MAIN_VIEW 2
#define UPDATE_TINY_VIEW 4
#define UPDATE_ALL 0xFF
//Screen selection
uint8_t screen_id = 0;
#define EXIT_SCREEN_ID 255
#define HELP_SCREEN_ID 1
#define VIEW_SCREEN_ID 0

#define RENDER_STEPS 10

Shader* fractal_render_shader = NULL;

void (*render_buf) (uint16_t* buf, uint16_t w, uint16_t h, uint8_t u, uint8_t d) = imm_render_buf;

void view_t::set_size(uint16_t nw, uint16_t nh) {
	w = nw; h = nh;
	if(dat) free(dat);
	dat = NULL;
	render = 1;
}
uint16_t render_view(view_t* v) {
	uint16_t rw = v->w * ctx->upscale, rh = v->h * ctx->upscale;
	uint32_t size = rw * rh * sizeof(uint16_t);
	if(v->render > 1) { rw /= v->render; rh /= v->render; }
	if(!v->dat) v->dat = (uint16_t*) malloc(size);
	mb_opts job;
	if(v->julia) job = new_julia_job(rw, rh, v->pos, ctx->iter, v->min, v->max);
	else job = new_mandelbrot_job(rw, rh, ctx->iter, v->min, v->max);
	clock_t start = clock();
	render_mb_buf(job, ctx->threads, v->dat);
	//job.print();
	uint16_t t = (1000 * (clock() - start)) / CLOCKS_PER_SEC;
	//printf("Took about %dms to complete\n", t);
	return t;
}
void draw_view(view_t* v) {
	uint8_t d = v->render;
	if(!v->dat || d) {
		render_view(v);
		v->render--;
	}
	glPushMatrix();
	glTranslatef(v->x, v->y, 0.0f);
	render_buf(v->dat, v->w, v->h, ctx->upscale, d);
	glPopMatrix();
}
void draw_stats() {
	uint8_t buflen = 128;
	char buf[buflen];
	uint16_t fh = font.getHeight();
	uint16_t pos = winH;
	strcpy(buf, "Press h for help");
	font.drawText(buf, 0, pos);
	current_pos.tostrn(buf + snprintf(buf, buflen, "C: "), buflen);
	font.drawText(buf, 0, pos -= fh);
	snprintf(buf, buflen, "dT: %.3f seconds", deltaTime);
	font.drawText(buf, 0, pos -= fh);
}
void drawMain() {
	if(mainView) draw_view(mainView);
	if(tinyView) draw_view(tinyView);
	if(draw_mouse_onto) {
		view_t* v = draw_mouse_onto;
		uint16_t x = round(map<float>(current_pos.real, mainView->min.real, mainView->max.real, 0, v->w));
		uint16_t y = round(map<float>(current_pos.imag, mainView->min.imag, mainView->max.imag, 0, v->h));
		glBegin(GL_LINES);
		glColor3ub(255, 255, 255);
		glVertex2f(v->x, y);
		glVertex2f(v->x + v->w, y);
		glColor3ub(255, 255, 255);
		glVertex2f(x, v->y);
		glVertex2f(x, v->y + v->h);
		glEnd();
	}

	draw_stats();
}
void draw_str(char* str, char* buf, uint16_t pos) {
	strcpy(buf, str);
	font.drawText(buf, 0, pos);
}
void drawHelp() {
	uint8_t buflen = 128;
	char buf[buflen];
	uint16_t fh = 0;
	uint16_t pos = 0;
	uint16_t h1 = font.getHeight(), h2 = (h1 * 2) / 3;
	font.setTextSize(h1); fh = font.getHeight();
	draw_str("---== Mouse controls ==---", buf, pos += fh);
	font.setTextSize(h2); fh = font.getHeight();
	draw_str("\tMove mouse to set view", buf, pos += fh);
	font.setTextSize(h1); fh = font.getHeight();
	draw_str("---== Key controls ==---", buf, pos += fh);
	font.setTextSize(h2); fh = font.getHeight();
	draw_str("m\tToggle between mouse/keyboard mode", buf, pos += fh);
	draw_str("j\tToggle between julia/mandelbrot views", buf, pos += fh);
	draw_str("ws\tIncrement/Decrement imaginary value of C", buf, pos += fh);
	draw_str("ad\tIncrement/Decrement real value of C", buf, pos += fh);
	draw_str("g\tShow next gradient", buf, pos += fh);
	draw_str("esc\tExit", buf, pos += fh);
	font.setTextSize(h1);
}

int8_t loop() {
	puts("Main loop started!");
	while(1){
		if(glfwWindowShouldClose(win)) return 0;
		if(update & UPDATE_VP) {
			update ^= UPDATE_VP;
			if(newW) { winW = newW; newW = 0; }
			if(newH) { winH = newH; newH = 0; }
			if(mainView)
				mainView->set_size(winW, winH);
			if(tinyView)
				tinyView->set_size((uint16_t)round(winW * tiny_view_size), (uint16_t)round(winH * tiny_view_size));
			if(mandlView) {
				float rmin = mb_min.real, rmax = mb_max.real;
				float imin = mb_min.imag, imax = mb_max.real;
				float r = rmax - rmin, i = imax - imin, rd = r / 3, id = i / 8;
				rmax += rd; rmin -= rd * 2;
				imax += id; imin -= id;
				mandlView->max = cmplx_f(rmax, imax);
				mandlView->min = cmplx_f(rmin, imin);
			}
			if(juliaView) {
				float rmin = ju_min.real, rmax = ju_max.real;
				float imin = ju_min.imag, imax = ju_max.real;
				float r = rmax - rmin, i = imax - imin, rd = r / 4, id = i / 4;
				rmax += rd; rmin -= rd;
				imax += id; imin -= id;
				juliaView->max = cmplx_f(rmax, imax);
				juliaView->min = cmplx_f(rmin, imin);
			}
			glLoadIdentity();
			font.setProjectionMatrix(glm::ortho(0.0f, (GLfloat)winW, (GLfloat)winH, 0.0f));
			glViewport(0, 0, winW, winH);
			glTranslatef(-1, 1, 0);
			glScalef(2 / (float) winW, -2 / (float) winH, 1);
		}
		if(update & UPDATE_MAIN_VIEW) {
			update ^= UPDATE_MAIN_VIEW;
			if(mainView) mainView->render = RENDER_STEPS;
		}
		if(update & UPDATE_TINY_VIEW) {
			update ^= UPDATE_TINY_VIEW;
			if(tinyView) tinyView->render = RENDER_STEPS;
		}
		uint64_t clk = clock();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		switch(screen_id) {
			case VIEW_SCREEN_ID: drawMain(); break;
			case HELP_SCREEN_ID: drawHelp(); break;
			case EXIT_SCREEN_ID: return 0;   break;
		}
		glfwSwapBuffers(win);
		glfwPollEvents();
		uint64_t dclk = clock() - clk;
		deltaTime = dclk / (double)CLOCKS_PER_SEC;
	}
	return -1;
}
int8_t go() {
	view_t mv; view_t tv;
	mainView = &mv;
	tinyView = &tv;
	if(ctx->julia) {
		juliaView = mainView;
		mandlView = tinyView;
	} else {
		juliaView = tinyView;
		mandlView = mainView;
	}
	juliaView->julia = 1;
	mandlView->julia = 0;
	draw_mouse_onto = mandlView;
	juliaView->pos = ctx->julia_pos;
	
	font.setFont("res/font.ttf");
	font.setTextSize(winW / 25);
	
	update = UPDATE_ALL;
	int8_t out = loop();

	if(mainView && mainView->dat) free(mainView->dat);
	if(tinyView && tinyView->dat) free(tinyView->dat);
	return out;
}
void core_render_buf(uint16_t* buf, uint16_t w, uint16_t h, uint8_t u, uint8_t d) {
	
}
void imm_render_buf_nds(uint16_t* buf, uint16_t w, uint16_t h, uint8_t u) {
	glBegin(GL_POINTS);
	uint16_t uu = u * u;
	for(uint16_t x = 0; x < w; x++) {
		for(uint16_t y = 0; y < h; y++) {
			Color c; uint16_t iter = 0;
			if(u > 1) {
				puts("test");
				for(uint8_t i = 0; i < u; i++)
					for(uint8_t j = 0; j < u; j++)
						iter += buf[x + i + w * (y + j)];
					iter /= uu;
			} else iter = buf[x + y * w];
			c = ctx->grad->at(iter, 0, ctx->iter);
			glVertex2s(x, y);
			glColor3ub(c.r, c.g, c.b);
		}
	}
	glEnd();
}
void imm_render_buf_ds(uint16_t* buf, uint16_t w, uint16_t h, uint8_t u, uint8_t d) {
	glBegin(GL_TRIANGLES);
	w /= d; h /= d;
	uint16_t uu = u * u;
	for(uint16_t x = 0; x < w; x++) {
		for(uint16_t y = 0; y < h; y++) {
			Color c; uint16_t iter = 0;
			if(u > 1) {
				puts("test");
				for(uint8_t i = 0; i < u; i++)
					for(uint8_t j = 0; j < u; j++)
						iter += buf[x + i + w * (y + j)];
					iter /= uu;
			} else iter = buf[x + y * w];
			c = ctx->grad->at(iter, 0, ctx->iter);
			uint16_t lx = x * d, ly = y * d;
			uint16_t rx = lx + d, ry = ly + d;
			glColor3ub(c.r, c.g, c.b);
			glVertex2s(rx, ry);
			glVertex2s(rx, ly);
			glVertex2s(lx, ry);
			glVertex2s(lx, ly);
			glVertex2s(rx, ly);
			glVertex2s(lx, ry);
		}
	}
	glEnd();
}
void imm_render_buf(uint16_t* buf, uint16_t w, uint16_t h, uint8_t u, uint8_t d) {
	if(d > 0) imm_render_buf_ds(buf, w, h, u, d);
	else imm_render_buf_nds(buf, w, h, u);
}

/*     ____      _ __  _       ___             __  _
      /  _/___  (_) /_(_)___ _/ (_)___  ____ _/ /_(_)___  ____
      / // __ \/ / __/ / __ `/ / /_  / / __ `/ __/ / __ \/ __ \
    _/ // / / / / /_/ / /_/ / / / / /_/ /_/ / /_/ / /_/ / / / /
   /___/_/ /_/_/\__/_/\__,_/_/_/ /___/\__,_/\__/_/\____/_/ /_/
==================================================================*/
#define __INIT(func) { out = func(); if(out) goto err; }
int8_t main_gui(Context* _ctx) {
	ctx = _ctx;
	int8_t out;
	__INIT(opengl_init)
	__INIT(init_ft_text)
	__INIT(go)
err:
	destroy_ft_text();
	glfwDestroyWindow(win);
	glfwTerminate();
	return out;
}
int8_t opengl_init() {
	puts("Initializing OpenGL!");
	if(!glfwInit()){
		printf("GLFW failed it initialize, OpenGL will not be supported!\n");
		return 1;
	}
	winW = 800; winH = 600;
	if(ctx->w && ctx->h) { winW = ctx->w; winH = ctx->h; }
	puts("Creating window!");
	win = glfwCreateWindow(winW, winH, "Mandelbrot", NULL, NULL);
	if(!win){
		printf("GLFW failed to initialize window!\n");
		return 1;
	}
	glfwMakeContextCurrent(win);
	if(glewInit() != GLEW_OK) {
		puts("Failed to initialize GLEW!");
		return 1;
	}
	init_glfw_callbacks(win);
	glfwSwapInterval(vsync);

	puts("OpenGL initialized!");
	return 0;
}
void gui_parse(uint16_t i, uint16_t cnt, char** args) {
	return;
}
void print_gui_help() {
	//puts("\t--fps/-f <fps>: Force a specific framerate");
	//puts("\t--no-vsync: Disables Vsync");
}
/*        ______                 __
         / ____/   _____  ____  / /______
        / __/ | | / / _ \/ __ \/ __/ ___/
       / /___ | |/ /  __/ / / / /_(__  )
      /_____/ |___/\___/_/ /_/\__/____/
 ==============================================*/
void init_glfw_callbacks(GLFWwindow* win) {
	glfwSetKeyCallback(win, onKeyCallback);
	glfwSetWindowSizeCallback(win, onWindowResize);
	glfwSetScrollCallback(win, onMouseScroll);
	glfwSetMouseButtonCallback(win, onMouseButton);
	glfwSetCursorPosCallback(win, onMouseMove);
}
void onWindowResize(GLFWwindow* win, int w, int h) {
	update |= UPDATE_ALL;
	newW = w; newH = h;
}
void onMouseMove(GLFWwindow*, double x, double y) {
	mouseX = round(x); mouseY = round(y);
	float lx = x / winW, ly = y / winH;
	view_t* ctx = mainView;
	current_pos.real = ctx->min.real + (ctx->max.real - ctx->min.real) * lx;
	current_pos.imag = ctx->min.imag + (ctx->max.imag - ctx->min.imag) * ly;
	if(juliaView) {
		juliaView->pos = current_pos;
		update |= juliaView == mainView ? UPDATE_MAIN_VIEW : UPDATE_TINY_VIEW;
	}
}
void onMouseScroll(GLFWwindow*, double x, double y) {}
void onMouseButton(GLFWwindow*, int button, int action, int mods) {}
void onKeyCallback(GLFWwindow*, int key, int scancode, int action, int mods) {
	if(action == GLFW_PRESS) {
		if(key == GLFW_KEY_H)
			screen_id = screen_id == HELP_SCREEN_ID ? VIEW_SCREEN_ID : HELP_SCREEN_ID;
		if(key == GLFW_KEY_ESCAPE)
			screen_id = EXIT_SCREEN_ID;
	}
}
