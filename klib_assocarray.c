/*===========================================================================
klib
klib_assocarray.c
(c)2000-2012 Kevin Boone
============================================================================*/

#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "klib_defs.h"
#include "klib_log.h"
#include "klib_assocarray.h"
#include "klib_error.h"
#include "klib_string.h"
#include "klib_list.h"
#include "klib_assocarray.h"
#include "klib_nvpair.h"
#include "klib_object.h"

/*===========================================================================
private data
============================================================================*/
extern void klib_assocarray_init (klib_Object *self);
void klib_assocarray_dispose (klib_Object *self);

static klib_Spec klib_spec_assocarray =
  {
  obj_size: sizeof (klib_AssocArray),
  init_fn: klib_assocarray_init,
  class_name: "klib_AssocArray"
  };

typedef struct _klib_Error_priv
  {
  int code;
  char *str;
  } klib_Error_priv;


typedef struct _klib_AssocArray_priv
  {
  klib_List *entries;
  } klib_AssocArray_priv;


/*===========================================================================
klib_assocarray_new_parse
============================================================================*/
klib_AssocArray *klib_assocarray_new ()
  {
  KLIB_IN
  klib_AssocArray *ret = (klib_AssocArray *)
     klib_object_new (&klib_spec_assocarray);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_assocarray_new_null
============================================================================*/
void klib_assocarray_init (klib_Object *_self)
  {
  KLIB_IN
  klib_object_init (_self);
  _self->dispose = klib_assocarray_dispose;
  klib_AssocArray *self = (klib_AssocArray *)_self;
  self->priv = (klib_AssocArray_priv *)malloc (sizeof (klib_AssocArray_priv));
  memset (self->priv, 0, sizeof (klib_AssocArray_priv));
  self->priv->entries = klib_list_new ();
  KLIB_OUT
  }

/*===========================================================================
klib_assocarray_free
============================================================================*/
void klib_assocarray_free (klib_AssocArray *self)
  {
  KLIB_IN
  klib_object_unref ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_assocarray_dispose
============================================================================*/
void klib_assocarray_dispose (klib_Object *_self)
  {
  KLIB_IN
  klib_AssocArray *self = (klib_AssocArray *)_self;
  if (!self->disposing)
    { 
    self->disposing = TRUE;
    if (self->priv) 
      {
      klib_assocarray_clear (self);
      klib_list_free (self->priv->entries);
      free (self->priv);
      }
    }
  klib_object_dispose ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_assocarray_get_nvpair
============================================================================*/
klib_NVPair *klib_assocarray_get_nvpair (klib_AssocArray *self, const char *key)
  {
  KLIB_IN
  klib_NVPair *ret = NULL;
  int i, l = klib_list_length (self->priv->entries);
  for (i = 0; i < l && !ret; i++)
    {
    klib_NVPair *pair = (klib_NVPair *) klib_list_get (self->priv->entries, i);
    if (strcmp (key, klib_nvpair_get_name (pair)) == 0)
      ret = pair; 
    }
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_assocarray_get
============================================================================*/
void *klib_assocarray_get (klib_AssocArray *self, const char *key)
  {
  KLIB_IN
  void *ret = NULL;
  int i, l = klib_list_length (self->priv->entries);
  for (i = 0; i < l && !ret; i++)
    {
    klib_NVPair *pair = (klib_NVPair *) klib_list_get (self->priv->entries, i);
    if (strcmp (key, klib_nvpair_get_name (pair)) == 0)
      ret = klib_nvpair_get_value (pair);
    }
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_assocarray_get_key
============================================================================*/
const char *klib_assocarray_get_key (const klib_AssocArray *self, 
     int n)
  {
  KLIB_IN
  const char *ret = NULL;
  int l = klib_list_length (self->priv->entries);
  if (n < l)
    {
    klib_NVPair *pair = (klib_NVPair *)klib_list_get (self->priv->entries, n);
    ret = klib_nvpair_get_name (pair);
    }
  else
    {
    klib_log_warning ("Index '%d' out of bounds in assocarray_get_key()", 
      n);
    }
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_assocarray_get_length
============================================================================*/
int klib_assocarray_length (const klib_AssocArray *self)
  {
  KLIB_IN
  int n = klib_list_length (self->priv->entries);
  KLIB_OUT
  return n;
  }

/*===========================================================================
klib_assocarray_get_add_nvpair
============================================================================*/
void klib_assocarray_add_nvpair (klib_AssocArray *self, klib_NVPair *pair)
  {
  KLIB_IN

  klib_list_append (self->priv->entries, (klib_Object *)pair);

  KLIB_OUT
  }

/*===========================================================================
klib_assocarray_get_remove
============================================================================*/
void klib_assocarray_remove (klib_AssocArray *self, const char *key) 
  {
  KLIB_IN
  klib_NVPair *pair = klib_assocarray_get_nvpair (self, key);
  if (pair)
    {
    klib_list_remove (self->priv->entries, (klib_Object *)pair);
    }
  KLIB_OUT
  }

/*===========================================================================
klib_assocarray_get_add
============================================================================*/
void klib_assocarray_add (klib_AssocArray *self, const char *key, 
    klib_Object *value)
  {
  KLIB_IN

  klib_assocarray_remove (self, key);

  klib_NVPair *p = klib_nvpair_new (key, value);
  klib_assocarray_add_nvpair (self, p);
  klib_nvpair_free (p);

  KLIB_OUT
  }

/*===========================================================================
klib_assocarray_clear
============================================================================*/
void klib_assocarray_clear (klib_AssocArray *self)
  {
  KLIB_IN

  klib_list_clear (self->priv->entries);

  KLIB_OUT
  }


