#include <common.h>

static void (*const LOAD_DriverMPK_SetPointer)(struct LoadQueueSlot *) = (void (*)(struct LoadQueueSlot *))-2;

void LOAD_DriverMPK(struct BigHeader *bigfile, int levelLOD, void (*callback)(struct LoadQueueSlot *))
{
	int i;
	int gameMode1;

	struct GameTracker *gGT = sdata->gGT;
	gameMode1 = gGT->gameMode1;

	int lastFileIndexMPK;

	// 3P/4P
	if (levelLOD - 3U < 2)
	{
		for (i = 0; i < levelLOD - 1; i++)
		{
			// low lod CTR model
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELLOW + data.characterIDs[i], &data.driverModelExtras[i], LOAD_DriverMPK_SetPointer);
		}

		// load 4P MPK of fourth player
		lastFileIndexMPK = BI_4PARCADEPACK + data.characterIDs[i];
	}

	else if (
	    // adv mpk when we just need text from MPK
	    ((gameMode1 & (GAME_CUTSCENE | ADVENTURE_ARENA | MAIN_MENU)) != 0) ||

	    // credits
	    ((gGT->gameMode2 & CREDITS) != 0))
	{
		lastFileIndexMPK = BI_ADVENTUREPACK + data.characterIDs[0];
	}

	else if ((gameMode1 & (ADVENTURE_BOSS | RELIC_RACE | TIME_TRIAL)) != 0)
	{
		// Do NOT switch the order to optimize Relic,
		// if HI+IDs[1] and PACK+IDs[0] is loaded,
		// then mask-grab breaks for all characters
		// on Hot Air Skyway (except Crash Bandicoot)

		// Load Player 1 [0]
		LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELHI + data.characterIDs[0], &data.driverModelExtras[0], LOAD_DriverMPK_SetPointer);

		// Load boss or ghost [1]
		lastFileIndexMPK = BI_TIMETRIALPACK + data.characterIDs[1];
	}

	else if (
	    // If you are in Adventure cup
	    ((gameMode1 & ADVENTURE_CUP) != 0) &&

	    // purple gem cup
	    (gGT->cup.cupID == 4))
	{
		data.characterIDs[1] = RIPPER_ROO;
		data.characterIDs[2] = PAPU_PAPU;
		data.characterIDs[3] = KOMODO_JOE;
		data.characterIDs[4] = PINSTRIPE;

		// high lod model
		LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELHI + data.characterIDs[0], &data.driverModelExtras[0], LOAD_DriverMPK_SetPointer);

		// pack of four AIs with bosses
		lastFileIndexMPK = BI_2PARCADEPACK + 7;
	}

	// any 1P mode,
	// not adv, not time trial, not gem cup, not credits
	else if (levelLOD == 1)
	{
	ForceOnlineLoad8:
		LOAD_Robots1P(data.characterIDs[0]);

		// arcade mpk
		lastFileIndexMPK = BI_1PARCADEPACK + data.characterIDs[0];
	}

	// else if(levelLOD == 2)
	else
	{
		// med models
		for (i = 0; i < 2; i++)
		{
			// med lod CTR model
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELMED + data.characterIDs[i], &data.driverModelExtras[i], LOAD_DriverMPK_SetPointer);
		}

		LOAD_Robots2P(bigfile, data.characterIDs[0], data.characterIDs[1], callback);
		return;
	}

	LOAD_AppendQueue(bigfile, LT_GETADDR, lastFileIndexMPK, NULL, callback);
}
