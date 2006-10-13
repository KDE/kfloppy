project(kfloppy)




########### next target ###############

set(kfloppy_SRCS main.cpp format.cpp floppy.cpp )

kde4_automoc(kfloppy ${kfloppy_SRCS})

kde4_add_executable(kfloppy ${kfloppy_SRCS})

target_link_libraries(kfloppy  ${KDE4_KIO_LIBS} )

install(TARGETS kfloppy  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES KFloppy.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES floppy_format.desktop  DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus)

kde4_install_icons( ${ICON_INSTALL_DIR}   )

