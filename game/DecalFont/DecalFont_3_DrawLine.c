#include <common.h>

void DECOMP_DecalFont_DrawLine(char *str, int posX, int posY, s16 fontType, int flags)
{
	DECOMP_DecalFont_DrawLineStrlen(str, -1, posX, posY, fontType, flags);
}
