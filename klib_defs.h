#pragma once

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

//Note broken name _vsnwprintf on Cygwin (and possibly others)
#ifdef WIN32 
#define VWSNPRINTF _vsnwprintf
#else
#define VWSNPRINTF vswprintf
#endif

#ifdef __cplusplus
#define KLIB_BEGIN_DECLS exetern "C" { 
#define KLIB_END_DECLS }
#else
#define KLIB_BEGIN_DECLS 
#define KLIB_END_DECLS 
#endif
