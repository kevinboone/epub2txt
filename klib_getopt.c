/*===========================================================================
klib
klib_getopt.c
(c)2000-2012 Kevin Boone
============================================================================*/

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "klib_defs.h"
#include "klib_log.h"
#include "klib_getopt.h"
#include "klib_getoptspec.h"
#include "klib_error.h"
#include "klib_string.h"
#include "klib_list.h"

/*===========================================================================
private data
============================================================================*/
extern void klib_getopt_init (klib_Object *self);
void klib_getopt_dispose (klib_Object *self);

static klib_Spec klib_spec_getopt =
  {
  obj_size: sizeof (klib_GetOpt), 
  init_fn: klib_getopt_init,
  class_name: "klib_GetOpt"
  };

typedef struct _klib_GetOpt_priv
  {
  klib_List *specs;
  klib_List *argv;
  } klib_GetOpt_priv;

extern void klib_getopt_add_to_argv (klib_GetOpt *self, const char *argv);

/*===========================================================================
klib_getopt_new
============================================================================*/
klib_GetOpt *klib_getopt_new ()
  {
  KLIB_IN
  klib_GetOpt *ret = (klib_GetOpt*) klib_object_new(&klib_spec_getopt);
  KLIB_OUT
  return ret;
  }


/*===========================================================================
klib_getopt_init
============================================================================*/
void klib_getopt_init (klib_Object *_self)
  {
  KLIB_IN
  klib_object_init (_self);
  _self->dispose = klib_getopt_dispose;
  klib_GetOpt *self = (klib_GetOpt *)_self;
  self->priv = (klib_GetOpt_priv *)malloc (sizeof (klib_GetOpt_priv));
  memset (self->priv, 0, sizeof (klib_GetOpt_priv));
  self->priv->specs = klib_list_new ();
  self->priv->argv = klib_list_new ();
  KLIB_OUT
  }

