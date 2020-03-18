/*       ____           __          __
 *        /  _/___  _____/ /_  ______/ /__  _____
 *        / // __ \/ ___/ / / / / __  / _ \/ ___/
 *      _/ // / / / /__/ / /_/ / /_/ /  __(__  )
 *     /___/_/ /_/\___/_/\__,_/\__,_/\___/____/
 * ================================================== */
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
#include "shader.h"
#include "debug.h"
#include "ft_text.h"

FontRenderer::FontRenderer() {
	glyphs = (ft_char_text_t*)malloc(sizeof(ft_char_text_t) * 255);
	update_glyphs = 1;
}
~FontRenderer::FontRenderer() {
	free(glyphs);
	glyphs = NULL;
}
void FontRenderer::setTextSize(uint16_t size) {
	
}
void FontRenderer::setTextScale(float scale) { 
	
}
uint16_t FontRenderer::drawChar(char c, uint16_t x, uint16_t y) { 
	char buf[2] = { c, 0 };
	return draw_str(buf, x, y);
}
uint16_t FontRenderer::drawText(char* str, uint16_t x, uint16_t y) {
	uint16_t ox = x;
	if(str && ft_core_draw_str_start()) {
		while(*str) {
			char c = *str++;
			if(c == '\r') x = ox;
			else if(c == '\n') y += glyphs[c].h;
			else x += ft_core_draw_char(glyphs[c], x, y);
		}
		ft_core_draw_str_finish();
	} return x;
}
void FontRenderer::load_font_glyphs() {
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
uint8_t FontRenderer::ft_core_draw_str_start() {
	if(!glyphs) {
		puts("FreeType glyph buffer is null, cannot continue");
		return 0;
	}
	GL_CALL(glEnable(GL_BLEND))
	GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
	
	GL_CALL(glActiveTexture(GL_TEXTURE0))
	
	font_shader->setUniform4f("textBackColor", 0.0f, 0.0f, 0.0f, 1.0f);
	font_shader->setUniform4f("textForeColor", 1.0f, 1.0f, 1.0f, 1.0f);
	font_shader->setUniform1f("threshold", 0.5f);
	font_shader->setUniform1f("textScale", 1.0f);
	
	glm::mat4 p = glm::ortho(0.0f, (GLfloat)winW, (GLfloat)winH, 0.0f);
	font_shader->setUniformMat4x4f("proj", GL_TRUE, glm::value_ptr(p));
	return 1;
}
void FontRenderer::ft_core_draw_str_finish() {
	GL_CALL(glBindVertexArray(0))
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0))
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0))
	font_shader->unbind();
}
uint16_t FontRenderer::ft_core_draw_char(ft_char_text_t fct, uint16_t x, uint16_t y) {
	GLfloat w = fct.w, h = fct.h;
	GLfloat xp = x + fct.bx;
	GLfloat yp = y - fct.by;
	
	GLfloat vbuf_dat[24] = {
		xp,     yp,     0.0, 0.0, 
		xp,     yp + h, 0.0, 1.0, 
		xp + w, yp + h, 1.0, 1.0,
		xp,     yp,     0.0, 0.0,
		xp + w, yp + h, 1.0, 1.0,
		xp + w, yp,     1.0, 0.0
	};
	
	GL_CALL(glBindTexture(GL_TEXTURE_2D, fct.id))
	GL_CALL(glBindVertexArray(font_varr_id))
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, font_vbuf_id))
	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vbuf_dat), vbuf_dat))
	GL_CALL(glEnableVertexAttribArray(0))
	GL_CALL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0))
	GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6))
	GL_CALL(glDisableVertexAttribArray(0))
	return x + fct.adv >> 6;
}

int8_t init_freetype_shaders() {
	int8_t shader_err = loadShadersFromFile("res/font.glsl", &font_shader);
	if(shader_err) return shader_err;
	puts("Generating font render buffers");
	uint8_t glfs = sizeof(GLfloat);
	//Make VAO
	GL_CALL_CHECK(glGenVertexArrays(1, &font_varr_id), goto err)
	GL_CALL_CHECK(glBindVertexArray(font_varr_id), goto err)
	//Make VB
	GL_CALL_CHECK(glGenBuffers(1, &font_vbuf_id), goto err)
	GL_CALL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, font_vbuf_id), goto err)
	GL_CALL_CHECK(glBufferData(GL_ARRAY_BUFFER, 24 * glfs, NULL, GL_STATIC_DRAW), goto err)	
	GL_CALL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, font_vbuf_id), goto err)
	
	
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0))
	GL_CALL(glBindVertexArray(0))
	puts("Font render buffers created!");
	return 0;
	err:
	puts("Unrecoverable FreeTpye/OpenGL error detected");
	return 1;
}
int8_t init_freetype() {
	puts("Initializing FreeType!");
	int8_t err = FT_Init_FreeType(&ftlib);
	if(err) {
		puts("Failed to initialize FreeType!");
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
