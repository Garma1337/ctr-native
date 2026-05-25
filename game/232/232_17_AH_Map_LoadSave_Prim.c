#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0b98-0x800b0ce0.
void AH_Map_LoadSave_Prim(s16 *vertPos, char *vertCol, void *ot, struct PrimMem *primMem)
{
	POLY_G4 *p = primMem->curr;

	if (primMem->end < (void *)p)
		return;

	primMem->curr = p + 1;

	setPolyG4(p);

	p->r0 = vertCol[0];
	p->g0 = vertCol[1];
	p->b0 = vertCol[2];

	p->r1 = vertCol[4];
	p->g1 = vertCol[5];
	p->b1 = vertCol[6];

	p->r2 = vertCol[8];
	p->g2 = vertCol[9];
	p->b2 = vertCol[10];

	p->r3 = vertCol[12];
	p->g3 = vertCol[13];
	p->b3 = vertCol[14];

	*(int *)&p->x0 = *(int *)&vertPos[0x0];
	*(int *)&p->x1 = *(int *)&vertPos[0x2];
	*(int *)&p->x2 = *(int *)&vertPos[0x4];
	*(int *)&p->x3 = *(int *)&vertPos[0x6];

	AddPrim(ot, p);
}
