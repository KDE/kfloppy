

kfloppy_LDFLAGS	= $(all_libraries)  $(KDE_RPATH)
kfloppy_LDADD =  $(LIB_KDEUI) -lm
INCLUDES =	$(all_includes)

####### Files

bin_PROGRAMS  = kfloppy kmkdosfs kfdformat

noinst_HEADERS = about.h floppy.h
kfloppy_SOURCES	= main.cpp about.cpp floppy.cpp mount.c

# to put kmke2fs into BUILT_SOURCES is somehow Trick 17B ;)
METASOURCES = about.moc floppy.moc

kmkdosfs_SOURCES = kmkdosfs.c

kfdformat_SOURCES = kfdformat.c

SUBDIRS = doc

all-local: kmke2fs

# since the e2fs sources are just bundled, we need some magic to build them
kmke2fs: e2fs/misc/mke2fs.c e2fs/misc/badblocks.c 
	(cd e2fs && $(MAKE) &&  cp misc/kmke2fs ../ &&  strip ../kmke2fs)

distclean-local: 
	(cd e2fs && $(MAKE) distclean)
	-rm -f kmke2fs

clean-local:
	(cd e2fs && $(MAKE) clean)
	-rm -f kmke2fs

icon_DATA = kfloppy.xpm
icondir = $(kde_icondir)

pics_DATA = kfloppylogo.xpm
picsdir = $(kde_datadir)/kfloppy/pics

apps_DATA = KFloppy.kdelnk
appsdir = $(kde_appsdir)/Utilities

install-exec-local: kmke2fs
	$(mkinstalldirs) $(bindir)
	$(INSTALL) kmke2fs $(bindir)
	
install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kfloppy.xpm $(kde_minidir)/kfloppy.xpm
	$(mkinstalldirs) $(kde_minidir)/../large
	$(INSTALL_DATA) $(srcdir)/large-kfloppy.xpm $(kde_minidir)/../large/kfloppy.xpm

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kfloppy_SOURCES) && mv messages.po ../po/kfloppy.pot
	
uninstall-local:
	rm -f $(BINDIR)/kmke2fs
	rm -f $(kde_minidir)/kfloppy.xpm
	rm -f $(kde_minidir)/../large/kfloppy.xpm

