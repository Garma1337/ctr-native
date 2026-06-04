#include "platform/native_replay_scheduler.h"

#if defined(CTR_INTERNAL)
#include "platform/native_input.h"
#include "platform/native_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE(aalhendi): Little-endian tags `CTRR`/`RFRM` = CTR native Replay.
#define NATIVE_REPLAY_FILE_MAGIC                  0x52525443u
#define NATIVE_REPLAY_FRAME_MAGIC                 0x4d524652u
#define NATIVE_REPLAY_FILE_VERSION                1u
#define NATIVE_REPLAY_FNV_OFFSET                  2166136261u
#define NATIVE_REPLAY_FNV_PRIME                   16777619u

enum NativeReplaySchedulerMode
{
	NATIVE_REPLAY_MODE_NONE = 0,
	NATIVE_REPLAY_MODE_RECORD,
	NATIVE_REPLAY_MODE_PLAYBACK
};

struct NativeReplayFileHeader
{
	u32 magic;
	u32 version;
	u32 headerSize;
	u32 frameRecordSize;
	u32 frameCount;
	u32 reserved[3];
};

struct NativeReplayFrameRecord
{
	u32 magic;
	u32 replayFrame;
	struct NativeReplaySchedulerFrameInfo beginInfo;
	struct NativeReplaySchedulerFrameInfo endInfo;
	struct PlatformInputPadSnapshot pads[PLATFORM_INPUT_PAD_COUNT];
	u32 padChecksum;
	u32 recordChecksum;
	u32 reserved[2];
};

static enum NativeReplaySchedulerMode s_mode;
static u32 s_replayFrame;
static u32 s_frameLimit;
static s32 s_beginOpen;
static s32 s_divergenceLogged;
static FILE *s_file;
static struct NativeReplayFileHeader s_header;
static struct NativeReplayFrameRecord s_pendingRecord;

static u32 NativeReplayScheduler_Fnv1a(const void *data, u32 size)
{
	const u8 *bytes = (const u8 *)data;
	u32 hash = NATIVE_REPLAY_FNV_OFFSET;
	u32 i;

	for (i = 0; i < size; i++)
	{
		hash ^= bytes[i];
		hash *= NATIVE_REPLAY_FNV_PRIME;
	}

	return hash;
}

static u32 NativeReplayScheduler_PadChecksum(const struct PlatformInputPadSnapshot *pads)
{
	return NativeReplayScheduler_Fnv1a(pads, sizeof(struct PlatformInputPadSnapshot) * PLATFORM_INPUT_PAD_COUNT);
}

static u32 NativeReplayScheduler_RecordChecksum(const struct NativeReplayFrameRecord *record)
{
	struct NativeReplayFrameRecord checksumRecord = *record;

	checksumRecord.recordChecksum = 0;
	return NativeReplayScheduler_Fnv1a(&checksumRecord, sizeof(checksumRecord));
}

static void NativeReplayScheduler_InitHeader(struct NativeReplayFileHeader *header)
{
	memset(header, 0, sizeof(*header));
	header->magic = NATIVE_REPLAY_FILE_MAGIC;
	header->version = NATIVE_REPLAY_FILE_VERSION;
	header->headerSize = sizeof(struct NativeReplayFileHeader);
	header->frameRecordSize = sizeof(struct NativeReplayFrameRecord);
}

static s32 NativeReplayScheduler_HeaderValid(const struct NativeReplayFileHeader *header)
{
	return (header->magic == NATIVE_REPLAY_FILE_MAGIC) && (header->version == NATIVE_REPLAY_FILE_VERSION) &&
	       (header->headerSize == sizeof(struct NativeReplayFileHeader)) && (header->frameRecordSize == sizeof(struct NativeReplayFrameRecord));
}

static const char *NativeReplayScheduler_ArgValue(int argc, char **argv, const char *arg)
{
	int i;

	for (i = 1; i < argc - 1; i++)
	{
		if (strcmp(argv[i], arg) == 0)
			return argv[i + 1];
	}

	return NULL;
}

