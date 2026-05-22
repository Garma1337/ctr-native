#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80020334-0x80020410; scrub depth is SPS+0x0e.
void COLL_MOVED_FindScrub(struct QuadBlock *qb, int triangleID, struct ScratchpadStruct *sps)
{
	struct ScratchpadStructExtended *ext = (struct ScratchpadStructExtended *)sps;
	u16 searchFlags = sps->Union.QuadBlockColl.searchFlags;

	if (qb == NULL)
	{
		sps->Union.QuadBlockColl.searchFlags = searchFlags & 0xffdf;
		CollFixed_WriteS16(sps, 0xe, 0);
		ext->numTriangles = 0;
		return;
	}

	for (s32 i = ext->numTriangles - 1; i >= 0; i--)
	{
		struct BspSearchTriangle *tri = &ext->bspSearchTriangle[i];

		if ((tri->quadblock == qb) && (tri->triangleID == triangleID))
		{
			s32 numCollision = tri->numCollision;
			s16 scrub = numCollision;

			if (numCollision < 0x401)
			{
				numCollision += 0x100;
				scrub = numCollision;
				tri->numCollision = numCollision;
			}

			sps->Union.QuadBlockColl.searchFlags = searchFlags | 0x20;
			CollFixed_WriteS16(sps, 0xe, scrub);
			return;
		}
	}

	{
		struct BspSearchTriangle *tri = &ext->bspSearchTriangle[ext->numTriangles];

		tri->quadblock = qb;
		tri->triangleID = triangleID;
		tri->numCollision = 0;
	}

	sps->Union.QuadBlockColl.searchFlags = searchFlags & 0xffdf;
	CollFixed_WriteS16(sps, 0xe, 0);
	ext->numTriangles++;
}
