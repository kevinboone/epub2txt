#pragma once

#include <stdarg.h>
#include "klib_defs.h"
#include "klib_string.h"
#include "klib_object.h"

struct _klib_WString_priv;
struct _klib_Error;

/** klib_Encoding defines encodings for use when writing klib_WString objects
to other places. Internally, the data is always stored as wchar_t */
typedef enum 
  {
  KLIB_ENCODING_UTF8 = 0,
  KLIB_ENCODING_UTF16BE = 1,
  KLIB_ENCODING_UTF16LE = 2
  } klib_Encoding;

/** klib_WString wraps a wchar_t*. It is useful for manipulating unicode
strings where character-level access is required. This is because there is
always a fixed relationship between the size of the string in bytes and the
number of characters. For manipulating UTF8 or ASCII data, klib_String is
faster and may well be more convenient; but klib_String does not offer
any form of character counting */
typedef struct _klib_WString
  {
  klib_Object base;
  BOOL disposing;
  struct _klib_WString_priv *priv;
  } klib_WString;

/** Definition of a function to be passed to 
  klib_string_search_replace_function. This function will be passed the
  matching text, and must return an initialized string containing the
  replacement value. */
typedef klib_WString *(*klib_wstring_replace_func) 
  (const wchar_t *pattern, void *userdata);

KLIB_BEGIN_DECLS

/** Creates a new klib_WString with null initial value. A return value of
NULL indicates that allocation failed. */
klib_WString *klib_wstring_new_null (void);

klib_WString *klib_wstring_new (const wchar_t *s);

/** Creates a new klib_WString by deep-copying another */
klib_WString *klib_wstring_new_clone (const klib_WString *url); 

/** Convert the buffer to a string. An error may be raised on OOM, or
encoding errors */
klib_WString *klib_wstring_new_from_buffer (const struct _klib_Buffer *s,
  klib_Encoding encoding, struct _klib_Error **error);

/** Creates a new klib_WString intially empty */
klib_WString *klib_wstring_new_empty (void);

/** Creates a new klib_WString from UTF8 encoded data*/
klib_WString *klib_wstring_new_from_utf8 (const unsigned char *s);

/** Returns a C representation of this string object */
const wchar_t *klib_wstring_cstr (const klib_WString *self);

/** Frees any memory associated with this string object */
void klib_wstring_free (klib_WString *self);

/** Sets the value of this WString */
void klib_wstring_set (klib_WString *self, const wchar_t *s);

klib_WString *klib_wstring_read_stream (FILE *stream, 
    klib_Encoding encoding, struct _klib_Error **error);

klib_WString *klib_wstring_read_file (const char *filename, 
    klib_Encoding encoding, struct _klib_Error **error);

/* Returns lengh of string in CHARACTERS */
int klib_wstring_length (const klib_WString *self);

/** Formats the arguments in the same way as printf. The function returns
    a newly-allocated string, which the caller must free */
wchar_t *klib_wstring_format_args (const wchar_t *fmt, va_list ap);

/** Formats the arguments in the same way as printf. Overwrites the
existing contents of string */
void klib_wstring_printf (klib_WString *self, const wchar_t *fmt,...);

/** Makes a new string using the printf-style argument list */
klib_WString *klib_wstring_new_printf (const wchar_t *fmt,...);

/** Replaces each instance of the patter that matches with the result of
calling the function. The string is modified in place. Note that
patter is not a full regex: it is a string of the form 
XY___X, where the ___ will match any string of characters. */
void klib_wstring_search_replace_function (klib_WString *self, 
  const wchar_t *pattern, klib_wstring_replace_func func, void *userdata);

/** Replaces each instance of the pattern that matches with the fixed 
text. The string is modified in place. Note that
patter is not a full regex: it is a string of the form 
XY___X, where the ___ will match any string of characters. */
void klib_wstring_search_replace (klib_WString *self, 
  const wchar_t *pattern, const wchar_t *replace);

/** Replaces each instance of the patter that matches with the result of
calling the function. A new string is returned */
klib_WString *klib_wstring_new_search_replace_function 
   (const klib_WString *self, const wchar_t *pattern,  
    klib_wstring_replace_func func, void *userdata);

klib_WString *klib_wstring_new_search_replace (const klib_WString *self,
     const wchar_t *pattern, const wchar_t *replace);

/** Returns a new string containing that part of the input string before,
but not including, the pattern. If the pattern is not found, returns
the original string */
klib_WString *klib_wstring_new_split_before (const wchar_t *s, 
    const wchar_t *pattern);

/** Returns a new string containing that part of the input string after,
but not including, the pattern. If the pattern is not found, returns
an empty string */
klib_WString *klib_wstring_new_split_after (const wchar_t *s, 
    const wchar_t *pattern);

/** Makes a copy of the string */
klib_WString *klib_wstring_strdup (const klib_WString *self);

/** Removes len character from position start */
void klib_wstring_remove (klib_WString *self, int start, int len);

/** Appends one character to the string. Returns FALSE on OOM. */
BOOL klib_wstring_append_char (klib_WString *self, wchar_t c);

/** Appends another string to this one. Returns TRUE if the append
succeeds. The only reason for failure is OOM. */
BOOL klib_wstring_append (klib_WString *self, const wchar_t *s);

BOOL klib_wstring_starts_with (const klib_WString *self, const wchar_t *s);

/* Returns TRUE if the string is zero-length. Note that a null string is
also considered to be empty */
BOOL klib_wstring_is_empty (const klib_WString *self);

/* Returns the index of 's' within the string. If not found, returns
-1 */
int klib_wstring_index_of (const klib_WString *self, const wchar_t *s);

/** Creates a new substring containing a substring of the source string */
klib_WString *klib_wstring_new_substring (const wchar_t *s, 
    int pos, int len);

/** Make a new string by splitting this string into lines no more than
len fixed-size columns long. */
klib_WString *klib_wstring_new_break_words (const wchar_t *s, int len, 
    void *reserved);

/** Split this string into lines no more than
len fixed-size columns long. */
void klib_wstring_break_words (klib_WString *self, int len, 
    void *reserved);

/** Replaces all occurences of 'pattern' with 'replace'. Pattern may 
contain the wildcard sequence ___ which matches anything */
klib_WString *klib_wstring_new_search_replace (const klib_WString *self,
     const wchar_t *pattern, const wchar_t *replace);


KLIB_END_DECLS

