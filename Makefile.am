kfloppy_LDFLAGS	= $(all_libraries)  $(KDE_RPATH)
kfloppy_LDADD =  $(LIB_KDEUI) -lm
INCLUDES =	$(all_includes)

####### Files

bin_PROGRAMS  = kfloppy

noinst_HEADERS = about.h floppy.h
kfloppy_SOURCES	= main.cpp about.cpp floppy.cpp

METASOURCES = about.moc floppy.moc

KDE_ICON = kfloppy

pics_DATA = kfloppylogo.png
picsdir = $(kde_datadir)/kfloppy/pics

apps_DATA = KFloppy.desktop
appsdir = $(kde_appsdir)/Utilities

messages:
	$(XGETTEXT) $(kfloppy_SOURCES) -o $(podir)/kfloppy.pot
	
