#include <common.h>

static void COLL_FIXED_TRIANGL_TestPoint_Body(u8 *sps, u8 *v1, u8 *v2, u8 *v3, s32 normalZW)
{
	s32 startX;
	s32 startY;
	s32 startZ;
	s32 deltaX;
	s32 deltaY;
	s32 deltaZ;
	s32 planeDot;
	s32 lineDot;
	s32 factor;
	s32 hitX;
	s32 hitY;
	s32 hitZ;
	s32 normalAxis;
	s32 firstA;
	s32 firstB;
	s32 firstHit;
	s32 secondA;
	s32 secondB;
	s32 secondHit;
	s32 denom;
	s32 baryA;
	s32 baryB;
	u8 *baryV2;
	u8 *baryV3;
	struct QuadBlock *quad;

	startX = CollFixed_ReadS16(sps, 0x10);
	startY = CollFixed_ReadS16(sps, 0x12);
	startZ = CollFixed_ReadS16(sps, 0x14);
	deltaX = CollFixed_ReadS16(sps, 0x1c) - startX;
	deltaY = CollFixed_ReadS16(sps, 0x1e) - startY;
	deltaZ = CollFixed_ReadS16(sps, 0x20) - startZ;

	CollFixed_GteLoadR11R12(CollFixed_ReadU32(sps, 0x10));
	CollFixed_GteLoadR13R21(CollFixed_PackS16Pair(startZ, deltaX));
	CollFixed_GteLoadR22R23(CollFixed_PackS16Pair(deltaY, deltaZ));
	CollFixed_GteLoadVXY0(CollFixed_ReadU32(sps, 0x54));
	CollFixed_GteLoadVZ0(normalZW);

	normalZW = CollFixed_Sll32(normalZW >> 16, 13);

	CollFixed_GteMVMVA();
	lineDot = CollFixed_GteReadMAC2();
	planeDot = CollFixed_GteReadMAC1() - normalZW;

	if (lineDot >= 0)
		return;

	factor = -planeDot / (lineDot >> 12);

	CollFixed_GteLoadMAC(startX, startY, startZ);
	CollFixed_GteLoadIR(deltaX, deltaY, deltaZ);
	CollFixed_GteLoadIR0(factor);

	if ((factor < 0) || (factor > 0x1000))
		return;

	CollFixed_GteGPL12();
	hitX = CollFixed_GteReadMAC1();
	hitY = CollFixed_GteReadMAC2();
	hitZ = CollFixed_GteReadMAC3();

	CollFixed_WriteS16(sps, 0x4c, hitX);
	CollFixed_WriteS16(sps, 0x4e, hitY);
	CollFixed_WriteS16(sps, 0x50, hitZ);

	baryV2 = v2;
	baryV3 = v3;
	normalAxis = CollFixed_ReadS16(sps, 0x52);

	if (normalAxis == 3)
	{
		s32 origin = CollFixed_ReadS16(v1, 4);
		firstA = CollFixed_ReadS16(v2, 4) - origin;
		firstB = CollFixed_ReadS16(v3, 4) - origin;
		firstHit = hitZ - origin;

		if (abs(firstA) < abs(firstB))
		{
			s32 tmp = firstA;
			firstA = firstB;
			firstB = tmp;
			baryV2 = v3;
			baryV3 = v2;
		}

		origin = CollFixed_ReadS16(v1, 0);
		secondA = CollFixed_ReadS16(baryV2, 0) - origin;
		secondB = CollFixed_ReadS16(baryV3, 0) - origin;
		secondHit = hitX - origin;
	}
	else if (normalAxis == 1)
	{
		s32 origin = CollFixed_ReadS16(v1, 0);
		firstA = CollFixed_ReadS16(v2, 0) - origin;
		firstB = CollFixed_ReadS16(v3, 0) - origin;
		firstHit = hitX - origin;

		if (abs(firstA) < abs(firstB))
		{
			s32 tmp = firstA;
			firstA = firstB;
			firstB = tmp;
			baryV2 = v3;
			baryV3 = v2;
		}

		origin = CollFixed_ReadS16(v1, 2);
		secondA = CollFixed_ReadS16(baryV2, 2) - origin;
		secondB = CollFixed_ReadS16(baryV3, 2) - origin;
		secondHit = hitY - origin;
	}
	else
	{
		s32 origin = CollFixed_ReadS16(v1, 2);
		firstA = CollFixed_ReadS16(v2, 2) - origin;
		firstB = CollFixed_ReadS16(v3, 2) - origin;
		firstHit = hitY - origin;

		if (abs(firstA) < abs(firstB))
		{
			s32 tmp = firstA;
			firstA = firstB;
			firstB = tmp;
			baryV2 = v3;
			baryV3 = v2;
		}

		origin = CollFixed_ReadS16(v1, 4);
		secondA = CollFixed_ReadS16(baryV2, 4) - origin;
		secondB = CollFixed_ReadS16(baryV3, 4) - origin;
		secondHit = hitZ - origin;
	}

	baryA = -0x1000;
	baryB = -0x1000;

	if (firstA != 0)
	{
		denom = (CollFixed_MulLo(secondB, firstA) - CollFixed_MulLo(firstB, secondA)) >> 6;

		if (denom != 0)
		{
			baryB = CollFixed_MulLo(CollFixed_MulLo(secondHit, firstA) - CollFixed_MulLo(firstHit, secondA), 0x40) / denom;

			if ((baryB >= 0) && (baryB <= 0x1000))
			{
				baryA = (CollFixed_Sll32(firstHit, 12) - CollFixed_MulLo(baryB, firstB)) / firstA;
			}
		}
	}
	else
	{
		if (firstB == 0)
			return;

		baryB = CollFixed_Sll32(firstHit, 12) / firstB;

		if ((baryB >= 0) && (baryB <= 0x1000))
		{
			baryA = (CollFixed_Sll32(secondHit, 12) - CollFixed_MulLo(baryB, secondB)) / secondA;
		}
	}

	quad = *(struct QuadBlock **)(sps + 0x64);

	if ((baryA < 0) || ((baryA + baryB - 0x1000) > 0))
		return;

	if ((quad->quadFlags & 0x40) != 0)
	{
		CollFixed_WriteU32(sps, 0x1a4, CollFixed_ReadU32(sps, 0x1a4) | (u8)quad->terrain_type);
		return;
	}

	*(struct QuadBlock **)(sps + 0x80) = quad;
	CollFixed_WriteS16(sps, 0xc8, baryA);
	CollFixed_WriteS16(sps, 0xca, baryB);
	*(struct LevVertex **)(sps + 0xcc) = *(struct LevVertex **)(v1 + 8);
	*(struct LevVertex **)(sps + 0xd0) = *(struct LevVertex **)(baryV2 + 8);
	*(struct LevVertex **)(sps + 0xd4) = *(struct LevVertex **)(baryV3 + 8);
	CollFixed_WriteS16(sps, 0x3e, CollFixed_ReadS16(sps, 0x3e) + 1);
	CollFixed_WriteU32(sps, 0x68, CollFixed_ReadU32(sps, 0x4c));
	CollFixed_WriteU32(sps, 0x1c, CollFixed_ReadU32(sps, 0x4c));
	CollFixed_WriteS16(sps, 0x6c, CollFixed_ReadS16(sps, 0x50));
	CollFixed_WriteS16(sps, 0x20, CollFixed_ReadS16(sps, 0x50));
	CollFixed_WriteU32(sps, 0x70, CollFixed_ReadU32(sps, 0x54));
	CollFixed_WriteU32(sps, 0x74, CollFixed_ReadU32(sps, 0x58));
}

void COLL_FIXED_TRIANGL_TestPoint(void *sps, void *v1, void *v2, void *v3)
{
	u8 *spsBytes = (u8 *)sps;
	u8 *v1Bytes = (u8 *)v1;
	s16 count = CollFixed_ReadS16(spsBytes, 0x3c);
	s32 normalZW = CollFixed_ReadS32(v1Bytes, 0x10);

	CollFixed_WriteS16(spsBytes, 0x3c, count + 1);
	CollFixed_WriteS16(spsBytes, 0x52, CollFixed_ReadS16(v1Bytes, 6));
	CollFixed_WriteU32(spsBytes, 0x54, CollFixed_ReadU32(v1Bytes, 0xc));
	CollFixed_WriteS32(spsBytes, 0x58, normalZW);

	COLL_FIXED_TRIANGL_TestPoint_Body(spsBytes, v1Bytes, (u8 *)v2, (u8 *)v3, normalZW);
}
