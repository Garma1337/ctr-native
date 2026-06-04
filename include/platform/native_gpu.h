/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/src/gpu/PsyX_GPU.h
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#ifndef NATIVE_GPU_H
#define NATIVE_GPU_H

#include <psx/libgte.h>
#include <psx/libgpu.h>
#include <psx/types.h>

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern OT_TAG prim_terminator;

extern int g_splitIndex;
extern DISPENV activeDispEnv;
extern DRAWENV activeDrawEnv;
extern int g_GPUDisabledState;

void ClearSplits(void);
void DrawAllSplits(void);
void ParsePrimitivesLinkedList(u_int *p, int singlePrimitive);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
