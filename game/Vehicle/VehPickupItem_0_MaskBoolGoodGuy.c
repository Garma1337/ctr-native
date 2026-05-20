#include <common.h>

int DECOMP_VehPickupItem_MaskBoolGoodGuy(struct Driver *d)
{
	int charID;
	charID = data.characterIDs[d->driverID];

	// Crash, Coco, Pura, Polar, Penta
	u32 maskBits = 0x20c9;

	return (maskBits >> charID) & 1;
}
