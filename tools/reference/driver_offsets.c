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
#include <stddef.h>
#include <stdio.h>

struct sData sdata_static;

#define ROW(field, type) printf("Driver\t%s\t%zu\t%zu\t%s\n", #field, offsetof(struct Driver, field), sizeof(((struct Driver *)0)->field), type)

int main(void)
{
	ROW(simpTurnState, "s8");
	ROW(multDrift, "s16");
	ROW(KartStates.Drifting.numFramesDrifting, "s16");
	ROW(KartStates.Drifting.driftBoostTimeMS, "s16");

	ROW(jump_LandingBoost, "s16");
	ROW(jumpHeightCurr, "s16");
	ROW(jumpHeightPrev, "s16");
	return 0;
}
