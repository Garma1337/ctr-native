#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80022c88-0x80022d2c.
int *DecalGlobal_FindInLEV(struct Level *level, char *str)
{
	struct LevTexLookup *ltl = level->levTexLookup;

	if (ltl == NULL)
		return NULL;

	struct IconGroup **curr = ltl->firstIconGroupPtr;
	struct IconGroup **end = &ltl->firstIconGroupPtr[ltl->numIconGroup];

	for (; curr < end; curr++)
	{
		struct IconGroup *group = *curr;

		if (*(int *)&group->name[0x0] == *(int *)&str[0x0] && *(int *)&group->name[0x4] == *(int *)&str[0x4] &&
		    *(int *)&group->name[0x8] == *(int *)&str[0x8] && *(int *)&group->name[0xc] == *(int *)&str[0xc])
		{
			return (int *)group;
		}
	}

	return NULL;
}
