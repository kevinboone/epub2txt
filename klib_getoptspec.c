/*===========================================================================
klib
klib_getoptspec.c
(c)2000-2012 Kevin Boone
============================================================================*/

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
#include "klib_getoptspec.h"
#include "klib_error.h"
#include "klib_string.h"
#include "klib_list.h"
#include "klib_object.h"

/*===========================================================================
private data
============================================================================*/
extern void klib_getoptspec_init (klib_Object *self);
void klib_getoptspec_dispose (klib_Object *self);

static klib_Spec klib_spec_getoptspec =
  {
  obj_size: sizeof (klib_GetOptSpec), 
  init_fn: klib_getoptspec_init,
  class_name: "klib_GetOptSpec"
  };

typedef struct _klib_GetOptSpec_priv
  {
  char *name;
  char *longopt;
  char *arg;
  char shortopt;
  int flags;
  } klib_GetOptSpec_priv;


/*===========================================================================
klib_getoptspec_new_parse
============================================================================*/
klib_GetOptSpec *klib_getoptspec_new (const char *name, const char * longopt, 
    char shortopt, int flags)
  {
  KLIB_IN
  klib_GetOptSpec *ret = 
    (klib_GetOptSpec *)klib_object_new(&klib_spec_getoptspec);
  ret->priv->name = strdup (name);
  if (longopt) ret->priv->longopt = strdup (longopt);
  ret->priv->shortopt = shortopt;
  ret->priv->flags = flags;
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_getoptspec_new_null
============================================================================*/
void klib_getoptspec_init (klib_Object *_self)
  {
  KLIB_IN
  klib_object_init (_self);
  _self->dispose = klib_getoptspec_dispose;
  klib_GetOptSpec *self = (klib_GetOptSpec *)_self;
  self->priv = (klib_GetOptSpec_priv *)malloc (sizeof (klib_GetOptSpec_priv));
  memset (self->priv, 0, sizeof (klib_GetOptSpec_priv));
  KLIB_OUT
  }

/*===========================================================================
klib_getoptspec_free
============================================================================*/
void klib_getoptspec_free (klib_GetOptSpec *self)
  {
  klib_object_unref ((klib_Object *)self);
  }

/*===========================================================================
klib_getoptspec_dipose
============================================================================*/
void klib_getoptspec_dispose (klib_Object *_self)
  {
  KLIB_IN
  klib_GetOptSpec *self = (klib_GetOptSpec *)_self;
  if (!self->disposing)
    {
    if (self->priv) 
      {
      if (self->priv->name) free (self->priv->name);
      if (self->priv->longopt) free (self->priv->longopt);
      if (self->priv->arg) free (self->priv->arg);
      free (self->priv);
      }
    }
  klib_object_dispose ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_getoptspec_get_longopt
============================================================================*/
const char *klib_getoptspec_get_longopt (const klib_GetOptSpec *self)
  {
  return self->priv->longopt;
  }

/*===========================================================================
klib_getoptspec_get_shortopt
============================================================================*/
const char klib_getoptspec_get_shortopt (const klib_GetOptSpec *self)
  {
  return self->priv->shortopt;
  }

/*===========================================================================
klib_getoptspec_get_name
============================================================================*/
const char *klib_getoptspec_get_name (const klib_GetOptSpec *self)
  {
  return self->priv->name;
  }

/*===========================================================================
klib_getoptspec_get_arg
============================================================================*/
const char *klib_getoptspec_get_arg (const klib_GetOptSpec *self)
  {
  return self->priv->arg;
  }

/*===========================================================================
klib_getoptspec_get_flags
============================================================================*/
int klib_getoptspec_get_flags (const klib_GetOptSpec *self)
  {
  return self->priv->flags;
  }

/*===========================================================================
klib_getoptspec_set_arg
============================================================================*/
void kilb_getoptspec_set_arg (klib_GetOptSpec *self, const char *arg)
  {
  KLIB_IN
  if (self->priv->arg) free (self->priv->arg);
  self->priv->arg = strdup (arg);  
  KLIB_OUT
  }


