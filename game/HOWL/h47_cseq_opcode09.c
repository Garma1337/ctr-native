#include <common.h>

void DECOMP_cseq_opcode09(struct SongSeq *seq)
{
	u8 *currNote = seq->currNote;
	seq->instrumentID = currNote[1];
}
