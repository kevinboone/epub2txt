#pragma once

#include "klib_error.h"
#include "klib_string.h"

// Global variable to indicate how often paragraphs should be marked 
//  in the output text
extern int para_mark;

// Global variable to indicate which paragraph to start output at 
extern int start_para;

void epub2txt_do_file (const char *file, BOOL ascii, int width, 
  BOOL notrim, klib_Error **error);

