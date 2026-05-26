#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80047da8-0x80047dfc.
void SelectProfile_QueueLoadHub_MenuProc(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;

	gGT->levelID = ADVENTURE_GARAGE;

	data.characterIDs[0] = sdata->advProgress.characterID;
	MainRaceTrack_RequestLoad(gGT->currLEV);
	RECTMENU_Hide(menu);
	return;
}
