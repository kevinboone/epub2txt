/*===========================================================================
klib
klib_list.c
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
#include "klib_list.h"
#include "klib_error.h"
#include "klib_string.h"
#include "klib_list.h"

/*===========================================================================
private data
============================================================================*/
extern void klib_list_init (klib_Object *self);
void klib_list_dispose (klib_Object *self);

static klib_Spec klib_spec_list =
  {
  obj_size: sizeof (klib_List), 
  init_fn: klib_list_init,
  class_name: "klib_List"
  };

typedef struct _klib_List_priv
  {
  klib_ListEntry *tail;
  } klib_List_priv;


/*===========================================================================
klib_list_new_parse
============================================================================*/
klib_List *klib_list_new ()
  {
  KLIB_IN
  klib_List *ret = (klib_List *)klib_object_new(&klib_spec_list);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_list_init
============================================================================*/
void klib_list_init (klib_Object *_self)
  {
  KLIB_IN
  klib_object_init (_self);
  _self->dispose = klib_list_dispose;
  klib_List *self = (klib_List *)_self;
  self->priv = (klib_List_priv *)malloc (sizeof (klib_List_priv));
  memset (self->priv, 0, sizeof (klib_List_priv));
  KLIB_OUT
  }

/*===========================================================================
klib_list_dispose
============================================================================*/
void klib_list_dispose (klib_Object *_self)
  {
  KLIB_IN
  klib_List *self = (klib_List *)_self;
  if (!self->disposing) 
    { 
    self->disposing = TRUE;
    if (self->priv) 
      {
      klib_list_clear (self);
      free (self->priv);
      }
    }
  klib_object_dispose (_self);
  KLIB_OUT
  }

/*===========================================================================
klib_list_free
============================================================================*/
void klib_list_free (klib_List *self)
  {
  KLIB_IN
  klib_object_unref ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_list_get
============================================================================*/
klib_Object *klib_list_get (klib_List *self, int n)
  {
  KLIB_IN
  void *ret = NULL;
  int count = 0;
  klib_ListEntry *p = self->priv->tail;
  if (p)
    {
    do
      {
      if (n == count)
        ret = p->data;
      else
        {
        p = p->next;
        count++;
        }
      } while (p && !ret);
    }

  if (ret == NULL)
    klib_log_warning ("List index %d out of bounds in list_get()", n);

  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_list_set
============================================================================*/
void klib_list_set (klib_List *self, int n, klib_Object *o, BOOL reref)
  {
  KLIB_IN
  void *ret = NULL;
  int count = 0;
  klib_ListEntry *p = self->priv->tail;
  if (p)
    {
    do
      {
      if (n == count)
        {
        ret = p->data;
        p->data = o;
        }
      else
        {
        p = p->next;
        count++;
        }
      } while (p && !ret);
    }

  if (ret == NULL)
    klib_log_warning ("List index %d out of bounds in list_get()", n);
  else
    {
    if (reref)
      {
      klib_object_add_ref (o);
      klib_object_unref (ret);
      }
    }

  KLIB_OUT
  }

/*===========================================================================
klib_list_get_head
============================================================================*/
klib_ListEntry *klib_list_get_head (klib_List *self)
  {
  KLIB_IN
  klib_ListEntry *ret = NULL;
  if (self->priv->tail)
    {
    ret = self->priv->tail;
    while (ret->next)
      {
      ret = ret->next;
      } 
    }

  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_list_get_length
============================================================================*/
int klib_list_length (const klib_List *self)
  {
  KLIB_IN
  int n = 0;
  klib_ListEntry *p = self->priv->tail;
  if (p)
    {
    do
      {
      p = p->next;
      n++;
      } while (p);
    }

  KLIB_OUT
  return n;
  }

/*===========================================================================
klib_list_get_append
============================================================================*/
void klib_list_append (klib_List *self, klib_Object *item)
  {
  KLIB_IN

  klib_ListEntry *this_entry = 
    malloc (sizeof (klib_ListEntry));
  
  this_entry->data = item;
  this_entry->next = NULL;
  klib_object_add_ref (this_entry->data);

  klib_ListEntry *head = klib_list_get_head (self);
  if (head == NULL)
    self->priv->tail = this_entry;
  else 
    head->next = this_entry; 

  KLIB_OUT
  }

/*===========================================================================
klib_list_clear
============================================================================*/
void klib_list_clear (klib_List *self)
  {
  KLIB_IN
  klib_ListEntry *p = self->priv->tail;
  if (p)
    {
    do
      {
      klib_ListEntry *temp = p;
      p = p->next;
      if (temp) 
        {
        if (temp->data) klib_object_unref (temp->data);
        free (temp);
        }
      } while (p);
    }

  KLIB_OUT
  self->priv->tail = NULL;
  }

/*===========================================================================
klib_list_sort
Trivial exchange sort. OK for small lists
============================================================================*/
void klib_list_sort (klib_List *self, klib_ListComparator comp)
  {
  int i, j, l = klib_list_length (self);
  for (i = 0; i < l; i++)
    {
    for (j = 0; j < l - 1; j++)
      {
      klib_Object *o1 = klib_list_get (self, j); 
      klib_Object *o2 = klib_list_get (self, j+ 1); 
      if (comp (o1, o2) > 0)
        {
        // Swap
        klib_Object *temp = o1;
        klib_list_set (self, j, o2, FALSE);
        klib_list_set (self, j + 1, temp, FALSE);
        }
      }
    }
  }

/*===========================================================================
klib_list_remove
Removes the object and unrefs it
============================================================================*/
void klib_list_remove (klib_List *self, klib_Object *o)
  {
  KLIB_IN
  klib_ListEntry *p = self->priv->tail;
  if (p)
    {
    if (p->data == o)
      {
      self->priv->tail = self->priv->tail->next;
      klib_object_unref (o);
      free (self->priv->tail);
      }
    }
  do
    {
    klib_ListEntry *pnext = p->next;
    if (pnext->data == o)
      {
      klib_object_unref (o);
      free (pnext);
      p->next = pnext->next;
      } 
    p = p->next;      
    } while (p != NULL);

  KLIB_OUT
  }



