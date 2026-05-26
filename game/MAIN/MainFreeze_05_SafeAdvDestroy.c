#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800399fc-0x80039a44.
void MainFreeze_SafeAdvDestroy(void)
{
	// If you're in Adventure Arena
	if ((sdata->gGT->gameMode1 & ADVENTURE_ARENA) == 0)
		return;

	// check if Adv Hub is loaded
	if (LOAD_IsOpen_AdvHub() == 0)
		return;

	AH_Pause_Destroy();
	return;
}
