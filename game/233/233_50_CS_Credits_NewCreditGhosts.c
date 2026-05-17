#include <common.h>

extern struct Ovr233_Credits_BSS* creditsBSS;

int CS_Credits_NewCreditGhosts()
{
	struct Model* model = creditsBSS->dancerInst_invisible->model;
	int i;

	for (i = 0; i < 5; i++)
	{
		if (creditsBSS->creditsObj.creditGhostModel[i] != model)
			return 0;
	}

	return 1;
}
