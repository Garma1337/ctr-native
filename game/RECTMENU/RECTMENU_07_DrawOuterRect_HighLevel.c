#include <common.h>

void DECOMP_RECTMENU_DrawOuterRect_HighLevel(RECT *r, Color color, s16 param_3, u_long *otMem)
{
	DECOMP_RECTMENU_DrawOuterRect_LowLevel(r, 3, 2, color, param_3, otMem);
	return;
}
