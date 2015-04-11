#!/usr/bin/perl -w
use strict;

sub dofile($)
  {
  my $file = $_[0];

  my $html = `man2html $file`;

  $html =~ s/.*<BODY>//ms;
  $html =~ s/<HR>/<p\/>/gimos;
  $html =~ s/This document was created.*//gimos;
  $html =~ s/.*?<H2>NAME<\/H2>/<H2>NAME<\/H2>/gimos;
  $html =~ s/<A NAME="index.*//gimos;
  $html =~ s/See <I><A HREF.*//gimos;
  $html =~ s/<H2>NAME<\/H2>(.*?)<A NAME.*?<\/A>/<hX>$1<\/hX><p\/>/gimos;
  $html =~ s/<H2>/<h3\/>/gimos;
  $html =~ s/<\/H2>/<\/h3\/>/gimos;
  $html =~ s/<hX>/<h2\/>/gimos;
  $html =~ s/<\/hX>/<\/h2\/>/gimos;

  printf ("$html\n");
  printf ("<hr/>");

  
  }

printf "<p/>\n<h1>epub2txt reference</h1>\n";

dofile ("man1/epub2txt.1");


