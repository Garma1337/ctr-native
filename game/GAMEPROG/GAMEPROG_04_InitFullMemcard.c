#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026c24-0x80026cb8.
void GAMEPROG_InitFullMemcard(struct MemcardProfile *mcp)
{
	int i;

	// clear
	memset(mcp, 0, sizeof(struct MemcardProfile));

	// header
	mcp->header[0] = 0xffee; // version (-18)
	mcp->header[1] = sizeof(struct MemcardProfile);

	// GameProgress and GameOptions
	GAMEPROG_NewProfile_OutsideAdv(&mcp->gameProgress);

	// 4 profiles
	for (i = 0; i < 4; i++)
	{
		// no character selected
		mcp->advProgress[i].characterID = -1;

		// N Sane Beach
		mcp->advProgress[i].HubLevYouSavedOn = 0x1a;
	}
}
