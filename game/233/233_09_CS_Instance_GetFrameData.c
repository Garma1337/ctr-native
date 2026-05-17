#include <common.h>

#define read_mt(r0, r1, r2) \
	{ r0 = MFC2(25); r1 = MFC2(26); r2 = MFC2(27); }

u_int DECOMP_CS_Instance_GetFrameData(
	struct Instance *inst, int animIndex, u_int animFrame,
	u_short *pos, u_short *param_5, int offset)
{
	int isOdd;
	int numFrames;
	struct ModelAnim *ptrAnim;
	short *framePos;
	char *bonePtr;
	u_int boneValX, boneValY, boneValZ;
	u_int boneDX, boneDY, boneDZ;
	struct ModelHeader *headers;
	int scaleX, scaleY, scaleZ;
	int deltaDX, deltaDY, deltaDZ;

	headers = inst->model->headers;
	ptrAnim = headers->ptrAnimations[animIndex];

	if ((int)animFrame < 0)
		animFrame = 0;

	numFrames = ptrAnim->numFrames;
	isOdd = 0;

	if (numFrames < 0)
	{
		numFrames = -numFrames;
		isOdd = animFrame & 1;
		animFrame = animFrame >> 1;
	}

	if ((numFrames - 1U) <= animFrame)
	{
		isOdd = 0;
		animFrame = numFrames - 1U;
	}

	framePos = (short *)((char *)ptrAnim + ptrAnim->frameSize * animFrame + sizeof(struct ModelAnim));

	{
		int boneOff = offset * 3 + 0x1c;
		bonePtr = (char *)framePos + boneOff;
	}

	boneValX = (u_int)bonePtr[0];
	boneValY = (u_int)bonePtr[2];
	boneValZ = (u_int)bonePtr[1];
	boneDX   = (u_int)bonePtr[3];
	boneDZ   = (u_int)bonePtr[5];
	boneDY   = (u_int)bonePtr[4];

	if (isOdd)
	{
		framePos = (short *)((char *)framePos + ptrAnim->frameSize);
		{
			int boneOff = offset * 3 + 0x1c;
			bonePtr = (char *)framePos + boneOff;
		}

		boneValX = (int)(boneValX + bonePtr[0]) >> 1;
		boneValY = (int)(boneValY + bonePtr[2]) >> 1;
		boneDZ  = (int)(boneDZ  + bonePtr[5]) >> 1;
		boneValZ = (int)(boneValZ + bonePtr[1]) >> 1;
		boneDX   = (int)(boneDX   + bonePtr[3]) >> 1;
		boneDY   = (int)(boneDY   + bonePtr[4]) >> 1;
	}

	deltaDX = boneValX - boneDX;

	{
		short instScale = inst->scale[0];

		scaleX = ((int)((boneValX + (int)framePos[0]) * instScale) >> 0xc) * (int)headers->scale[0] >> 0xc;
		scaleY = ((int)((boneValY + (int)framePos[1]) * instScale) >> 0xc) * (int)headers->scale[1] >> 0xc;
		scaleZ = ((int)((boneValZ + (int)framePos[2]) * instScale) >> 0xc) * (int)headers->scale[2] >> 0xc;
	}

	deltaDY = boneValY - boneDZ;
	deltaDZ = boneValZ - boneDY;

	gte_SetLightMatrix(&inst->matrix);

	MTC2((scaleX & 0xffff) | (scaleY << 0x10), 0);
	MTC2(scaleZ, 1);
	gte_llv0();

	{
		int rx, ry, rz;
		read_mt(rx, ry, rz);

		pos[0] = (short)rx;
		pos[1] = (short)ry;
		pos[2] = (short)rz;
	}

	if (param_5 != NULL)
	{
		MTC2((deltaDX & 0xffff) | (deltaDY << 0x10), 0);
		MTC2(deltaDZ, 1);
		gte_llv0();

		{
			int dvx, dvy, dvz;
			read_mt(dvx, dvy, dvz);

			int pitch = ratan2(-dvy, SquareRoot0_stub(dvx * dvx + dvz * dvz));
			param_5[0] = (short)pitch;

			int yaw = ratan2(dvx, dvz);
			param_5[1] = (short)yaw;
			param_5[2] = 0;
		}
	}

	return 0;
}
