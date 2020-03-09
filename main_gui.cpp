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
// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H
// GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
// My bullshit
#include "main.h"
#include "shader.h"
#include "debug.h"

/*      ________      __          __
       / ____/ /___  / /_  ____ _/ /____
      / / __/ / __ \/ __ \/ __ `/ / ___/
     / /_/ / / /_/ / /_/ / /_/ / (__  )
     \____/_/\____/_.___/\__,_/_/____/
==========================================*/
GLFWwindow* win;
Context* ctx;
FT_Library ftlib;
FT_Face ftface;
mb_opts cur_job;
uint8_t vsync = 0, core_mode = 0;
uint16_t winW = 0, winH = 0;
uint16_t newW = 0, newH = 0;
uint16_t mouseX = 0, mouseY = 0;
uint32_t render_time_ms = 0;
float tiny_view_size = 0.25f;

struct view_t {
	uint8_t julia = 0;
	cmplx_f min = cmplx_f(-1.0f, -1.0f);
	cmplx_f max = cmplx_f(1.0f, 1.0f);
	cmplx_f pos = cmplx_f(0.0f, 0.0f);
	uint16_t x = 0, y = 0, w = 0, h = 0;
	uint16_t* dat = NULL;
	uint8_t render = 0;

	view_t() {};
	void set_size(uint16_t w, uint16_t h);
};

//Font bullshit
struct ft_char_text_t {
	GLuint id, adv;
	int16_t w, h, bx, by;
	ft_char_text_t(GLuint id, int16_t w, int16_t h, int16_t x, int16_t y, GLuint adv) :
		w(w), h(h), bx(x), by(y), id(id), adv(adv) {}
};
uint8_t glyph_cnt = 255;
ft_char_text_t* glyphs = NULL; //Contains glyphs from ' ' to '~'
Shader* font_vshader = NULL;
Shader* font_fshader = NULL;
GLuint font_vbuf_id = 0;
GLuint font_varr_id = 0;

//View pointers
view_t* mainView = NULL;
view_t* tinyView = NULL;
view_t* mandlView = NULL;
view_t* juliaView = NULL;
view_t* draw_mouse_onto = NULL;

/*     __  __               __
      / / / /__  ____ _____/ /__  __________
     / /_/ / _ \/ __ `/ __  / _ \/ ___/ ___/
    / __  /  __/ /_/ / /_/ /  __/ /  (__  )
   /_/ /_/\___/\__,_/\__,_/\___/_/  /____/
============================================*/
//GLFW Callbacks
void onWindowResize(GLFWwindow* win, int w, int h);
void onMouseMove(GLFWwindow*, double x, double y);
void onMouseScroll(GLFWwindow*, double x, double y);
void onMouseButton(GLFWwindow*, int button, int action, int mods);

//Functions
void imm_render_buf(uint16_t* buf, uint16_t w, uint16_t h, uint8_t u);
uint16_t draw_str(char* str, uint16_t x, uint16_t y);
uint16_t draw_char(char c, uint16_t x, uint16_t y);

//FreeType
int8_t freetype_init();
void load_font_glyphs();
uint16_t ft_core_draw_char(char c, uint16_t x, uint16_t y);
uint8_t ft_core_draw_str_start();
void ft_core_draw_str_finish();

//Random
int8_t opengl_init();
int8_t go();
int8_t loop();
void draw();
void render();

// Update mask
uint8_t update = 0;
#define UPDATE_VP        1
#define UPDATE_MAIN_VIEW 2
#define UPDATE_TINY_VIEW 4
#define UPDATE_FONT      8
#define UPDATE_ALL 0xFF

