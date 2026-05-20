#include <common.h>

u32 DECOMP_UI_VsQuipReadDriver(struct Driver *d, int offset, int size)
{
	char *byteArr = (char *)d;
	byteArr += offset;

	if (size == 1)
		return *(char *)byteArr;
	if (size == 2)
		return *(s16 *)byteArr;
	/*if (size == 4)*/ return *(int *)byteArr;
}
