project(kfloppy)
cmake_minimum_required(VERSION 2.8.12)

if(NOT UNIX)
  message(FATAL_ERROR "KFloppy is only supposed to be built on Unix platforms.")
endif(NOT UNIX)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(KDEInstallDirs)
    include(KDECompilerSettings)
    include(KDECMakeSettings)
    include(FeatureSummary)
    include(ECMInstallIcons)

    find_package(Qt5 5.2.0 CONFIG REQUIRED Core DBus Widgets)
    find_package(KF5 REQUIRED COMPONENTS I18n KIO DocTools KDELibs4Support XmlGui)

    add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
endif()

set(kfloppy_SRCS main.cpp format.cpp floppy.cpp )

add_executable(kfloppy ${kfloppy_SRCS})

target_link_libraries(kfloppy KF5::KIOWidgets KF5::I18n KF5::KDELibs4Support Qt5::DBus)

install(TARGETS kfloppy  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS KFloppy.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

# The mimetypes used in this servicemenu (e.g. floppy_unmounted) don't exist anymore.
# This needs to be done differently.
#install( FILES floppy_format.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

ecm_install_icons(ICONS
    hi128-app-kfloppy.png
    hi16-app-kfloppy.png
    hi22-app-kfloppy.png
    hi32-app-kfloppy.png
    hi48-app-kfloppy.png
    hi64-app-kfloppy.png
    DESTINATION ${ICON_INSTALL_DIR}
    THEME hicolor
)

add_subdirectory(doc)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

