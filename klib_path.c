/*===========================================================================
klib
klib_path.c
(c)2000-2012 Kevin Boone
============================================================================*/
#ifdef WIN32
#include <windows.h>
#endif
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "klib_defs.h"
#include "klib_log.h"
#include "klib_string.h"
#include "klib_path.h"
#include "klib_error.h"

/*===========================================================================
private data
============================================================================*/
extern void klib_path_init (klib_Object *self);
void klib_path_dispose (klib_Object *self);

static klib_Spec klib_spec_path =
  {
  obj_size: sizeof (klib_Path), 
  init_fn: klib_path_init,
  class_name: "klib_Path"
  };

typedef struct _klib_Path_priv
  {
  struct stat sb;
  BOOL stat_valid;
  } klib_Path_priv;

/*===========================================================================
klib_path_init
============================================================================*/
void klib_path_init (klib_Object *_self)
  {
  KLIB_IN
  klib_string_init (_self);
  _self->dispose = klib_path_dispose;
  klib_Path *self = (klib_Path *)_self;
  self->priv = (klib_Path_priv *) malloc (sizeof (klib_Path_priv));
  memset (self->priv, 0, sizeof (klib_Path_priv));
  KLIB_OUT
  } 


/*===========================================================================
klib_path_new
============================================================================*/
klib_Path *klib_path_new (const char *s)
  {
  KLIB_IN
  klib_Path *self = (klib_Path *)klib_object_new (&klib_spec_path); 
  klib_string_set ((klib_String *)self, s);
  self->priv->stat_valid = FALSE;
  KLIB_OUT
  return self;
  }

/*===========================================================================
klib_path_new
============================================================================*/
void klib_path_append (klib_Path *self, const char *file)
  {
  klib_String *_self = (klib_String *)self;
  const char *s_self = klib_string_cstr (_self);
  if (strlen (s_self) == 0)
    {
    // Don't accidentally make an absolute path by appending a
    //  separator onto an empty string
    }
  else
    {
 #ifdef WIN32
    if (s_self[strlen(s_self) - 1] != '\\')
       klib_string_append (_self, "\\");
#else
    if (s_self[strlen(s_self) - 1] != '/')
       klib_string_append (_self, "/");
#endif
    }
  klib_string_append (_self, file);
  self->priv->stat_valid = FALSE;
  }

/*===========================================================================
klib_path_new
============================================================================*/
void klib_path_free (klib_Path *self)
  {
  klib_object_unref ((klib_Object *)self);
  }

/*===========================================================================
klib_path_new
============================================================================*/
const char *klib_path_cstr (const klib_Path *self)
  {
  return klib_string_cstr ((klib_String *)self);
  }

/*===========================================================================
klib_path_refresh_info 
============================================================================*/
void klib_path_refresh_info (klib_Path *self, klib_Error **error)
  {
  self->priv->stat_valid = FALSE;

  char *_path = strdup (klib_path_cstr (self));

#ifdef WIN32
   if (_path[strlen(_path) - 1] == '\\')
     _path[strlen(_path) - 1] = 0;
#endif

  if (stat (_path, &self->priv->sb) == 0)
    {
    self->priv->stat_valid = TRUE;
    }
  else
    {
    *error = klib_error_new (errno, "Error %d: %s (%s)",
      klib_error_strerror (errno), "Stat call failed", 
        klib_path_cstr (self));
    }

  free (_path);
  }

/*===========================================================================
klib_path_get_mtime
============================================================================*/
time_t klib_path_get_mtime (klib_Path *self)
  {
  if (!self->priv->stat_valid)
    {
    klib_Error *error = NULL;
    klib_path_refresh_info (self, &error);
    if (error) klib_error_free (error);
    }
  return self->priv->sb.st_mtime;
  }


/*===========================================================================
klib_path_get_is_file
============================================================================*/
BOOL klib_path_is_file (klib_Path *self)
  {
  if (!self->priv->stat_valid)
    {
    klib_Error *error = NULL;
    klib_path_refresh_info (self, &error);
    if (error) klib_error_free (error);
    }
  return S_ISREG (self->priv->sb.st_mode);
  }


