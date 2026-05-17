#include <common.h>

#define PSX_OVR233_BASE 0x800AB9F0U

extern typeof(OVR_233) OVR_233;

void DECOMP_CS_Instance_InitMatrix(void)
{
	char *boolInit = (char *)((uintptr_t)&OVR_233 + (0x800b7350 - PSX_OVR233_BASE));
	void **table = (void **)((uintptr_t)&OVR_233 + (0x800b7330 - PSX_OVR233_BASE));

	if (*boolInit != 0)
		return;

	*boolInit = 1;

	MATRIX mat;
	short pos[3];

	for (int i = 0; i < 4; i++)
	{
		char *data = (char *)table[i * 2];
		int count = (int)(uintptr_t)table[i * 2 + 1];

		if (data == NULL || count <= 0)
			continue;

		for (int j = 0; j < count; j++)
		{
			char *entry = data + j * 0x20;

			ConvertRotToMatrix(&mat, (short *)(entry + 8));

			pos[0] = *(short *)(entry + 0x10);
			pos[1] = *(short *)(entry + 0x12);
			pos[2] = *(short *)(entry + 0x14);

			MatrixRotate((MATRIX *)(entry + 8), pos, &mat);
		}
	}
}
