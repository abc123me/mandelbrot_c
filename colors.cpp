#include "colors.h"

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "math.h"
#include <algorithm>

#include "util.h"

// Default gradients
const uint8_t gradient_cnt = 3;
Gradient Gradients::gradients[gradient_cnt] = {
	Gradient("Rainbow",   {Color(0), Color(255, 0, 0), Color(255, 255, 0), Color(0, 255, 0), Color(0, 255, 255), Color(0, 0, 255), Color(255, 0, 255), Color(255, 0, 0)}),
	Gradient("Heatmap",   {Color(0, 0, 0), Color(255, 0, 0), Color(255, 255, 0), Color(255, 255, 255)}),
	Gradient("Grayscale", {Color(0), Color(255)})
};

// Colors
Color::Color(float r, float g, float b) {
	this->r = floor(r * 255);
	this->g = floor(g * 255);
	this->b = floor(b * 255);
}
uint8_t Color::to3bit(){
	uint8_t o = 0;
	if(r > 127) o |= 1;
	if(g > 127) o |= 2;
	if(b > 127) o |= 4;
	return o;
};
Color color_lerp(Color a, Color cb, float perc) {
        int16_t r = (cb.r - a.r) * perc + a.r;
        int16_t g = (cb.g - a.g) * perc + a.g;
        int16_t b = (cb.b - a.b) * perc + a.b;
        return Color((uint8_t)r, (uint8_t)g, (uint8_t)b);
}

// Gradients
Gradient::Gradient(char* n, Color* dat, uint8_t len) : length(len) {
	name_str = strcpy((char*) malloc(strlen(n)), n);
	size_t size = sizeof(Color) * length;
	data = (Color*) memcpy(malloc(size), dat, size);
}
Gradient::Gradient(char* n, std::initializer_list<Color> il) : length(il.size()) {
	name_str = strcpy((char*) malloc(strlen(n)), n);
	data = (Color*) malloc(sizeof(Color) * length);
	std::copy(il.begin(), il.end(), data);
}
Gradient::~Gradient() { free(data); free(name_str); }
void Gradient::print() { puts(name_str); }
inline Color Gradient::at(float p) {
	return at(p, 0.0f, 1.0f);
}
Color Gradient::at(float val, float min, float max) {
        int pos = round(((val - min) / (max - min)) * (length - 2.0f));
        float perc = (val - min) / (max - min);
	if(actions & GRADIENT_INVERT) perc = 1 - perc;
	if(actions & GRADIENT_SQRT) perc = sqrt(perc);
        return color_lerp(data[pos], data[pos + 1], perc * length);
}




Gradient* Gradients::get_gradient(uint16_t id) {
	if(id >= gradient_cnt)
		return NULL;
	return &gradients[id];
}
void Gradients::print_gradients() {
	puts("----------- Gradients --------------");
	for(uint8_t i = 0; i < gradient_cnt; i++) {
		printf("%d: ", i);
		get_gradient(i)->print();
	}
}
Gradient* Gradients::find_gradient(char* name_or_id) {
	if(!name_or_id) return NULL;
	char* buf = (char*) alloca(strlen(name_or_id));
	name_or_id = strtrim(strcpy(buf, name_or_id));
	if(numeric(name_or_id))
		return get_gradient(atoi(name_or_id));
	return NULL;
}
