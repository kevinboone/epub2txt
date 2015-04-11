#pragma once

#include <stdarg.h>
#include "klib_defs.h"
#include "klib_object.h"

struct _klib_Buffer_priv;
struct _klib_Error;

typedef struct _klib_Buffer
  {
  klib_Object base;
  BOOL disposing;
  struct _klib_Buffer_priv *priv;
  } klib_Buffer;

KLIB_BEGIN_DECLS

/** Creates a new klib_Buffer initially empty. A return value of
NULL indicates that allocation failed. */
klib_Buffer *klib_buffer_new_empty (void);

/** Creates a new klib_Buffer with void* initial value. The buffer will copy
the data, so the caller can, and probably should, free its own copy. */
klib_Buffer *klib_buffer_new (int len, const void *s);

/** Sets the value of this buffer object. Any memory previously 
used is freed. */
void klib_buffer_set (klib_Buffer *self, int len, const void *s);

/** Frees any memory associated with this buffer object */
void klib_buffer_free (klib_Buffer *self);

/** Reads the whole file, assumed to be UTF-8 encoded, into a buffer object */
klib_Buffer *klib_buffer_read_file (const char *filename, 
  struct _klib_Error **error);

/** Reads the whole file, assumed to be UTF-8 encoded, into a buffer object */
klib_Buffer *klib_buffer_read_stream (FILE *stream, 
  struct _klib_Error **error);

/** Sets the buffer to 'null' state and frees any associated data */
void klib_buffer_clear (klib_Buffer *self);

/** Appends len bytes. Returns TRUE on success,
FALSE on OOM */
BOOL klib_buffer_append (klib_Buffer *self, int len, BYTE *c);

/** Appends a single byte. */
BOOL klib_buffer_append_byte (klib_Buffer *self, BYTE c);

/** Returns TRUE if the last len data bytes match c */
BOOL klib_buffer_ends_with (klib_Buffer *self, int len, BYTE *c);

int klib_buffer_get_length (const klib_Buffer *self);
const BYTE *klib_buffer_get_data (const klib_Buffer *self);

KLIB_END_DECLS


