#pragma once


#include <stdarg.h>
#include "klib_defs.h"
#include "klib_object.h"
#include "sxmlc.h" 

struct _klib_Xml_priv;
struct _klib_Error;
struct _klib_Buffer;

typedef struct _klib_Xml
  {
  klib_Object base;
  BOOL disposing;
  struct _klib_Xml_priv *priv;
  } klib_Xml;

KLIB_BEGIN_DECLS

/** Creates a new klib_Xml with null initial value*/
klib_Xml *klib_xml_new (void);

/** Frees any memory associated with this string object */
void klib_xml_free (klib_Xml *self);

/** Reads and parses a whole file */
klib_Xml *klib_xml_read_file (const char *filename, struct _klib_Error **error);

/** Returns the root node of the current XML document, or null if the
document is empty */
XMLNode *klib_xml_get_root (klib_Xml *self);

/** Removes XML unicode escapes, etc., from the string. This function 
will always return a value, but it might be a null klib_String if the
input is null. In any case, the return value will _always_ be something the
caller has to free */
struct _klib_String *klib_xml_unescape_string (const char *s);

/* Reads and parses the specified buffer. The buffname argument is
the arbitrary name of the buffer, used for error messages. */
klib_Xml *klib_xml_read_buffer (const struct _klib_Buffer *buffer, 
     const char *buffname, struct _klib_Error **error);

KLIB_END_DECLS


