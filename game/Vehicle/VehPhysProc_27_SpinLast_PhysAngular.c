#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800640a4-0x80064254.
void VehPhysProc_SpinLast_PhysAngular(struct Thread *t, struct Driver *d)
{
	int elapsedTimeMS = sdata->gGT->elapsedTimeMS;
	int driftAngleCurr;
	driftAngleCurr = d->turnAngleCurr;

	d->numFramesSpentSteering = 10000;

	d->rotationSpinRate -= d->rotationSpinRate >> 3;
	d->unk3D4[0] -= d->unk3D4[0] >> 3;

	d->ampTurnState = d->rotationSpinRate;

	if (driftAngleCurr < 0)
	{
		if ((d->KartStates.Spinning.driftSpinRate > 0) && (driftAngleCurr > -400))
		{
			d->KartStates.Spinning.driftSpinRate = (driftAngleCurr * -4) >> 3;

			if (d->KartStates.Spinning.driftSpinRate < 0x20)
				d->KartStates.Spinning.driftSpinRate = 0x20;
		}

		d->turnAngleCurr += d->KartStates.Spinning.driftSpinRate;
		d->turnAngleCurr += 0x800U;
		d->turnAngleCurr &= 0xfff;
		d->turnAngleCurr -= 0x800;

		if ((d->KartStates.Spinning.driftSpinRate > 0) && (d->turnAngleCurr > 0))
		{
			d->turnAngleCurr = 0;
		}
	}

	if (driftAngleCurr > 0)
	{
		if ((d->KartStates.Spinning.driftSpinRate < 0) && (driftAngleCurr < 400))
		{
			d->KartStates.Spinning.driftSpinRate = (driftAngleCurr * -4) >> 3;

			if (d->KartStates.Spinning.driftSpinRate > -0x20)
				d->KartStates.Spinning.driftSpinRate = -0x20;
		}

		d->turnAngleCurr += d->KartStates.Spinning.driftSpinRate;
		d->turnAngleCurr += 0x800U;
		d->turnAngleCurr &= 0xfff;
		d->turnAngleCurr -= 0x800;

		if ((d->KartStates.Spinning.driftSpinRate < 0) && (d->turnAngleCurr < 0))
		{
			d->turnAngleCurr = 0;
		}
	}

	d->angle += (s16)((d->ampTurnState * elapsedTimeMS) >> 0xd);
	d->angle &= 0xfff;

	d->rotCurr.y = d->unk3D4[0] + d->angle + d->turnAngleCurr;

	d->rotCurr.w = VehCalc_InterpBySpeed(d->rotCurr.w, (elapsedTimeMS << 5) >> 5, 0);

	VehPhysForce_RotAxisAngle(&d->matrixMovingDir, &d->AxisAngle1_normalVec.x, d->angle);
}
