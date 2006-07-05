project(kfloppy)




########### next target ###############

set(kfloppy_SRCS main.cpp format.cpp floppy.cpp )

kde4_automoc(${kfloppy_SRCS})

kde4_add_executable(kfloppy ${kfloppy_SRCS})

target_link_libraries(kfloppy  ${KDE4_KIO_LIBS} )

install(TARGETS kfloppy  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES KFloppy.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES floppy_format.desktop  DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus)

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




#original Makefile.am contents follow:

#kfloppy_LDFLAGS	= $(all_libraries)  $(KDE_RPATH)
#kfloppy_LDADD =  $(LIB_KDEUI) -lm
#INCLUDES =	$(all_includes)
#
######## Files
#
#bin_PROGRAMS  = kfloppy
#
#noinst_HEADERS = floppy.h
#kfloppy_SOURCES	= main.cpp format.cpp floppy.cpp 
#
#METASOURCES = format.moc floppy.moc
#
#KDE_ICON = kfloppy
#
#xdg_apps_DATA = KFloppy.desktop
#
#servicedata_DATA = floppy_format.desktop
#servicedatadir = $(kde_datadir)/konqueror/servicemenus
#
#messages:
#	$(XGETTEXT) $(kfloppy_SOURCES) -o $(podir)/kfloppy.pot
#	
