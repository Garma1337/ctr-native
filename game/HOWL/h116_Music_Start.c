#include <common.h>

void DECOMP_Music_Start(u32 songID)
{
	sdata->cseqBoolPlay = true;

	// set highest song index
	sdata->cseqHighestIndex = songID & 0xffff;
}
