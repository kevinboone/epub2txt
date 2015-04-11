#pragma once

#include <stdarg.h>
#include <wchar.h>
#include "klib_defs.h"
#include "klib_object.h"

struct _klib_String_priv;
struct _klib_Error;
struct _klib_Buffer;
struct _klib_WString;
struct _klib_List;

/** klib_String wraps an array of single-byte character values. It can be
used as an alternative to char*, and to some extent will hold a UTF-8 string.
However, in such a case there is no straightforward relationship between the
length of the string in bytes, and the number of characters. klib_String is
a useful class when manipulating ASCII or UTF-8 that just has to be read and
copied from place to place. For manupulating unicode strings that require
character-level access, klib_WString is a better choice */
typedef struct _klib_String
  {
  klib_Object base;
  BOOL disposing;
  struct _klib_String_priv *priv;
  } klib_String;

/** Definition of a function to be passed to 
  klib_string_search_replace_function. This function will be passed the
  matching text, and must return an initialized string containing the
  replacement value. */
typedef klib_String *(*klib_string_replace_func) 
  (const char *pattern, void *userdata);

KLIB_BEGIN_DECLS

/** Creates a new klib_String with null initial value. A return value of
NULL indicates that allocation failed. */
klib_String *klib_string_new_null (void);

/** Creates a new klib_String initially empty (but not null). A return value of
NULL indicates that allocation failed. */
klib_String *klib_string_new_empty (void);

/** Creates a new klib_String with char* initial value */
klib_String *klib_string_new (const char *s);

/** Returns a C representation of this string object */
const char *klib_string_cstr (const klib_String *self);

/** Creates a string from a wstring, converting all extended 
characters to UTF-8 unicode */
klib_String *klib_string_new_from_wstring (const struct _klib_WString *self);

/** Creates a new string from w wchar_t*, converting all extended 
characters to UTF-8 unicode */
klib_String *klib_string_new_from_wchar_string (const wchar_t *wide);

/** Sets the value of this string object. Any memory previously 
used is freed. */
void klib_string_set (klib_String *self, const char *s);

/** Frees any memory associated with this string object */
void klib_string_free (klib_String *self);

/** Copy the buffer to a string. */
klib_String *klib_string_new_from_buffer (const struct _klib_Buffer *s,
  struct _klib_Error **error);

/** Reads the whole file, assumed to be UTF8 or ASCII, into a string object */
klib_String *klib_string_read_file (const char *filename, 
  struct _klib_Error **error);

/** Reads the whole file, assumed to be UTF-8 encoded, into a string object */
klib_String *klib_string_read_stream (FILE *stream, 
  struct _klib_Error **error);

/** Formats the arguments in the same way as printf. The function returns
    a newly-allocated string, which the caller must free */
char *klib_string_format_args (const char *fmt, va_list ap);

/** Formats the arguments in the same way as printf. Overwrites the
existing contents of string */
void klib_string_printf (klib_String *self, const char *fmt,...);

/** Replaces each instance of the patter that matches with the result of
calling the function. The string is modified in place. Note that
patter is not a full regex: it is a string of the form 
XY___X, where the ___ will match any string of characters. */
void klib_string_search_replace_function (klib_String *self, 
  const char *pattern, klib_string_replace_func func, void *userdata);

/** Replaces each instance of the patter that matches with the result of
calling the function. A new string is returned */
klib_String *klib_string_new_search_replace_function (const klib_String *self,
   const char *pattern, klib_string_replace_func func, void *userdata);

/** Returns a new string containing that part of the input string before,
but not including, the pattern. If the pattern is not found, returns
the original string */
klib_String *klib_string_new_split_before (const char *s, 
    const char *pattern);

/** Returns a new string containing that part of the input string after,
but not including, the pattern. If the pattern is not found, returns
an empty string */
klib_String *klib_string_new_split_after (const char *s, 
    const char *pattern);

/** Creates a new substring containing a substring of the source string */
klib_String *klib_string_new_substring (const char *s, 
    int pos, int len);

/** Returns the length of this string, in BYTES. The number of characters
may be shorter than this. 
Length of a null string is zero, as is an empty string */
int klib_string_length (const klib_String *self);

/** Makes a copy of the string */
klib_String *klib_string_strdup (const klib_String *self);

/** Removes len character from position start */
void klib_string_remove (klib_String *self, int start, int len);

/** Appends on byte to the string */
BOOL klib_string_append_byte (klib_String *self, BYTE byte);

/** Appends another string to this one. Returns TRUE if the append
succeeds. The only reason for failure is OOM. */
BOOL klib_string_append (klib_String *self, const char *s);

BOOL klib_string_starts_with (const klib_String *self, const char *s);

/* Returns TRUE if the string is zero-length. Note that a null string is
also considered to be empty */
BOOL klib_string_is_empty (const klib_String *self);

/* Returns the index of 's' within the string. If not found, returns
-1 */
int klib_string_index_of (const klib_String *self, const char *s);

void klib_string_search_replace (klib_String *self,
     const char *pattern, const char *replace);

/** Replaces all occurences of 'pattern' with 'replace'. Pattern may 
contain the wildcard sequence ___ which matches anything */
klib_String *klib_string_new_search_replace (const klib_String *self,
     const char *pattern, const char *replace);

/** Appends a unicode code-point to this string. The string's length in
bytes may grow between one and five */
void klib_string_append_unicode_char (klib_String *self, int ch);

/** Makes a new string use the printf-style argument list */
klib_String *klib_string_new_printf (const char *fmt,...);

/** Returns true if strings have identical contents */ 
BOOL klib_string_equals (const klib_String *self, const char *other); 

/** Read text from stdin until EOF, or memory runs out. This function
may return NULL, but only if memory fails */
klib_String *klib_string_read_stdin (void);

/** Remove CR/LF from end of string */
void klib_string_chomp (klib_String *self);

void klib_string_init (klib_Object *self);

void klib_string_dispose (klib_Object *self);

/** Splits a string into a list of strings, separated by delim. 
The delimiter is not included in the output. The return may be an
empty list, but should never by NULL */
struct _klib_List *klib_string_split (const char *s, char delim);

void klib_string_append_wchar (klib_String *self, wchar_t c);
 

KLIB_END_DECLS

