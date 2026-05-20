#include <common.h>

void DECOMP_howl_ParseCseqHeader(struct CseqHeader *ch)
{
	u32 addr = (u32)ch;

	sdata->ptrCseqHeader = (struct CseqHeader *)addr;
	addr += sizeof(struct CseqHeader);

	sdata->ptrCseqLongSamples = (struct SampleInstrument *)addr;
	addr += sizeof(struct SampleInstrument) * ch->numLongSamples;

	sdata->ptrCseqShortSamples = (struct SampleDrums *)addr;
	addr += sizeof(struct SampleDrums) * ch->numShortSamples;

	sdata->ptrCseqSongStartOffset = (s16 *)addr;
	addr += sizeof(s16) * ch->numSongs;

	// align up by 4
	addr += 2;

	sdata->ptrCseqSongData = (char *)addr;
}
