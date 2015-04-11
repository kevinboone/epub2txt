#pragma once

#include <stdarg.h>
#include "klib_object.h"

struct _klib_List_priv;
struct _klib_Error;

typedef struct _klib_ListEntry
  {
  klib_Object *data;
  struct _klib_ListEntry *next;
  } klib_ListEntry;

typedef struct _klib_List
  {
  klib_Object base;
  BOOL disposing;
  struct _klib_List_priv *priv;
  } klib_List;

typedef int (*klib_ListComparator) 
  (const klib_Object *o1, const klib_Object *o2);

KLIB_BEGIN_DECLS

/** Creates a new, empty klib_List */
klib_List *klib_list_new (void);

/** Frees any memory associated with this list object. 
 */
void klib_list_free (klib_List *self);

/** Frees any memory associated with this list object, and 
sets it to zero length */
void klib_list_clear (klib_List *self);

/** Get the n'th item in the list */
klib_Object *klib_list_get (klib_List *self, int n);

/** Get the number of items in the list */
int klib_list_length (const klib_List *self);

/** Append an item to the list. Note that this object will add a ref
to the item appended, so the caller can (and perhaps should) unref its
own copy */
void klib_list_append (klib_List *self, klib_Object *item);

/** Get the ListEntry item pointing to the final data element */
klib_ListEntry *klib_list_get_head (klib_List *self);

void klib_list_sort (klib_List *self, klib_ListComparator comp);

/** Set the value of a specific list element. If reref==TRUE, remove
a reference from the old element, and add one to the new one. The only
time you'd probably want reref==FALSE is when moving items within the
list */
void klib_list_set (klib_List *self, int n, klib_Object *o, BOOL reref);

/** Removes the object and unrefs it */
void klib_list_remove (klib_List *self, klib_Object *o);

void klib_list_free (klib_List *self);

KLIB_END_DECLS

