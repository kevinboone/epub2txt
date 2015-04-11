#pragma once

#include "klib_object.h"

#define KLIB_ERR_PARSE_CMDLINE 100
#define KLIB_ERR_PARSE_XML     101
#define  KLIB_ERR_LAST         200
#define  KLIB_ERR_FIRST        100 

struct _klib_Error_priv *priv;

typedef struct _klib_Error
  {
  klib_Object base;
  BOOL disposing;
  struct _klib_Error_priv *priv;
  } klib_Error;

KLIB_BEGIN_DECLS

/** Create a new error object with specific code and text */
klib_Error *klib_error_new (int code, const char *fmt,...);

/** Free this oject */
void klib_error_free (klib_Error *self);

// strerror function is really intended for use only by other klib functions
const char *klib_error_strerror (int code);

// Return a string representation of the error 
const char *klib_error_cstr (const klib_Error *self);

int klib_error_get_code (const klib_Error *self);

KLIB_END_DECLS


