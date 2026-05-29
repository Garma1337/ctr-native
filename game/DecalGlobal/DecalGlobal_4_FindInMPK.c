#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80022d2c-0x80022db0.
int *DecalGlobal_FindInMPK(u32 *icons, char *str)
{
	struct Icon *icon = (struct Icon *)icons;

	for (; icon->name[0] != '\0'; icon++)
	{
		if (*(int *)&icon->name[0x0] == *(int *)&str[0x0] && *(int *)&icon->name[0x4] == *(int *)&str[0x4] && *(int *)&icon->name[0x8] == *(int *)&str[0x8] &&
		    *(int *)&icon->name[0xc] == *(int *)&str[0xc])
		{
			return (int *)icon;
		}
	}

	return NULL;
}
