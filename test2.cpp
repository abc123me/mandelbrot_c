#include "stdio.h"
#include "util.h"

int main(int argc, char** argv) {
	cmplx_f c = atocmplx_f(argv[1]);
	c.print(); puts("");
	return 0;
}
