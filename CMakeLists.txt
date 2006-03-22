kde4_header()
project(kfloppy)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfloppy_SRCS main.cpp format.cpp floppy.cpp )

kde4_automoc(${kfloppy_SRCS})

kde4_add_executable(kfloppy ${kfloppy_SRCS})

target_link_libraries(kfloppy  ${KDE4_KIO_LIBRARIES} )

install_targets(/bin kfloppy )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES KFloppy.desktop )
install_files( ${DATA_INSTALL_DIR}/konqueror/servicemenus FILES floppy_format.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



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
