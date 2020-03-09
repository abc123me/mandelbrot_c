#ifndef _GRADIENTS_H
#define _GRADIENTS_H

#include "stdint.h"
#include <initializer_list>

struct Color {
        uint8_t r, g, b;
	Color() : r(0), g(0), b(0) {}
        Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
        Color(int r, int g, int b) : r(r), g(g), b(b) {}
        Color(float r, float g, float b);
        Color(uint8_t w) : r(w), g(w), b(w) {}
        uint8_t to3bit();
};
#define GRADIENT_SQRT  1
#define GRADIENT_INVERT  2
class Gradient {
private:
	uint8_t length;
	Color* data;
	char* name_str;
public:
	uint8_t actions = 0;
	char* name() { return name_str; }
	uint8_t len() { return length; }
	Color at(float val, float min, float max);
	Color at(float p);
	Gradient(char* n, Color* dat, uint8_t len);
	Gradient(char* n, std::initializer_list<Color> il);
	~Gradient();
	void print();
	void setAction(uint8_t action) { actions |= action; }
	void unsetAction(uint8_t action) { actions &= ~action; }
};
namespace Gradients {
	void print_gradients();
	Gradient* get_gradient(uint16_t gid);
	Gradient* find_gradient(char* name_or_id);
	extern Gradient gradients[];
}
Color color_lerp(Color a, Color b, float perc);

#endif
