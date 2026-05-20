#include <common.h>

int DECOMP_DecalFont_GetLineWidth(char *str, s16 fontType)
{
	return DECOMP_DecalFont_GetLineWidthStrlen(str, -1, fontType);
}
