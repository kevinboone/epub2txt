#pragma once

#include <stdarg.h>
#include <time.h>
#include "klib_defs.h"
#include "klib_string.h"
#include "klib_error.h"

struct _klib_Path_priv;
struct _klib_Error;
struct _klib_Path;

typedef struct _klib_Path
  {
  klib_String base;
  BOOL disposing;
  struct _klib_Path_priv *priv;
  } klib_Path;


KLIB_BEGIN_DECLS

/** Creates a new klib_Path with char* initial value */
klib_Path *klib_path_new (const char *s);

void klib_path_append (klib_Path *self, const char *file);
void klib_path_free (klib_Path *self);
const char *klib_path_cstr (const klib_Path *self);

/** Get mtime. Returns zero if mtime cannot be read */
time_t klib_path_get_mtime (klib_Path *self);

/** Gets the file size, time, etc., info. This function is called
implicitly by get_XXX methods that require the information */
void klib_info_refresh_info (klib_Path *self, klib_Error **error);

BOOL klib_path_is_file (klib_Path *self);
BOOL klib_path_is_dir (klib_Path *self);

char * _klib_path_get_shortname (const char *path);
char *klib_path_get_shortname (const klib_Path *self);

char *_klib_path_get_dir (const klib_Path *self);
klib_Path *klib_path_get_dir (const klib_Path *self);

/** Returns the full path, even in the program was invoked using a relative
path or link */
klib_Path *klib_path_get_full_argv0 (void);

/** Returns a home directory appropriate for the platform */
klib_Path *klib_path_get_home_dir (void);

/** Returns the file extension, or an empty string if there is none.
Caller must free the string, which will always be non-null except 
in memory failures */
char *klib_path_get_ext (const klib_Path *self);

KLIB_END_DECLS

