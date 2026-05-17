#include <common.h>

void MatrixRotate(MATRIX *dst, MATRIX *src, MATRIX *rot)
{
	MulMatrix0(rot, src, dst);
}
