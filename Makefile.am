kfloppy_LDFLAGS	= $(all_libraries)  $(KDE_RPATH)
kfloppy_LDADD =  $(LIB_KDEUI) -lm
INCLUDES =	$(all_includes)

####### Files

bin_PROGRAMS  = kfloppy

noinst_HEADERS = floppy.h
kfloppy_SOURCES	= main.cpp floppy.cpp

METASOURCES = floppy.moc

KDE_ICON = kfloppy

apps_DATA = KFloppy.desktop
appsdir = $(kde_appsdir)/Utilities

messages:
	$(XGETTEXT) $(kfloppy_SOURCES) -o $(podir)/kfloppy.pot
	
