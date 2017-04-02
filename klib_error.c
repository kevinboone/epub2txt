/*===========================================================================
klib
klib_error.c
(c)2000-2012 Kevin Boone
============================================================================*/

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <string.h>
#include "klib_string.h"
#include "klib_error.h"

/*===========================================================================
private data
============================================================================*/
extern void klib_error_init (klib_Object *self);
void klib_error_dispose (klib_Object *self);

static klib_Spec klib_spec_error =
  {
  obj_size: sizeof (klib_Error), 
  init_fn: klib_error_init,
  class_name: "klib_Error"
  };

typedef struct _klib_Error_priv
  {
  int code;
  char *str;
  } klib_Error_priv;


/*===========================================================================
klib_error_to_string
============================================================================*/
klib_String *klib_error_to_string (const klib_Object *self)
  {
  return klib_string_new (((klib_Error *)self)->priv->str);
  }

/*===========================================================================
klib_error_init
============================================================================*/
void klib_error_init (klib_Object *_self)
  {
  klib_object_init (_self);
  _self->dispose = klib_error_dispose;
  _self->to_string = klib_error_to_string;
  klib_Error *self = (klib_Error *)_self;
  self->priv = (klib_Error_priv *)malloc (sizeof (klib_Error_priv));
  memset (self->priv, 0, sizeof (klib_Error_priv));
  }

/*===========================================================================
klib_error_new
============================================================================*/
klib_Error *klib_error_new (int code, const char *fmt,...)
  {
  klib_Error *self = (klib_Error *)klib_object_new (&klib_spec_error);
  va_list ap;
  va_start (ap, fmt);
  self->priv->str = klib_string_format_args (fmt, ap); 
  va_end (ap);
  self->priv->code = code;
  return self;
  }

/*===========================================================================
klib_error_dispose
============================================================================*/
void klib_error_dispose (klib_Object *_self)
  {
  klib_Error *self = (klib_Error *)_self;
  if (!self->disposing)
    {
    self->disposing = TRUE;
    if (self->priv->str) 
        free (self->priv->str);
    free (self->priv);
    }
  klib_object_dispose (_self);
  }

/*===========================================================================
klib_error_free
============================================================================*/
void klib_error_free (klib_Error *self)
  {
  klib_object_unref ((klib_Object *)self);
  }

/*===========================================================================
klib_error_strerror
============================================================================*/
const char *klib_error_strerror (int code)
  {
  if (code < KLIB_ERR_FIRST) return strerror (code);
  switch (code)
    {
    case KLIB_ERR_PARSE_CMDLINE: return "Command line syntax error: %s";
    }
  return "Unknown error";
  }

/*===========================================================================
klib_error_cstr
============================================================================*/
const char *klib_error_cstr (const klib_Error *self) 
  {
  return self->priv->str;
  }

/*===========================================================================
klib_error_get_code
============================================================================*/
int klib_error_get_code (const klib_Error *self) 
  {
  return self->priv->code;
  }



