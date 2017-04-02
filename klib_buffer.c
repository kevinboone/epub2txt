/*===========================================================================
klib
klib_buffer.c
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
#include <sys/types.h>
#include <sys/stat.h>
#include "klib_log.h"
#include "klib_buffer.h"
#include "klib_error.h"

/*===========================================================================
private data
============================================================================*/
extern void klib_buffer_init (klib_Object *self);
void klib_buffer_dispose (klib_Object *self);

static klib_Spec klib_spec_buffer =
  {
  obj_size: sizeof (klib_Buffer), 
  init_fn: klib_buffer_init,
  class_name: "klib_Buffer"
  };

typedef struct _klib_Buffer_priv
  {
  BYTE *data;
  // Note that a zero-length buffer is legitimate. The presence of some
  //  data is signalled by 'data' being non-null
  int len;
  } klib_Buffer_priv;


/*===========================================================================
klib_buffer_init
============================================================================*/
void klib_buffer_init (klib_Object *_self)
  {
  KLIB_IN
  klib_object_init (_self);
  _self->dispose = klib_buffer_dispose;
  klib_Buffer *self = (klib_Buffer*)_self;
  self->priv = (klib_Buffer_priv *)malloc (sizeof (klib_Buffer_priv));
  memset (self->priv, 0, sizeof (klib_Buffer_priv));
  self->priv->len = 0;
  self->priv->data = malloc(0);
  KLIB_OUT
  }

/*===========================================================================
klib_buffer_set
============================================================================*/
void klib_buffer_set (klib_Buffer *self, int len, const void *data)
  {
  KLIB_IN
  if (self->priv->data)
    free (self->priv->data);
  if (data == NULL) 
    self->priv->data = NULL;
  else
    {
    self->priv->data = malloc (len); 
    self->priv->len = len;
    memcpy (self->priv->data, data, len);
    }
  KLIB_OUT
  }

/*===========================================================================
klib_buffer_clear
============================================================================*/
void klib_buffer_clear (klib_Buffer *self)
  {
  KLIB_IN
  klib_buffer_set (self, 0, NULL);
  KLIB_OUT
  }

/*===========================================================================
klib_buffer_new
============================================================================*/
klib_Buffer *klib_buffer_new (int len, const void *data)
  {
  KLIB_IN
  klib_Buffer *self = (klib_Buffer *)klib_object_new(&klib_spec_buffer); 
  klib_buffer_set (self, len, data);
  KLIB_OUT
  return self;
  }

/*===========================================================================
klib_buffer_new_empty
============================================================================*/
klib_Buffer *klib_buffer_new_empty (void)
  {
  KLIB_IN
  klib_Buffer *self = (klib_Buffer *)klib_object_new(&klib_spec_buffer); 
  klib_buffer_set (self, 0, (void *)"");
  KLIB_OUT
  return self;
  }

/*===========================================================================
klib_buffer_dispose
============================================================================*/
void klib_buffer_dispose (klib_Object *_self)
  {
  KLIB_IN
  klib_Buffer *self = (klib_Buffer *)_self;
  if (!self->disposing)
    {
    self->disposing = TRUE;
    if (self->priv) 
      {
      self->priv->len = 0;
      if (self->priv->data)
        free (self->priv->data);
      free (self->priv);
      }
    }
  klib_object_dispose ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_buffer_free
============================================================================*/
void klib_buffer_free (klib_Buffer *self)
  {
  KLIB_IN
  klib_object_unref ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_buffer_read_file
============================================================================*/
klib_Buffer *klib_buffer_read_file (const char *filename, 
    struct _klib_Error **error)
  {
  KLIB_IN
  klib_Buffer *ret = NULL;
  klib_log_debug ("klib_buffer_read_file: Reading file %s", filename);
  FILE *stream = fopen (filename, "rb");
  if (stream)
    {
    ret = klib_buffer_read_stream (stream, error);
    fclose (stream);
    }
  else
    *error = klib_error_new (ENOENT, "Can't open file for reading: %s\n", 
      filename);
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_buffer_read_stream
//TODO -- alternative character encodings
============================================================================*/
klib_Buffer *klib_buffer_read_stream (FILE *stream, 
    struct _klib_Error **error)
  {
  KLIB_IN
  klib_Buffer *ret = NULL;
  struct stat sb;
  if (fstat (fileno (stream), &sb) == 0)
    {
    void *buff = malloc (sb.st_size);
    read (fileno (stream), buff, sb.st_size);
    ret = klib_buffer_new_empty ();
    // Convert to UTF-8 here
    klib_buffer_set (ret, sb.st_size, buff);
    }
  KLIB_OUT
  return ret; 
  }

/*===========================================================================
klib_buffer_append_byte
============================================================================*/
BOOL klib_buffer_append_byte (klib_Buffer *self, BYTE c)
  {
  KLIB_IN
  BOOL ret = FALSE;

  self->priv->data = realloc (self->priv->data, self->priv->len + 1);
  if (self->priv->data)
    {
    self->priv->data[self->priv->len] = c;  
    self->priv->len++;
    ret = TRUE;
    }

  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_buffer_append
============================================================================*/
BOOL klib_buffer_append (klib_Buffer *self, int len, BYTE *c)
  {
  KLIB_IN
  BOOL ret = FALSE;

  self->priv->data = realloc (self->priv->data, self->priv->len + len);
  if (self->priv->data)
    {
    memcpy (self->priv->data + len, c, len);
    self->priv->len += len;
    ret = TRUE;
    }

  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_buffer_ends_with
============================================================================*/
BOOL klib_buffer_ends_with (klib_Buffer *self, int len, BYTE *c)
  {
  KLIB_IN
  BOOL ret = FALSE;

  ret = (memcmp (self->priv->data + (self->priv->len - len), c, len) == 0);

  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_buffer_get_length
============================================================================*/
int klib_buffer_get_length (const klib_Buffer *self)
  {
  return self->priv->len;
  }

/*===========================================================================
klib_buffer_get_data
============================================================================*/
const BYTE *klib_buffer_get_data (const klib_Buffer *self)
  {
  return self->priv->data;
  }