static s32 NativeReplayScheduler_ArgMissingValue(int argc, char **argv, const char *arg)
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if ((strcmp(argv[i], arg) == 0) && (i + 1 >= argc))
			return 1;
	}

	return 0;
}

static s32 NativeReplayScheduler_ParseU32(const char *text, u32 *out)
{
	char *end;
	unsigned long value;

	if ((text == NULL) || (text[0] == '\0') || (out == NULL))
		return 0;

	value = strtoul(text, &end, 10);
	if ((end == text) || (*end != '\0') || (value > 0xffffffffUL))
		return 0;

	*out = (u32)value;
	return 1;
}

static s32 NativeReplayScheduler_WriteHeader(void)
{
	long oldPos;

	if (s_file == NULL)
		return 0;

	oldPos = ftell(s_file);
	if (oldPos < 0)
		return 0;

	if (fseek(s_file, 0, SEEK_SET) != 0)
		return 0;

	if (fwrite(&s_header, sizeof(s_header), 1, s_file) != 1)
		return 0;

	if (fseek(s_file, oldPos, SEEK_SET) != 0)
		return 0;

	return 1;
}

static void NativeReplayScheduler_CloseFile(void)
{
	if (s_file == NULL)
		return;

	if (s_mode == NATIVE_REPLAY_MODE_RECORD)
	{
		if (!NativeReplayScheduler_WriteHeader())
			Platform_Log("[CTR Replay] failed to finalize replay header\n");
	}

	fclose(s_file);
	s_file = NULL;
}

static s32 NativeReplayScheduler_OpenRecord(const char *path)
{
	NativeReplayScheduler_InitHeader(&s_header);
	s_file = fopen(path, "wb+");
	if (s_file == NULL)
	{
		Platform_Log("[CTR Replay] failed to open replay for record: %s\n", path);
		return 0;
	}

	if (fwrite(&s_header, sizeof(s_header), 1, s_file) != 1)
	{
		Platform_Log("[CTR Replay] failed to write replay header: %s\n", path);
		NativeReplayScheduler_CloseFile();
		return 0;
	}

	s_mode = NATIVE_REPLAY_MODE_RECORD;
	Platform_Log("[CTR Replay] recording input replay: %s\n", path);
	return 1;
}

static s32 NativeReplayScheduler_OpenPlayback(const char *path)
{
	s_file = fopen(path, "rb");
	if (s_file == NULL)
	{
		Platform_Log("[CTR Replay] failed to open replay for playback: %s\n", path);
		return 0;
	}

	if (fread(&s_header, sizeof(s_header), 1, s_file) != 1)
	{
		Platform_Log("[CTR Replay] failed to read replay header: %s\n", path);
		NativeReplayScheduler_CloseFile();
		return 0;
	}

	if (!NativeReplayScheduler_HeaderValid(&s_header))
	{
		Platform_Log("[CTR Replay] invalid replay header: %s\n", path);
		NativeReplayScheduler_CloseFile();
		return 0;
	}

	s_mode = NATIVE_REPLAY_MODE_PLAYBACK;
	Platform_Log("[CTR Replay] playing input replay: %s frames=%u\n", path, s_header.frameCount);
	return 1;
}

static void NativeReplayScheduler_LogFrameInfo(const char *prefix, const struct NativeReplaySchedulerFrameInfo *info)
{
	Platform_Log("[CTR Replay] %s vsync=%d frameCounter=%d state=%d loading=%d level=%d rng=(mix=0x%08x audio=0x%08x dead=0x%08x,0x%08x adv=0x%08x,0x%08x)\n",
	             prefix, info->frameTimer, info->frameCounter, info->mainGameState, info->loadingStage, info->levelID, info->mixRandomNumber, info->audioRNG,
	             info->deadcoed0, info->deadcoed1, info->advRng0, info->advRng1);
}

