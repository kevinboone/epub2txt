#pragma once

#include "klib_defs.h"

struct _klib_String;
struct _klib_Error;
struct _klib_Object;

// Max length of classname
#define CLASS_NAME_MAX 30

typedef void (*klib_object_init_fn) (struct _klib_Object *self);
typedef void (*klib_object_dispose_fn) (struct _klib_Object *self);
typedef struct _klib_String *(*klib_object_tostring_fn) 
  (const struct _klib_Object *self);

typedef struct klib_Spec
  {
  int obj_size;
  klib_object_init_fn init_fn;
  const char *class_name;
  } klib_Spec;

typedef struct _klib_Object
  {
  char class_name[CLASS_NAME_MAX];
  int ref_count;
  klib_object_dispose_fn dispose;
  klib_object_tostring_fn to_string;
  } klib_Object;


KLIB_BEGIN_DECLS

klib_Object *klib_object_new (const klib_Spec *spec);

void klib_object_init (klib_Object *self);

void klib_object_dispose (klib_Object *self);

void klib_object_unref (klib_Object *self);

struct _klib_String *klib_object_to_string (const klib_Object *self);

void klib_object_add_ref (klib_Object *self);

int klib_object_get_ref_count (const klib_Object *self);

KLIB_END_DECLS


