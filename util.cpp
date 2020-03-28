#include "util.h"

#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "pthread.h"
#include "math.h"

// Mandelbrot rendering
#define _RENDER_MB_BUF_ST_N(NAME, CODE) void NAME(mb_opts opts, uint16_t* buf) {\
	for(uint16_t x = opts.sx; x < opts.ex; x++) {\
		float a = map<float>(x, 0, opts.w, opts.min_r, opts.max_r);\
		for(uint16_t y = opts.sy; y < opts.ey; y++) {\
			float b = map<float>(y, 0, opts.h, opts.min_i, opts.max_i);\
			CODE;\
		}\
	}\
}
_RENDER_MB_BUF_ST_N(_render_mb_buf_st_m, buf[x + y * opts.w] = count_mb_iter(a, b, a, b, opts.max_iter))
_RENDER_MB_BUF_ST_N(_render_mb_buf_st_j, buf[x + y * opts.w] = count_mb_iter(a, b, opts.c_real, opts.c_imag, opts.max_iter))
#undef _RENDER_MB_BUF_ST_N
#include "time.h"
void render_mb_buf_st(mb_opts opts, uint16_t* buf) {
	if(opts.julia) _render_mb_buf_st_j(opts, buf);
	else _render_mb_buf_st_m(opts, buf);
}
void* thr_render_mb_buf(void* dat) { //mb_opts opts, uint16_t* buf
	mb_opts o = *((mb_opts*) dat);
	dat += sizeof(mb_opts);
	uint16_t* b = *((uint16_t**) dat);
	render_mb_buf_st(o, b);
	return NULL;
}
void render_mb_buf(mb_opts opts, uint16_t thrs, uint16_t* buf) {
	if(thrs > 1) {
		pthread_t* thr = (pthread_t*) malloc(sizeof(pthread_t) * thrs);
		mb_opts* nopts = (mb_opts*) malloc(sizeof(mb_opts) * thrs);
		void** dat = (void**) malloc(thrs * sizeof(void*));
		opts.splitup_job(thrs, nopts);
		for(uint8_t i = 0; i < thrs; i++) {
			dat[i] = malloc(sizeof(mb_opts) + sizeof(uint16_t*));
			void* tmp = dat[i];
			memcpy(tmp, &nopts[i], sizeof(mb_opts));
			tmp += sizeof(mb_opts);
			memcpy(tmp, &buf, sizeof(uint16_t*));
			pthread_create(&thr[i], NULL, thr_render_mb_buf, dat[i]);
		}
		for(uint8_t i = 0; i < thrs; i++) {
			pthread_join(thr[i], NULL);
			free(dat[i]);
		}
		free(dat); free(thr); free(nopts);
	} else render_mb_buf_st(opts, buf);
}

uint16_t count_mb_iter(float a, float b, float ca, float cb, uint16_t max) {
	float a2, b2, c;
	for(uint16_t i = 0; i < max; i++){
		a2 = a * a;
		b2 = b * b;
		c = 2 * a * b;
		a = a2 - b2 + ca;
		b = c + cb;
		if(a + b > 16)
			return i;
	}
	return max;
}


// Math stuff
uint16_t _atocmplx_f_rm_ws(char* s, char* tmp) {
	uint16_t j = 0;
	for(uint16_t i = 0; i < strlen(s); i++)
		if(is_numf(s[i]) || s[i] == 'i')
			tmp[j++] = s[i];
	tmp[j] = 0; return j;
}
void _atocmplx_f(char* a, char* rstr, char* istr) {
	if(!a[0]) return;
	a = strtrim(a);
	uint16_t len = strlen(a);
	uint16_t i;
	uint8_t stop = 0;
	for(i = len - 1; i > 0; i--)
		if((a[i] == '-' || a[i] == '+') && (i < 1 || a[i - 1] != 'e'))
			break;
	char* tmp = (char*) alloca(len);
	len = _atocmplx_f_rm_ws(a, tmp);
	if(tmp[len - 1] == 'i') {
		strcpy(istr, tmp + i);
		if(strcmp(istr, "i") == 0 || strcmp(istr, "-i") == 0 || strcmp(istr, "+i") == 0)
			istr[strlen(istr) - 1] = '1';
	} else strcpy(rstr, tmp + i);
	a[i] = 0;
}
cmplx_f atocmplx_f(char* a) {
	a = lower(a);
	uint16_t len = strlen(a);
	char* rstr = (char*) alloca(len); strcpy(rstr, "0");
	char* istr = (char*) alloca(len); strcpy(istr, "0");
	_atocmplx_f(a, rstr, istr);
	_atocmplx_f(a, rstr, istr);
	return cmplx_f(atof(rstr), atof(istr));
}
void cmplx_f::print() {
	if(imag < 0) printf("%.3f-%.3fi", real, -imag);
	else printf("%.3f+%.3fi", real, imag);
}


