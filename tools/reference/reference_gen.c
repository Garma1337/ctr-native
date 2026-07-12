#include <stdio.h>
#include <stdlib.h>

#include "psx/inline_c.h"
#include "psx/libapi.h"
#include "psx/libcd.h"
#include "psx/libetc.h"
#include "psx/libgpu.h"
#include "psx/libgte.h"
#include "psx/libspu.h"
#include "psx/strings.h"
#define __attribute__(x)
#define RECT RECT16

#include <common.h>

struct sData sdata_static;

#include "game/Vehicle/VehCalc.c"
#include "game/Vehicle/VehPhysJoystick.c"
#include "game/MixRNG/MixRNG.c"
#include "game/MixRNG/RngDeadCoed.c"

static const s32 kValues[] = {0, 1, -1, 2, -2, 7, -8, 255, 256, -256, 0x10000, 0x7FFFFFFF, (s32)0x80000000, -123456789, 123456789, 0x55555555, (s32)0xAAAAAAAA};
static const u32 kShifts[] = {0, 1, 2, 3, 4, 12, 16, 31, 32};

#define NUM_VALUES ((int)(sizeof(kValues) / sizeof(kValues[0])))
#define NUM_SHIFTS ((int)(sizeof(kShifts) / sizeof(kShifts[0])))

static FILE *OpenCsv(const char *dir, const char *name, const char *header)
{
	char path[512];
	snprintf(path, sizeof(path), "%s/%s", dir, name);
	FILE *file = fopen(path, "wb");
	if (file == NULL)
	{
		fprintf(stderr, "reference_gen: cannot open %s\n", path);
		exit(1);
	}
	fprintf(file, "%s\n", header);
	return file;
}

static void GenMips(const char *dir)
{
	FILE *f = OpenCsv(dir, "mips.csv", "op,a,b,result");
	for (int i = 0; i < NUM_VALUES; i++)
	{
		s32 a = kValues[i];
		fprintf(f, "NegLo,%d,0,%d\n", a, CTR_MipsNegLo(a));
		for (int s = 0; s < NUM_SHIFTS; s++)
		{
			u32 sh = kShifts[s];
			fprintf(f, "Sll,%d,%d,%d\n", a, (int)sh, CTR_MipsSll(a, sh));
			fprintf(f, "Sra,%d,%d,%d\n", a, (int)sh, CTR_MipsSra(a, sh));
			fprintf(f, "Srl,%d,%d,%d\n", a, (int)sh, (s32)CTR_MipsSrl(a, sh));
		}
		for (int j = 0; j < NUM_VALUES; j++)
		{
			s32 b = kValues[j];
			fprintf(f, "MulLo,%d,%d,%d\n", a, b, CTR_MipsMulLo(a, b));
			fprintf(f, "AddLo,%d,%d,%d\n", a, b, CTR_MipsAddLo(a, b));
			fprintf(f, "SubLo,%d,%d,%d\n", a, b, CTR_MipsSubLo(a, b));
			fprintf(f, "PackS16Pair,%d,%d,%d\n", a, b, (s32)CTR_PackS16Pair(a, b));
		}
	}
	fclose(f);
}

static void GenInterpBySpeed(const char *dir)
{
	static const int vals[] = {-100, -1, 0, 1, 5, 10, 100, 1000};
	static const int speeds[] = {0, 1, 3, 7, 50};
	static const int desired[] = {-50, 0, 5, 10, 100};
	FILE *f = OpenCsv(dir, "interpbyspeed.csv", "val,speed,desired,result");
	for (int i = 0; i < (int)(sizeof(vals) / sizeof(vals[0])); i++)
		for (int s = 0; s < (int)(sizeof(speeds) / sizeof(speeds[0])); s++)
			for (int d = 0; d < (int)(sizeof(desired) / sizeof(desired[0])); d++)
				fprintf(f, "%d,%d,%d,%d\n", vals[i], speeds[s], desired[d], VehCalc_InterpBySpeed(vals[i], speeds[s], desired[d]));
	fclose(f);
}

