#
# Filename:	Makefile
# Author:	Dave Hudson
# Version:	0.3
# Started:	24th August 1993
# Last Updated:	22th April 1995
# Updated by:   H. Peter Anvin <hpa@yggdrasil.com>
#
# Description:	Makefile for the "mkdosfs" command
#

CC=		gcc
O=		-O2 -fomit-frame-pointer
CFLAGS=		$(O) -Wall
LDFLAGS=	-s
SBINDIR=	/sbin
MANDIR=		/usr/man/man8
LN=		ln -s

all :	mkdosfs

mkdosfs: mkdosfs.o
	$(CC) $(LDFLAGS) mkdosfs.o -o mkdosfs

mkdosfs.o : mkdosfs.c
	$(CC) -c $(CFLAGS) mkdosfs.c

install :
	install -m 755 mkdosfs $(SBINDIR)/mkfs.msdos
	install -m 644 mkdosfs.8 $(MANDIR)
	rm -f $(SBINDIR)/mkdosfs
	$(LN) $(SBINDIR)/mkfs.msdos $(SBINDIR)/mkdosfs
	rm -f $(MANDIR)/mkfs.msdos.8
	$(LN) $(MANDIR)/mkdosfs.8 $(MANDIR)/mkfs.msdos.8

clean :
	rm -f *.s *.o *.a *~ core mkdosfs
