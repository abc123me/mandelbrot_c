#ifndef _FT_TEXT_H
#define _FT_TEXT_H

// Standard
#include "stdint.h"
// OpenGL
#include "GL/glew.h"
#include "GLFW/glfw3.h"
// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H
// GLM
#include "glm/glm.hpp"

#define CHARSET_SIZE 255
#define FR_CMASK_UPDATE_GLYPHS 1
#define FR_CMASK_FTFACE_VALID 2

struct ft_char_text_t {
	GLuint id, adv;
	int16_t w, h, bx, by;
	ft_char_text_t(GLuint id, int16_t w, int16_t h, int16_t x, int16_t y, GLuint adv) :
	w(w), h(h), bx(x), by(y), id(id), adv(adv) {}
};
struct gcache_entry_t {
	ft_char_text_t* glyphs;
	uint16_t uses, size;
	
	gcache_entry_t();
	gcache_entry_t(uint16_t size);
	void load(FT_Face ftface);
	void unload();
	uint8_t is_free();
	uint8_t needs_loaded();
};
class FontRenderer { 
private:
	FT_Face ftface;
	float threshold = 0.5f;
	uint16_t size = 36;
	uint8_t tabWidth = 4;
	float br = 0.0f, bg = 0.0f, bb = 0.0f, ba = 0.0f;
	float fr = 1.0f, fg = 1.0f, fb = 1.0f, fa = 1.0f; 
	glm::mat4 proj = glm::mat4(1.0f);
	gcache_entry_t* entries = NULL;
	gcache_entry_t* cur = NULL;
	uint8_t gcache_len = 0;
	//Control mask
	// Bit 2: FTFace valid
	uint8_t cmask = 0;
	
	void load_glyphs();
	uint16_t ft_core_draw_char(ft_char_text_t c, uint16_t x, uint16_t y);
	uint8_t ft_core_draw_str_start();
	void ft_core_draw_str_finish();
	gcache_entry_t* cache_glyph_entry(gcache_entry_t* cache);
	void destroy_glyph_cache();
	void destroy_tmp_gcache_entry();
public:
	FontRenderer();
	~FontRenderer();
	int8_t setFont(char* font);
	
	void setTextSize(uint16_t size);
	void setBlending(uint8_t blend);
	void setProjectionMatrix(glm::mat4 proj);
	void setBackground(float r, float g, float b, float a);
	void setForeground(float r, float g, float b, float a);
	uint16_t drawChar(char c, uint16_t x, uint16_t y);
	uint16_t drawText(char* str, uint16_t x, uint16_t y);
	uint16_t getHeight();
	uint8_t getTabSpaces();
	void setTabSpaces(uint8_t cnt);
	void setGlyphCacheSize(uint8_t size);
	uint8_t getGlyphCacheSize();
	void printGlyphCache();
};

int8_t init_ft_text();
void destroy_ft_text();
#endif
