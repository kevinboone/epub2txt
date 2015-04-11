#pragma once

#include <stdarg.h>
#include "klib_defs.h"
#include "klib_object.h"

struct _klib_GetOptSpec_priv;
struct _klib_Error;

#define KLIB_GETOPT_NOARG    1 
#define KLIB_GETOPT_COMPARG  2 

// A string value to set in a boolean switch to indicate
//  that it was set. If unset, get_arg == NULL
#define KLIB_GETOPT_ARGSET  "set"

typedef struct _klib_GetOptSpec
  {
  klib_Object base;
  BOOL disposing;
  struct _klib_GetOptSpec_priv *priv;
  } klib_GetOptSpec;

KLIB_BEGIN_DECLS

/** Creates a new klib_GetOptSpec from the supplied values */
klib_GetOptSpec *klib_getoptspec_new (const char *name, const char * longopt, 
  char shortopt, int flags);

/** Frees any memory associated with this string object */
void klib_getoptspec_free (klib_GetOptSpec *self);

const char *klib_getoptspec_get_longopt (const klib_GetOptSpec *self);
const char klib_getoptspec_get_shortopt (const klib_GetOptSpec *self);
const char *klib_getoptspec_get_name (const klib_GetOptSpec *self);
int klib_getoptspec_get_flags (const klib_GetOptSpec *self);

/** The 'arg' property is an aribitrary string with get and set methods.
It is typically used to associate this GetOptSpec object with a command
line argument entered by the user */
void kilb_getoptspec_set_arg (klib_GetOptSpec *self, const char *arg);
const char *klib_getoptspec_get_arg (const klib_GetOptSpec *self);

KLIB_END_DECLS

