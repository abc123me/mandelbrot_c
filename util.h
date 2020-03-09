#ifndef _UTIL_H
#define _UTIL_H

#include "stdint.h"
#include "stdio.h"

struct mb_opts {
        uint16_t max_iter = 255;
        float c_real = 0, c_imag = 0;
	uint16_t w = 0,  h = 0;
	uint16_t sx = 0, sy = 0;
	uint16_t ex = 0, ey = 0;
	uint8_t julia = 0;
	float max_i, max_r;
	float min_i, min_r;

	void splitup_job(uint16_t by, mb_opts* buf);
	mb_opts new_job(uint16_t nw, uint16_t nh, uint16_t nsx, uint16_t nsy);
	mb_opts clone();
	void print();
};
struct cmplx_f {
	float real, imag;
	cmplx_f(float r, float i) : real(r), imag(i) {}
	void print();
};

template <typename T> inline T map(T val, T min, T max, T nmin, T nmax);

void render_mb_buf(mb_opts opts, uint16_t threads, uint16_t* buf);
uint16_t count_mb_iter(float a, float b, float ca, float cb, uint16_t max);
mb_opts new_mandelbrot_job(uint16_t w, uint16_t h, uint16_t i, cmplx_f min, cmplx_f max);
mb_opts new_julia_job(uint16_t w, uint16_t h, cmplx_f c, uint16_t i, cmplx_f min, cmplx_f max);

uint16_t get_threads();

#define is_ws(c) (c <= ' ' || c > '~')
#define is_num(c) (c >= '0' && c <= '9')
#define is_numf(c) (is_num(c) || c == '.' || c == '+' || c == '-' || c == 'e' || c == 'E')

uint8_t str_startswith(char* str, char* with);
uint8_t compare_arg(char* arg, char* lng, char* sht);
uint8_t numeric(char* str);
char* malloc_str(uint32_t len, char c);
char* strtrim(char* str);
int32_t indexof(char* str, char of);
int32_t indexofany(char* str, char* anyof);
char* upper(char* s);
char* lower(char* s);
cmplx_f atocmplx_f(char* a);

uint64_t fileSize(FILE* fp);
char* readFile(FILE* fp);
void readFileInto(FILE* fp, char* buf);

template<typename T> T wrap(T val, T min, T max);
void rotateArray(void* array, unsigned long int arraySize, int by);

#endif
