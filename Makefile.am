

kfloppy_LDFLAGS	= $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 -lm
LDFLAGS =        -lm
INCLUDES =	$(all_includes)

####### Files

bin_PROGRAMS  = kfloppy kmkdosfs kfdformat

noinst_HEADERS = about.h floppy.h
kfloppy_SOURCES	= main.cpp about.cpp floppy.cpp mount.c

# to put kmke2fs into BUILT_SOURCES is somehow Trick 17B ;)
BUILT_SOURCES = kmke2fs about.moc floppy.moc
DISTCLEANFILES = $(BUILT_SOURCES)

kmkdosfs_SOURCES = kmkdosfs.c

kfdformat_SOURCES = kfdformat.c

SUBDIRS = doc

# since the e2fs sources are just bundled, we need some magic to build them
kmke2fs: e2fs/misc/mke2fs.c e2fs/misc/badblocks.c 
	(cd e2fs && CC=$(CC) ./configure -host=$(host_triplet) && \
	$(MAKE) &&  cp misc/kmke2fs ../ &&  strip ../kmke2fs)

distclean-local:
	(cd e2fs && $(MAKE) distclean)

clean-local:
	(cd e2fs && $(MAKE) clean)

icon_DATA = kfloppy.xpm
icondir = $(kde_icondir)

pics_DATA = kfloppylogo.xpm
picsdir = $(kde_datadir)/kfloppy/pics

apps_DATA = KFloppy.kdelnk
appsdir = $(kde_appsdir)/Utilities

install-local-exec: kmke2fs
	$(mkinstalldirs) $(bindir)
	$(INSTALL) kmke2fs $(bindir)
	@echo ""
	@echo "Bernd Wuebben"
	@echo "wuebben@kde.org"
	@echo ""	

uninstall-local:
	rm -f $(BINDIR)/kmke2fs

####### Explicit dependencies
about.cpp: about.moc
about.moc: about.h
	$(MOC) about.h -o about.moc

floppy.cpp: floppy.moc
floppy.moc: floppy.h
	$(MOC) floppy.h -o floppy.moc






