#include <common.h>

void COLL_FIXED_TRIANGL_GetNormVec(void *sps, void *v1, void *v2, void *v3)
{
	u8 *spsBytes = (u8 *)sps;
	u8 *v1Bytes = (u8 *)v1;
	s32 v1x = CollFixed_ReadS16(v1Bytes, 0);
	s32 v1y = CollFixed_ReadS16(v1Bytes, 2);
	s32 v1z = CollFixed_ReadS16(v1Bytes, 4);
	s32 edgeAX = CollFixed_ReadS16(v3, 0) - v1x;
	s32 edgeAY = CollFixed_ReadS16(v3, 2) - v1y;
	s32 edgeAZ = CollFixed_ReadS16(v3, 4) - v1z;
	s32 edgeBX = CollFixed_ReadS16(v2, 0) - v1x;
	s32 edgeBY = CollFixed_ReadS16(v2, 2) - v1y;
	s32 edgeBZ = CollFixed_ReadS16(v2, 4) - v1z;
	u32 lodShift;
	u32 normalShift;
	s32 scale;
	s32 nx;
	s32 ny;
	s32 nz;
	s32 plane;
	s32 absX;
	s32 absY;
	s32 absZ;
	s16 dominantAxis;

	CollFixed_GteLoadR11R12((u16)edgeAX);
	CollFixed_GteLoadR22R23((u16)edgeAY);
	CollFixed_GteLoadR33((u16)edgeAZ);
	CollFixed_GteLoadIR(edgeBX, edgeBY, edgeBZ);

	lodShift = *(u8 *)(spsBytes + 0x1ab);
	normalShift = *(u8 *)(spsBytes + 0x1aa);
	scale = CollFixed_ReadS16(spsBytes, 0x1a8);

	CollFixed_GteOP0();

	nx = CollFixed_GteReadMAC1();
	ny = CollFixed_GteReadMAC2();
	nz = CollFixed_GteReadMAC3();

	CollFixed_GteLoadR11R12(CollFixed_ReadU32(v1Bytes, 0));
	CollFixed_GteLoadR13R21(CollFixed_ReadU16(v1Bytes, 4));

	nx = CollFixed_MulLo(nx >> (lodShift & 0x1f), scale) >> (normalShift & 0x1f);
	ny = CollFixed_MulLo(ny >> (lodShift & 0x1f), scale) >> (normalShift & 0x1f);
	nz = CollFixed_MulLo(nz >> (lodShift & 0x1f), scale) >> (normalShift & 0x1f);

	CollFixed_GteLoadIR(nx, ny, nz);
	CollFixed_WriteS16(v1Bytes, 0xc, nx);
	CollFixed_WriteS16(v1Bytes, 0xe, ny);

	CollFixed_GteRTIR();
	plane = CollFixed_GteReadMAC1();

	CollFixed_WriteS16(v1Bytes, 0x10, nz);
	CollFixed_WriteS16(v1Bytes, 0x12, plane >> 1);

	absX = nx < 0 ? -nx : nx;
	absY = ny < 0 ? -ny : ny;
	absZ = nz < 0 ? -nz : nz;
	dominantAxis = 1;

	if ((absX - absY) < 0)
	{
		if ((absY - absZ) >= 0)
		{
			dominantAxis = 3;
		}
	}
	else if ((absX - absZ) >= 0)
	{
		dominantAxis = 2;
	}

	CollFixed_WriteS16(v1Bytes, 6, dominantAxis);
}