void view_t::set_size(uint16_t nw, uint16_t nh) {
	w = nw; h = nh;
	if(dat) free(dat);
	dat = NULL;
	render = 1;
}
void render_view(view_t* v) {
	uint16_t w = v->w, h = v->h;
	uint16_t rw = w * ctx->upscale, rh = h * ctx->upscale;
	if(!v->dat) v->dat = (uint16_t*) malloc(rw * rh * 2);
	mb_opts job;
	if(v->julia) job = new_julia_job(rw, rh, v->pos, ctx->iter, v->min, v->max);
	else job = new_mandelbrot_job(rw, rh, ctx->iter, v->min, v->max);
	clock_t start = clock();
	render_mb_buf(job, ctx->threads, v->dat);
	job.print();
	printf("Took about %dms to complete\n", (1000 * (clock() - start)) / CLOCKS_PER_SEC);
	v->render = 0;
}
void draw_view(view_t* v) {
	if(!v->dat || v->render) render_view(v);
	glPushMatrix();
	glTranslatef(v->x, v->y, 0.0f);
	imm_render_buf(v->dat, v->w, v->h, ctx->upscale);
	glPopMatrix();
}
void draw_stats() {
	draw_str("1", 100, 100);
}
void draw() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	if(mainView) draw_view(mainView);
	if(tinyView) draw_view(tinyView);
	if(draw_mouse_onto) {
		view_t* v = draw_mouse_onto;
		uint16_t x = round((mouseX / (float) winW) * v->w) + v->x;
		uint16_t y = round((mouseY / (float) winH) * v->h) + v->y;
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
	
	glfwSwapBuffers(win);
}

