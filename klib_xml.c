/*===========================================================================
klib
klib_xml.c
(c)2000-2016 Kevin Boone
============================================================================*/

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "klib_log.h"
#include "klib_xml.h"
#include "klib_error.h"
#include "klib_buffer.h"
#include "klib_wstring.h"
#include "sxmlc.h"

/*===========================================================================
private data
============================================================================*/
extern void klib_xml_init (klib_Object *self);
void klib_xml_dispose (klib_Object *self);

static klib_Spec klib_spec_xml =
  {
  obj_size: sizeof (klib_Xml), 
  init_fn: klib_xml_init,
  class_name: "klib_Xml"
  };

typedef struct _klib_Xml_priv
  {
  XMLDoc doc;
  } klib_Xml_priv;


/*===========================================================================
klib_xml_init
============================================================================*/
void klib_xml_init (klib_Object *_self)
  {
  KLIB_IN
  klib_object_init (_self);
  _self->dispose = klib_xml_dispose;
  klib_Xml *self = (klib_Xml *)_self;
  self->priv = (klib_Xml_priv *)malloc (sizeof (klib_Xml_priv));
  memset (self->priv, 0, sizeof (klib_Xml_priv));
  XMLDoc_init (&self->priv->doc);
  KLIB_OUT
  }


/*===========================================================================
klib_xml_new
============================================================================*/
klib_Xml *klib_xml_new (void)
  {
  KLIB_IN
  klib_Xml *self = (klib_Xml *)klib_object_new (&klib_spec_xml);
  KLIB_OUT
  return self;
  }

/*===========================================================================
klib_xml_dispose
============================================================================*/
void klib_xml_dispose (klib_Object *_self)
  {
  KLIB_IN
  klib_Xml *self = (klib_Xml*)_self;
  if (!self->disposing) 
    { 
    self->disposing = TRUE;
    if (self->priv) 
      {
      XMLDoc_free (&self->priv->doc);
      free (self->priv);
      }
    }
  klib_object_dispose (_self);
  KLIB_OUT
  }

/*===========================================================================
klib_xml_free
============================================================================*/
void klib_xml_free (klib_Xml *self)
  {
  KLIB_IN
  klib_object_unref ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_xml_get_root
============================================================================*/
XMLNode *klib_xml_get_root (klib_Xml *self)
  {
  return XMLDoc_root (&self->priv->doc);
  }

/*===========================================================================
klib_xml_read_file
============================================================================*/
klib_Xml *klib_xml_read_file (const char *filename, struct _klib_Error **error)
  {
  KLIB_IN
  klib_log_debug ("klib_xml_read_file: Reading file %s", filename);
  klib_Xml *ret = NULL; 
  if (1)
    {
    FILE *f = fopen (filename, "r");
    if (f)
      {
      fclose (f);
        
      ret = (klib_Xml *)klib_object_new (&klib_spec_xml);

      if (XMLDoc_parse_file_DOM (filename, &ret->priv->doc))
        {
        }
      else
        {
        *error = klib_error_new 
          (KLIB_ERR_PARSE_XML, klib_error_strerror (KLIB_ERR_PARSE_XML), 
            filename);
        klib_xml_free (ret);
        ret = NULL;
        }
      }
    else
      {
      *error = klib_error_new (ENOENT, "%s: %s", filename, klib_error_strerror (ENOENT));
      }
    }
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_xml_read_buffer
// At present, we assume the buffer is full of utf-8 data
============================================================================*/
klib_Xml *klib_xml_read_buffer (const klib_Buffer *buffer, 
     const char *buffname, klib_Error **error)
  {
  KLIB_IN
  klib_log_debug ("klib_xml_read_buffer");
  klib_Xml *ret = NULL; 

  ret = (klib_Xml *)klib_object_new(&klib_spec_xml);

  // TODO read encoding from the buffer
  klib_String *s = klib_string_new_from_buffer (buffer, error);
  if (!*error)
    {
    if (XMLDoc_parse_buffer_DOM (klib_string_cstr(s), buffname, 
        &ret->priv->doc))
     {
     }
    else
     {
     *error = klib_error_new 
          (KLIB_ERR_PARSE_XML, klib_error_strerror (KLIB_ERR_PARSE_XML), 
            buffname);
     klib_xml_free (ret);
     ret = NULL;
     }
    }
  klib_string_free (s);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
_klib_xml_replace_unicode
============================================================================*/
klib_String *_klib_xml_replace_unicode (const char *pattern, void *userdata)
  {
  KLIB_IN
  klib_String *ret = NULL;
  if (pattern)
    {
    if (strlen (pattern) >= 1)
      {
      int num = '?';
      if (pattern[0] == 'x' || pattern[0] == 'X')
        {
        // Hex number
        sscanf (pattern + 1, "%x", &num); 
        }
      else
        {
        // Assume decimal
        sscanf (pattern , "%d", &num); 
        }
      ret = klib_string_new_empty();
      klib_string_append_unicode_char (ret, num);
      }
    else
      ret = klib_string_new ("?");
    }
  else
    ret = klib_string_new ("?");
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_xml_unescape_string
============================================================================*/
klib_String *klib_xml_unescape_string (const char *s)
  {
  KLIB_IN
  klib_String *ret = NULL;
  if (s) 
    {
    if (s[0]) 
      { 
      char *buff = malloc (strlen(s) + 10);
      html2str ((char *)s, buff);
      // That takes care of &amp, and the like; now deal with &#num;
      ret = klib_string_new (buff);
      free (buff);
      klib_string_search_replace_function 
       (ret, "&#___;", _klib_xml_replace_unicode, NULL);
      }
    else
      {
      ret = klib_string_new_empty();
      }
    }
  else
    ret = klib_string_new_empty(); 
  return ret; 
  KLIB_OUT
  }


