#include <common.h>

#ifdef CTR_NATIVE
struct CtrNativeCollVec
{
	s64 x;
	s64 y;
	s64 z;
};

static struct CtrNativeCollVec CtrNativeColl_Sub(struct CtrNativeCollVec a, struct CtrNativeCollVec b)
{
	struct CtrNativeCollVec out;
	out.x = a.x - b.x;
	out.y = a.y - b.y;
	out.z = a.z - b.z;
	return out;
}

static s64 CtrNativeColl_Dot(struct CtrNativeCollVec a, struct CtrNativeCollVec b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static struct CtrNativeCollVec CtrNativeColl_Cross(struct CtrNativeCollVec a, struct CtrNativeCollVec b)
{
	struct CtrNativeCollVec out;
	out.x = a.y * b.z - a.z * b.y;
	out.y = a.z * b.x - a.x * b.z;
	out.z = a.x * b.y - a.y * b.x;
	return out;
}

static u64 CtrNativeColl_ISqrt64(u64 value)
{
	u64 bit = 1ULL << 62;
	u64 result = 0;

	while (bit > value)
		bit >>= 2;

	while (bit != 0)
	{
		if (value >= result + bit)
		{
			value -= result + bit;
			result = (result >> 1) + bit;
		}
		else
		{
			result >>= 1;
		}

		bit >>= 2;
	}

	return result;
}

static struct CtrNativeCollVec CtrNativeColl_FromLevVertex(const struct LevVertex *vert)
{
	struct CtrNativeCollVec out;
	out.x = vert->pos[0];
	out.y = vert->pos[1];
	out.z = vert->pos[2];
	return out;
}

static void CtrNativeColl_WriteNormal(s16 *normalVec, struct CtrNativeCollVec normal)
{
	u64 length;
	u64 normalLenSquared;

	if (normal.y < 0)
	{
		normal.x = -normal.x;
		normal.y = -normal.y;
		normal.z = -normal.z;
	}

	normalLenSquared = (u64)(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	length = CtrNativeColl_ISqrt64(normalLenSquared);

	if (length == 0)
	{
		normalVec[0] = 0;
		normalVec[1] = 0x1000;
		normalVec[2] = 0;
		normalVec[3] = 0;
		return;
	}

	normalVec[0] = (s16)((normal.x * 0x1000) / (s64)length);
	normalVec[1] = (s16)((normal.y * 0x1000) / (s64)length);
	normalVec[2] = (s16)((normal.z * 0x1000) / (s64)length);
	normalVec[3] = 0;
}

static s16 CtrNativeColl_NormalDominantAxis(const s16 *normalVec)
{
	int absX = normalVec[0] < 0 ? -normalVec[0] : normalVec[0];
	int absY = normalVec[1] < 0 ? -normalVec[1] : normalVec[1];
	int absZ = normalVec[2] < 0 ? -normalVec[2] : normalVec[2];

	if ((absY > absX) && (absY >= absZ))
		return 3;

	if (absX >= absZ)
		return 2;

	return 1;
}

static int CtrNativeColl_IntersectSegmentTriangle(struct CtrNativeCollVec top, struct CtrNativeCollVec bottom, struct CtrNativeCollVec a,
                                                  struct CtrNativeCollVec b, struct CtrNativeCollVec c, double *outT, s16 *outHit, s16 *outNormal)
{
	struct CtrNativeCollVec ab = CtrNativeColl_Sub(b, a);
	struct CtrNativeCollVec ac = CtrNativeColl_Sub(c, a);
	struct CtrNativeCollVec dir = CtrNativeColl_Sub(bottom, top);
	struct CtrNativeCollVec normal = CtrNativeColl_Cross(ab, ac);
	double denom = (double)CtrNativeColl_Dot(normal, dir);
	double t;
	double hitX;
	double hitY;
	double hitZ;
	double d00;
	double d01;
	double d11;
	double d20;
	double d21;
	double baryDenom;
	double u;
	double v;

	if ((denom > -0.000001) && (denom < 0.000001))
		return 0;

	t = (double)CtrNativeColl_Dot(normal, CtrNativeColl_Sub(a, top)) / denom;
	if ((t < -0.000001) || (t > 1.000001))
		return 0;

	hitX = (double)top.x + ((double)dir.x * t);
	hitY = (double)top.y + ((double)dir.y * t);
	hitZ = (double)top.z + ((double)dir.z * t);

	d00 = (double)CtrNativeColl_Dot(ac, ac);
	d01 = (double)CtrNativeColl_Dot(ac, ab);
	d11 = (double)CtrNativeColl_Dot(ab, ab);
	d20 = (hitX - (double)a.x) * (double)ac.x + (hitY - (double)a.y) * (double)ac.y + (hitZ - (double)a.z) * (double)ac.z;
	d21 = (hitX - (double)a.x) * (double)ab.x + (hitY - (double)a.y) * (double)ab.y + (hitZ - (double)a.z) * (double)ab.z;
	baryDenom = d00 * d11 - d01 * d01;

	if ((baryDenom > -0.000001) && (baryDenom < 0.000001))
		return 0;

	u = (d11 * d20 - d01 * d21) / baryDenom;
	v = (d00 * d21 - d01 * d20) / baryDenom;

	if ((u < -0.000001) || (v < -0.000001) || ((u + v) > 1.000001))
		return 0;

	*outT = t;
	outHit[0] = (s16)(hitX >= 0 ? hitX + 0.5 : hitX - 0.5);
	outHit[1] = (s16)(hitY >= 0 ? hitY + 0.5 : hitY - 0.5);
	outHit[2] = (s16)(hitZ >= 0 ? hitZ + 0.5 : hitZ - 0.5);
	CtrNativeColl_WriteNormal(outNormal, normal);
	return 1;
}

static int CtrNativeColl_TryQuadTriangle(struct mesh_info *meshInfo, struct QuadBlock *quad, int indexA, int indexB, int indexC, struct CtrNativeCollVec top,
                                         struct CtrNativeCollVec bottom, double *bestT, s16 *bestHit, s16 *bestNormal, struct QuadBlock **bestQuad)
{
	s16 hit[3];
	s16 normal[4];
	double t;
	struct LevVertex *verts = meshInfo->ptrVertexArray;

	if (CtrNativeColl_IntersectSegmentTriangle(top, bottom, CtrNativeColl_FromLevVertex(&verts[quad->index[indexA]]),
	                                           CtrNativeColl_FromLevVertex(&verts[quad->index[indexB]]),
	                                           CtrNativeColl_FromLevVertex(&verts[quad->index[indexC]]), &t, hit, normal) == 0)
	{
		return 0;
	}

	if ((t < *bestT) || (*bestQuad == NULL))
	{
		*bestT = t;
		bestHit[0] = hit[0];
		bestHit[1] = hit[1];
		bestHit[2] = hit[2];
		bestNormal[0] = normal[0];
		bestNormal[1] = normal[1];
		bestNormal[2] = normal[2];
		bestNormal[3] = normal[3];
		*bestQuad = quad;
	}

	return 1;
}

// NOTE(aalhendi): CTR_NATIVE bridge, not ASM-verified. Retail traverses BSP and
// tests quadblock triangles through scratchpad code; native mirrors the outputs
// used by spawn/camera until the full COLL path is ported.
void COLL_SearchBSP_CallbackQUADBLK(u32 *posTop, u32 *posBottom, struct ScratchpadStruct *sps, int hitRadius)
{
	s16 *topShorts = (s16 *)posTop;
	s16 *bottomShorts = (s16 *)posBottom;
	struct mesh_info *meshInfo = sps->ptr_mesh_info;
	struct CtrNativeCollVec top;
	struct CtrNativeCollVec bottom;
	struct QuadBlock *bestQuad = NULL;
	s16 bestHit[3];
	s16 bestNormal[4] = {0, 0x1000, 0, 0};
	double bestT = 2.0;
	s16 minX;
	s16 minY;
	s16 minZ;
	s16 maxX;
	s16 maxY;
	s16 maxZ;

	sps->Input1.pos[0] = topShorts[0];
	sps->Input1.pos[1] = topShorts[1];
	sps->Input1.pos[2] = topShorts[2];
	sps->Input1.hitRadius = (s16)hitRadius;
	sps->Input1.hitRadiusSquared = hitRadius * hitRadius;

	sps->Union.QuadBlockColl.pos[0] = bottomShorts[0];
	sps->Union.QuadBlockColl.pos[1] = bottomShorts[1];
	sps->Union.QuadBlockColl.pos[2] = bottomShorts[2];
	sps->Union.QuadBlockColl.hitRadius = (s16)hitRadius;
	sps->Union.QuadBlockColl.hitRadiusSquared = hitRadius * hitRadius;
	sps->Union.QuadBlockColl.hitPos[0] = topShorts[0];
	sps->Union.QuadBlockColl.hitPos[1] = topShorts[1];
	sps->Union.QuadBlockColl.hitPos[2] = topShorts[2];

	sps->boolDidTouchQuadblock = 0;
	sps->unk40 = 0;
	sps->boolDidTouchHitbox = 0;
	sps->Set1.ptrQuadblock = NULL;
	sps->Set2.ptrQuadblock = NULL;
	*(u32 *)&sps->dataOutput[0] = 0;

	if ((meshInfo == NULL) || (meshInfo->ptrQuadBlockArray == NULL) || (meshInfo->ptrVertexArray == NULL))
		return;

	minX = topShorts[0] < bottomShorts[0] ? topShorts[0] : bottomShorts[0];
	minY = topShorts[1] < bottomShorts[1] ? topShorts[1] : bottomShorts[1];
	minZ = topShorts[2] < bottomShorts[2] ? topShorts[2] : bottomShorts[2];
	maxX = topShorts[0] > bottomShorts[0] ? topShorts[0] : bottomShorts[0];
	maxY = topShorts[1] > bottomShorts[1] ? topShorts[1] : bottomShorts[1];
	maxZ = topShorts[2] > bottomShorts[2] ? topShorts[2] : bottomShorts[2];

	sps->bbox.min[0] = minX - hitRadius;
	sps->bbox.min[1] = minY - hitRadius;
	sps->bbox.min[2] = minZ - hitRadius;
	sps->bbox.max[0] = maxX + hitRadius;
	sps->bbox.max[1] = maxY + hitRadius;
	sps->bbox.max[2] = maxZ + hitRadius;

	top.x = topShorts[0];
	top.y = topShorts[1];
	top.z = topShorts[2];
	bottom.x = bottomShorts[0];
	bottom.y = bottomShorts[1];
	bottom.z = bottomShorts[2];

	for (int i = 0; i < meshInfo->numQuadBlock; i++)
	{
		struct QuadBlock *quad = &meshInfo->ptrQuadBlockArray[i];

		if (((sps->Union.QuadBlockColl.qbFlagsWanted & quad->quadFlags) == 0) || ((sps->Union.QuadBlockColl.qbFlagsIgnored & quad->quadFlags) != 0))
			continue;

		if ((quad->bbox.min[0] > sps->bbox.max[0]) || (quad->bbox.min[1] > sps->bbox.max[1]) || (quad->bbox.min[2] > sps->bbox.max[2]) ||
		    (sps->bbox.min[0] > quad->bbox.max[0]) || (sps->bbox.min[1] > quad->bbox.max[1]) || (sps->bbox.min[2] > quad->bbox.max[2]))
		{
			continue;
		}

		CtrNativeColl_TryQuadTriangle(meshInfo, quad, 0, 1, 2, top, bottom, &bestT, bestHit, bestNormal, &bestQuad);
		if (quad->index[2] != quad->index[3])
			CtrNativeColl_TryQuadTriangle(meshInfo, quad, 1, 3, 2, top, bottom, &bestT, bestHit, bestNormal, &bestQuad);
	}

	if (bestQuad == NULL)
		return;

	sps->boolDidTouchQuadblock = 1;
	sps->Union.QuadBlockColl.hitPos[0] = bestHit[0];
	sps->Union.QuadBlockColl.hitPos[1] = bestHit[1];
	sps->Union.QuadBlockColl.hitPos[2] = bestHit[2];
	sps->Set1.hitPos[0] = bestHit[0];
	sps->Set1.hitPos[1] = bestHit[1];
	sps->Set1.hitPos[2] = bestHit[2];
	sps->Set1.normalVec[0] = bestNormal[0];
	sps->Set1.normalVec[1] = bestNormal[1];
	sps->Set1.normalVec[2] = bestNormal[2];
	sps->Set1.normalVec[3] = bestNormal[3];
	sps->Set1.BspSearchVertexFlags = CtrNativeColl_NormalDominantAxis(bestNormal);
	sps->Set1.ptrQuadblock = bestQuad;
	sps->Set2.hitPos[0] = bestHit[0];
	sps->Set2.hitPos[1] = bestHit[1];
	sps->Set2.hitPos[2] = bestHit[2];
	sps->Set2.normalVec[0] = bestNormal[0];
	sps->Set2.normalVec[1] = bestNormal[1];
	sps->Set2.normalVec[2] = bestNormal[2];
	sps->Set2.normalVec[3] = bestNormal[3];
	sps->Set2.BspSearchVertexFlags = sps->Set1.BspSearchVertexFlags;
	sps->Set2.ptrQuadblock = bestQuad;
}
#endif