/*===========================================================================
klib_getopt_dispose
============================================================================*/
void klib_getopt_dispose (klib_Object *_self)
  {
  KLIB_IN
  klib_GetOpt *self = (klib_GetOpt *)_self;
  if (!self->disposing)
    {
    self->disposing = TRUE;
    if (self->priv) 
      {
      int i, l = klib_list_length (self->priv->specs);
      for (i = 0; i < l; i++)
        {
        klib_GetOptSpec *spec = 
          (klib_GetOptSpec*) klib_list_get (self->priv->specs, i);
        klib_getoptspec_free (spec);
        }
      klib_list_free (self->priv->specs);
      klib_list_free (self->priv->argv);
      free (self->priv);
      }
    }
  klib_object_dispose ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_getopt_free
============================================================================*/
void klib_getopt_free (klib_GetOpt *self)
  {
  KLIB_IN
  klib_object_unref ((klib_Object *)self);
  KLIB_OUT
  }

/*===========================================================================
klib_getopt_add_spec_object
============================================================================*/
void klib_getopt_add_spec_object (klib_GetOpt *self, klib_GetOptSpec *spec)
  {
  KLIB_IN
  klib_list_append (self->priv->specs, (klib_Object *)spec);
  KLIB_OUT
  }

/*===========================================================================
klib_getopt_add_spec
============================================================================*/
void klib_getopt_add_spec (klib_GetOpt *self, const char *name,
    const char *longopt, char shortopt, int flags)
  {
  KLIB_IN

  klib_GetOptSpec *spec = klib_getoptspec_new (name, longopt, shortopt, flags);
  klib_getopt_add_spec_object (self, spec);
  // We don't need to keep a ref to this object
  //klib_object_unref ((klib_Object *)spec);

  KLIB_OUT
  }

/*===========================================================================
klib_getopt_add_to_argv
============================================================================*/
void klib_getopt_add_to_argv (klib_GetOpt *self, const char *argv)
  {
  klib_String *arg = klib_string_new (argv);
  klib_list_append (self->priv->argv, (klib_Object *)arg);
  klib_object_unref ((klib_Object *)arg);
  }

/*===========================================================================
klib_getopt_get_spec_by_longname
============================================================================*/
klib_GetOptSpec *klib_getopt_get_spec_by_longname (klib_GetOpt *self, 
    const char *arg)
  {
  KLIB_IN
  klib_GetOptSpec * ret = NULL;
  
  int i, l = klib_list_length (self->priv->specs);
  for (i = 0; i < l && !ret; i++)
    {
    klib_GetOptSpec *spec = (klib_GetOptSpec *)klib_list_get 
      (self->priv->specs, i);
    if (strcmp (klib_getoptspec_get_longopt (spec), arg) == 0)
      ret = spec;
    }
  
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_getopt_get_spec_by_longname
============================================================================*/
klib_GetOptSpec *klib_getopt_get_spec_by_shortname (klib_GetOpt *self, 
    char arg)
  {
  KLIB_IN
  klib_GetOptSpec * ret = NULL;
 
  int i, l = klib_list_length (self->priv->specs);
  for (i = 0; i < l && !ret; i++)
    {
    klib_GetOptSpec *spec = (klib_GetOptSpec *)klib_list_get 
      (self->priv->specs, i);
    if (klib_getoptspec_get_shortopt (spec) == arg)
      ret = spec;
    }
  
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_getopt_get_spec_by_name
============================================================================*/
klib_GetOptSpec *klib_getopt_get_spec_by_name (klib_GetOpt *self, 
    const char* name)
  {
  KLIB_IN
  klib_GetOptSpec * ret = NULL;
  
  int i, l = klib_list_length (self->priv->specs);
  for (i = 0; i < l && !ret; i++)
    {
    klib_GetOptSpec *spec = (klib_GetOptSpec *)klib_list_get 
      (self->priv->specs, i);
    if (strcmp (klib_getoptspec_get_name (spec), name) == 0)
      ret = spec;
    }
  
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_getopt_parse
============================================================================*/
void klib_getopt_parse (klib_GetOpt *self, int argc, const char **argv, 
    klib_Error **error)
  {
  KLIB_IN

  int i = 1; // skip argv0

  while (i < argc && !*error)
    {
    if (strlen (argv[i]) > 1)
      {
      BOOL is_switch = FALSE;
      if (argv[i][0] == '-')
        {
        // -1 is not a switch -- it is likely to be a number
        is_switch = TRUE;
        if (strlen (argv[i])) 
          {
          if (isdigit (argv[i][1]))
            is_switch = FALSE;
          }
        }
      if (is_switch)
        {
        // It's a switch
        const char *argstr = argv[i] + 1; // skip -    
        BOOL longopt = FALSE;
        if (argstr[0] == '-')
          {
          argstr++;
          longopt = TRUE;
          }
        if (strlen (argstr) >= 1)
          {
          char *p = strchr (argstr, '=');
          if (p > 0)
            {
            // arg of the form --a=b

            klib_String *key = klib_string_new_split_before (argstr, "=");
            klib_String *value =klib_string_new_split_after (argstr, "=");
    
            klib_GetOptSpec *spec = klib_getopt_get_spec_by_longname 
             (self, klib_string_cstr (key));
            if (spec) 
              {
              kilb_getoptspec_set_arg (spec, klib_string_cstr(value));
              }
            else
              {
              char buff[256];
              snprintf (buff, sizeof (buff), 
                  "Unrecognized command-line switch: %s", 
                    klib_string_cstr (key));
              *error = klib_error_new (KLIB_ERR_PARSE_CMDLINE, 
                  klib_error_strerror (KLIB_ERR_PARSE_CMDLINE), 
                    buff); 
              }
           
            klib_string_free (key);
            klib_string_free (value);

            i++;
            }
          else
            {
            klib_GetOptSpec *spec = NULL; 
            if (longopt) 
              {
              // TODO
              spec = klib_getopt_get_spec_by_longname (self, argstr);
              if (spec) 
                {
                if (klib_getoptspec_get_flags (spec) & KLIB_GETOPT_COMPARG)
                  {
                  // Needs an arg
                  if (i < argc - 1)
                    {
                    // TODO set spec flag and arg to match
                    kilb_getoptspec_set_arg (spec, argv[i+1]);

                    i++;
                    }
                  else
                    {
                    char buff[256];
                   snprintf (buff, sizeof (buff), 
                     "Command-line switch '%s' needs an argument", 
                       argstr);
                   *error = klib_error_new (KLIB_ERR_PARSE_CMDLINE, 
                      klib_error_strerror (KLIB_ERR_PARSE_CMDLINE), 
                        buff); 
                    }
                  }
                else
                  {
                  // Does not need an arg
                  //TODO set spec flag to match
                  kilb_getoptspec_set_arg (spec, KLIB_GETOPT_ARGSET);
                  }
                }
              else
                {
                char buff[256];
                snprintf (buff, sizeof (buff), 
                  "Unrecognized command-line switch: %s", argv[i]);
                *error = klib_error_new (KLIB_ERR_PARSE_CMDLINE, 
                  klib_error_strerror (KLIB_ERR_PARSE_CMDLINE), 
                    buff); 
                }
              }
            else
              {
              int j, m = strlen (argstr);
              for (j = 0; j < m && !*error; j++)
                {
                spec = klib_getopt_get_spec_by_shortname (self, argstr[j]);
                if (spec)
                  {
                  if (klib_getoptspec_get_flags (spec) & KLIB_GETOPT_COMPARG)
                    {
                    // Needs an arg
                    if (i < argc - 1)
                      {
                      // TODO set spec flag and arg to match
                      kilb_getoptspec_set_arg (spec, argv[i+1]);

                      i++;
                      }
                    else
                      {
                      char buff[256];
                     snprintf (buff, sizeof (buff), 
                       "Command-line switch '%c' needs an argument", 
                         argstr[j]);
                     *error = klib_error_new (KLIB_ERR_PARSE_CMDLINE, 
                        klib_error_strerror (KLIB_ERR_PARSE_CMDLINE), 
                          buff); 
                      }
                    }
                  else
                    {
                    // Does not need an arg
                    //TODO set spec flag to match
                    kilb_getoptspec_set_arg (spec, KLIB_GETOPT_ARGSET);
                    }
                  }
                else
                  {
                  char buff[256];
                  snprintf (buff, sizeof (buff), 
                    "Unrecognized command-line switch: %c", argstr[j]);
                  *error = klib_error_new (KLIB_ERR_PARSE_CMDLINE, 
                    klib_error_strerror (KLIB_ERR_PARSE_CMDLINE), 
                      buff); 
                  }
                }
              }
            i++;
            }
          }
        else
          {
          char buff[256];
          snprintf (buff, sizeof (buff), 
            "Empty command-line switch: %s", argv[i]);
          *error = klib_error_new (KLIB_ERR_PARSE_CMDLINE, 
            klib_error_strerror (KLIB_ERR_PARSE_CMDLINE), 
             buff); 
          }
        }
      else
        {
        // Not a switch
        klib_getopt_add_to_argv (self, argv[i]);
        i++;
        }
      }
    else
      {
      klib_getopt_add_to_argv (self, argv[i]);
      i++;
      }
    }

  KLIB_OUT
  }


/*===========================================================================
klib_getopt_argc
============================================================================*/
int klib_getopt_argc (const klib_GetOpt *self)
  {
  return klib_list_length (self->priv->argv);
  }

/*===========================================================================
klib_getopt_argv
============================================================================*/
const char *klib_getopt_argv (const klib_GetOpt *self, int index)
  {
  KLIB_IN
  const char *ret = NULL;
  int l = klib_list_length (self->priv->argv);
  if (index < l && index >= 0)
    {
    klib_String *s = (klib_String *)klib_list_get (self->priv->argv, index);
    ret = klib_string_cstr (s); 
    }
   else
     klib_log_warning ("Index %d out of range in getopt_argv", index);
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_getopt_arg_set
============================================================================*/
BOOL klib_getopt_arg_set (const klib_GetOpt *self, const char *name)
  {
  KLIB_IN
  BOOL ret = (klib_getopt_get_arg (self, name) != NULL);
  KLIB_OUT
  return ret;
  }

/*===========================================================================
klib_getopt_get_arg
============================================================================*/
const char *klib_getopt_get_arg (const klib_GetOpt *self, const char *name)
  {
  KLIB_IN
  const char *ret = NULL;
  klib_GetOptSpec *spec = klib_getopt_get_spec_by_name 
    ((klib_GetOpt *)self, name);
  if (spec)
    {
    ret = klib_getoptspec_get_arg (spec);
    }
 // else
  //  klib_log_warning ("");

  KLIB_OUT
  return ret;
  }


