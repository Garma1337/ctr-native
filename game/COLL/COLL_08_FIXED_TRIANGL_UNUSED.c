#include <common.h>

void COLL_FIXED_TRIANGL_UNUSED(void)
{
	// NOTE(aalhendi): Retail is a hand-written trampoline into
	// COLL_FIXED_TRIANGL_TestPoint+0x24 that relies on live MIPS register
	// state. No native caller is known yet; keep this unstamped until a real
	// callsite or backfeed need proves the expected calling convention.
}
