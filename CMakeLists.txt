project(kfloppy)




########### next target ###############

set(kfloppy_SRCS main.cpp format.cpp floppy.cpp )


kde4_add_app_icon(kfloppy_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-app*-kfloppy.png")

kde4_add_executable(kfloppy ${kfloppy_SRCS})

target_link_libraries(kfloppy ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS kfloppy  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES KFloppy.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES floppy_format.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

kde4_install_icons( ${ICON_INSTALL_DIR}   )

