#ifndef _FT_TEXT_H
#define _FT_TEXT_H

struct ft_char_text_t {
	GLuint id, adv;
	int16_t w, h, bx, by;
	ft_char_text_t(GLuint id, int16_t w, int16_t h, int16_t x, int16_t y, GLuint adv) :
	w(w), h(h), bx(x), by(y), id(id), adv(adv) {}
};
class FontRenderer { 
private:
	static Shader* font_shader = NULL;
	static GLuint font_varr_id = 0;
	static GLuint font_vbuf_id = 0;
	static FT_Library ftlib;
	
	FT_Face ftface;
	float scale;
	uint16_t size;
	ft_char_text_t* glyphs = NULL;
	uint8_t update_glyphs = 0;	
	
	void load_glyphs();
	uint16_t ft_core_draw_char(ft_char_text_t c, uint16_t x, uint16_t y);
	uint8_t ft_core_draw_str_start();
	void ft_core_draw_str_finish();
public:
	FontRenderer();
	~FontRenderer();
	void setFont(char* font);
	void setTextSize(uint16_t size);
	void setTextScale(float scale);
	uint16_t drawChar(char c, uint16_t x, uint16_t y);
	uint16_t drawText(char* str, uint16_t x, uint16_t y);
}

int8_t init_freetype();

#endif
