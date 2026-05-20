#include <common.h>

void *DECOMP_VehAfterColl_GetSurface(u32 scrubId)
{
	struct Scrub *sc = &data.MetaDataScrub[0];

	if (scrubId < 7)
		return &sc[scrubId];

	return sc;
}
