/*========================================================================
  epub2txt
  Copyright (c)2012-2016 Kevin Boone
  Distributed under the terms of the GPV, version 2.0 
=========================================================================*/

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "klib_log.h"
#include "klib_path.h"
#include "klib_list.h"
#include "klib_getopt.h" 
#include "klib_getoptspec.h" 
#include "epub2txt.h" 


/*========================================================================
  show_short_usage 
=========================================================================*/
void show_short_usage (FILE *f, const char *argv0)
  {
  fprintf (f, "Usage: %s [-anvw] {files...}\n", argv0);
  fprintf (f, "'%s --longhelp' for more details.\n", argv0); 
  }


/*========================================================================
  show_long_usage 
=========================================================================*/
void show_long_usage (FILE *f, const char *argv0)
  {
  fprintf (f, "Usage: %s [options...] [expression]\n", argv0);
  fprintf (f, "  -a,--ascii                ASCII output\n");
  fprintf (f, "  -d,--debug {level}        Set debug level (0-4)\n");
  fprintf (f, "  --longhelp                Detailed usage\n");
  fprintf (f, "  -n,--notrim               Do not trim whitespace\n");
  fprintf (f, 
   "  -p,--paras {count}        Write paragraph count every {count} paras\n");
  fprintf (f, 
   "  -s,--start {para}         Start output from paragraph {para}\n");
  fprintf (f, "  -v,--version              Show version and configuration\n");
  fprintf (f, "  -w,--width {cols}         Format for cols columns\n");
  fprintf (f, "If no width is specified, lines will not be broken except\n");
  fprintf (f, "on paragraph boundaries\n");
  }


/*========================================================================
  show_version
=========================================================================*/
void show_version (FILE *f, const char *argv0)
  {
  fprintf (f, "epub2txt version " VERSION "\n");
  fprintf (f, "Copyright (c)2013-2016 Kevin Boone\n");
  }


/*=============================================================================
  main()
=============================================================================*/
int main (int argc, char **argv)
  {
  const char *argv0 = argv[0];

  klib_GetOpt *getopt = klib_getopt_new ();
  klib_getopt_add_spec (getopt, "help", "help", 'h', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "longhelp", "longhelp", 0, KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "version", "version", 'v', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "ascii", "ascii", 'a', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "paras", "paras", 'p', KLIB_GETOPT_COMPARG);
  klib_getopt_add_spec (getopt, "start", "start", 's', KLIB_GETOPT_COMPARG);
  klib_getopt_add_spec (getopt, "width", "width", 'w', KLIB_GETOPT_COMPARG);
  klib_getopt_add_spec (getopt, "debug", "debug", 'd', KLIB_GETOPT_COMPARG);
  klib_getopt_add_spec (getopt, "notrim", "notrim", 'n', KLIB_GETOPT_NOARG);

  klib_Error *error = NULL;

  klib_getopt_parse (getopt, argc, (const char **)argv, &error);

  if (!error)
    {
    BOOL stopping_option = FALSE;

    if (klib_getopt_arg_set (getopt, "help"))
      {
      show_short_usage (stdout, argv[0]);
      stopping_option = TRUE;
      }
    else if (klib_getopt_arg_set (getopt, "longhelp"))
      {
      show_long_usage (stdout, argv[0]);
      stopping_option = TRUE;
      }
    else if (klib_getopt_arg_set (getopt, "version"))
      {
      show_version (stdout, argv[0]);
      stopping_option = TRUE;
      }

    if (!stopping_option)
      {
      BOOL ascii = klib_getopt_arg_set (getopt, "ascii");
      BOOL notrim = klib_getopt_arg_set (getopt, "notrim");
      const char *s_width = klib_getopt_get_arg (getopt, "width");
      int width = 0;
      if (s_width)
        width = atoi (s_width); 
      const char *s_paras = klib_getopt_get_arg (getopt, "paras");
      para_mark = 0;
      if (s_paras)
        para_mark = atoi (s_paras); 
      const char *s_start = klib_getopt_get_arg (getopt, "start");
      start_para = 0;
      if (s_start)
        start_para = atoi (s_start); 
      const char *s_debug = klib_getopt_get_arg (getopt, "debug");
      if (s_debug)
        klib_log_set_level (atoi (s_debug)); 
      else
        klib_log_set_level (KLIB_LOG_WARNING);
      int i, argc = klib_getopt_argc (getopt);
      for (i = 0; i < argc; i++)
        {
        const char *argv = klib_getopt_argv (getopt, i);
        klib_Error *error = NULL;
        klib_log_info ("Processing EPUB file %s", argv);
        epub2txt_do_file (argv, ascii, width, notrim, &error); 
        if (error)
          {
          klib_log_error ("%s: %s\n", argv0, klib_error_cstr (error));
          klib_error_free (error);
          }
        }
      }
    }
  else
    {
    // Error parsing command line
    fprintf (stderr, "%s\n", klib_error_cstr (error));
    klib_error_free (error);
    }
  klib_getopt_free (getopt);
  return 0;
  }

