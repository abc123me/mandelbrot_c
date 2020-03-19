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

struct ft_char_text_t {
	GLuint id, adv;
	int16_t w, h, bx, by;
	ft_char_text_t(GLuint id, int16_t w, int16_t h, int16_t x, int16_t y, GLuint adv) :
	w(w), h(h), bx(x), by(y), id(id), adv(adv) {}
};
class FontRenderer { 
private:
	static const uint8_t glyph_cnt = 255;
	
	FT_Face ftface;
	float scale = 1.0f;
	float threshold = 0.5f;
	uint16_t size = 36;
	float br = 0.0f, bg = 0.0f, bb = 0.0f, ba = 0.0f;
	float fr = 1.0f, fg = 1.0f, fb = 1.0f, fa = 1.0f; 
	glm::mat4 proj = glm::mat4(1.0f);
	ft_char_text_t* glyphs = NULL;
	uint8_t update_glyphs = 0, ftface_valid = 0;	
	
	void load_glyphs();
	uint16_t ft_core_draw_char(ft_char_text_t c, uint16_t x, uint16_t y);
	uint8_t ft_core_draw_str_start();
	void ft_core_draw_str_finish();
public:
	FontRenderer();
	~FontRenderer();
	int8_t setFont(char* font);
	void setTextSize(uint16_t size);
	void setTextScale(float scale);
	void setBlending(uint8_t blend);
	void setProjectionMatrix(glm::mat4 proj);
	void setBackground(float r, float g, float b, float a);
	void setForeground(float r, float g, float b, float a);
	void reload();
	uint16_t drawChar(char c, uint16_t x, uint16_t y);
	uint16_t drawText(char* str, uint16_t x, uint16_t y);
};

int8_t init_ft_text();
void destroy_ft_text();
#endif
