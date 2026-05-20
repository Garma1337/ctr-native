#include <common.h>

void DECOMP_LOAD_HowlCallback(CdlIntrResult result, u8 *unk)
{
	// disable callback
	CdReadCallback(0);

	if (result == CdlComplete)
		sdata->howlChainState = 0;
	else
		sdata->howlChainState = -1;
}
