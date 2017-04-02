/*===========================================================================
klib
klib_object.c
(c)2000-2012 Kevin Boone
============================================================================*/

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <stdio.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "klib_defs.h"
#include "klib_log.h"
#include "klib_object.h"
#include "klib_string.h"

/*===========================================================================
klib_object_to_string
============================================================================*/
klib_String *_klib_object_to_string (const klib_Object *self)
  {
  return klib_string_new (self->class_name);
  }

/*===========================================================================
klib_object_to_string
============================================================================*/
klib_String *klib_object_to_string (const klib_Object *self)
  {
  KLIB_IN
  klib_String *ret = self->to_string (self);
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_object_init
============================================================================*/
void klib_object_init (klib_Object *self)
  {
  KLIB_IN
  self->to_string = _klib_object_to_string;
  KLIB_OUT
  }

/*===========================================================================
klib_object_dispose
============================================================================*/
void klib_object_dispose (klib_Object *self)
  {
  KLIB_IN
  if (self) free (self);
  KLIB_OUT
  }

/*===========================================================================
klib_object_unref
============================================================================*/
void klib_object_unref (klib_Object *self)
  {
  KLIB_IN
  if (self) 
    {
    self->ref_count--;
    if (self->ref_count < 0)
      klib_log_error ("Negative ref count in klib_object_unref: %s", 
        self->class_name);
    if (self->ref_count == 0)
      self->dispose (self);
    }
  KLIB_OUT
  }

/*===========================================================================
klib_object_new
============================================================================*/
klib_Object *klib_object_new (const klib_Spec *spec)
  {
  klib_Object *self = malloc (spec->obj_size);
  memset (self, 0, spec->obj_size); 
  strncpy (self->class_name, spec->class_name, CLASS_NAME_MAX - 2);
  spec->init_fn (self); 
  self->ref_count++;
  return self;
  }


/*===========================================================================
klib_object_add_ref
============================================================================*/
void klib_object_add_ref (klib_Object *self)
  {
  KLIB_IN
  self->ref_count++;
  KLIB_OUT
  }

/*===========================================================================
klib_object_get_ref_count
============================================================================*/
int klib_object_get_ref_count (const klib_Object *self)
  {
  KLIB_IN
  return self->ref_count;
  KLIB_OUT
  }



