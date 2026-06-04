#ifndef NATIVE_PSYX_SHELL_H
#define NATIVE_PSYX_SHELL_H

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern int g_vmode;

int PsyX_Sys_GetVBlankCount(void);
int PsyX_Sys_SetVMode(int mode);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
