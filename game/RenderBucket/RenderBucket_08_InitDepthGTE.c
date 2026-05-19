#include <common.h>

void RenderBucket_InitDepthGTE(void)
{
	// NOTE(aalhendi): ASM-verified against US 926 0x8006ae74-0x8006ae90.
	// Retail initializes DQA/DQB to 0, ZSF3 to 0x555, and ZSF4 to 0x400.
	CTC2(0, 27);
	CTC2(0, 28);
	CTC2(0x555, 29);
	CTC2(0x400, 30);
}
