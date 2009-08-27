project(kfloppy)


set(kfloppy_SRCS main.cpp format.cpp floppy.cpp )


kde4_add_executable(kfloppy ${kfloppy_SRCS})

target_link_libraries(kfloppy ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS kfloppy  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES KFloppy.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

# The mimetypes used in this servicemenu (e.g. floppy_unmounted) don't exist anymore.
# This needs to be done differently.
#install( FILES floppy_format.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

kde4_install_icons( ${ICON_INSTALL_DIR}   )

