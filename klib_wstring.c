/*===========================================================================
klib
klib_wstring.c
(c)2000-2012 Kevin Boone
============================================================================*/
#undef __STRICT_ANSI__

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "klib_defs.h"
#include "klib_log.h"
#include "klib_wstring.h"
#include "klib_error.h"
#include "klib_wstring.h"
#include "klib_convertutf.h"
#include "klib_buffer.h"

/*===========================================================================
private data
============================================================================*/
extern void klib_wstring_init (klib_Object *self);
void klib_wstring_dispose (klib_Object *self);

static klib_Spec klib_spec_wstring =
  {
  obj_size: sizeof (klib_WString), 
  init_fn: klib_wstring_init,
  class_name: "klib_WString"
  };

typedef struct _klib_WString_priv
  {
  wchar_t *str;
  } klib_WString_priv;


/*===========================================================================
klib_wstring_new
============================================================================*/
klib_WString *klib_wstring_new (const wchar_t *s)
  {
  KLIB_IN
  klib_WString *ret = (klib_WString *)klib_object_new (&klib_spec_wstring);
  klib_wstring_set (ret, s);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_wstring_new_null
============================================================================*/
klib_WString *klib_wstring_new_null (void)
  {
  KLIB_IN
  klib_WString *ret = (klib_WString *)klib_object_new (&klib_spec_wstring);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_wstring_set
============================================================================*/
void klib_wstring_set (klib_WString *self, const wchar_t *s)
  {
  KLIB_IN
  if (self->priv->str) free (self->priv->str);
  self->priv->str = malloc ((wcslen (s) + 1) * sizeof (wchar_t)); 
  // Include terminating null in copy
  memcpy (self->priv->str, s, (wcslen(s) + 1) * sizeof (wchar_t));
  KLIB_OUT
  }

/*===========================================================================
klib_wstring_read_file
============================================================================*/
klib_WString *klib_wstring_read_file (const char *filename, 
    klib_Encoding encoding, klib_Error **error)
  {
  KLIB_IN
  klib_WString *ret = NULL;
  klib_log_debug ("klib_string_read_file: Reading file %s", filename);
  FILE *stream = fopen (filename, "rb");
  if (stream)
    {
    ret = klib_wstring_read_stream (stream, encoding, error);
    fclose (stream);
    }
  else
    *error = klib_error_new (ENOENT, "Can't open file for reading: %s\n", 
      filename);
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_wstring_read_stream
============================================================================*/
/** Reads the whole file, assumed to be UTF-8 encoded, into a string object */
klib_WString *klib_wstring_read_stream (FILE *stream, 
    klib_Encoding encoding, klib_Error **error)
  {
  KLIB_IN
  klib_WString *ret = NULL;
  struct stat sb;
  if (fstat (fileno (stream), &sb) == 0)
    {
    char *buff = (char *)malloc (sb.st_size + 1);
    read (fileno (stream), buff, sb.st_size);
    buff[sb.st_size - 1] = 0;
    ret = klib_wstring_new_null ();
    if (encoding == KLIB_ENCODING_UTF16BE)
      klib_wstring_set (ret, (wchar_t*)buff);
    else if (encoding == KLIB_ENCODING_UTF16LE) // FRIG
      klib_wstring_set (ret, (wchar_t*)buff);
    else
      {
      wchar_t *buff2 = malloc ((sb.st_size + 1) * sizeof (wchar_t));
      wchar_t *targetStart = buff2;
      memset (buff2, 0, (sb.st_size + 1) * sizeof (wchar_t));
      int utf16lenchars = (sb.st_size + 1);
      const UTF8 *_buff = (BYTE *)buff;
      if (sizeof (wchar_t) == 4)
        ConvertUTF8toUTF32 (&_buff, (UTF8*) buff + sb.st_size, 
         (UTF32 **)&targetStart, (UTF32 *)targetStart + utf16lenchars, 0);
      else
        ConvertUTF8toUTF16 (&_buff, (UTF8*) buff + sb.st_size, 
         (UTF16 **) &targetStart, (UTF16 *) targetStart + utf16lenchars, 0);
      klib_wstring_set (ret, (wchar_t*)buff2);
      free (buff2);
      }
    free (buff);
    }
  KLIB_OUT
  return ret; 
  }


/*===========================================================================
klib_wstring_init
============================================================================*/
void klib_wstring_init (klib_Object *_self)
  {
  KLIB_IN
  klib_object_init (_self);
  _self->dispose = klib_wstring_dispose;
  klib_WString *self = (klib_WString *)_self;
  self->priv = (klib_WString_priv *)malloc (sizeof (klib_WString_priv));
  memset (self->priv, 0, sizeof (klib_WString_priv));
  KLIB_OUT
  }

/*===========================================================================
klib_wstring_c_str
============================================================================*/
const wchar_t *klib_wstring_cstr (const klib_WString *self)
  {
  KLIB_IN
  const wchar_t *ret = NULL;
  if (self)
    ret = self->priv->str;
  else
    ret = NULL;
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_wstring_dispose
============================================================================*/
void klib_wstring_dispose (klib_Object *_self)
  {
  KLIB_IN
  klib_WString *self = (klib_WString *)_self;
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
  klib_object_dispose (_self);
  KLIB_OUT
  }

/*===========================================================================
klib_wstring_free
============================================================================*/
void klib_wstring_free (klib_WString *self)
  {
  KLIB_IN
  klib_object_unref ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_wstring_length
============================================================================*/
int klib_wstring_length (const klib_WString *self)
  {
  KLIB_IN
  int ret = 0;
  if (self) 
    { 
    if (self->priv) 
      {
      if (self->priv->str)
         ret = wcslen (self->priv->str);
      }
    }
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_wstring_format_args
============================================================================*/
wchar_t *klib_wstring_format_args (const wchar_t *fmt, va_list ap)
  {
  int n;
  int size = 100;     
  wchar_t *p, *np;

  if ((p = malloc(size * 2)) == NULL)
    return NULL;

  while (1) 
    {
    n = VWSNPRINTF(p, size, fmt, ap);

    if (n > -1 && n < size)
       return p;

    if (n > -1) 
      size = n+1; 
    else         
      size *= 2; 

    if ((np = realloc (p, size * 2)) == NULL) 
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
klib_wstring_format_args
============================================================================*/
void klib_wstring_printf (klib_WString *self, const wchar_t *fmt,...)
  {
  KLIB_IN
  va_list ap;
  va_start (ap, fmt);
  wchar_t *buff = klib_wstring_format_args (fmt, ap); 
  va_end (ap);
  klib_wstring_set (self, buff);
  free (buff);
  KLIB_OUT
  }


/*===========================================================================
klib_wstring_format_args
============================================================================*/
klib_WString *klib_wstring_new_printf (const wchar_t *fmt,...)
  {
  KLIB_IN
  va_list ap;
  va_start (ap, fmt);
  wchar_t *buff = klib_wstring_format_args (fmt, ap); 
  va_end (ap);
  klib_WString *ret = klib_wstring_new (buff);
  free (buff);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_wstring_new_empty
============================================================================*/
klib_WString *klib_wstring_new_empty (void)
  {
  KLIB_IN
  klib_WString *ret = klib_wstring_new (L"");
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_wstring_new_from_buffer
============================================================================*/
klib_WString *klib_wstring_new_from_buffer (const klib_Buffer *s,
    klib_Encoding encoding, struct _klib_Error **error)
  {
  KLIB_IN

  klib_WString *self = klib_wstring_new_null();

  int len = klib_buffer_get_length (s);
  const BYTE *data = klib_buffer_get_data (s);
  wchar_t *buff2;

  if (encoding == KLIB_ENCODING_UTF16BE)
      {
      int len_chars = len / 2;
      buff2 = malloc (len + sizeof (wchar_t));
      memcpy (buff2, data, len * sizeof (wchar_t));
      buff2[len_chars] = (wchar_t) 0;
      }
    else if (encoding == KLIB_ENCODING_UTF16LE) // FRIG
      {
      int len_chars = len / 2;
      buff2 = malloc (len + sizeof (wchar_t));
      memcpy (buff2, data, len * sizeof (wchar_t));
      buff2[len_chars] = (wchar_t) 0;
      }
    else
      {
      buff2 = malloc ((len + 1) * sizeof (wchar_t));
      wchar_t *targetStart = buff2;
      memset (buff2, 0, (len + 1) * sizeof (wchar_t));
      int utf16lenchars = (len + 1);
      const UTF8 *_buff = (BYTE *)data;
      if (sizeof (wchar_t) == 4)
        ConvertUTF8toUTF32 (&_buff, _buff+ len, 
         (UTF32 **)&targetStart, (UTF32 *)targetStart + utf16lenchars, 0);
      else
        ConvertUTF8toUTF16 (&_buff, _buff+ len, 
         (UTF16 **)&targetStart, (UTF16 *)targetStart + utf16lenchars, 0);
      }

  klib_wstring_set (self, buff2);
  free (buff2);

  return self;

  }

/*===========================================================================
klib_wstring_search_replace 
============================================================================*/
klib_WString *_klib_wstring_replace_func_static (const wchar_t *pattern, 
    void *userdata)
  { 
  return klib_wstring_new ((wchar_t *)userdata);
  }

/*===========================================================================
klib_wstring_search_replace_function
============================================================================*/
void klib_wstring_search_replace_function (klib_WString *self, 
    const wchar_t *pattern, klib_wstring_replace_func func, void *userdata)
  {
  KLIB_IN
  klib_WString *ret = klib_wstring_new_search_replace_function (self,
     pattern, func, userdata); 
  klib_wstring_set (self, klib_wstring_cstr(ret));
  KLIB_OUT
  }

/*===========================================================================
klib_wstring_new_search_replace_function
============================================================================*/
klib_WString *klib_wstring_new_search_replace_function 
    (const klib_WString *self, const wchar_t *pattern, 
    klib_wstring_replace_func func, void *userdata)
  {
  KLIB_IN
  klib_WString *match_before = klib_wstring_new_split_before (pattern, L"___");
  klib_WString *match_after = klib_wstring_new_split_after (pattern, L"___");

  klib_WString *ret = klib_wstring_new_empty();

  //printf ("before = %ls\n", klib_string_cstr(match_before));
  //printf ("after = %ls\n", klib_string_cstr(match_after));

  klib_WString *remaining_string = klib_wstring_strdup (self);

  while (klib_wstring_length (remaining_string) > 0)
    {
    int rem = 1;
    wchar_t c = klib_wstring_cstr (remaining_string)[0];
    if (klib_wstring_starts_with (remaining_string, 
        klib_wstring_cstr (match_before)))
      {
      // We have matched the 'before' part of the pattern.
      if (klib_wstring_is_empty (match_after))
        {
        // There is no ater -- we are done
        rem = klib_wstring_length (match_before); 
        klib_WString *replace = func (klib_wstring_cstr(match_before), 
          userdata);
        klib_wstring_append (ret, klib_wstring_cstr(replace));
        klib_wstring_free (replace);
        }
      else
        {
        // We must now match the 'after' part
        int n = klib_wstring_index_of 
         (remaining_string, klib_wstring_cstr(match_after));
        if (n > 0)
          { 
          klib_WString *term = klib_wstring_new_substring 
           (klib_wstring_cstr (remaining_string), 
             klib_wstring_length(match_before), 
             n - 1 - klib_wstring_length (match_after));
          rem = n + 1; 
          klib_WString *replace = func (klib_wstring_cstr(term), userdata);
          klib_wstring_append (ret, klib_wstring_cstr (replace)); 
          klib_wstring_free (replace);
          klib_wstring_free (term);
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
      klib_wstring_append_char (ret, c);
      } 
    klib_wstring_remove (remaining_string, 0, rem);
    }
 
  klib_wstring_free (remaining_string);
  klib_wstring_free (match_after);
  klib_wstring_free (match_before);

  KLIB_OUT
  return ret; 
  }

/*===========================================================================
klib_wstring_new_search_replace_function
============================================================================*/
klib_WString *klib_wstring_new_search_replace (const klib_WString *self,
     const wchar_t *pattern, const wchar_t *replace)
  {
  KLIB_IN
  klib_WString *ret = klib_wstring_new_search_replace_function (self, pattern, 
   _klib_wstring_replace_func_static, (void *)replace);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_wstring_new_split_before
============================================================================*/
klib_WString *klib_wstring_new_split_before (const wchar_t *s, 
    const wchar_t *pattern)
  {
  KLIB_IN
  klib_WString *ret = NULL;
  wchar_t *p = wcsstr (s, pattern);
  if (p)
    ret = klib_wstring_new_substring (s, 0, p - s);
  else
    ret = klib_wstring_new (s);
  return ret;
  KLIB_OUT
  }

/*===========================================================================
klib_wstring_new_split_after
============================================================================*/
klib_WString *klib_wstring_new_split_after (const wchar_t *s, 
    const wchar_t *pattern)
  {
  KLIB_IN
  klib_WString *ret = NULL;
  wchar_t *p = wcsstr (s, pattern);
  if (p)
    ret = klib_wstring_new_substring (s, p - s + wcslen (pattern), 
      wcslen (p) - wcslen (pattern));
  else
    ret = klib_wstring_new_empty();
  return ret;
  KLIB_OUT
  }

/*===========================================================================
klib_wstring_new_substring
============================================================================*/
klib_WString *klib_wstring_new_substring (const wchar_t *s, 
    int pos, int len)
  {
  KLIB_IN
  wchar_t *buff = malloc ((len + 1) * sizeof (wchar_t));
  wcsncpy (buff, s + pos, len);
  buff[len] = (wchar_t)0;
  klib_WString *ret = klib_wstring_new (buff);
  free (buff); 
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_wstring_strdup
============================================================================*/
klib_WString *klib_wstring_clone (const klib_WString *self)
  {
  return klib_wstring_strdup (self);
  }

/*===========================================================================
klib_wstring_strdup
============================================================================*/
klib_WString *klib_wstring_strdup (const klib_WString *self)
  {
  KLIB_IN
  klib_WString *ret = NULL;

  if (self)
    {
    if (self->priv->str)
      ret = klib_wstring_new (self->priv->str);
    else
      ret = klib_wstring_new_null();
    }

  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_wstring_is_empty
============================================================================*/
BOOL klib_wstring_is_empty (const klib_WString *self)
  {
  KLIB_IN
  BOOL ret = TRUE;
  if (self)
    {
    if (self->priv->str)
      {
      if (wcslen (self->priv->str) > 0)
        ret = FALSE;
      }
    }
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_wstring_index_of
============================================================================*/
int klib_wstring_index_of (const klib_WString *self, const wchar_t *s)
  {
  KLIB_IN
  int ret = -1; 
  if (self)
    {
    if (self->priv->str)
      {
      wchar_t *p = wcsstr (self->priv->str, s);
      if (p)
        ret = p - self->priv->str;
      }
    }
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_wstring_append_char
============================================================================*/
BOOL klib_wstring_append_char (klib_WString *self, wchar_t c)
  {
  KLIB_IN
  wchar_t buff[2];
  buff[0] = c;
  buff[1] = 0;
  BOOL ret = klib_wstring_append (self, buff);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_string_append
============================================================================*/
BOOL klib_wstring_append (klib_WString *self, const wchar_t *s)
  {
  KLIB_IN
  BOOL ret = FALSE;
  wchar_t *buff = malloc 
      ((wcslen (self->priv->str) + wcslen (s) + 1) * sizeof (wchar_t));
  if (buff)
    {
    wcscpy (buff, self->priv->str);
    wcscat (buff, s);
    klib_wstring_set (self, buff);
    free (buff);
    ret = TRUE;
    }
  else
    ret = FALSE;
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_wstring_starts_with
============================================================================*/
BOOL klib_wstring_starts_with (const klib_WString *self, const wchar_t *s)
  {
  KLIB_IN
  BOOL ret = FALSE;
  if (self)
    {
    if (self->priv->str)
      {
      if (wcsstr (self->priv->str, s) == self->priv->str)
        ret = TRUE;
      }
    }
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_wstring_remove
============================================================================*/
void klib_wstring_remove (klib_WString *self, int start, int len)
  {
  KLIB_IN
  klib_WString *ret = klib_wstring_new_substring (self->priv->str, 0, start);
  wchar_t *after = self->priv->str + start + len; 
  klib_wstring_append (ret, after);
  klib_wstring_set (self, klib_wstring_cstr(ret));
  klib_wstring_free (ret);
  KLIB_OUT
  }

/*===========================================================================
klib_wstring_new_from_utf8
============================================================================*/
klib_WString *klib_wstring_new_from_utf8 (const unsigned char *s)
  {
  KLIB_IN
  // Make the internal wchar_t* at least as large as the UTF8 string.
  //  This might be wasteful, but the alternative is to decode the UTF8
  //  and count the characters
  int utf8len = strlen ((const char *)s);
  int utf16lenchars = utf8len;
  wchar_t *buff2 = malloc ((utf8len + 1) * sizeof (wchar_t));
  wchar_t *targetStart = buff2;
  memset (buff2, 0, (utf8len + 1) * sizeof (wchar_t));
  const UTF8 *_buff = (BYTE *)s;
  if (sizeof (wchar_t) == 4)
    ConvertUTF8toUTF32 (&_buff, _buff + utf8len, 
       (UTF32 **)&targetStart, (UTF32*)targetStart + utf16lenchars, 0);
  else
    ConvertUTF8toUTF16 (&_buff, _buff + utf8len, 
       (UTF16 **) &targetStart, (UTF16 *)targetStart + utf16lenchars, 0);
  buff2[utf16lenchars] = 0;
  klib_WString *ret = klib_wstring_new (buff2); 
  free (buff2);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_wstring_new_break_words
============================================================================*/
klib_WString *klib_wstring_new_break_words (const wchar_t *s, int len, 
    void *reserved)
  {
  KLIB_IN
  klib_WString *ret = klib_wstring_new_empty();
  klib_WString *working = klib_wstring_new_empty();
  int i, l = wcslen (s);
  int w = 0;
  for (i = 0 ;i < l; i++)
    {
    wchar_t c = s[i];
    switch (c)
      {
      case 10:
        w = 0;
        klib_wstring_append_char (ret, (wchar_t)10);
        break;

      case ' ':
        if (w >= len)
          {
          klib_wstring_append_char (working, (wchar_t)10);
          w = 0;
          }
        else
          {
          w++;
          klib_wstring_append_char (working, c);
          }
        break;

      default:
        w++;
        klib_wstring_append_char (working, c);
      }
    }

  klib_wstring_append (ret, klib_wstring_cstr (working));
  klib_wstring_free (working);

  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_wstring_new_break_words
============================================================================*/
void klib_wstring_break_words (klib_WString *self, int len, 
    void *reserved)
  {
  KLIB_IN
  klib_WString *s = klib_wstring_new_break_words (self->priv->str, len, 
    reserved);
  klib_wstring_set (self, klib_wstring_cstr (s));
  klib_wstring_free (s);
  KLIB_OUT
  }


/*===========================================================================
klib_wstring_search_replace
============================================================================*/
void klib_wstring_search_replace (klib_WString *self, 
  const wchar_t *pattern, const wchar_t *replace)
  {
  KLIB_IN
  klib_WString *s = klib_wstring_new_search_replace 
    (self, pattern, replace);
  klib_wstring_set (self, klib_wstring_cstr (s));
  klib_wstring_free (s);
  KLIB_OUT
  }



