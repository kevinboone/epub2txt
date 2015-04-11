#pragma once

#include <stdarg.h>
#include "klib_defs.h"
#include "klib_object.h"

struct _klib_AssocArray_priv;
struct _klib_Error;
struct _klib_NVPair;

typedef struct _klib_AssocArray
  {
  klib_Object base;
  BOOL disposing;
  struct _klib_AssocArray_priv *priv;
  } klib_AssocArray;

KLIB_BEGIN_DECLS

/** Creates a new, empty klib_AssocArray */
klib_AssocArray *klib_assocarray_new (void);

/** Frees any memory associated with this assoc object. 
 */
void klib_assocarray_free (klib_AssocArray *self);

/** Frees any memory associated with this array object. */
void klib_assocarray_clear (klib_AssocArray *self);

/** Get the specified item in the array */
void *klib_assocarray_get (klib_AssocArray *self, const char *key);

/** Get the number of items in the array */
int klib_assocarray_length (const klib_AssocArray *self);

/** Append an item to the list. Note that this object does not own the 
item, but it copies the name. The caller can free the name, but 
must not free the
item during the lifetime of the array */ 
void klib_assocarray_add (klib_AssocArray *self, const char *key, 
  klib_Object *item);

void klib_assocarray_add_nvpair (klib_AssocArray *self, 
  struct _klib_NVPair *pair);

/** Get the i'th key in the array. Note that the array is not ordered in
any particular way, so this method is only useful in the context of
iterating the whole array */
const char *klib_assocarray_get_key (const klib_AssocArray *self, 
     int n);

/** Remove the specified item from the array */
void klib_assocarray_remove (klib_AssocArray *self, const char *key);

struct _klib_NVPair *klib_assocarray_get_nvpair (klib_AssocArray *self, 
  const char *key);

KLIB_END_DECLS