// String stuff
uint8_t numeric(char* str) {
	for(uint16_t i = 0; i < strlen(str); i++)
		if(!is_num(str[i])) return 0;
	return 1;
}
char* upper(char* in) {
	for(uint16_t i = 0; i < strlen(in); i++)
		if(in[i] >= 'a' && in[i] <= 'z')
			in[i] ^= 32;
	return in;
}
char* lower(char* in) {
	for(uint16_t i = 0; i < strlen(in); i++)
		if(in[i] >= 'A' && in[i] <= 'Z')
			in[i] |= 32;
	return in;
}
int32_t indexof(char* str, char of) {
	for(uint16_t i = 0; i < strlen(str); i++)
		if(str[i] == of) return i;
	return -1;
}
int32_t indexofany(char* str, char* of) {
	uint16_t of_len = strlen(of), str_len = strlen(str);
	for(uint16_t i = 0; i < str_len; i++)
		for(uint16_t j = 0; j < of_len; j++)
			if(str[i] == of[j])
				return i;
	return -1;
}
char* strtrim(char* str) {
	char c = *str;
	while(c && is_ws(c))
		c = *(++str);
	uint16_t len = strlen(str);
	if(!len) return str;
	char* end = str + len - 1;
	while(is_ws(*end))
		end--;
	*++end = 0;
	return str;
}
uint8_t str_startswith(char* str, char* with) {
	uint16_t len = strlen(with);
	if(len > strlen(str))
		return 0;
	return strncmp(str, with, len) == 0;
}
uint8_t compare_arg(char* str, char* with, char* shorthand) {
	uint16_t len = strlen(str);
	if(len < (shorthand ? strlen(shorthand) : strlen(with)))
		return 0;
	return (strcmp(str, with) == 0) || (shorthand && (strcmp(str, shorthand) == 0));
}
char* malloc_str(uint32_t len, char c) {
	char* out = (char*) malloc(len + 1);
	for(uint32_t i = 0; i < len; i++)
		out[i] = c;
	out[len] = 0;
	return out;
}


// struct mb_opts
void mb_opts::print() {
	printf("%s job @ %.3f+%.3fi w/ %d iterations\n", julia ? "Julia set" : "Mandelbrot", c_real, c_imag, max_iter);
	printf("   %d, %d -> %d, %d (%dx%d) [%dx%d]\n", sx, sy, ex, ey, ex - sx, ey - sy, w, h);
}
mb_opts mb_opts::clone() {
	mb_opts o;
	o.max_iter = max_iter;
	o.c_real = c_real;
	o.c_imag = c_imag;
	o.sx = sx; o.sy = sy;
	o.ex = ex; o.ey = ey;
	o.min_r = min_r;
	o.min_i = min_i;
	o.max_r = max_r;
	o.max_i = max_i;
	o.w = w; o.h = h;
	o.julia = julia;
	return o;
}
mb_opts new_empty_job(uint16_t w, uint16_t h, uint16_t max_iter, cmplx_f min, cmplx_f max) {
	mb_opts o;
	o.min_r = min.real;
	o.min_i = min.imag;
	o.max_r = max.real;
	o.max_i = max.imag;
	o.max_iter = max_iter;
	o.c_real = 0; o.c_imag = 0;
	o.sx = 0; o.sy = 0;
	o.ex = w; o.ey = h;
	o.w = w; o.h = h;
	o.julia = 0;
	return o;
}
mb_opts new_mandelbrot_job(uint16_t w, uint16_t h, uint16_t max_iter, cmplx_f min, cmplx_f max) {
	return new_empty_job(w, h, max_iter, min, max);
}
mb_opts new_julia_job(uint16_t w, uint16_t h, cmplx_f c, uint16_t max_iter, cmplx_f min, cmplx_f max) {
	mb_opts o = new_empty_job(w, h, max_iter, min, max);
	o.c_real = c.real;
	o.c_imag = c.imag;
	o.julia = 1;
	return o;
}
mb_opts mb_opts::new_job(uint16_t dw, uint16_t dh, uint16_t sx, uint16_t sy) {
	mb_opts o = clone();
	o.sx = sx; o.sy = sy;
	o.ex = sx + dw;
	o.ey = sy + dh;
	return o;
}
void mb_opts::splitup_job(uint16_t by, mb_opts* buf) {
	uint16_t ind_w = w, ind_h = h;
	uint16_t dlt_w = 0, dlt_h = 0;
	uint16_t rem_w = 0, rem_h = 0;
	if(w > h) { ind_w = w / by; dlt_w = ind_w; rem_w = w % by; }
	else {      ind_h = h / by; dlt_h = ind_h; rem_h = h % by; }
	uint16_t px = sx, py = sy;
	for(uint16_t i = 0; i < by; i++) {
		buf[i] = new_job(ind_w, ind_h, px, py);
		px += dlt_w; py += dlt_h;
	}
	buf[by - 1].ex += rem_w;
	buf[by - 1].ey += rem_h;
}

// File IO
char* readFile(FILE* fp){
	uint64_t size = fileSize(fp);
	char* dat = new char[size + 1];
	readFileInto(fp, dat);
	dat[size] = 0x00;
	return dat;
}
void readFileInto(FILE* fp, char* dat){
	uint64_t size = fileSize(fp);
	fseek(fp, 0L, SEEK_SET);
	fread(dat, 1, size, fp);
}
uint64_t fileSize(FILE* fp){
	fseek(fp, 0L, SEEK_SET);
	fseek(fp, 0L, SEEK_END);
	uint64_t fileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return fileSize;
}

// Miscellaneous
#ifdef __linux__
#include "unistd.h"
uint16_t get_threads() { return sysconf(_SC_NPROCESSORS_ONLN);}
#else
#pragma warn OS Not supported, get_threads() wont work properly!
uint16_t get_threads() { return 2; }
#endif
void rotateArray(void* array, unsigned long int size, int by){
	void* newArray = alloca(size);
	for(unsigned int i = 0; i < size; i++){
		long int k = i + by;
		if(k >= size)
			k -= size;
		if(k < 0)
			k += size;
		((uint8_t*)newArray)[i] = ((uint8_t*)array)[k];
	}
	memcpy(array, newArray, size);
}
uint16_t cmplx_f::tostrn(char* s, uint16_t l) {
	if(imag == 0) return snprintf(s, l, "%f", real);
	else if(real == 0) return snprintf(s, l, "%fi", imag);
	else return snprintf(s, l, "%f%s%fi", real, imag > 0 ? "+" : "", imag); 
}
