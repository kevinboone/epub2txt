#pragma once

#include <stdarg.h>
#include "klib_defs.h"
#include "klib_object.h"

struct _klib_GetOpt_priv;
struct _klib_Error;
struct _klib_GetOptSpec;

typedef struct _klib_GetOpt
  {
  klib_Object base;
  BOOL disposing;
  struct _klib_GetOpt_priv *priv;
  } klib_GetOpt;

KLIB_BEGIN_DECLS

/** Creates a new klib_GetOpt with no switches set */
klib_GetOpt *klib_getopt_new (void);

/** Frees any memory associated with this string object */
void klib_getopt_free (klib_GetOpt *self);

/** Adds an argument spec */
void klib_getopt_add_spec (klib_GetOpt *self, const char *name,
   const char *longopt, char shortopt, int flags);

/** Adds an argument spec in the form of a GetOptSpec object. NOTE
the GetObj object will _own_ the object supplied, and free it when it
itself is freed. The caller should not free it. */
void klib_getopt_add_spec_object (klib_GetOpt *self, 
    struct _klib_GetOptSpec *spec);

/** Parse the command line against the switches set up with add_spec. */
void klib_getopt_parse (klib_GetOpt *self, int argc, const char **argv, 
    struct _klib_Error **error);

/** Get the number of arguments left over after parsing (that is, the
maximum size of argv */
int klib_getopt_argc (const klib_GetOpt *self);

/** Get a particular left-over argument. Returns null if index is
out of range. */
const char *klib_getopt_argv (const klib_GetOpt *self, int index);

/** Returns true if the argument has a value or, in the case of boolean
switch, it it was specified at all. If the name does not correspond to an
known switch spec, returns FALSE and emits a warning */
BOOL klib_getopt_arg_set (const klib_GetOpt *self, const char *name);

/** Returns the value of the argument set against the named switch, or the
word 'set' if boolean.If the name does not correspond to an
known switch spec, returns NULL and emits a warning */
const char *klib_getopt_get_arg (const klib_GetOpt *self, const char *name);

KLIB_END_DECLS

