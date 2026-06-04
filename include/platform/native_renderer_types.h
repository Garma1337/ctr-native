/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/include/PsyX/PsyX_render.h
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#ifndef NATIVE_RENDERER_TYPES_H
#define NATIVE_RENDERER_TYPES_H

#include <PsyX/PsyX_config.h>
#include <psx/libgte.h>
#include <psx/libgpu.h>
#include <psx/types.h>

#if (defined(_WIN32) || defined(__APPLE__) || defined(__linux__)) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__) && !defined(__RPI__)
#define RENDERER_OGL
#define USE_GLAD
#elif defined(__RPI__)
#define RENDERER_OGLES
#define OGLES_VERSION (3)
#elif defined(__EMSCRIPTEN__)
#define RENDERER_OGLES
#define OGLES_VERSION (2)
#elif defined(__ANDROID__)
#define RENDERER_OGLES
#define OGLES_VERSION (3)
#endif

#ifndef OGLES_VERSION
#define OGLES_VERSION (0)
#endif

#if defined(RENDERER_OGL) || defined(RENDERER_OGLES)
#define USE_OPENGL 1
#else
#define USE_OPENGL 0
#endif

#if OGLES_VERSION == 2
#define ES2_SHADERS
#elif OGLES_VERSION == 3
#define ES3_SHADERS
#endif

#define LUT_WIDTH              (256)
#define LUT_HEIGHT             (256)

#define VRAM_WIDTH             (1024)
#define VRAM_HEIGHT            (512)

#define TPAGE_WIDTH            (256)
#define TPAGE_HEIGHT           (256)

#define MAX_VERTEX_BUFFER_SIZE (1 << (sizeof(ushort) * 8))

#pragma pack(push, 1)
typedef struct
{
#if USE_PGXP
	float x, y, page, clut;
	float z, scr_h, ofsX, ofsY;
#else
	short x, y, page, clut;
#endif

	u_char u, v, bright, dither;
	u_char r, g, b, a;

	char tcx, tcy, _p0, _p1;
} GrVertex;
#pragma pack(pop)

typedef enum
{
	a_position,
	a_zw,
	a_texcoord,
	a_color,
	a_extra,
} ShaderAttrib;

typedef enum
{
	BM_NONE,
	BM_AVERAGE,
	BM_ADD,
	BM_SUBTRACT,
	BM_ADD_QUATER_SOURCE
} BlendMode;

typedef enum
{
	TF_4_BIT,
	TF_8_BIT,
	TF_16_BIT,

	TF_32_BIT_RGBA
} TexFormat;

typedef uint TextureID;
typedef uint ShaderID;

#endif
