#include <common.h>

u8 DECOMP_MM_TransitionInOut(struct TransitionMeta *meta, int framesPassed, int numFrames)
{
	u8 bool_Transitioning;
	s16 start;
	s16 framesLeft;

	bool_Transitioning = 1;
	start = meta->headStart;
	framesLeft = ((s16)framesPassed - start);

	if (framesLeft == 4)
	{
		// Play "swoosh" sound for menu transition
		DECOMP_OtherFX_Play(0x65, 0);
	}

	// last member of array is null-terminated with 0xFFFF
	for (/**/; start > -1; meta++)
	{
		start = meta->headStart;
		framesLeft = ((s16)framesPassed - start);

		if (framesLeft < 1)
		{
			bool_Transitioning = 0;
			meta->currX = 0;
			meta->currY = 0;
			continue;
		}

		// else if
		if (framesLeft < (s16)numFrames)
		{
			bool_Transitioning = 0;
			meta->currX = framesLeft * meta->distX / (s16)numFrames;
			meta->currY = framesLeft * meta->distY / (s16)numFrames;
			continue;
		}

		// else
		meta->currX = meta->distX;
		meta->currY = meta->distY;
	}
	return bool_Transitioning;
}