int8_t loop() {
	puts("Main loop started!");
	while(1){
		if(glfwWindowShouldClose(win)) return 0;
		if(update & UPDATE_VP) {
			update ^= UPDATE_VP;
			if(newW) { winW = newW; newW = 0; }
			if(newH) { winH = newH; newH = 0; }
			if(mainView) mainView->set_size(winW, winH);
			if(tinyView) tinyView->set_size((uint16_t)round(winW * tiny_view_size), (uint16_t)round(winH * tiny_view_size));
			glLoadIdentity();
			glViewport(0, 0, winW, winH);
			glTranslatef(-1, 1, 0);
			glScalef(2 / (float) winW, -2 / (float) winH, 1);
		}
		if(update & UPDATE_MAIN_VIEW) {
			update ^= UPDATE_MAIN_VIEW;
			if(mainView) mainView->render = 1;
		}
		if(update & UPDATE_TINY_VIEW) {
			update ^= UPDATE_TINY_VIEW;
			if(tinyView) tinyView->render = 1;
		}
		if(update & UPDATE_FONT) {
			update ^= UPDATE_FONT;
			FT_Set_Pixel_Sizes(ftface, 0, 20);
			load_font_glyphs();
		}
		draw();
		glfwPollEvents();
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

	update = UPDATE_ALL;
	int8_t out = loop();

	if(mainView && mainView->dat) free(mainView->dat);
	if(tinyView && tinyView->dat) free(tinyView->dat);
	return out;
}
uint16_t draw_str(char* str, uint16_t x, uint16_t y) {
	if(str && ft_core_draw_str_start()) {
		while(*str)
			x += ft_core_draw_char(*str++, x, y);
		ft_core_draw_str_finish();
	} return x;
}
uint16_t draw_char(char c, uint16_t x, uint16_t y) {
	char buf[2] = { c, 0 };
	return draw_str(buf, x, y);
}
void core_render_buf(uint16_t* buf, uint16_t w, uint16_t h, uint8_t u) {

}
void imm_render_buf(uint16_t* buf, uint16_t w, uint16_t h, uint8_t u) {
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
/*       ______                 __
        / ____/   _____  ____  / /______
       / __/ | | / / _ \/ __ \/ __/ ___/
      / /___ | |/ /  __/ / / / /_(__  )
     /_____/ |___/\___/_/ /_/\__/____/
============================================*/
void onWindowResize(GLFWwindow* win, int w, int h) {
	update |= UPDATE_ALL;
	newW = w; newH = h;
}
void onMouseMove(GLFWwindow*, double x, double y) {
	mouseX = round(x); mouseY = round(y);
}
void onMouseScroll(GLFWwindow*, double x, double y) {}
void onMouseButton(GLFWwindow*, int button, int action, int mods) {}

/*     ____      _ __  _       ___             __  _
      /  _/___  (_) /_(_)___ _/ (_)___  ____ _/ /_(_)___  ____
      / // __ \/ / __/ / __ `/ / /_  / / __ `/ __/ / __ \/ __ \
    _/ // / / / / /_/ / /_/ / / / / /_/ /_/ / /_/ / /_/ / / / /
   /___/_/ /_/_/\__/_/\__,_/_/_/ /___/\__,_/\__/_/\____/_/ /_/
==================================================================*/
Shader* shaderBS(ShaderSource* src, ShaderType st) {
	Shader* sptr = new Shader();
	char* stype = ShaderType_tostr(st);
	printf("Compiling %s shader!\n", stype);
	CompiledShader cshader = src->getShaderFromSource(st);
	sptr->attach(cshader);
	if(!sptr->link()) {
		printf("Failed to link %s shader!\n", stype);
		delete sptr; return NULL;
	}
	if(!sptr->validate()) {
		printf("Failed to validate %s shader!\n", stype);
		delete sptr; return NULL;
	}
	return sptr;
}
int8_t loadShadersFromFile(char* fname, Shader** vert, Shader** frag, Shader** geom) { 
	FILE* fp = fopen(fname, "r");
	printf("Loading shaders from \"%s\"\n", fname);
	if(fp) {
		ShaderSource src(fp, UnknownShader);
		fclose(fp);
		if(vert) *vert = shaderBS(&src, VertexShader);
		if(frag) *frag = shaderBS(&src, FragmentShader);
		if(geom) *geom = shaderBS(&src, GeometryShader);
		src.destroy();
		printf("Loaded shaders from \"%s\"\n", fname);
	} else {
		printf("Failed to load shaders from \"%s\"!\n", fname);
		return 1;
	}
	return 0;
}
#define __INIT(func) { out = func(); if(out) goto err; }
int8_t main_gui(Context* _ctx) {
	ctx = _ctx;
	int8_t out;
	__INIT(opengl_init)
	__INIT(freetype_init)
	__INIT(go)
err:
	if(font_fshader) delete font_fshader;
	if(font_vshader) delete font_vshader;
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
	glfwSetWindowSizeCallback(win, onWindowResize);
	glfwSetScrollCallback(win, onMouseScroll);
	glfwSetMouseButtonCallback(win, onMouseButton);
	glfwSetCursorPosCallback(win, onMouseMove);
	glfwSwapInterval(vsync);
	
	puts("OpenGL initialized!");
	return 0;
}
void gui_parse(uint16_t i, uint16_t cnt, char** args) {
	return;
}
void print_gui_help() {
	puts("\t--fps/-f <fps>: Force a specific framerate");
	puts("\t--no-vsync: Disables Vsync");
}

/*         ______             ______
          / ____/_______  ___/_  __/_  ______  ___
         / /_  / ___/ _ \/ _ \/ / / / / / __ \/ _ \
        / __/ / /  /  __/  __/ / / /_/ / /_/ /  __/
       /_/   /_/   \___/\___/_/  \__, / .___/\___/
                                /____/_/
==========================================================*/
int8_t init_freetype_shaders() {
	int8_t shader_err = loadShadersFromFile("res/font.glsl", &font_vshader, &font_fshader, NULL);
	if(shader_err) return shader_err;
	puts("Generating font render buffers");
	uint8_t glfs = sizeof(GLfloat);
	GL_CALL_CHECK(glGenVertexArrays(1, &font_varr_id), goto err)
	GL_CALL_CHECK(glGenBuffers(1, &font_vbuf_id), goto err)
	GL_CALL_CHECK(glBindVertexArray(font_varr_id), goto err)
	GL_CALL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, font_vbuf_id), goto err)
	GL_CALL_CHECK(glBufferData(GL_ARRAY_BUFFER, glfs * 24, NULL, GL_DYNAMIC_DRAW), goto err)
	GL_CALL_CHECK(glEnableVertexAttribArray(0), goto err)
	GL_CALL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * glfs, 0), goto err)
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0))
	GL_CALL(glBindVertexArray(0))
	puts("Font render buffers created!");
	return 0;