/*===========================================================================
klib_path_get_is_dir
============================================================================*/
BOOL klib_path_is_dir (klib_Path *self)
  {
  if (!self->priv->stat_valid)
    {
    klib_Error *error = NULL;
    klib_path_refresh_info (self, &error);
if (error)
    if (error) klib_error_free (error);
    }
  return S_ISDIR (self->priv->sb.st_mode);
  }


/*===========================================================================
klib_path_dispose
============================================================================*/
void klib_path_dispose (klib_Object *_self)
  {
  KLIB_IN
  klib_Path *self = (klib_Path *)_self;
  if (!self->disposing)
    {
    self->disposing = TRUE;
    if (self->priv) 
      {
      free (self->priv);
      }
    }
  klib_string_dispose ((klib_Object *)self);
  KLIB_OUT
  }


/*===========================================================================
_klib_path_get_shortname
============================================================================*/
char *_klib_path_get_shortname (const char *path)
  {
  char *ret = NULL;
  KLIB_IN
  if (path) 
    {
#ifdef WIN32
    const char *p = strrchr (path, '\\');
#else
    const char *p = strrchr (path, '/');
#endif
    if (p)
      {
      ret = strdup (p + 1);
      }   
    else
      ret = strdup (path);
 
    }
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_path_get_shortname
============================================================================*/
char *klib_path_get_shortname (const klib_Path *self)
  {
  KLIB_IN
  char *ret = _klib_path_get_shortname (klib_path_cstr (self));
  KLIB_OUT
  return ret;
  }


/*===========================================================================
_klib_path_get_dir
============================================================================*/
char *_klib_path_get_dir (const klib_Path *self)
  {
  KLIB_IN
  char *ret = NULL;
  char *path = strdup (klib_path_cstr (self));
  if (path) 
    {
#ifdef WIN32
    char *p = strrchr (path, '\\');
#else
    char *p = strrchr (path, '/');
#endif
    if (p)
      {
      *(p) = 0;
      ret = strdup (path);
      }   
    else
      ret = strdup (""); // No path component
 
    }
  free (path);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
_klib_path_get_dir
============================================================================*/
klib_Path *klib_path_get_dir (const klib_Path *self)
  {
  KLIB_IN
  char *dir = _klib_path_get_dir (self);
  klib_Path *ret = klib_path_new (dir); 
  free (dir);
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_path_get_full_argv0
============================================================================*/
// The argument passed as argv[0] on Linux will not be very useful,
//  as it is not guaranteed to be a full path to anything. In fact, it's
//  not guaranteed even to be a relative path to anything, if we found
//  the file via $PATH. /proc/id/exe is the standard solution to
//  this problem on Linux
//  Caller must free the path object returned

klib_Path *klib_path_get_full_argv0 (void)
  {
#ifdef WIN32
  char buff[1024];
  GetModuleFileName (NULL, buff, sizeof (buff));
  return klib_path_new (buff);
#else
  char szTmp[32];
  char *buff = (char *) malloc (1024); // Should never be this long!
  memset (buff, 0, 1024);
  snprintf(szTmp, sizeof (szTmp), "/proc/%d/exe", getpid());
  readlink (szTmp, buff, 1023);
  klib_Path *p = klib_path_new (buff);
  free (buff);
  return p;
#endif
  }


/*==========================================================================
klib_path_get_home_dir
Returns a home directory appropriate for the plaform
============================================================================*/
klib_Path *klib_path_get_home_dir (void)
  {
#ifdef WIN32
  return klib_path_new (getenv("APPDATA"));
#else
  return klib_path_new (getenv("HOME"));
#endif
  }


/*==========================================================================
klib_path_get_ext
============================================================================*/
char *klib_path_get_ext (const klib_Path *self)
  {
  if (!self) return strdup ("");
  const char *path = klib_path_cstr (self);
  if (!path) return strdup ("");
  char *p = strrchr (path, '.');
  if (p) return strdup (p + 1);
  return strdup ("");
  }




