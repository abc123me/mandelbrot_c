// Standard
#include "stdio.h"
#include "stdlib.h"
#include "strings.h"
#include "time.h"
#include "math.h"
// OpenGL
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

FT_Library ftlib;
Shader* font_shader = NULL;
GLuint font_varr_id = 0;
GLuint font_vbuf_id = 0;

gcache_entry_t::gcache_entry_t() {
	this->size = 0;
	this->uses = 0;
	this->glyphs = NULL;
}
gcache_entry_t::gcache_entry_t(uint16_t size) {
	this->size = size;
	this->uses = 0;
	this->glyphs = NULL;
}
void gcache_entry_t::unload() {
	if(glyphs) {
		for(uint8_t i = 0; i < CHARSET_SIZE; i++) {
			GL_CALL(glDeleteTextures(1, &glyphs[i].id));
		}
		free(glyphs);
		size = 0;
		glyphs = NULL;
	}
}
void gcache_entry_t::load(FT_Face ftface) {
	this->unload();
	FT_Set_Pixel_Sizes(ftface, 0, size);
	glyphs = (ft_char_text_t*) malloc(sizeof(ft_char_text_t) * CHARSET_SIZE);
	GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1)); // 4 byte allignment restriction, the glyphs are 1 byte grayscale
	for(uint8_t i = 0; i < CHARSET_SIZE; i++) {
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
		if(i == ' ') w = (ftface->glyph->advance.x >> 6);
		glyphs[i] = ft_char_text_t(id, w, h, ftface->glyph->bitmap_left, ftface->glyph->bitmap_top, ftface->glyph->advance.x);
	}
}
uint8_t gcache_entry_t::needs_loaded() { return size > 0 && !glyphs; }
uint8_t gcache_entry_t::is_free() { return size == 0 && !glyphs; }