err:
	puts("Unrecoverable FreeTpye/OpenGL error detected");
	return 1;
}
int8_t freetype_init() {
	puts("Initializing FreeType!");
	int8_t err = FT_Init_FreeType(&ftlib);
	if(err) {
		puts("Failed to initialize FreeType!");
		return err;
	}
	err = FT_New_Face(ftlib, "res/font.ttf", 0, &ftface);
	if(err) {
		puts("Failed to load font!");
		return err;
	}
	err = init_freetype_shaders();
	if(err) {
		puts("Failed to load freetype shaders!");
		return err;
	}
	puts("FreeType initialized, font loaded!");
	return 0;
}
void load_font_glyphs() {
	if(glyphs) free(glyphs);
	glyphs = (ft_char_text_t*) malloc(sizeof(ft_char_text_t) * glyph_cnt);
	GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1)); // 4 byte allignment restriction, the glyphs are 1 byte grayscale
	for(uint8_t i = 0; i < glyph_cnt; i++) {
		if(FT_Load_Char(ftface, i, FT_LOAD_RENDER))
			continue;
		uint8_t w = ftface->glyph->bitmap.width, h = ftface->glyph->bitmap.rows;
		void* buf = ftface->glyph->bitmap.buffer;
		GLuint id;
		GL_CALL(glGenTextures(1, &id));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, id));
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, buf));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glyphs[i] = ft_char_text_t(id, w, h, ftface->glyph->bitmap_left, ftface->glyph->bitmap_top, ftface->glyph->advance.x);
	}
}
uint8_t ft_core_draw_str_start() {
	GL_CALL(glEnable(GL_BLEND))
	GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
	glPushMatrix();
	glLoadIdentity();
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glm::mat4 proj = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);//(GLfloat) winW, 0.0f, (GLfloat) winH);
	font_vshader->setUniformMat4x4f("proj", GL_FALSE, glm::value_ptr(proj));
	font_fshader->setUniform3f("textColor", 1.0f, 1.0f, 1.0f);
	GL_CALL(glActiveTexture(GL_TEXTURE0))
	GL_CALL(glBindVertexArray(font_varr_id))
	return 1;
}
void ft_core_draw_str_finish() {
	GL_CALL(glBindVertexArray(0))
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0))
	font_fshader->unbind();
	glPopMatrix();
}
uint16_t ft_core_draw_char(char c, uint16_t x, uint16_t y) {
	ft_char_text_t fct = glyphs[c];
	float scale = 0.05f;
	GLfloat w = fct.w * scale, h = fct.h * scale;
	GLfloat xp = 0.0f;//glfwMakeContextCurrent + fct.bx * scale;
	GLfloat yp = 0.0f;//y - (fct.h - fct.by) * scale;
	printf("%.1f, %.1f: %.1fx%.1f\n", xp, yp, w, h);
	GLfloat vbuf_dat[6][4] = {
		{ xp,     yp + h,   0.0, 0.0 },
		{ xp,     yp,       0.0, 1.0 },
		{ xp + w, yp,       1.0, 1.0 },
		{ xp,     yp + h,   0.0, 0.0 },
		{ xp + w, yp,       1.0, 1.0 },
		{ xp + w, yp + h,   1.0, 0.0 }
	};
	GL_CALL(glBindTexture(GL_TEXTURE_2D, fct.id))
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, font_vbuf_id))
	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vbuf_dat), vbuf_dat))
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0))
	GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6))
	x += (fct.adv >> 6) * scale;
	return x;
}