static void GenMapToRange(const char *dir)
{
	static const int tuples[][4] = {{0, 10, 0, 100}, {0, 4, 0x800, 0xC00}, {12, 64, 0xC00, 0}, {-10, 10, -100, 100}};
	FILE *f = OpenCsv(dir, "maptorange.csv", "val,oldMin,oldMax,newMin,newMax,result");
	for (int t = 0; t < (int)(sizeof(tuples) / sizeof(tuples[0])); t++)
	{
		int oldMin = tuples[t][0], oldMax = tuples[t][1], newMin = tuples[t][2], newMax = tuples[t][3];
		for (int val = oldMin - 2; val <= oldMax + 2; val++)
			fprintf(f, "%d,%d,%d,%d,%d,%d\n", val, oldMin, oldMax, newMin, newMax, VehCalc_MapToRange(val, oldMin, oldMax, newMin, newMax));
	}
	fclose(f);
}

static void GenSteerAccel(const char *dir)
{
	FILE *f = OpenCsv(dir, "steeraccel.csv", "frame,stage2First,stage2Len,stage4First,minSteer,maxSteer,result");
	for (int frame = 0; frame <= 70; frame++)
		fprintf(f, "%d,4,8,64,2048,3072,%d\n", frame, VehCalc_SteerAccel(frame, 4, 8, 64, 0x800, 0xC00));
	fclose(f);
}

static void GenFastSqrt(const char *dir)
{
	static const u32 ns[] = {0, 1, 2, 3, 4, 9, 15, 16, 100, 255, 256, 1000, 65535, 65536, 0x10000000u, 0x7FFFFFFFu, 0xFFFFFFFFu};
	static const u32 shifts[] = {0, 2, 4, 8, 12, 16};
	FILE *f = OpenCsv(dir, "fastsqrt.csv", "n,shift,result");
	for (int i = 0; i < (int)(sizeof(ns) / sizeof(ns[0])); i++)
		for (int s = 0; s < (int)(sizeof(shifts) / sizeof(shifts[0])); s++)
			fprintf(f, "%d,%d,%d\n", (s32)ns[i], (s32)shifts[s], (s32)VehCalc_FastSqrt(ns[i], shifts[s]));
	fclose(f);
}

static void GenStick(const char *dir)
{
	FILE *f = OpenCsv(dir, "joystick.csv", "fn,stickVal,arg,result");
	for (int v = 0; v <= 255; v++)
	{
		fprintf(f, "ReturnToRest,%d,128,%d\n", v, VehPhysJoystick_ReturnToRest(v, 0x80, NULL));
		fprintf(f, "GetStrength,%d,64,%d\n", v, VehPhysJoystick_GetStrength(v, 0x40, NULL));
		fprintf(f, "GetStrengthAbsolute,%d,64,%d\n", v, VehPhysJoystick_GetStrengthAbsolute(v, 0x40, NULL));
	}
	fclose(f);
}

static void GenRng(const char *dir)
{
	static const u32 seeds[] = {0, 1, 2, 0x1234, 0x3619, 0x6255, 0xABCD, 0xFFFF, 0x8000, 12345, 0x1D2C, 54321};
	FILE *f = OpenCsv(dir, "rng.csv", "seed,step,value");
	for (int i = 0; i < (int)(sizeof(seeds) / sizeof(seeds[0])); i++)
	{
		sdata->randomNumber = (s32)seeds[i];
		for (int step = 1; step <= 16; step++)
			fprintf(f, "%d,%d,%d\n", (s32)seeds[i], step, MixRNG_Scramble());
	}
	fclose(f);
}

int main(int argc, char **argv)
{
	const char *dir = argc > 1 ? argv[1] : ".";
	GenMips(dir);
	GenInterpBySpeed(dir);
	GenMapToRange(dir);
	GenSteerAccel(dir);
	GenFastSqrt(dir);
	GenStick(dir);
	GenRng(dir);
	printf("reference_gen: wrote mips/interpbyspeed/maptorange/steeraccel/fastsqrt/joystick/rng CSVs to %s\n", dir);
	return 0;
}
