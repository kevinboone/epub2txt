/*========================================================================
  epub2txt
  Copyright (c)2012-2015 Kevin Boone
  Distributed under the terms of the GPV, version 2.0 
=========================================================================*/

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <errno.h>
#include <ctype.h>
#include "klib_log.h"
#include "klib_error.h"
#include "klib_path.h"
#include "klib_list.h"
#include "klib_getopt.h" 
#include "klib_getoptspec.h" 
#include "klib_xml.h" 
#include "klib_wstring.h" 
#include "epub2txt.h" 

/*========================================================================
  globals 
=========================================================================*/
// Output para is incremented each time a paragraph is completed and
//  rendered. It's global because it is used in a number of different
//  functions, as paras are contiguous across multiple source documents
int output_para = 0;

// Paragraph to start output at
int start_para = 0;

int para_mark = 0;

/*========================================================================
  epub2txt_get_items
=========================================================================*/
klib_List *epub2txt_get_items (const char *opf, klib_Error **error)
  {
  KLIB_OUT
  klib_Xml *x = klib_xml_read_file (opf, error);
  if (*error == NULL)
    {
    klib_log_debug ("Opened file %s", opf);
    BOOL got_manifest = FALSE;
    XMLNode *manifest = NULL;

    XMLNode *root = klib_xml_get_root (x); // package
    int i, l = root->n_children;
    for (i = 0; i < l; i++)
      {
      XMLNode *r1 = root->children[i];
      if (strcmp (r1->tag, "manifest") == 0)
        {
        manifest = r1;
        got_manifest = TRUE;
        }
      }

    if (!got_manifest)
      {
      *error = klib_error_new (ENOENT, "File %s has no manifest", opf);
      KLIB_OUT
      return NULL; 
      }
 
    klib_List *ret = klib_list_new ();

    for (i = 0; i < l; i++)
      {
      XMLNode *r1 = root->children[i];
      if (strcmp (r1->tag, "spine") == 0)
        {
        int j, l2 = r1->n_children;
        for (j = 0; j < l2; j++)
          {
          XMLNode *r2 = r1->children[j]; // itemref
          int k, nattrs = r2->n_attributes;
          for (k = 0; k < nattrs; k++)
            {
            char *name = r2->attributes[k].name;
            if (strcmp (name, "idref") == 0)
              {
              char *value= r2->attributes[k].value;
              int m;
              for (m = 0; m < manifest->n_children; m++)
                {
                XMLNode *r3 = manifest->children[m]; // item
                int n, nattrs = r3->n_attributes;
                for (n = 0; n < nattrs; n++)
                  {
                  char *name2 = r3->attributes[n].name;
                  char *val2 = r3->attributes[n].value;
                  if (strcmp (name2, "id") == 0 && 
                      strcmp (val2, value) == 0)
                    {
                    int p;
                    for (p = 0; p < nattrs; p++)
                      {
                      char *name2 = r3->attributes[p].name;
                      char *val2 = r3->attributes[p].value;
                      if (strcmp (name2, "href") == 0)
                        {
                        klib_String *ss = klib_string_new (val2);
                        klib_list_append (ret, (klib_Object *)ss);
                        klib_string_free (ss);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }

    klib_xml_free (x);
    KLIB_OUT
    return ret;
    }
  else
    {
    KLIB_OUT
    return NULL;
    }
  }

/*========================================================================
  epub2txt_translate_entity
  out must have space for at least five bytes
  // TODO -- ascii mode subsitutions
=========================================================================*/
void epub2txt_translate_entity (const char *in, char *out)
  {
  KLIB_IN
  if (strcasecmp (in, "amp") == 0) 
    strcpy (out, "&");
  else if (strcasecmp (in, "nbsp") == 0) 
    strcpy (out, " ");
  else if (strcasecmp (in, "lt") == 0) 
    strcpy (out, "<");
  else if (strcasecmp (in, "gt") == 0) 
    strcpy (out, ">");
  else if (strcasecmp (in, "cent") == 0) 
    strcpy (out, "¢");
  else if (strcasecmp (in, "pound") == 0) 
    strcpy (out, "£");
  else if (strcasecmp (in, "yen") == 0) 
    strcpy (out, "£");
  else if (strcasecmp (in, "euro") == 0) 
    strcpy (out, "€");
  else if (strcasecmp (in, "sect") == 0) 
    strcpy (out, "§");
  else if (strcasecmp (in, "copy") == 0) 
    strcpy (out, "©");
  else if (strcasecmp (in, "reg") == 0) 
    strcpy (out, "®");
  else if (strcasecmp (in, "trade") == 0) 
    strcpy (out, "™");
  else if (strcasecmp (in, "quot") == 0) 
    strcpy (out, "\"");
  else if (in[0] == '#')
    {
    char *s = strdup (in);
    s[0] = '0';
    int v = 0;
    if (sscanf (s, "%x", &v) == 1)
      {
      klib_String *ss = klib_string_new_empty ();
      klib_string_append_wchar (ss, (wchar_t)v);
      strcpy (out, klib_string_cstr (ss));
      klib_string_free (ss);
      } 
    free (s);
    }
  else
    strcpy (out, "?");
  KLIB_OUT
  }


/*========================================================================
  epub2txt_transform_char
=========================================================================*/
klib_String *epub2txt_transform_char (wchar_t c, BOOL ascii)
  {
  KLIB_IN
  // if (c > 127) printf ("XXXX c=%04X\n", c);
  if (ascii)
    {
    if (c == 0x00B4) return klib_string_new ("\'");
    if (c == 0x0304) return klib_string_new ("-");
    if (c == 0x2010) return klib_string_new ("-");
    if (c == 0x2013) return klib_string_new ("-");
    if (c == 0x2014) return klib_string_new ("-");
    if (c == 0x2018) return klib_string_new ("'");
    if (c == 0x2019) return klib_string_new ("\'");
    if (c == 0x201C) return klib_string_new ("\"");
    if (c == 0x201D) return klib_string_new ("\"");
    if (c == 0xC2A0) return klib_string_new ("(c)"); // copyright 
    if (c == 0x00A9) return klib_string_new ("(c)"); // ditto
    if (c == 0xC2A9) return klib_string_new (" "); // nbsp
    if (c == 0x00A0) return klib_string_new (" "); // nbsp
    if (c == 0x2026) return klib_string_new (",,,"); // elipsis 
    if (c == 0x2022) return klib_string_new ("."); // dot
    if (c == 0x00B5) return klib_string_new ("u"); // mu
    if (c == 0x00C0) return klib_string_new ("A"); // accented A 
    if (c == 0x00C1) return klib_string_new ("A"); // accented A 
    if (c == 0x00C2) return klib_string_new ("A"); // accented A 
    if (c == 0x00C3) return klib_string_new ("A"); // accented A 
    if (c == 0x00C4) return klib_string_new ("A"); // accented A 
    if (c == 0x00C5) return klib_string_new ("A"); // accented A 
    if (c == 0x00C6) return klib_string_new ("AE"); // accented A 
    if (c == 0x00C7) return klib_string_new ("C"); // cedilla 
    if (c == 0x00C8) return klib_string_new ("E"); // accented E
    if (c == 0x00C9) return klib_string_new ("E"); // accented E
    if (c == 0x00CA) return klib_string_new ("E"); // accented E
    if (c == 0x00CB) return klib_string_new ("E"); // accented E
    if (c == 0x00CC) return klib_string_new ("I"); // accented I
    if (c == 0x00CD) return klib_string_new ("I"); // accented I
    if (c == 0x00CE) return klib_string_new ("I"); // accented I
    if (c == 0x00CF) return klib_string_new ("I"); // accented I
    if (c == 0x00D0) return klib_string_new ("D"); // accented D
    if (c == 0x00D1) return klib_string_new ("N"); // accented N
    if (c == 0x00D2) return klib_string_new ("O"); // accented O
    if (c == 0x00D3) return klib_string_new ("O"); // accented O
    if (c == 0x00D4) return klib_string_new ("O"); // accented O
    if (c == 0x00D5) return klib_string_new ("O"); // accented O
    if (c == 0x00D6) return klib_string_new ("O"); // accented O
    if (c == 0x00D7) return klib_string_new ("x"); // Multiply 
    if (c == 0x00D8) return klib_string_new ("O"); // accented O
    if (c == 0x00D9) return klib_string_new ("U"); // accented U
    if (c == 0x00DA) return klib_string_new ("U"); // accented U
    if (c == 0x00DB) return klib_string_new ("U"); // accented U
    if (c == 0x00DC) return klib_string_new ("U"); // accented U
    if (c == 0x00DD) return klib_string_new ("Y"); // accented Y
    if (c == 0x00DE) return klib_string_new ("Y"); // thorn 
    if (c == 0x00DF) return klib_string_new ("sz"); // esszet 
    if (c == 0x00E0) return klib_string_new ("a"); // accepted a 
    if (c == 0x00E1) return klib_string_new ("a"); // accepted a 
    if (c == 0x00E2) return klib_string_new ("a"); // accepted a 
    if (c == 0x00E3) return klib_string_new ("a"); // accepted a 
    if (c == 0x00E4) return klib_string_new ("a"); // accepted a 
    if (c == 0x00E5) return klib_string_new ("a"); // accepted a 
    if (c == 0x00E6) return klib_string_new ("ae"); // ae
    if (c == 0x00E7) return klib_string_new ("c"); // cedilla
    if (c == 0x00E8) return klib_string_new ("e"); //a ceepnted e 
    if (c == 0x00E9) return klib_string_new ("e"); //a ceepnted e 
    if (c == 0x00EA) return klib_string_new ("e"); //a ceepnted e 
    if (c == 0x00EB) return klib_string_new ("e"); //a ceepnted e 
    if (c == 0x00EC) return klib_string_new ("i"); //a ceepnted i 
    if (c == 0x00ED) return klib_string_new ("i"); //a ceepnted i 
    if (c == 0x00EE) return klib_string_new ("i"); //a ceepnted i 
    if (c == 0x00EF) return klib_string_new ("i"); //a ceepnted i 
    if (c == 0x00F0) return klib_string_new ("o"); //a ceepnted o 
    if (c == 0x00F1) return klib_string_new ("n"); //a ceepnted n 
    if (c == 0x00F2) return klib_string_new ("o"); //a ceepnted o 
    if (c == 0x00F3) return klib_string_new ("o"); //a ceepnted o 
    if (c == 0x00F4) return klib_string_new ("o"); //a ceepnted o 
    if (c == 0x00F5) return klib_string_new ("o"); //a ceepnted o 
    if (c == 0x00F6) return klib_string_new ("o"); //a ceepnted o 
    if (c == 0x00F7) return klib_string_new ("/"); // divide
    if (c == 0x00F8) return klib_string_new ("o"); //a ceepnted o 
    if (c == 0x00F9) return klib_string_new ("u"); //a ceepnted u 
    if (c == 0x00FA) return klib_string_new ("u"); //a ceepnted u 
    if (c == 0x00FB) return klib_string_new ("u"); //a ceepnted u 
    if (c == 0x00FC) return klib_string_new ("u"); //a ceepnted u 
    if (c == 0x00FD) return klib_string_new ("y"); //a ceepnted y 
    if (c == 0x00FE) return klib_string_new ("y"); //a thorn 
    if (c == 0x00FF) return klib_string_new ("y"); //a ceepnted y 
    if (c == 0x0100) return klib_string_new ("A"); //a ceepnted A 
    if (c == 0x0101) return klib_string_new ("a"); //a ceepnted a 
    if (c == 0x0102) return klib_string_new ("A"); //a ceepnted A 
    if (c == 0x0103) return klib_string_new ("a"); //a ceepnted a 
    if (c == 0x0104) return klib_string_new ("A"); //a ceepnted A 
    if (c == 0x0105) return klib_string_new ("a"); //a ceepnted a 
    if (c == 0x0106) return klib_string_new ("C"); //a ceepnted C 
    if (c == 0x0107) return klib_string_new ("c"); //a ceepnted c 
    if (c == 0x0108) return klib_string_new ("C"); //a ceepnted C 
    if (c == 0x0109) return klib_string_new ("c"); //a ceepnted c 
    if (c == 0x010A) return klib_string_new ("C"); //a ceepnted C 
    if (c == 0x010B) return klib_string_new ("c"); //a ceepnted c 
    if (c == 0x010C) return klib_string_new ("C"); //a ceepnted C 
    if (c == 0x010D) return klib_string_new ("c"); //a ceepnted c 
    if (c == 0x010E) return klib_string_new ("D"); //a ceepnted D 
    if (c == 0x010F) return klib_string_new ("d"); //a ceepnted d 
    if (c == 0x0110) return klib_string_new ("D"); //a ceepnted D 
    if (c == 0x0111) return klib_string_new ("d"); //a ceepnted d 
    if (c == 0x0112) return klib_string_new ("E"); //a ceepnted E 
    if (c == 0x0113) return klib_string_new ("e"); //a ceepnted e 
    if (c == 0x0114) return klib_string_new ("E"); //a ceepnted E 
    if (c == 0x0115) return klib_string_new ("e"); //a ceepnted e 
    if (c == 0x0116) return klib_string_new ("E"); //a ceepnted E 
    if (c == 0x0117) return klib_string_new ("e"); //a ceepnted e 
    if (c == 0x0118) return klib_string_new ("E"); //a ceepnted E 
    if (c == 0x0119) return klib_string_new ("e"); //a ceepnted e 
    if (c == 0x011A) return klib_string_new ("E"); //a ceepnted E 
    if (c == 0x011B) return klib_string_new ("e"); //a ceepnted e 
    if (c == 0x011C) return klib_string_new ("G"); //a ceepnted G 
    if (c == 0x011D) return klib_string_new ("g"); //a ceepnted g 
    if (c == 0x011E) return klib_string_new ("G"); //a ceepnted G 
    if (c == 0x011F) return klib_string_new ("g"); //a ceepnted g 
    if (c == 0x0120) return klib_string_new ("G"); //a ceepnted G 
    if (c == 0x0121) return klib_string_new ("g"); //a ceepnted g 
    if (c == 0x0122) return klib_string_new ("G"); //a ceepnted G 
    if (c == 0x0123) return klib_string_new ("g"); //a ceepnted g 
    if (c == 0x0124) return klib_string_new ("H"); //a ceepnted H 
    if (c == 0x0125) return klib_string_new ("h"); //a ceepnted h 
    if (c == 0x0126) return klib_string_new ("H"); //a ceepnted H 
    if (c == 0x0127) return klib_string_new ("h"); //a ceepnted h 
    if (c == 0x0128) return klib_string_new ("I"); //a ceepnted I 
    if (c == 0x0129) return klib_string_new ("i"); //a ceepnted i 
    if (c == 0x012A) return klib_string_new ("I"); //a ceepnted I 
    if (c == 0x012B) return klib_string_new ("i"); //a ceepnted i 
    if (c == 0x012C) return klib_string_new ("I"); //a ceepnted I 
    if (c == 0x012D) return klib_string_new ("i"); //a ceepnted i 
    if (c == 0x012E) return klib_string_new ("I"); //a ceepnted I 
    if (c == 0x012F) return klib_string_new ("i"); //a ceepnted i 
    if (c == 0x0130) return klib_string_new ("I"); //a ceepnted I 
    if (c == 0x0131) return klib_string_new ("i"); //a ceepnted i 
    if (c == 0x0132) return klib_string_new ("IJ"); 
    if (c == 0x0133) return klib_string_new ("ij"); 
    if (c == 0x0134) return klib_string_new ("J"); //a ceepnted J 
    if (c == 0x0135) return klib_string_new ("j"); //a ceepnted j 
    if (c == 0x0136) return klib_string_new ("K"); //a ceepnted K 
    if (c == 0x0138) return klib_string_new ("K"); //a ceepnted K 
    if (c == 0x0138) return klib_string_new ("k"); //a ceepnted k 
    if (c == 0x0139) return klib_string_new ("L"); //a ceepnted L 
    if (c == 0x013A) return klib_string_new ("l"); //a ceepnted l 
    if (c == 0x013B) return klib_string_new ("L"); //a ceepnted L 
    if (c == 0x013C) return klib_string_new ("l"); //a ceepnted l 
    if (c == 0x013D) return klib_string_new ("L"); //a ceepnted L 
    if (c == 0x013E) return klib_string_new ("l"); //a ceepnted l 
    if (c == 0x013F) return klib_string_new ("L"); //a ceepnted L 
    if (c == 0x0140) return klib_string_new ("l"); //a ceepnted l 
    if (c == 0x0141) return klib_string_new ("L"); //a ceepnted L 
    if (c == 0x0142) return klib_string_new ("l"); //a ceepnted l 
    if (c == 0x0143) return klib_string_new ("N"); //a ceepnted N 
    if (c == 0x0144) return klib_string_new ("n"); //a ceepnted N 
    if (c == 0x0145) return klib_string_new ("N"); //a ceepnted N 
    if (c == 0x0146) return klib_string_new ("n"); //a ceepnted N 
    if (c == 0x0147) return klib_string_new ("N"); //a ceepnted N 
    if (c == 0x0148) return klib_string_new ("n"); //a ceepnted N 
    if (c == 0x0149) return klib_string_new ("N"); //a ceepnted N 
    if (c == 0x014A) return klib_string_new ("n"); //a ceepnted N 
    if (c == 0x014B) return klib_string_new ("n"); //a ceepnted n 
    if (c == 0x014C) return klib_string_new ("O"); //a ceepnted O 
    if (c == 0x014D) return klib_string_new ("o"); //a ceepnted o 
    if (c == 0x014E) return klib_string_new ("O"); //a ceepnted O 
    if (c == 0x014F) return klib_string_new ("o"); //a ceepnted o 
    if (c == 0x0150) return klib_string_new ("O"); //a ceepnted O 
    if (c == 0x0151) return klib_string_new ("o"); //a ceepnted o 
    if (c == 0x0152) return klib_string_new ("OE"); 
    if (c == 0x0153) return klib_string_new ("oe"); 
    if (c == 0x0154) return klib_string_new ("R"); // accepted R
    if (c == 0x0155) return klib_string_new ("r"); // accepted r
    if (c == 0x0156) return klib_string_new ("R"); // accepted R
    if (c == 0x0157) return klib_string_new ("r"); // accepted r
    if (c == 0x0158) return klib_string_new ("R"); // accepted R
    if (c == 0x0159) return klib_string_new ("r"); // accepted r
    if (c == 0x015A) return klib_string_new ("S"); // accepted S
    if (c == 0x015B) return klib_string_new ("s"); // accepted s
    if (c == 0x015C) return klib_string_new ("S"); // accepted S
    if (c == 0x015D) return klib_string_new ("s"); // accepted s
    if (c == 0x015E) return klib_string_new ("S"); // accepted S
    if (c == 0x015F) return klib_string_new ("s"); // accepted s
    if (c == 0x0160) return klib_string_new ("S"); // accepted S
    if (c == 0x0161) return klib_string_new ("s"); // accepted s
    if (c == 0x0162) return klib_string_new ("T"); // accepted T
    if (c == 0x0163) return klib_string_new ("t"); // accepted t
    if (c == 0x0164) return klib_string_new ("T"); // accepted T
    if (c == 0x0165) return klib_string_new ("t"); // accepted t
    if (c == 0x0166) return klib_string_new ("T"); // accepted T
    if (c == 0x0167) return klib_string_new ("t"); // accepted t
    if (c == 0x0168) return klib_string_new ("U"); // accepted U
    if (c == 0x0169) return klib_string_new ("u"); // accepted u
    if (c == 0x016A) return klib_string_new ("U"); // accepted U
    if (c == 0x016B) return klib_string_new ("u"); // accepted u
    if (c == 0x016C) return klib_string_new ("U"); // accepted U
    if (c == 0x016D) return klib_string_new ("u"); // accepted u
    if (c == 0x016E) return klib_string_new ("U"); // accepted U
    if (c == 0x016F) return klib_string_new ("u"); // accepted u
    if (c == 0x0170) return klib_string_new ("U"); // accepted U
    if (c == 0x0171) return klib_string_new ("u"); // accepted u
    if (c == 0x0172) return klib_string_new ("U"); // accepted U
    if (c == 0x0173) return klib_string_new ("u"); // accepted u
    if (c == 0x0174) return klib_string_new ("W"); // accepted W
    if (c == 0x0175) return klib_string_new ("w"); // accepted w
    if (c == 0x0176) return klib_string_new ("Y"); // accepted Y
    if (c == 0x0177) return klib_string_new ("y"); // accepted y
    if (c == 0x0178) return klib_string_new ("Y"); // accepted Y
    if (c == 0x00) return klib_string_new (""); // 
    if (c > 127) return klib_string_new ("?");
    }
     
  klib_String *ret = klib_string_new_empty();
  klib_string_append_wchar (ret, c);
  KLIB_OUT
  return ret; 
  }


/*========================================================================
  epub2txt_all_white
  Note that an empty string is considered to be whitespace
  Note also that we struggle here with unicode non-breaking spaces,
  since some of these are multibyte, and we get the data in this
  function byte-by-byte
=========================================================================*/
BOOL epub2txt_all_white (const char *s)
  {
  KLIB_IN
  int i, l = strlen (s);
  for (i = 0; i < l; i++)
    {
    BOOL iswhite = FALSE;
    char c = s[i];
    if (c == ' ') iswhite = TRUE;
    if (c == '\n') iswhite = TRUE;
    if (c == (char)0xC2) // nbsp 
      {
      if (i < l - 1)
        {
        if ((unsigned char)s[i + 1] == (unsigned char)0xA0) 
          {
          iswhite = TRUE;
          i++;
          }
        }
      }
    if (!iswhite) 
      {
      KLIB_OUT
      return FALSE;
      }
    }
      KLIB_OUT
  return TRUE;
  }


/*========================================================================
  epub2txt_flush_para
=========================================================================*/
void epub2txt_flush_para (const klib_String *para, int width, BOOL notrim)
  {
  KLIB_IN
  output_para++;
  if (start_para != 0 && output_para < start_para) return;

  // While it is quicker just to dump the para to stdout in
  //  unlimited-line-length mode, doing this doesn't get us the
  //  benefit of trimming whitespace, etc
  if (width == 0 && notrim)
    {
    if (!epub2txt_all_white (klib_string_cstr (para)))
      {
      if (para_mark != 0)
        if (output_para % para_mark == 0)
          {
          printf ("\n\n*** PARA %d\n\n", output_para);
          }
      puts (klib_string_cstr (para));
      }
    }
   else
    {
    if (!epub2txt_all_white (klib_string_cstr (para)))
      {
      if (para_mark != 0)
        if (output_para % para_mark == 0) 
          {
          printf ("\n\n*** PARA %d\n\n", output_para);
          }

      typedef enum {MODE_START = 0, MODE_WORD = 1, MODE_SPACE = 2} Mode;
      Mode mode = MODE_START;
      char *s = strdup (klib_string_cstr (para));
      int i, l = strlen (s), col = 0;
      klib_String *word = klib_string_new_empty ();
      for (i = 0; i < l; i++)
        {
        char c = s[i];

        if (mode == MODE_START && (c == ' ' 
             || (unsigned char) c == (unsigned char)0xC2))
          {
          if (i < l - 1)
            {
            if ((unsigned char)s[i + 1] == (unsigned char)0xA0) 
              {
              i++;
              }
            }
          // Absorb leading spaces
          }	
        
/*
        if ((mode == MODE_START && (c == ' ' 
             || (unsigned char) c == (unsigned char)0xC2))
          &&
           (i < l - 1)
          &&
             ((unsigned char)s[i + 1] == (unsigned char)0xA0)) 
          {
          i++;
          }
          // Absorb leading spaces
*/
        else if (mode == MODE_START)
          {
          klib_string_append_byte (word, c);
          mode = MODE_WORD;
          }
        else if ((mode == MODE_SPACE && (c == ' '
             || (unsigned char) c == (unsigned char)0xC2))
             &&
             ((unsigned char)s[i + 1] == (unsigned char)0xA0)) 
          {
          i++;
          }
/*
        else if (mode == MODE_SPACE && (c == ' '
             || (unsigned char) c == (unsigned char)0xC2))
          {
            if ((unsigned char)s[i + 1] == (unsigned char)0xA0) 
              {
              i++;
              }
          }
*/
        else if (mode == MODE_SPACE)
          {
          mode = MODE_WORD;
          klib_string_append_byte (word, c);
          }
        else if (mode == MODE_WORD && c == ' ')
          {
          int wordlen = klib_string_length (word);
          if (col + wordlen >= width && width != 0)
            {
            printf ("\n");
            col = 0;
            }
          printf ("%s", klib_string_cstr (word));
          printf (" ");
          col += wordlen + 1;
          klib_string_set (word, "");
          mode = MODE_SPACE;
          }
        else if (mode == MODE_WORD)
          {
          klib_string_append_byte (word, c);
          }
        else
          {
          fprintf (stderr, "Internal error: bad mode/char combination in"
             " epub2txt_flush_para: mode=%d char=%d\n", mode, c);
          exit (-1);
          }
        }
      int wordlen = klib_string_length (word);
      if (col + wordlen >= width && width != 0)
        {
        printf ("\n");
        }
      printf ("%s", klib_string_cstr (word)); 
      klib_string_free (word);
      free (s);
      }
    }
  KLIB_OUT
  }


/*========================================================================
  epub2txt_line_break
=========================================================================*/
void epub2txt_line_break (void)
  {
  KLIB_IN
  if (start_para != 0 && output_para < start_para) return;
  printf ("\n");
  KLIB_OUT
  }


/*========================================================================
  epub2txt_para_break
=========================================================================*/
void epub2txt_para_break (void)
  {
  KLIB_IN
  if (start_para != 0 && output_para < start_para) return;
  printf ("\n\n");
  KLIB_OUT
  }


/*========================================================================
  epub2txt_parse_html
=========================================================================*/
void epub2txt_parse_html (const char *filename, BOOL ascii, 
    int width, BOOL notrim, klib_Error **error)
  {
  KLIB_IN
  typedef enum {MODE_ANY=0, MODE_INTAG = 1, MODE_ENTITY = 2} Mode;

  klib_log_info ("Parsing %s", filename);
  klib_WString *s = klib_wstring_read_file (filename, KLIB_ENCODING_UTF8, 
      error); 
  if (*error == NULL)
    {
    const wchar_t *text = klib_wstring_cstr (s);
    int i, l = wcslen (text);
    Mode mode = MODE_ANY;
    klib_WString *tag = klib_wstring_new_empty();
    klib_WString *entity = klib_wstring_new_empty();
    BOOL inbody = FALSE;
    BOOL can_newline = FALSE;
    wchar_t last_c = 0;
    klib_String *para = klib_string_new_empty ();

    for (i = 0; i < l; i++)
      {
      wchar_t c = text[i];
      if (c == 13) // DOS EOL
        continue;

      if (c == 9) // Tab
          c = ' ';

      //printf ("c=%c %04x\n", (char)c, c);
      if (mode == MODE_ANY && c == '<')
        {
        mode = MODE_INTAG;
        }
      else if (mode == MODE_ANY && c == '\n')
        {
        if (inbody)
          {
          if (last_c != ' ')
            {
            klib_string_append (para, " ");
            }
          }
        }
      else if (mode == MODE_ANY && c == '&')
        {
        mode = MODE_ENTITY;
        }
      else if (mode == MODE_ANY)
        {
        if (inbody)
          {
          if (c == ' ' && last_c == ' ')
            {
            }
          else
            {
            klib_String *s = epub2txt_transform_char (c, ascii);
            klib_string_append (para, klib_string_cstr (s));
            klib_string_free (s);
            }
          }
        }
      else if (mode == MODE_ENTITY && c == ';')
        {
        if (inbody)
          {
          klib_String *s_entity = klib_string_new_from_wstring (entity);
          char *ss_entity = strdup (klib_string_cstr (s_entity));
          klib_string_free (s_entity);
          char trans[20];
          epub2txt_translate_entity (ss_entity, trans);
          klib_string_append (para, trans);
          free (ss_entity);
          }
        klib_wstring_set (entity, L"");
        mode = MODE_ANY;
        }
      else if (mode == MODE_ENTITY)
        {
        klib_wstring_append_char (entity, c);
        }
      else if (mode == MODE_INTAG && c == '>')
        {
        klib_String *s_tag = klib_string_new_from_wstring (tag);
        char *ss_tag = strdup (klib_string_cstr (s_tag));
        klib_string_free (s_tag);
        char *p = strchr (ss_tag, ' ');
        if (p) *p = 0;
        if (strcasecmp (ss_tag, "body") == 0) 
          {
          inbody = TRUE;
          }
        else if (strcasecmp (ss_tag, "/body") == 0) 
          {
          if (epub2txt_all_white(klib_string_cstr(para)))
            can_newline = FALSE; 
          else
            can_newline = TRUE; 
          epub2txt_flush_para (para, width, notrim); 
          klib_string_set (para, "");
          if (can_newline)
            {
            epub2txt_para_break();
            can_newline = FALSE;
            }
          inbody = FALSE;
          }
        else if (strcasecmp (ss_tag, "/blockquote") == 0
            || strcasecmp (ss_tag, "/h1") == 0
            || strcasecmp (ss_tag, "/h2") == 0 
            || strcasecmp (ss_tag, "/h3") == 0
            || strcasecmp (ss_tag, "/h4") == 0
            || strcasecmp (ss_tag, "/div") == 0) 
          {
          if (epub2txt_all_white(klib_string_cstr (para)))
            can_newline = FALSE; 
          else
            can_newline = TRUE; 
          epub2txt_flush_para (para, width, notrim); 
          klib_string_set (para, "");
          if (can_newline)
            {
            epub2txt_para_break();
            can_newline = FALSE;
            }
          }
        else if ((strcasecmp (ss_tag, "p/") == 0) 
            || (strcasecmp (ss_tag, "/p") == 0))
          {
          if (inbody)
            {
            if (epub2txt_all_white(klib_string_cstr(para)))
              can_newline = FALSE; 
            else
              {
              can_newline = TRUE; 
              }
            epub2txt_flush_para (para, width, notrim); 
            klib_string_set (para, "");
            if (can_newline)
              {
              epub2txt_para_break();
              can_newline = FALSE;
              }
            }
          }
        else if ((strcasecmp (ss_tag, "br/") == 0) 
            || (strcasecmp (ss_tag, "br") == 0))
          {
          if (inbody)
            {
            if (epub2txt_all_white(klib_string_cstr(para)))
              can_newline = FALSE; 
            else
              can_newline = TRUE; 
            epub2txt_flush_para (para, width, notrim); 
            klib_string_set (para, "");
            if (can_newline)
              {
              epub2txt_line_break();
              can_newline = FALSE;
              }
            }
          }
        else if ((strcasecmp (ss_tag, "b/") == 0) 
            || (strcasecmp (ss_tag, "b") == 0))
          {
          if (inbody)
            {
            if (epub2txt_all_white(klib_string_cstr(para)))
              can_newline = FALSE; 
            else
              can_newline = TRUE; 
            epub2txt_flush_para (para, width, notrim); 
            klib_string_set (para, "");
            if (can_newline)
              {
              epub2txt_line_break();
              can_newline = FALSE;
              }
            }
          }
        free (ss_tag);
        klib_wstring_set (tag, L"");
        mode = MODE_ANY;
        }
      else if (mode == MODE_INTAG)
       {
       klib_wstring_append_char (tag, c);
       }
      else
        klib_log_error ("Unexpected character %d in mode %d", c, mode);
      last_c = c;
      }
   
    if (klib_string_length (para) > 0);
     epub2txt_flush_para (para, width, notrim); 
    klib_string_free (para);
    klib_wstring_free (tag);
    klib_wstring_free (s);
    klib_wstring_free (entity);
    } 
  else
    *error = klib_error_new (ENOENT, "Can't read file %s\n", filename);
  KLIB_IN
  }


/*========================================================================
  epub2txt_get_root_file
=========================================================================*/
klib_String *epub2txt_get_root_file (const char *container, klib_Error **error)
  {
  KLIB_IN
  klib_String *ret = NULL;
  klib_Xml *x = klib_xml_read_file (container, error);
  if (*error == NULL)
    {
    XMLNode *root = klib_xml_get_root (x); // container 
    int i, l = root->n_children;
    for (i = 0; i < l; i++)
      {
      XMLNode *r1 = root->children[i];
      if (strcmp (r1->tag, "rootfiles") == 0)
        {
        XMLNode *rootfiles = r1;
        int i, l = rootfiles->n_children;
        for (i = 0; i < l; i++)
          {
          XMLNode *r1 = rootfiles->children[i];
          if (strcmp (r1->tag, "rootfile") == 0)
            {
            int k, nattrs = r1->n_attributes;
            for (k = 0; k < nattrs; k++)
              {
              char *name = r1->attributes[k].name;
              char *value = r1->attributes[k].value;
              if (strcmp (name, "full-path") == 0)
                {
                ret = klib_string_new (value);
                }
              }
            }
          }
        }
      }

    klib_xml_free (x);
    }

  if (ret == NULL)
    *error = klib_error_new  
      (ENOENT, "container.xml does not specify a root file\n");
  KLIB_OUT
  return ret;
  }

/*========================================================================
  epub2txt_do_file 
=========================================================================*/
void epub2txt_do_file (const char *file, BOOL ascii, int width, BOOL notrim, 
    klib_Error **error)
  {
  KLIB_IN
  if (access (file, R_OK) == 0)
    {
    output_para = 0;
    char tempdir[256];
    char tempbase[256];
    char cmd[512];


    //This tmpdir offset is no longer necessary in KBOX3+
    //const char *kbox = getenv ("KBOX");
    //if (kbox)
    //  sprintf (tempbase, "%s/tmp", kbox);
    //else
      {
      if (getenv ("TMP"))
        strcpy (tempbase, getenv("TMP"));
      else
        strcpy (tempbase, "/tmp");
      }

    //sprintf (tempdir, "/tmp/epub2txt"); 
    sprintf (tempdir, "%s/epub2txt%d", tempbase, getpid()); 

    sprintf (cmd, "mkdir -p \"%s\"", tempdir);
    system (cmd);
   

    sprintf (cmd, "unzip -o -qq \"%s\" -d \"%s\"", file, tempdir);
    klib_log_debug ("Running unzip command; %s", cmd);
    system (cmd);
    klib_log_debug ("Unzip finished");
    sprintf (cmd, "chmod -R 744 \"%s\"", tempdir);
    klib_log_debug ("Fix permissions: %s", cmd);
    system (cmd);
    klib_log_debug ("Permissions fixed");
    
    char opf[512];
    sprintf (opf, "%s/META-INF/container.xml", tempdir);
    klib_String *rootfile = epub2txt_get_root_file (opf, error);
    if (*error == NULL)
      {
      klib_log_debug ("rootfile is %s", klib_string_cstr (rootfile));
      sprintf (opf, "%s/%s", tempdir, klib_string_cstr (rootfile));
      char *content_dir = strdup (opf);
      char *p = strrchr (content_dir, '/');
      *p = 0; 
      klib_List *list = epub2txt_get_items (opf, error);
      if (*error == NULL)
        {
        klib_log_debug ("EPUB spine has %d items", klib_list_length (list));
        int i, l = klib_list_length (list);
        for (i = 0; i < l; i++)
          {
          klib_String *item = (klib_String *)klib_list_get (list, i);
          sprintf (opf, "%s/%s", content_dir, klib_string_cstr (item));
          epub2txt_parse_html (opf, ascii, width, notrim, error);
          }
        }
      free (content_dir);
      }
      
    if (rootfile) klib_string_free (rootfile);
    sprintf (cmd, "rm -rf \"%s\"", tempdir);
    system (cmd);
    }
  else
    {
    *error = klib_error_new (ENOENT, "File not found: %s", file);
    }
  KLIB_OUT 
  }

