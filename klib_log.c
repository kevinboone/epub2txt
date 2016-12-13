/*===========================================================================
klib
klib_log.c
(c)2000-2012 Kevin Boone
============================================================================*/

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "klib_log.h"
#include "klib_string.h"

static int klib_log_level = KLIB_LOG_ERROR;
static KlibLogHandler klib_log_handler = NULL;

/*===========================================================================
klib_log_set_handler
============================================================================*/
void klib_log_set_handler (KlibLogHandler handler)
  {
  klib_log_handler = handler;
  }

/*===========================================================================
klib_log_set_level
============================================================================*/
void klib_log_set_level (int level)
  {
  klib_log_level = level;
  }

/*===========================================================================
klib_log_v
============================================================================*/
void klib_log_v (int level, const char *fmt, va_list ap)
  {
  if (level > klib_log_level) return;
  const char *s = klib_string_format_args (fmt, ap); 
  if (klib_log_handler)
    klib_log_handler (level, s);
  else
    fprintf (stderr, "%s\n", s);
  }


/*===========================================================================
klib_log_info
============================================================================*/
void klib_log_info (const char *fmt, ...)
  {
  va_list ap;
  va_start (ap, fmt);
  klib_log_v (KLIB_LOG_INFO, fmt, ap);
  va_end (ap);
  }

/*===========================================================================
klib_log_error
============================================================================*/
void klib_log_error (const char *fmt, ...)
  {
  va_list ap;
  va_start (ap, fmt);
  klib_log_v (KLIB_LOG_ERROR, fmt, ap);
  va_end (ap);
  }

/*===========================================================================
klib_log_warning
============================================================================*/
void klib_log_warning (const char *fmt, ...)
  {
  va_list ap;
  va_start (ap, fmt);
  klib_log_v (KLIB_LOG_WARNING, fmt, ap);
  va_end (ap);
  }

/*===========================================================================
klib_log_debug
============================================================================*/
void klib_log_debug (const char *fmt, ...)
  {
  va_list ap;
  va_start (ap, fmt);
  klib_log_v (KLIB_LOG_DEBUG, fmt, ap);
  va_end (ap);
  }


/*===========================================================================
klib_log_trace
============================================================================*/
void klib_log_trace (const char *fmt, ...)
  {
  va_list ap;
  va_start (ap, fmt);
  klib_log_v (KLIB_LOG_TRACE, fmt, ap);
  va_end (ap);
  }


