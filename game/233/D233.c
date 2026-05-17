// TODO(aalhendi): This entire file is a hack. D230/D231/D232 use proper C struct
// initializers with &D230.field self-references (see game/230/D230.c). This file
// instead embeds the raw overlay binary via .incbin and does runtime memcpy + pointer
// rebasing. Replace with proper static initialization (D233 pattern) once the overlay
// data is fully reverse-engineered into named struct fields.

#include <common.h>

#define PSX_OVR233_BASE 0x800AB9F0U
#define OVR233_DATA_SIZE 48528

asm(
	".section .rodata\n"
	".global _ovr233_overlay_data\n"
	".align 4\n"
	"_ovr233_overlay_data:\n"
	".incbin \"/home/aalhendi/dev/ctr/CTR-ModSDK/build/ctr-u/bigfile/overlays/233_Threads_Cutscene.bin\"\n"
	".previous\n"
);

extern char _ovr233_overlay_data[];

extern typeof(OVR_233) OVR_233;

void DECOMP_CS_OVR233_LoadData(void)
{
	uint32_t delta = (uintptr_t)&OVR_233 - PSX_OVR233_BASE;

	memcpy(&OVR_233, _ovr233_overlay_data, OVR233_DATA_SIZE);

	uint32_t *p = (uint32_t *)&OVR_233;
	uint32_t ovr_start = PSX_OVR233_BASE;
	uint32_t ovr_end = PSX_OVR233_BASE + 56844;
	int count = OVR233_DATA_SIZE / 4;

	// TODO(aalhendi): Skip first 0x3E4 bytes (code/string region) to avoid
	// falsely rebasing MIPS instructions that coincidentally look like PSX pointers.
	// Once we use proper static init, this hack goes away entirely.
#define CODE_REGION_END 0x3E4

	for (int i = CODE_REGION_END / 4; i < count; i++)
	{
		if (p[i] >= ovr_start && p[i] < ovr_end)
		{
			p[i] += delta;
		}
	}
}
