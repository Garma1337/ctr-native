#include <common.h>

// NOTE(aalhendi): PSX path ASM-verified NTSC-U 926 0x8003c41c-0x8003c480.
void MainKillGame_StopCTR(void)
{
	DrawSyncCallback(0);
	StopCallback();

#ifndef CTR_NATIVE
	MEMCARD_CloseCard();
#endif

	PadStopCom();
	ResetGraph(3);
	VSyncCallback(0);

	Timer_Destroy();
}
