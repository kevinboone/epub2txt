APPNAME=epub2txt
BINDIR=/usr/bin

MANDIR=/usr/share/man
ETCDIR=/etc

.SUFFIXES: .o .c

all: epub2txt 

VERSION=0.1.2

CFLAGS=-g -Wall -DVERSION=\"$(VERSION)\" $(EXTRA_CFLAGS)
LDFLAGS=$(EXTRA_LDFLAGS)


APP_OBJS=main.o epub2txt.o
KLIB_OBJS=klib_error.o klib_object.o klib_string.o klib_log.o klib_buffer.o klib_wstring.o klib_convertutf.o klib_getopt.o klib_getoptspec.o klib_list.o klib_path.o klib_xml.o sxmlc.o sxmlutils.o

OBJS=$(APP_OBJS) $(KLIB_OBJS)

epub2txt: $(OBJS)
	#$(CC) $(LDFLAGS) -s -o $(APPNAME) $(OBJS) 
	$(CC) $(LDFLAGS) -o $(APPNAME) $(OBJS) 

.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $*.c

clean:
	rm -f *.o $(APPNAME) *stackdump

install:
	cp -p $(APPNAME) $(BINDIR)
	cp -pr man1/* $(MANDIR)/man1/

srcdist: clean
	(cd ..; tar cvfz /tmp/$(APPNAME)-$(VERSION).tar.gz $(APPNAME))


web: srcdist
	./makeman.pl > $(APPNAME).man.html
	cp /tmp/$(APPNAME)-$(VERSION).tar.gz /home/kevin/docs/kzone5/target/
	cp $(APPNAME).man.html /home/kevin/docs/kzone5/target/
	cp README_$(APPNAME).html /home/kevin/docs/kzone5/source
	(cd /home/kevin/docs/kzone5; ./make.pl epub2txt)


include dependencies.mak
include dependencies_klib.mak

