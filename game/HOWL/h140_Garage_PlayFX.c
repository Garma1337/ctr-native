#include <common.h>

void DECOMP_Garage_PlayFX(u_int soundId, char charId)
{
	if (charId < PINSTRIPE)
	{
	    // if sound == BIRD_RANDOM
		if (soundId == 0xf6)
		{
			sdata->audioRNG = ((sdata->audioRNG >> 3) + sdata->audioRNG * 0x20000000) * 5 + 1;
			soundId = (sdata->audioRNG % 3) + 0xf3;
		}

		DECOMP_OtherFX_Play_LowLevel(soundId & 0xffff, 1,
			sdata->garageSoundPool[charId].volume << 0x10 |
			sdata->garageSoundPool[charId].LR |
			0x8000);
	}
}
