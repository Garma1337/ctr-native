#include "platform/native_assets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NATIVE_ASSETS_BIGFILE_PATH               "assets/BIGFILE.BIG"
#define NATIVE_ASSETS_KART_HWL_PATH              "assets/SOUNDS/KART.HWL"
#define NATIVE_ASSETS_TEST_STR_PATH              "assets/TEST.STR"
#define NATIVE_ASSETS_XNF_PATH                   "assets/XA/ENG.XNF"

#define NATIVE_ASSETS_XA_TYPE_COUNT              3
#define NATIVE_ASSETS_XA_HEADER_SIZE             0x44
#define NATIVE_ASSETS_XA_NUM_XAS_TOTAL_OFFSET    0x0c
#define NATIVE_ASSETS_XA_NUM_TRACKS_TOTAL_OFFSET 0x10
#define NATIVE_ASSETS_XA_NUM_SONGS_OFFSET        0x2c
#define NATIVE_ASSETS_XA_FIRST_SONG_INDEX_OFFSET 0x38
#define NATIVE_ASSETS_XA_ENTRY_BYTES             4
#define NATIVE_ASSETS_XA_MAX_FILE_NUMBER         256

struct NativeAssetsByteBuffer
{
	u8 *data;
	int size;
};

static int NativeAssets_FileExists(const char *path)
{
	FILE *file = fopen(path, "rb");

	if (file == NULL)
		return 0;

	fclose(file);
	return 1;
}

static int NativeAssets_ReadExact(FILE *file, void *dst, size_t size)
{
	return fread(dst, 1, size, file) == size;
}

static u32 NativeAssets_ReadLE32(const u8 *data)
{
	return ((u32)data[0]) | ((u32)data[1] << 8) | ((u32)data[2] << 16) | ((u32)data[3] << 24);
}

static int NativeAssets_ReadFileBytes(const char *path, struct NativeAssetsByteBuffer *bytes)
{
	FILE *file;
	long size;

	bytes->data = NULL;
	bytes->size = 0;

	file = fopen(path, "rb");
	if (file == NULL)
		return 0;

	if (fseek(file, 0, SEEK_END) != 0)
	{
		fclose(file);
		return 0;
	}

	size = ftell(file);
	if ((size <= 0) || (size > 0x7fffffff))
	{
		fclose(file);
		return 0;
	}

	if (fseek(file, 0, SEEK_SET) != 0)
	{
		fclose(file);
		return 0;
	}

	bytes->data = (u8 *)malloc((size_t)size);
	if (bytes->data == NULL)
	{
		fclose(file);
		return 0;
	}

	if (!NativeAssets_ReadExact(file, bytes->data, (size_t)size))
	{
		free(bytes->data);
		bytes->data = NULL;
		fclose(file);
		return 0;
	}

	fclose(file);
	bytes->size = (int)size;
	return 1;
}

static void NativeAssets_FreeByteBuffer(struct NativeAssetsByteBuffer *bytes)
{
	free(bytes->data);
	bytes->data = NULL;
	bytes->size = 0;
}

static void NativeAssets_PrintHeader(void)
{
	fprintf(stderr, "[CTR Native] Missing or incomplete assets.\n");
	fprintf(stderr, "[CTR Native] Expected NTSC-U retail assets under ./assets next to the source checkout.\n");
}

static void NativeAssets_PrintFooter(void)
{
	fprintf(stderr, "[CTR Native] Required files: %s, %s, %s, %s, plus XA files referenced by %s\n", NATIVE_ASSETS_BIGFILE_PATH, NATIVE_ASSETS_KART_HWL_PATH,
	        NATIVE_ASSETS_TEST_STR_PATH, NATIVE_ASSETS_XNF_PATH, NATIVE_ASSETS_XNF_PATH);
}

static int NativeAssets_CheckRequiredFile(const char *path)
{
	if (NativeAssets_FileExists(path))
		return 1;

	fprintf(stderr, "[CTR Native] missing asset: %s\n", path);
	return 0;
}

