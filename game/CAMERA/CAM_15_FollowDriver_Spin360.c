#include <common.h>

void DECOMP_CAM_FollowDriver_Spin360(struct CameraDC *cDC, int param_2, struct Driver *d, s16 *desiredPos, s16 *desiredRot)
{
	int ratio;
	s16 spinSpeed;

	// === Union Missing ===
	// Not really "transitionTo" but the variables
	// are shared with other camera modes, therefore
	// need a union with proper names for each mode

	// rotate one way
	spinSpeed = cDC->transitionTo.pos[0];

	// rotate other way for odd number
	if ((d->driverID & 1) != 0)
		spinSpeed = -spinSpeed;

	int angle = cDC->unk90 + spinSpeed;
	cDC->unk90 = angle;

	ratio = DECOMP_MATH_Sin(angle);
	desiredPos[0] = (s16)(d->posCurr.x >> 8) + (s16)((ratio * cDC->transitionTo.pos[2]) >> 0xc);

	ratio = DECOMP_MATH_Cos(angle);
	desiredPos[2] = (s16)(d->posCurr.z >> 8) + (s16)((ratio * cDC->transitionTo.pos[2]) >> 0xc);

	desiredPos[1] = (s16)(d->posCurr.y >> 8) + cDC->transitionTo.pos[1];

	DECOMP_CAM_LookAtPosition(param_2, (int *)&d->posCurr.x, desiredPos, desiredRot);
	return;
}
