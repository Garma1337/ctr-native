#include <common.h>

void DECOMP_CDSYS_XaCallbackCdSync(CdlIntrResult result, u8 *unk) //+unk to adhere to *CdlCB
{
	u8 com;

	if (result == CdlComplete)
	{
		// determine CdlSeekL or CdlSeekP
		com = CdLastCom() - 0x15;

		if (com < 2)
		{
			sdata->XA_State = 0;
		}

		return;
	}

	sdata->countFail_CdSyncCallback++;
}
