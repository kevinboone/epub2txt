#pragma once

#include <stdarg.h>
#include "klib_defs.h"
#include "klib_object.h"

struct _klib_NVPair_priv;
struct _klib_Error;

/** NVPair is a name-value pair. The name (a klib_String) is managed by
the object; the value (a void*) can be managed by this object if required. */
typedef struct _klib_NVPair
  {
  klib_Object base;
  BOOL disposing;
  struct _klib_NVPair_priv *priv;
  } klib_NVPair;

KLIB_BEGIN_DECLS

/** Creates a new klib_NVPair from the supplied values */
klib_NVPair *klib_nvpair_new (const char *name,  klib_Object *value);

/* Unefs this object */
void klib_nvpair_free (klib_NVPair *self);

const char *klib_nvpair_get_name (const klib_NVPair *self);
klib_Object *klib_nvpair_get_value (const klib_NVPair *self);

KLIB_END_DECLS

