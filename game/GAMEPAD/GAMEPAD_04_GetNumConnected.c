#include <common.h>

// NOTE(aalhendi): PSX path ASM-verified NTSC-U 926 0x800255b4-0x80025718.
int GAMEPAD_GetNumConnected(struct GamepadSystem *gGamepads)
{
	int padIndex;
	int bitwiseConnected;

	int numSlots;
	int numPortsPerSlot;

	struct MultitapPacket *ptrControllerPacket;
	struct GamepadBuffer *padCurr;

	// 2 players, no multitap
	numSlots = 2;
	numPortsPerSlot = 1;

	if (
	    // multitap detected
	    (gGamepads->slotBuffer[0].plugged == PLUGGED) && (gGamepads->slotBuffer[0].controllerData == (PAD_ID_MULTITAP << 4)))
	{
		// 4 players, with multitap
		numSlots = 1;
		numPortsPerSlot = 4;
	}

#ifdef CTR_NATIVE
	// NOTE(aalhendi): Native keeps four local pad packets addressable without
	// requiring a retail multitap packet in slot 0.
	numSlots = 1;
	numPortsPerSlot = 4;
#endif

	padIndex = 0;
	bitwiseConnected = 0;
	gGamepads->numGamepadsConnected = 0;
	padCurr = &gGamepads->gamepad[0];

	// TODO: Rename to match PollVsync
	// should be ports and padsPerPort

	for (int Slot = 0; Slot < numSlots; Slot++)
	{
		for (int Port = 0; Port < numPortsPerSlot; Port++)
		{
			ptrControllerPacket = &gGamepads->slotBuffer[Slot];
			if (ptrControllerPacket->plugged == PLUGGED)
			{
				// if multitap plugged in
				if (ptrControllerPacket->controllerData == (PAD_ID_MULTITAP << 4))
				{
					ptrControllerPacket = (struct MultitapPacket *)&ptrControllerPacket->controllers[Port];
				}

				if (ptrControllerPacket->plugged == PLUGGED)
				{
					bitwiseConnected |= 1 << (Slot * 4 + Port);
					gGamepads->numGamepadsConnected = padIndex + 1;

					padCurr->ptrControllerPacket = (struct ControllerPacket *)ptrControllerPacket;
					padCurr->gamepadID = Slot * 0x10 + Port;
				}
			}

			padIndex++;
			padCurr++;
		}
	}

	while (padCurr < &gGamepads->gamepad[8])
	{
		// pad is now unplugged
		padCurr->ptrControllerPacket = 0;
		padCurr++;
	}

	// this name is way too long
	int *ptrToSet = &gGamepads->gamepadsConnectedByFlag;
	int oldVal = *ptrToSet;
	*ptrToSet = bitwiseConnected;

	if (oldVal == -1)
		return 0;
	if (oldVal == bitwiseConnected)
		return 0;

	// return change
	return (u32)((bitwiseConnected ^ oldVal) & oldVal) != 0;
}
