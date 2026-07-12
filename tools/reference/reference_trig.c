#include <stdio.h>
#include <stdlib.h>

#include "game/MATH/MATH_0_Sin.c"
#include "game/MATH/MATH_1_Cos.c"
#include "game/zGlobal_DATA.c"
#include <common.h>

int main(int argc, char **argv)
{
	const char *dir = argc > 1 ? argv[1] : ".";
	char path[512];
	snprintf(path, sizeof(path), "%s/trig.csv", dir);
	FILE *f = fopen(path, "wb");
	if (f == NULL)
	{
		fprintf(stderr, "reference_trig: cannot open %s\n", path);
		return 1;
	}

	fprintf(f, "angle,sin,cos\n");
	for (u32 angle = 0; angle < 0x1000; angle++)
	{
		fprintf(f, "%u,%d,%d\n", angle, MATH_Sin(angle), MATH_Cos(angle));
	}
	fclose(f);
	printf("reference_trig: wrote %s (4096 angles)\n", path);
	return 0;
}