static s32 NativeReplayScheduler_FrameInfoMatches(const struct NativeReplaySchedulerFrameInfo *expected, const struct NativeReplaySchedulerFrameInfo *live)
{
	// NOTE(aalhendi): frameTimer is logged but not compared yet; it is still a
	// native pacing counter, not a replay-owned deterministic clock.
	return (expected->frameCounter == live->frameCounter) && (expected->mainGameState == live->mainGameState) &&
	       (expected->loadingStage == live->loadingStage) && (expected->levelID == live->levelID) && (expected->mixRandomNumber == live->mixRandomNumber) &&
	       (expected->audioRNG == live->audioRNG) && (expected->deadcoed0 == live->deadcoed0) && (expected->deadcoed1 == live->deadcoed1) &&
	       (expected->advRng0 == live->advRng0) && (expected->advRng1 == live->advRng1);
}

static void NativeReplayScheduler_ReportDivergence(const struct NativeReplayFrameRecord *expected, const struct NativeReplaySchedulerFrameInfo *live,
                                                   u32 livePadChecksum)
{
	if (s_divergenceLogged != 0)
		return;

	s_divergenceLogged = 1;
	Platform_Log("[CTR Replay] divergence at replay frame %u\n", expected->replayFrame);
	NativeReplayScheduler_LogFrameInfo("expected", &expected->endInfo);
	NativeReplayScheduler_LogFrameInfo("live    ", live);
	Platform_Log("[CTR Replay] expected padChecksum=0x%08x live padChecksum=0x%08x\n", expected->padChecksum, livePadChecksum);
}

int NativeReplayScheduler_ConfigureFromArgs(int argc, char **argv)
{
	const char *recordPath = NativeReplayScheduler_ArgValue(argc, argv, "--record-replay");
	const char *playbackPath = NativeReplayScheduler_ArgValue(argc, argv, "--replay");
	const char *frameLimitText = NativeReplayScheduler_ArgValue(argc, argv, "--replay-frame-limit");

	if (NativeReplayScheduler_ArgMissingValue(argc, argv, "--record-replay") || NativeReplayScheduler_ArgMissingValue(argc, argv, "--replay") ||
	    NativeReplayScheduler_ArgMissingValue(argc, argv, "--replay-frame-limit"))
	{
		Platform_Log("[CTR Replay] missing replay command value\n");
		return 1;
	}

	if ((recordPath != NULL) && (playbackPath != NULL))
	{
		Platform_Log("[CTR Replay] choose either --record-replay or --replay, not both\n");
		return 1;
	}

	if ((frameLimitText != NULL) && !NativeReplayScheduler_ParseU32(frameLimitText, &s_frameLimit))
	{
		Platform_Log("[CTR Replay] invalid --replay-frame-limit value: %s\n", frameLimitText);
		return 1;
	}

	s_replayFrame = 0;
	s_beginOpen = 0;
	s_divergenceLogged = 0;

	if (recordPath != NULL)
		return NativeReplayScheduler_OpenRecord(recordPath) ? 0 : 1;

	if (playbackPath != NULL)
		return NativeReplayScheduler_OpenPlayback(playbackPath) ? 0 : 1;

	return 0;
}

void NativeReplayScheduler_Shutdown(void)
{
	NativeReplayScheduler_CloseFile();
	Platform_InputClearInstalledPadSnapshots();
	s_mode = NATIVE_REPLAY_MODE_NONE;
}

