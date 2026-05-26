#include <common.h>

extern void *PlayerDrivingFuncTable[13];

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80062b74-0x80062ca8.
void VehPhysProc_Driving_Init(struct Thread *t, struct Driver *d)
{
	struct GameTracker *gGT = sdata->gGT;

	if (((u32)(gGT->levelID - GEM_STONE_VALLEY) >= 5) || (LOAD_IsOpen_AdvHub() != 0))
	{
		// Turbo meter = full
		d->turbo_MeterRoomLeft = 0;

		d->StartDriving_0x60 = 0x60;
		d->StartRollback_0x280 = 0x280;

		d->unknownTraction = 0;

		for (int i = 0; i < 13; i++)
		{
			d->funcPtrs[i] = PlayerDrivingFuncTable[i];
		}

		if (((gGT->gameMode1 & BATTLE_MODE) != 0) && (d->kartState == KS_BLASTED))
		{
			d->invincibleTimer = 0xb40;
		}

		// must put this HERE, so that
		// the above IF-statement works
		d->kartState = KS_NORMAL;
	}
}

void *PlayerDrivingFuncTable[13] = {
    NULL,
    VehPhysProc_Driving_Update,
    VehPhysProc_Driving_PhysLinear,
    VehPhysProc_Driving_Audio,
    VehPhysGeneral_PhysAngular,
    VehPhysForce_OnApplyForces,
    COLL_MOVED_PlayerSearch,
    VehPhysForce_CollideDrivers,
    COLL_FIXED_PlayerSearch,
    VehPhysGeneral_JumpAndFriction,
    VehPhysForce_TranslateMatrix,
    VehFrameProc_Driving,
    VehEmitter_DriverMain,
};
