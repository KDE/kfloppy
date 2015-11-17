project(kfloppy)

if(NOT UNIX)
  message(FATAL_ERROR "KFloppy is only supposed to be built on Unix platforms.")
endif(NOT UNIX)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})

    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
    add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

    # If definitions like -D_GNU_SOURCE are needed for these checks they
    # should be added to _KDE4_PLATFORM_DEFINITIONS when it is originally
    # defined outside this file.  Here we include these definitions in
    # CMAKE_REQUIRED_DEFINITIONS so they will be included in the build of
    # checks below.
    set( CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} )
endif()

set(kfloppy_SRCS main.cpp format.cpp floppy.cpp )

kde4_add_executable(kfloppy ${kfloppy_SRCS})

target_link_libraries(kfloppy ${KDE4_KIO_LIBS} )

install(TARGETS kfloppy  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS KFloppy.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

# The mimetypes used in this servicemenu (e.g. floppy_unmounted) don't exist anymore.
# This needs to be done differently.
#install( FILES floppy_format.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

kde4_install_icons( ${ICON_INSTALL_DIR}   )

add_subdirectory(doc)
