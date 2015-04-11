#pragma once

#include "klib_defs.h"

#define KLIB_LOG_ERROR 0
#define KLIB_LOG_WARNING 1
#define KLIB_LOG_INFO 2
#define KLIB_LOG_DEBUG 3
#define KLIB_LOG_TRACE 4 

#define KLIB_IN klib_log_trace ("Entering %s", __PRETTY_FUNCTION__);
#define KLIB_OUT klib_log_trace ("Leaving %s", __PRETTY_FUNCTION__);

typedef void (*KlibLogHandler)(int level, const char *message);

KLIB_BEGIN_DECLS

/** Log a message at INFO level */
void klib_log_info (const char *fmt,...);

/** Log a message at ERROR level */
void klib_log_error (const char *fmt,...);

/** Log a message at WARNING level */
void klib_log_warning (const char *fmt,...);

/** Log a message at DEBUG level */
void klib_log_debug (const char *fmt,...);

/** Log a message at TRACE level */
void klib_log_trace (const char *fmt,...);

/** Set the overal log level to one of the KLIB_LOG_XXX values */
void klib_log_set_level (int level);

void klib_log_set_handler (KlibLogHandler handler);

KLIB_END_DECLS



