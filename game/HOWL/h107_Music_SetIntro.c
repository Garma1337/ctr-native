#include <common.h>

void DECOMP_Music_SetIntro(void)
{
	u32 boolSuccess;
	struct Bank thisBank;

	sdata->audioDefaults[7] = 0;

	DECOMP_Bank_Load(33, &thisBank);

	while (DECOMP_Bank_AssignSpuAddrs() == 0)
	{
	}

	DECOMP_howl_SetSong(28);

	while (DECOMP_howl_LoadSong() == 0)
	{
	}
}