static int NativeAssets_ValidateXA(void)
{
	static const char *xaDirs[NATIVE_ASSETS_XA_TYPE_COUNT] = {
	    "assets/XA/MUSIC",
	    "assets/XA/ENG/EXTRA",
	    "assets/XA/ENG/GAME",
	};
	struct NativeAssetsByteBuffer xnf;
	u8 required[NATIVE_ASSETS_XA_TYPE_COUNT][NATIVE_ASSETS_XA_MAX_FILE_NUMBER];
	u32 numXasTotal;
	u32 numTracksTotal;
	u32 entryOffset;
	u32 entryEnd;
	u32 missing = 0;
	u32 categoryID;

	memset(required, 0, sizeof(required));

	if (!NativeAssets_ReadFileBytes(NATIVE_ASSETS_XNF_PATH, &xnf))
		return 0;

	if ((xnf.size < NATIVE_ASSETS_XA_HEADER_SIZE) || (NativeAssets_ReadLE32(&xnf.data[0]) != 0x464e4958) || (NativeAssets_ReadLE32(&xnf.data[4]) != 102) ||
	    (NativeAssets_ReadLE32(&xnf.data[8]) != NATIVE_ASSETS_XA_TYPE_COUNT))
	{
		NativeAssets_FreeByteBuffer(&xnf);
		fprintf(stderr, "[CTR Native] invalid XA manifest: %s\n", NATIVE_ASSETS_XNF_PATH);
		return 0;
	}

	numXasTotal = NativeAssets_ReadLE32(&xnf.data[NATIVE_ASSETS_XA_NUM_XAS_TOTAL_OFFSET]);
	numTracksTotal = NativeAssets_ReadLE32(&xnf.data[NATIVE_ASSETS_XA_NUM_TRACKS_TOTAL_OFFSET]);
	entryOffset = NATIVE_ASSETS_XA_HEADER_SIZE + numXasTotal * 4u;
	entryEnd = entryOffset + numTracksTotal * NATIVE_ASSETS_XA_ENTRY_BYTES;

	if ((entryEnd < entryOffset) || (entryEnd > (u32)xnf.size))
	{
		NativeAssets_FreeByteBuffer(&xnf);
		fprintf(stderr, "[CTR Native] invalid XA entry table: %s\n", NATIVE_ASSETS_XNF_PATH);
		return 0;
	}

	for (categoryID = 0; categoryID < NATIVE_ASSETS_XA_TYPE_COUNT; categoryID++)
	{
		u32 numSongs = NativeAssets_ReadLE32(&xnf.data[NATIVE_ASSETS_XA_NUM_SONGS_OFFSET + categoryID * 4u]);
		u32 firstSongIndex = NativeAssets_ReadLE32(&xnf.data[NATIVE_ASSETS_XA_FIRST_SONG_INDEX_OFFSET + categoryID * 4u]);
		u32 xaID;

		if ((firstSongIndex > numTracksTotal) || (numSongs > numTracksTotal) || (firstSongIndex + numSongs > numTracksTotal))
		{
			NativeAssets_FreeByteBuffer(&xnf);
			fprintf(stderr, "[CTR Native] invalid XA song range in %s\n", NATIVE_ASSETS_XNF_PATH);
			return 0;
		}

		for (xaID = 0; xaID < numSongs; xaID++)
		{
			const u8 *entry = &xnf.data[entryOffset + (firstSongIndex + xaID) * NATIVE_ASSETS_XA_ENTRY_BYTES];
			required[categoryID][entry[1]] = 1;
		}
	}

	NativeAssets_FreeByteBuffer(&xnf);

	for (categoryID = 0; categoryID < NATIVE_ASSETS_XA_TYPE_COUNT; categoryID++)
	{
		u32 fileNumber;

		for (fileNumber = 0; fileNumber < NATIVE_ASSETS_XA_MAX_FILE_NUMBER; fileNumber++)
		{
			char path[256];
			int written;

			if (!required[categoryID][fileNumber])
				continue;

			written = snprintf(path, sizeof(path), "%s/S%02u.XA", xaDirs[categoryID], (unsigned int)fileNumber);
			if ((written <= 0) || ((size_t)written >= sizeof(path)) || !NativeAssets_FileExists(path))
			{
				fprintf(stderr, "[CTR Native] missing XA asset: %s\n", path);
				missing++;
			}
		}
	}

	return missing == 0;
}

int NativeAssets_Validate(void)
{
	int ok = 1;

	ok &= NativeAssets_CheckRequiredFile(NATIVE_ASSETS_BIGFILE_PATH);
	ok &= NativeAssets_CheckRequiredFile(NATIVE_ASSETS_KART_HWL_PATH);
	ok &= NativeAssets_CheckRequiredFile(NATIVE_ASSETS_TEST_STR_PATH);
	ok &= NativeAssets_CheckRequiredFile(NATIVE_ASSETS_XNF_PATH);

	if (ok)
	{
		ok &= NativeAssets_ValidateXA();
	}

	if (!ok)
	{
		NativeAssets_PrintHeader();
		NativeAssets_PrintFooter();
	}

	return ok;
}
