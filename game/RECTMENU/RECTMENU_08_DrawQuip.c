#include <common.h>

void DECOMP_RECTMENU_DrawQuip(char *comment, s16 startX, int startY, u32 sizeX, s16 fontType, int textFlag, s16 boxFlag)
{
	int posX = startX;
	int width;
	u32 sizeY;

	if ((sizeX & 0xffff) == 0)
	{
		width = DECOMP_DecalFont_GetLineWidth(comment, fontType);
		sizeX = width + 0xc;
	}

	// if text is not centered
	if ((textFlag & 0x8000) != 0)
	{
		// posX with text un-centered
		posX = startX - (sizeX >> 1);
	}

	sizeY = (u32)data.PlayerCommentBoxParams[fontType];

	// Draw string
	DECOMP_DecalFont_DrawLine(comment, startX, (data.PlayerCommentBoxParams[4 + fontType] + startY), fontType, textFlag);

	RECT r;
	r.x = posX;
	r.y = startY;
	r.w = sizeX;
	r.h = sizeY;
	DECOMP_RECTMENU_DrawInnerRect(&r, boxFlag, sdata->gGT->backBuffer->otMem.startPlusFour);
}
