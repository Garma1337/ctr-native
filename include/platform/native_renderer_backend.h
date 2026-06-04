/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/include/PsyX/PsyX_render.h
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#ifndef NATIVE_RENDERER_BACKEND_H
#define NATIVE_RENDERER_BACKEND_H

#include <platform/native_renderer_types.h>

#if !defined(__cplusplus)
#include <stdbool.h>
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

int NativeRendererBackend_InitialiseRender(char *windowName, int width, int height, int fullscreen);
int NativeRendererBackend_InitialisePSX(void);
void NativeRendererBackend_Shutdown(void);
void NativeRendererBackend_ResetDevice(void);
void NativeRendererBackend_BeginScene(void);
void NativeRendererBackend_EndScene(void);
void NativeRendererBackend_UpdateSwapIntervalState(int swapInterval);
void NativeRendererBackend_SwapWindow(void);
void NativeRendererBackend_StoreFrameBuffer(int x, int y, int w, int h);
void NativeRendererBackend_PresentVRAMDisplay(void);
void NativeRendererBackend_SaveVRAM(const char *outputFileName, int x, int y, int width, int height, int readFromFramebuffer);
void NativeRendererBackend_Clear(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);
void NativeRendererBackend_ClearVRAM(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);
void NativeRendererBackend_CopyVRAM(unsigned short *src, int x, int y, int w, int h, int dstX, int dstY);
void NativeRendererBackend_ReadVRAM(unsigned short *dst, int x, int y, int dstW, int dstH);
void NativeRendererBackend_UpdateVRAM(void);
void NativeRendererBackend_ReadFramebufferDataToVRAM(void);
TextureID NativeRendererBackend_GetVRAMTexture(void);
TextureID NativeRendererBackend_GetWhiteTexture(void);
TextureID NativeRendererBackend_CreateRGBATexture(int width, int height, u_char *data);
ShaderID NativeRendererBackend_Shader_Compile(const char *source, bool isPsxShader);
void NativeRendererBackend_SetShader(const ShaderID shader);
void NativeRendererBackend_Perspective3D(const float fov, const float width, const float height, const float zNear, const float zFar);
void NativeRendererBackend_Ortho2D(float left, float right, float bottom, float top, float znear, float zfar);
void NativeRendererBackend_SetBlendMode(BlendMode blendMode);
void NativeRendererBackend_SetPolygonOffset(float ofs);
void NativeRendererBackend_SetStencilMode(int drawPrim);
void NativeRendererBackend_EnableDepth(int enable);
void NativeRendererBackend_SetScissorState(int enable);
void NativeRendererBackend_SetOffscreenState(const RECT16 *offscreenRect, int enable);
void NativeRendererBackend_SetupClipMode(const RECT16 *clipRect, int enable);
void NativeRendererBackend_SetViewPort(int x, int y, int width, int height);
void NativeRendererBackend_SetTexture(TextureID texture, TexFormat texFormat);
void NativeRendererBackend_SetOverrideTextureSize(int width, int height);
void NativeRendererBackend_SetPSXTextureSemiTransPass(int pass);
void NativeRendererBackend_SetPSXDrawMaskSet(int maskSet);
void NativeRendererBackend_SetWireframe(int enable);
void NativeRendererBackend_DestroyTexture(TextureID texture);
void NativeRendererBackend_UpdateVertexBuffer(const GrVertex *vertices, int count);
void NativeRendererBackend_DrawTriangles(int startVertex, int triangles);
void NativeRendererBackend_PushDebugLabel(const char *label);
void NativeRendererBackend_PopDebugLabel(void);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