FontRenderer::FontRenderer() {
	gcache_len = 0;
	entries = NULL;
	cur = NULL;
	setGlyphCacheSize(5);
	setTextSize(36);
}
FontRenderer::~FontRenderer() {
	if(cmask & FR_CMASK_FTFACE_VALID)
		FT_Done_Face(ftface);
	destroy_tmp_gcache_entry();
	destroy_glyph_cache();
}
int8_t FontRenderer::setFont(char* name) {
	if(cmask & FR_CMASK_FTFACE_VALID) {
		FT_Done_Face(ftface);
		cmask ^= FR_CMASK_FTFACE_VALID;
	}
	int8_t out = FT_New_Face(ftlib, name, 0, &ftface);
	if(out) return out;
	cmask |= FR_CMASK_UPDATE_GLYPHS;
	cmask |= FR_CMASK_FTFACE_VALID;
	return 0;
}
uint8_t FontRenderer::getTabSpaces() { return tabWidth; }
void FontRenderer::setTabSpaces(uint8_t cnt) { tabWidth = cnt; }
void FontRenderer::setTextSize(uint16_t size) {
	destroy_tmp_gcache_entry();
	gcache_entry_t* pos = NULL;
	// Check cache for existing
	for(uint8_t i = 0; i < gcache_len; i++)
		if(entries[i].size == size)
			pos = &entries[i];
	if(pos) {
		pos->uses++;
		if(pos->uses > 65000)
			pos->uses = 65000;
		cur = pos;
	} else {
		cur = new gcache_entry_t(size);
		cur->uses = 1;
		gcache_entry_t* pos = cache_glyph_entry(cur);
		if(pos) {
			free(cur);
			cur = pos;
		}
	}
}
void FontRenderer::setProjectionMatrix(glm::mat4 proj) { this->proj = proj; }
void FontRenderer::setBlending(uint8_t b) {
	if(b) threshold =  0.5f;
	else  threshold = -1.0f;
}
uint16_t FontRenderer::drawChar(char c, uint16_t x, uint16_t y) { 
	if(c == '\n') return x;
	if(c == '\r') return 0;
	if(c == '\t') return x += tabWidth;
	char buf[2] = { c, 0 };
	return drawText(buf, x, y);
}
uint16_t FontRenderer::drawText(char* str, uint16_t x, uint16_t y) {
	uint16_t ox = x;
	if(str && ft_core_draw_str_start()) {
		ft_char_text_t* glyphs = cur->glyphs;
		while(*str) {
			char c = *str++;
			if(c == '\r') x = ox;
			else if(c == '\n') y -= (glyphs[c].h * 1.5);
			else if(c == '\t') {
				uint16_t w = tabWidth * glyphs[' '].w;
				uint8_t ph = x / w + 1;
				x = ph * w;
			}
			else x += ft_core_draw_char(glyphs[c], x, y);
		}
		ft_core_draw_str_finish();
	} return x;
}
uint8_t FontRenderer::ft_core_draw_str_start() {
	if(!cur) return 0;
	if(cur->needs_loaded())
		cur->load(ftface);
	GL_CALL(glEnable(GL_BLEND))
	GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
	GL_CALL(glActiveTexture(GL_TEXTURE0))
	font_shader->setUniform4f("textBackColor", br, bg, bb, ba);
	font_shader->setUniform4f("textForeColor", fr, fg, fb, fa);
	font_shader->setUniform1f("threshold", this->threshold);
	font_shader->setUniformMat4x4f("proj", GL_TRUE, glm::value_ptr(this->proj));
	return 1;
}
void FontRenderer::ft_core_draw_str_finish() {
	GL_CALL(glBindVertexArray(0))
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0))
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0))
	font_shader->unbind();
}
uint16_t FontRenderer::ft_core_draw_char(ft_char_text_t fct, uint16_t x, uint16_t y) {
	GLfloat w = (GLfloat) fct.w;
	GLfloat h = (GLfloat) fct.h;
	GLfloat xp = (GLfloat) (x + fct.bx);
	GLfloat yp = (GLfloat) (y - fct.by);
	
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
void FontRenderer::setGlyphCacheSize(uint8_t size) {
	gcache_entry_t* new_entries = (gcache_entry_t*) malloc(sizeof(gcache_entry_t) * size);
	memcpy(new_entries, entries, sizeof(gcache_entry_t) * (size < gcache_len ? size : gcache_len));
	for(uint8_t i = gcache_len; i < size; i++)
		new_entries[i] = gcache_entry_t();
	if(entries) free(entries);
	entries = new_entries;
	gcache_len = size;
}
void FontRenderer::destroy_tmp_gcache_entry() {
	if(cur) {
		for(uint8_t i = 0; i < gcache_len; i++)
			if(cur->size == entries[i].size)
				return;
		cur->unload();
		free(cur);
		cur = NULL;
	}
}
void FontRenderer::destroy_glyph_cache() {
	if(entries){
		for(uint8_t i = 0; i < gcache_len; i++)
			entries[i].unload();
		free(entries);
		entries = NULL;
	}
}
gcache_entry_t* FontRenderer::cache_glyph_entry(gcache_entry_t* cache) {
	uint8_t min_pos = 0;
	uint16_t min_uses = entries[0].uses;
	for(uint8_t i = 0; i < gcache_len; i++) {
		gcache_entry_t* cur = &entries[i];
		if(cur->is_free()) {
			memcpy(cur, cache, sizeof(gcache_entry_t));
			return cur;
		}
		if(cur->uses < min_uses) {
			min_pos = i;
			min_uses = cur->uses;
		}
	}
	if(cache->uses <= min_uses){
		gcache_entry_t* pos = &entries[min_pos];
		pos->unload();
		memcpy(pos, cache, sizeof(gcache_entry_t));
		return pos;
	}
	return 0;
}
uint16_t FontRenderer::getHeight() { return size; }
uint8_t FontRenderer::getGlyphCacheSize() { return gcache_len; }
void FontRenderer::setBackground(float r, float g, float b, float a) { br = r; bg = g; bb = b; ba = a; }
void FontRenderer::setForeground(float r, float g, float b, float a) { fr = r; fg = g; fb = b; fa = a; }
void FontRenderer::printGlyphCache() {
	printf("Cache: [\n");
	for(uint8_t i = 0; i < gcache_len; i++) {
		gcache_entry_t cur = entries[i];
		const char* str = (i >= gcache_len - 1) ? "\n]\n" : ",";
		if(cur.is_free()) printf("  %d: Free%s\n", i, str);
		else printf("  %d: Fontsize %d, used %d times%s\n", i, cur.size, cur.uses, str);
	}
}

void destroy_ft_text() {
	delete font_shader;
	glDeleteBuffers(1, &font_vbuf_id);
	glDeleteVertexArrays(1, &font_varr_id);
}
int8_t init_freetype_shaders() {
	font_shader = Shader::loadShaderFromFile("res/font.glsl");
	if(!font_shader) {
		puts("Failed to load font shader!");
		return 1;
	}
	puts("Generating font render buffers");
	uint8_t glfs = sizeof(GLfloat);
	// Make VAO
	GL_CALL_CHECK(glGenVertexArrays(1, &font_varr_id), goto err)
	GL_CALL_CHECK(glBindVertexArray(font_varr_id), goto err)
	// Make VB
	GL_CALL_CHECK(glGenBuffers(1, &font_vbuf_id), goto err)
	GL_CALL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, font_vbuf_id), goto err)
	GL_CALL_CHECK(glBufferData(GL_ARRAY_BUFFER, 24 * glfs, NULL, GL_STATIC_DRAW), goto err)	
	GL_CALL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, font_vbuf_id), goto err)
	// Unbind all
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0))
	GL_CALL(glBindVertexArray(0))
	puts("Font render buffers created!");
	return 0;
err:
	puts("Unrecoverable FreeType/OpenGL error detected");
	return 1;
}
int8_t init_ft_text() {
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