int NativeReplayScheduler_BeginFrame(const struct NativeReplaySchedulerFrameInfo *info)
{
	if ((s_mode == NATIVE_REPLAY_MODE_NONE) || (info == NULL))
		return 0;

	if ((s_frameLimit != 0) && (s_replayFrame >= s_frameLimit))
	{
		if (s_mode == NATIVE_REPLAY_MODE_PLAYBACK)
			Platform_Log("[CTR Replay] playback frame limit reached: frames=%u\n", s_replayFrame);
		return 1;
	}

	if (s_mode == NATIVE_REPLAY_MODE_RECORD)
	{
		memset(&s_pendingRecord, 0, sizeof(s_pendingRecord));
		s_pendingRecord.magic = NATIVE_REPLAY_FRAME_MAGIC;
		s_pendingRecord.replayFrame = s_replayFrame;
		s_pendingRecord.beginInfo = *info;
		if (Platform_InputCapturePadSnapshots(s_pendingRecord.pads, PLATFORM_INPUT_PAD_COUNT) == 0)
		{
			Platform_Log("[CTR Replay] failed to capture input snapshots\n");
			return 1;
		}
		s_pendingRecord.padChecksum = NativeReplayScheduler_PadChecksum(s_pendingRecord.pads);
		s_beginOpen = 1;
		return 0;
	}

	if (s_mode == NATIVE_REPLAY_MODE_PLAYBACK)
	{
		u32 checksum;

		if (s_replayFrame >= s_header.frameCount)
		{
			Platform_Log("[CTR Replay] replay finished after %u frames\n", s_replayFrame);
			return 1;
		}

		if (fread(&s_pendingRecord, sizeof(s_pendingRecord), 1, s_file) != 1)
		{
			Platform_Log("[CTR Replay] failed to read replay frame %u\n", s_replayFrame);
			return 1;
		}

		checksum = NativeReplayScheduler_RecordChecksum(&s_pendingRecord);
		if ((s_pendingRecord.magic != NATIVE_REPLAY_FRAME_MAGIC) || (s_pendingRecord.replayFrame != s_replayFrame) ||
		    (checksum != s_pendingRecord.recordChecksum) || (NativeReplayScheduler_PadChecksum(s_pendingRecord.pads) != s_pendingRecord.padChecksum))
		{
			Platform_Log("[CTR Replay] corrupt replay frame %u\n", s_replayFrame);
			return 1;
		}

		if (Platform_InputInstallPadSnapshots(s_pendingRecord.pads, PLATFORM_INPUT_PAD_COUNT) == 0)
		{
			Platform_Log("[CTR Replay] failed to install replay input frame %u\n", s_replayFrame);
			return 1;
		}

		s_beginOpen = 1;
	}

	return 0;
}

int NativeReplayScheduler_EndFrame(const struct NativeReplaySchedulerFrameInfo *info)
{
	struct PlatformInputPadSnapshot livePads[PLATFORM_INPUT_PAD_COUNT];
	u32 livePadChecksum;

	if ((s_mode == NATIVE_REPLAY_MODE_NONE) || (info == NULL))
		return 0;

	if (s_beginOpen == 0)
		return 0;

	if (Platform_InputCapturePadSnapshots(livePads, PLATFORM_INPUT_PAD_COUNT) == 0)
		livePadChecksum = 0;
	else
		livePadChecksum = NativeReplayScheduler_PadChecksum(livePads);

	if (s_mode == NATIVE_REPLAY_MODE_RECORD)
	{
		s_pendingRecord.endInfo = *info;
		s_pendingRecord.recordChecksum = NativeReplayScheduler_RecordChecksum(&s_pendingRecord);

		if (fwrite(&s_pendingRecord, sizeof(s_pendingRecord), 1, s_file) != 1)
		{
			Platform_Log("[CTR Replay] failed to write replay frame %u\n", s_replayFrame);
			return 1;
		}

		s_header.frameCount++;
		s_replayFrame++;
		s_beginOpen = 0;

		if ((s_frameLimit != 0) && (s_replayFrame >= s_frameLimit))
		{
			Platform_Log("[CTR Replay] recorded input replay frames=%u\n", s_replayFrame);
			NativeReplayScheduler_CloseFile();
			return 1;
		}

		return 0;
	}

	if (s_mode == NATIVE_REPLAY_MODE_PLAYBACK)
	{
		if (!NativeReplayScheduler_FrameInfoMatches(&s_pendingRecord.endInfo, info) || (s_pendingRecord.padChecksum != livePadChecksum))
			NativeReplayScheduler_ReportDivergence(&s_pendingRecord, info, livePadChecksum);

		s_replayFrame++;
		s_beginOpen = 0;
	}

	return 0;
}
#endif
