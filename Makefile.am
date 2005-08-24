kfloppy_LDFLAGS	= $(all_libraries)  $(KDE_RPATH)
kfloppy_LDADD =  $(LIB_KDEUI) -lm
INCLUDES =	$(all_includes)

####### Files

bin_PROGRAMS  = kfloppy

noinst_HEADERS = floppy.h
kfloppy_SOURCES	= main.cpp format.cpp floppy.cpp 

METASOURCES = format.moc floppy.moc

KDE_ICON = kfloppy

xdg_apps_DATA = KFloppy.desktop

servicedata_DATA = floppy_format.desktop
servicedatadir = $(kde_datadir)/konqueror/servicemenus

messages:
	$(XGETTEXT) $(kfloppy_SOURCES) -o $(podir)/kfloppy.pot
	
