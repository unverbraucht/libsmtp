# Generated automatically from Makefile.in by configure.
# Main Makefile for Kevins stupid "libsmtp" library
# Copyright (C) 2001 Kevin Read.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

SHELL = /bin/sh
top_srcdir = ..
srcdir = .

.SUFFIXES:
.SUFFIXES: .c .o

OPT=-g -O

AR = ar
AR_FLAGS = rc
RANLIB = ranlib

CC = gcc
CFLAGS = -I. -I../include `glib-config --cflags` -g -O2
LDFLAGS = -L. -L../smtp -L../mime 
LIBS = -lsmtp -lsmtp_mime -lglib -lglib 
INSTALL = /usr/bin/install -c
prefix = /usr/local
exec_prefix = ${prefix}
bindir = $(exec_prefix)/bin
libdir = $(prefix)/lib
infodir = $(prefix)/info

ifeq (yes,yes)
  SOURCES=smtpmail.c multipart.c singlepart.c
  OBJS=smtpmail.o multipart.o singlepart.o
else
  SOURCES=smtpmail.c
  OBJS=smtpmail.o
endif

DOCS=
MISC=configure mkinstalldirs install-sh aclocal.m4
LIB_OBJS=

all: singlepart multipart smtpmail

singlepart: singlepart.o
	$(CC) $(CFLAGS) -o singlepart singlepart.o $(LIBS) $(LDFLAGS)

ifeq (yes,yes)
multipart: multipart.o
	$(CC) $(CFLAGS) -o multipart multipart.o $(LIBS) $(LDFLAGS)

smtpmail: smtpmail.o
	$(CC) $(CFLAGS) -o smtpmail smtpmail.o $(LDFLAGS) -lsmtp -L`glib-config --libs` -lglib
endif

clean:
	/bin/rm -f core *.o $(OBJS) $(LIB_OBJS) multipart singlepart \
	smtpmail

distclean: clean
	/bin/rm -f Makefile config.h config.status config.cache config.log

mostlyclean: clean

maintainer-clean: clean

# automatic re-running of configure if the ocnfigure.in file has changed
#${srcdir}/configure: configure.in aclocal.m4
#	cd ${srcdir} &#38;&#38; autoconf

## autoheader might not change config.h.in, so touch a stamp file
#${srcdir}/config.h.in: stamp-h.in
#${srcdir}/stamp-h.in: configure.in aclocal.m4
#		cd ${srcdir} &#38;&#38; autoheader
#		echo timestamp &#62; ${srcdir}/stamp-h.in

#config.h: stamp-h
#stamp-h: config.h.in config.status
#	./config.status
#Makefile: Makefile.in config.status
#	./config.status
#config.status: configure
#	./config.status --recheck
