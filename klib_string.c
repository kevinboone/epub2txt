/*===========================================================================
klib
klib_string.c
(c)2000-2012 Kevin Boone
============================================================================*/

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "klib_defs.h"
#include "klib_log.h"
#include "klib_string.h"
#include "klib_error.h"
#include "klib_buffer.h"
#include "klib_wstring.h"
#include "klib_list.h"

/*===========================================================================
private data
============================================================================*/
extern void klib_string_init (klib_Object *self);
void klib_string_dispose (klib_Object *self);

static klib_Spec klib_spec_string =
  {
  obj_size: sizeof (klib_String), 
  init_fn: klib_string_init,
  class_name: "klib_String"
  };

typedef struct _klib_String_priv
  {
  char *str;
  } klib_String_priv;


/*===========================================================================
klib_string_init
============================================================================*/
void klib_string_init (klib_Object *_self)
  {
  KLIB_IN
  klib_object_init (_self);
  _self->dispose = klib_string_dispose;
  klib_String *self = (klib_String*)_self;
  self->priv = (klib_String_priv *)malloc (sizeof (klib_String_priv));
  memset (self->priv, 0, sizeof (klib_String_priv));
  KLIB_OUT
  } 


/*===========================================================================
klib_string_new_printf
============================================================================*/
klib_String *klib_string_new_printf (const char *fmt,...)
  {
  KLIB_IN
  va_list ap;
  va_start (ap, fmt);
  char *buff = klib_string_format_args (fmt, ap); 
  va_end (ap);
  klib_String *ret = klib_string_new (buff);
  free (buff);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_string_printf
============================================================================*/
void klib_string_printf (klib_String *self, const char *fmt,...)
  {
  KLIB_IN
  va_list ap;
  va_start (ap, fmt);
  char *buff = klib_string_format_args (fmt, ap); 
  va_end (ap);
  klib_string_set (self, buff);
  free (buff);
  KLIB_OUT
  }

/*===========================================================================
klib_string_set
============================================================================*/
void klib_string_set (klib_String *self, const char *s)
  {
  KLIB_IN
  if (self->priv->str)
    free (self->priv->str);
  if (s == NULL) 
    self->priv->str = NULL;
  else
    self->priv->str = strdup (s);
  KLIB_OUT
  }

/*===========================================================================
klib_string_new
============================================================================*/
klib_String *klib_string_new (const char *s)
  {
  KLIB_IN
  klib_String *self = (klib_String *)klib_object_new (&klib_spec_string); 
  klib_string_set (self, s);
  KLIB_OUT
  return self;
  }

/*===========================================================================
klib_string_new_null
============================================================================*/
klib_String *klib_string_new_null (void)
  {
  KLIB_IN
  klib_String *ret = klib_string_new (NULL);
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_string_empty
============================================================================*/
klib_String *klib_string_new_empty (void)
  {
  KLIB_IN
  klib_String *self = klib_string_new(""); 
  KLIB_OUT
  return self;
  }

/*===========================================================================
klib_string_c_str
============================================================================*/
const char *klib_string_cstr (const klib_String *self)
  {
  KLIB_IN
  char *ret = NULL;
  if (self)
    {
    if (self->priv) 
      ret = self->priv->str;
    }
  else
    ret = NULL;
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_string_dispose
============================================================================*/
void klib_string_dispose (klib_Object *_self)
  {
  KLIB_IN
  klib_String *self = (klib_String *)_self;
  if (!self) return;
  if (!self->disposing)
    {
    self->disposing = TRUE;
    if (self->priv) 
      {
      if (self->priv->str)
        free (self->priv->str);
      free (self->priv);
      }
    }
  klib_object_dispose ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_string_free
============================================================================*/
void klib_string_free (klib_String *self)
  {
  KLIB_IN
  klib_object_unref ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_string_read_file
============================================================================*/
klib_String *klib_string_read_file (const char *filename, 
    struct _klib_Error **error)
  {
  KLIB_IN
  klib_String *ret = NULL;
  klib_log_debug ("klib_string_read_file: Reading file %s", filename);
  FILE *stream = fopen (filename, "rb");
  if (stream)
    {
    ret = klib_string_read_stream (stream, error);
    fclose (stream);
    }
  else
    *error = klib_error_new (ENOENT, "Can't open file for reading: %s", 
      filename);
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_string_read_stream
============================================================================*/
klib_String *klib_string_read_stream (FILE *stream, 
    struct _klib_Error **error)
  {
  KLIB_IN
  klib_String *ret = NULL;
  struct stat sb;
  if (fstat (fileno (stream), &sb) == 0)
    {
    char *buff = (char *)malloc (sb.st_size + 1);
    read (fileno (stream), buff, sb.st_size);
    buff[sb.st_size] = 0;
    ret = (klib_String *)klib_object_new (&klib_spec_string);
    klib_string_set (ret, buff);
    free (buff);
    }
  KLIB_OUT
  return ret; 
  }

/*===========================================================================
klib_string_format_args
============================================================================*/
char *klib_string_format_args (const char *fmt, va_list ap)
  {
  int n;
  int size = 100;
  char *p, *np;

  if ((p = malloc(size)) == NULL)
    return NULL;

  while (1) 
    {
    va_list ap2;
    va_copy (ap2, ap);

    n = vsnprintf(p, size, fmt, ap2);
 
    va_end(ap2);

    if (n > -1 && n < size)
       return p;

    if (n > -1) 
      size = n+1; 
    else         
      size *= 2; 

    if ((np = realloc (p, size)) == NULL) 
      {
      free(p);
      return NULL;
      } 
    else 
      {
      p = np;
      }
    }
  return NULL; // Never get here 
  }

/*===========================================================================
klib_string_search_replace_function 
============================================================================*/
void klib_string_search_replace_function (klib_String *self, 
    const char *pattern, klib_string_replace_func func, void *userdata)
  {
  KLIB_IN
  klib_String *ret = klib_string_new_search_replace_function (self,
     pattern, func, userdata); 
  klib_string_set (self, klib_string_cstr(ret));
  klib_string_free (ret);
  KLIB_OUT
  }

/*===========================================================================
klib_string_length
============================================================================*/
int klib_string_length (const klib_String *self) 
  {
  KLIB_IN
  int ret = 0;
  if (self)
    {
    if (self->priv->str)
      return strlen (self->priv->str);
    else
      return 0;
    }
  else
    ret = 0;

  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_string_new_split_before 
============================================================================*/
klib_String *klib_string_new_split_before (const char *s, 
    const char *pattern)
  {
  KLIB_IN
  klib_String *ret = NULL;
  char *p = strstr (s, pattern);
  if (p)
    ret = klib_string_new_substring (s, 0, p - s);
  else
    ret = klib_string_new (s);
  return ret;
  KLIB_OUT
  }

/*===========================================================================
klib_string_new_substring
============================================================================*/
klib_String *klib_string_new_substring (const char *s, 
    int pos, int len)
  {
  KLIB_IN
  char *buff = malloc (len + 1);
  strncpy (buff, s + pos, len);
  buff[len] = 0;
  klib_String *ret = klib_string_new (buff);
  free (buff); 
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_string_strdup
============================================================================*/
klib_String *klib_string_strdup (const klib_String *self)
  {
  KLIB_IN
  klib_String *ret = NULL;

  if (self)
    {
    if (self->priv->str)
      ret = klib_string_new (self->priv->str);
    else
      ret = klib_string_new_null();
    }

  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_string_remove
============================================================================*/
void klib_string_remove (klib_String *self, int start, int len)
  {
  KLIB_IN
  klib_String *ret = klib_string_new_substring (self->priv->str, 0, start);
  char *after = self->priv->str + start + len; 
  klib_string_append (ret, after);
  klib_string_set (self, klib_string_cstr(ret));
  klib_string_free (ret);
  KLIB_OUT
  }


/*===========================================================================
klib_string_append_byte
============================================================================*/
BOOL klib_string_append_byte (klib_String *self, BYTE byte)
  {
  KLIB_IN
  char buff[2];
  buff[0] = byte;
  buff[1] = 0;
  BOOL ret = klib_string_append (self, buff);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_string_append
============================================================================*/
BOOL klib_string_append (klib_String *self, const char *s)
  {
  KLIB_IN
  BOOL ret = FALSE;
  char *buff = malloc (strlen (self->priv->str) + strlen (s) + 1);
  if (buff)
    {
    strcpy (buff, self->priv->str);
    strcat (buff, s);
    klib_string_set (self, buff);
    free (buff);
    ret = TRUE;
    }
  else
    ret = FALSE;
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_string_new_split_after
============================================================================*/
klib_String *klib_string_new_split_after (const char *s, 
    const char *pattern)
  {
  KLIB_IN
  klib_String *ret = NULL;
  char *p = strstr (s, pattern);
  if (p)
    ret = klib_string_new_substring (s, p - s + strlen (pattern), 
      strlen (p) - strlen (pattern));
  else
    ret = klib_string_new_empty();
  return ret;
  KLIB_OUT
  }

/*===========================================================================
klib_string_starts_with
============================================================================*/
BOOL klib_string_starts_with (const klib_String *self, const char *s)
  {
  KLIB_IN
  BOOL ret = FALSE;
  if (self)
    {
    if (self->priv->str)
      {
      if (strstr (self->priv->str, s) == self->priv->str)
        ret = TRUE;
      }
    }
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_string_is_empty
============================================================================*/
BOOL klib_string_is_empty (const klib_String *self)
  {
  KLIB_IN
  BOOL ret = TRUE;
  if (self)
    {
    if (self->priv->str)
      {
      if (strlen (self->priv->str) > 0)
        ret = FALSE;
      }
    }
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_string_index_of
============================================================================*/
int klib_string_index_of (const klib_String *self, const char *s)
  {
  KLIB_IN
  int ret = -1; 
  if (self)
    {
    if (self->priv->str)
      {
      char *p = strstr (self->priv->str, s);
      if (p)
        ret = p - self->priv->str;
      }
    }
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_string_search_replace 
============================================================================*/
klib_String *_klib_string_replace_func_static (const char *pattern, 
    void *userdata)
  { 
  return klib_string_new ((char *)userdata);
  }

/*===========================================================================
klib_string_new_search_replace 
============================================================================*/
klib_String *klib_string_new_search_replace (const klib_String *self,
     const char *pattern, const char *replace)
  {
  KLIB_IN
  klib_String *ret = klib_string_new_search_replace_function (self, pattern, 
   _klib_string_replace_func_static, (void *)replace);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_string_search_replace 
============================================================================*/
void klib_string_search_replace (klib_String *self,
     const char *pattern, const char *replace)
  {
  KLIB_IN
  klib_string_search_replace_function (self, pattern, 
   _klib_string_replace_func_static, (void *)replace);
  KLIB_OUT
  }


/*===========================================================================
klib_string_search_replace_function 
============================================================================*/
klib_String *klib_string_new_search_replace_function (const klib_String *self,
     const char *pattern, klib_string_replace_func func, void *userdata)
  {
  KLIB_IN
  klib_String *match_before = klib_string_new_split_before (pattern, "___");
  klib_String *match_after = klib_string_new_split_after (pattern, "___");
  klib_String *ret = klib_string_new_empty();

  //printf ("before = %s\n", klib_string_cstr(match_before));
  //printf ("after = %s\n", klib_string_cstr(match_after));

  klib_String *remaining_string = klib_string_strdup (self);

  while (klib_string_length (remaining_string) > 0)
    {
    int rem = 1;
    BYTE c = klib_string_cstr (remaining_string)[0];
    if (klib_string_starts_with (remaining_string, 
        klib_string_cstr (match_before)))
      {
      // We have matched the 'before' part of the pattern.
      if (klib_string_is_empty (match_after))
        {
        // There is no ater -- we are done
        rem = klib_string_length (match_before); 
        klib_String *replace = func (klib_string_cstr(match_before), userdata);
        klib_string_append (ret, klib_string_cstr(replace));
        klib_string_free (replace);
        }
      else
        {
        // We must now match the 'after' part
        int n = klib_string_index_of 
         (remaining_string, klib_string_cstr(match_after));
        if (n > 0)
          { 
          klib_String *term = klib_string_new_substring 
           (klib_string_cstr (remaining_string), 
             klib_string_length(match_before), 
             n - 1 - klib_string_length (match_after));
          rem = n + 1; 
          klib_String *replace = func (klib_string_cstr(term), userdata);
          klib_string_append (ret, klib_string_cstr (replace)); 
          klib_string_free (replace);
          klib_string_free (term);
          }
        else
          {
          // 'After' part did not match -- we are done
          rem = 1; 
          }
        }
      }
    else
      {
      klib_string_append_byte (ret, c);
      } 
    klib_string_remove (remaining_string, 0, rem);
    }
 
  klib_string_free (remaining_string);
  klib_string_free (match_after);
  klib_string_free (match_before);

  KLIB_OUT
  return ret; 
  }


/*===========================================================================
klib_string_append_unicode_char
============================================================================*/
void klib_string_append_unicode_char (klib_String *self, int ch)
  {
  KLIB_IN
  if (ch < 0x80) 
    {
    klib_string_append_byte (self, (BYTE)ch);
    }
  else if (ch < 0x0800) 
    {
    klib_string_append_byte (self, (BYTE)((ch >> 6) | 0xC0));
    klib_string_append_byte (self, (BYTE)((ch & 0x3F) | 0x80));
    }
  else if (ch < 0x10000) 
    {
    klib_string_append_byte (self, (BYTE)((ch >> 12) | 0xE0));
    klib_string_append_byte (self, (BYTE)((ch >> 6 & 0x3F) | 0x80));
    klib_string_append_byte (self, (BYTE)((ch & 0x3F) | 0x80));
    }
  else 
    {
    klib_string_append_byte (self, (BYTE)((ch >> 18) | 0xF0));
    klib_string_append_byte (self, (BYTE)(((ch >> 12) & 0x3F) | 0x80));
    klib_string_append_byte (self, (BYTE)(((ch >> 6) & 0x3F) | 0x80));
    klib_string_append_byte (self, (BYTE)((ch & 0x3F) | 0x80));
    }
  KLIB_OUT
  }

/*===========================================================================
klib_string_equals
============================================================================*/
BOOL klib_string_equals (const klib_String *self, const char *other)
  {
  if (self == NULL) return other==NULL;
  if (self->priv->str == NULL) return other == NULL;
  if (other == NULL) return self->priv->str == NULL;
  return (strcmp (self->priv->str, other) == 0);
  }

/*===========================================================================
klib_string_new_from_buffer
============================================================================*/
klib_String *klib_string_new_from_buffer (const klib_Buffer *s,
    struct _klib_Error **error)
  {
  KLIB_IN

  klib_String *self = klib_string_new_null();

  int len = klib_buffer_get_length (s);
  self->priv->str = malloc (len + 1);
  memcpy (self->priv->str, klib_buffer_get_data (s), len);
  self->priv->str[len] = 0;

  return self;

  KLIB_OUT
  }

/*===========================================================================
klib_string_chomp
============================================================================*/
void klib_string_chomp (klib_String *self)
  {
  KLIB_IN
  if (self)
    {
    if (self->priv->str)
      {
      BOOL carry_on = TRUE;
      while (carry_on)
        {
        int len = strlen (self->priv->str);
        BYTE c = self->priv->str[len - 1];
        if ((c == 10 || c == 13))
          {
          klib_string_remove (self, len - 1, 1);
          if (strlen (self->priv->str) == 0) carry_on = FALSE;
          }
        else
          carry_on = FALSE;
        }
      }
    }
  KLIB_OUT
  }

/*===========================================================================
klib_string_new_from_wstring
============================================================================*/
klib_String *klib_string_new_from_wstring (const klib_WString *wide)
  {
  KLIB_IN
  klib_String *ret = klib_string_new_from_wchar_string 
    (klib_wstring_cstr(wide));
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_string_new_from_wchar_string
============================================================================*/
klib_String *klib_string_new_from_wchar_string (const wchar_t *wide)
  {
  KLIB_IN
  klib_String *ret = klib_string_new_empty();
  const wchar_t *utf16in = wide;
  int utf16lenchars = wcslen (wide);
  int i;
  if (utf16in)
    {
    for (i = 0; i < utf16lenchars; i++)
      {
      klib_string_append_unicode_char (ret, utf16in[i]);
      }
    }
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_string_read_stdio
============================================================================*/
klib_String *klib_string_read_stdin (void)
  {
  KLIB_IN
  klib_String *s = klib_string_new_empty (); 
  int c = 0;
  do
    {
    c = getchar();
    if (c > 0) klib_string_append_byte (s, c); 
    } while (c > 0);
  KLIB_OUT
  return s;
  }


/*===========================================================================
klib_string_split
============================================================================*/
klib_List *klib_string_split (const char *_s, char delim)
  {
  KLIB_IN
  klib_List *ret = klib_list_new ();

  char *s = strdup (_s);
  char *f = s;

  char *p = NULL;
  do
    {
    p = strchr (f,  delim);
    if (p)
      {
      *p = 0;
      klib_String *ss = klib_string_new (f);
      klib_list_append (ret, (klib_Object *)ss);
      klib_string_free (ss);
      f = p + 1;
      }
    else
      {
      klib_String *ss = klib_string_new (f);
      klib_list_append (ret, (klib_Object *)ss);
      klib_string_free (ss);
      }
    } while (p); 
  
  free (s);

  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_string_append_wchar
============================================================================*/
void klib_string_append_wchar (klib_String *self, wchar_t c)
  {
  wchar_t s[2];
  s[0] = c;
  s[1] = 0;
  klib_String *ss = klib_string_new_from_wchar_string (s); 
  klib_string_append (self, klib_string_cstr (ss));  
  klib_string_free (ss);
  }



