#include <common.h>

static u32 CollFixed_PackS16Pair(s32 lo, s32 hi)
{
	return (u16)lo | ((u32)(u16)hi << 16);
}

static s32 CollFixed_Sll32(s32 value, u32 shift)
{
	return (s32)((u32)value << (shift & 0x1f));
}

static s32 CollFixed_MulLo(s32 a, s32 b)
{
	return (s32)(u32)((s64)a * (s64)b);
}

static s16 CollFixed_ReadS16(const void *base, int offset)
{
	return *(const s16 *)((const u8 *)base + offset);
}

static u16 CollFixed_ReadU16(const void *base, int offset)
{
	return *(const u16 *)((const u8 *)base + offset);
}

static s32 CollFixed_ReadS32(const void *base, int offset)
{
	return *(const s32 *)((const u8 *)base + offset);
}

static u32 CollFixed_ReadU32(const void *base, int offset)
{
	return *(const u32 *)((const u8 *)base + offset);
}

static void CollFixed_WriteS16(void *base, int offset, s32 value)
{
	*(s16 *)((u8 *)base + offset) = (s16)value;
}

static void CollFixed_WriteS32(void *base, int offset, s32 value)
{
	*(s32 *)((u8 *)base + offset) = value;
}

static void CollFixed_WriteU32(void *base, int offset, u32 value)
{
	*(u32 *)((u8 *)base + offset) = value;
}

static void CollFixed_GteLoadR11R12(u32 value)
{
	CTC2(value, 0);
}

static void CollFixed_GteLoadR13R21(u32 value)
{
	CTC2(value, 1);
}

static void CollFixed_GteLoadR22R23(u32 value)
{
	CTC2(value, 2);
}

static void CollFixed_GteLoadR33(u32 value)
{
	CTC2(value, 4);
}

static void CollFixed_GteLoadVXY0(u32 value)
{
	MTC2(value, 0);
}

static void CollFixed_GteLoadVZ0(s32 value)
{
	MTC2_S(value, 1);
}

static void CollFixed_GteLoadIR0(s32 value)
{
	MTC2_S(value, 8);
}

static void CollFixed_GteLoadIR(s32 x, s32 y, s32 z)
{
	MTC2_S(x, 9);
	MTC2_S(y, 10);
	MTC2_S(z, 11);
}

static void CollFixed_GteLoadMAC(s32 x, s32 y, s32 z)
{
	MTC2_S(x, 25);
	MTC2_S(y, 26);
	MTC2_S(z, 27);
}

static s32 CollFixed_GteReadMAC1(void)
{
	return MFC2_S(25);
}

static s32 CollFixed_GteReadMAC2(void)
{
	return MFC2_S(26);
}

static s32 CollFixed_GteReadMAC3(void)
{
	return MFC2_S(27);
}

static void CollFixed_GteLoadLZCS(s32 value)
{
	MTC2_S(value, 30);
}

static s32 CollFixed_GteReadLZCR(void)
{
	return MFC2_S(31);
}

static void CollFixed_GteMVMVA(void)
{
	doCOP2(0x0406012);
}

static void CollFixed_GteGPL12(void)
{
	doCOP2(0x01a8003e);
}

static void CollFixed_GteOP0(void)
{
	doCOP2(0x0170000c);
}

static void CollFixed_GteRTIR(void)
{
	doCOP2(0x049e012);
}

void COLL_FIXED_TRIANGL_Barycentrics(s16 *out, s16 *v1, s16 *v2, s16 *point)
{
	s32 v1x = v1[0];
	s32 v1y = v1[1];
	s32 v1z = v1[2];
	s32 edgeX = v2[0] - v1x;
	s32 edgeY = v2[1] - v1y;
	s32 edgeZ = v2[2] - v1z;
	s32 pointX = point[0] - v1x;
	s32 pointY = point[1] - v1y;
	s32 pointZ = point[2] - v1z;
	s32 pointDot;
	s32 edgeDot;
	s32 shift;
	s32 factor;

	CollFixed_GteLoadR11R12(CollFixed_PackS16Pair(edgeX, edgeY));
	CollFixed_GteLoadR13R21(CollFixed_PackS16Pair(edgeZ, pointX));
	CollFixed_GteLoadR22R23(CollFixed_PackS16Pair(pointY, pointZ));
	CollFixed_GteLoadVXY0(CollFixed_PackS16Pair(edgeX, edgeY));
	CollFixed_GteLoadVZ0(edgeZ);

	CollFixed_GteMVMVA();
	pointDot = CollFixed_GteReadMAC2();
	edgeDot = CollFixed_GteReadMAC1();

	CollFixed_GteLoadLZCS(pointDot);
	shift = CollFixed_GteReadLZCR() - 2;

	if (shift < 0)
	{
		shift = 0;
	}
	else if (shift > 12)
	{
		shift = 12;
	}

	pointDot = CollFixed_Sll32(pointDot, shift);

	if (shift < 12)
	{
		edgeDot >>= 12 - shift;
	}

	factor = 0;

	if (edgeDot != 0)
	{
		factor = pointDot / edgeDot;

		if (factor < 0)
		{
			factor = 0;
		}
		else if (factor > 0x1000)
		{
			factor = 0x1000;
		}
	}

	CollFixed_GteLoadIR0(factor);
	CollFixed_GteLoadMAC(v1x, v1y, v1z);
	CollFixed_GteLoadIR(edgeX, edgeY, edgeZ);
	CollFixed_GteGPL12();

	out[0] = (s16)CollFixed_GteReadMAC1();
	out[1] = (s16)CollFixed_GteReadMAC2();
	out[2] = (s16)CollFixed_GteReadMAC3();
}
