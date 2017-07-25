project(kfloppy)
cmake_minimum_required(VERSION 2.8.12)

if(NOT UNIX)
  message(FATAL_ERROR "KFloppy is only supposed to be built on Unix platforms.")
endif(NOT UNIX)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(KDEInstallDirs)
    include(KDECompilerSettings NO_POLICY_SCOPE)
    include(KDECMakeSettings)
    include(FeatureSummary)
    include(ECMInstallIcons)

    find_package(Qt5 5.2.0 CONFIG REQUIRED Core DBus Widgets)
    find_package(KF5 5.2.0 REQUIRED COMPONENTS I18n CoreAddons Completion DocTools XmlGui)

    find_package(KF5Crash 5.15.0) 
    if(KF5Crash_FOUND)
       set(HAVE_KCRASH 1)
    else(KF5Crash_FOUND)
       set(HAVE_KCRASH 0)
    endif(KF5Crash_FOUND)
    set_package_properties(KF5Crash PROPERTIES
        DESCRIPTION "KDE's Crash Handler"
        URL "http://api.kde.org/frameworks-api/frameworks5-apidocs/kcrash/html/index.html"
        TYPE OPTIONAL
        PURPOSE "Optionally used to provide crash reporting")

    add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
endif()

set(kfloppy_SRCS main.cpp format.cpp floppy.cpp )

add_executable(kfloppy ${kfloppy_SRCS})

target_link_libraries(kfloppy KF5::CoreAddons KF5::I18n KF5::Completion KF5::XmlGui Qt5::DBus)
if(HAVE_KCRASH)
    target_link_libraries(kfloppy KF5::Crash)
endif()

install(TARGETS kfloppy  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install(PROGRAMS org.kde.kfloppy.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES org.kde.kfloppy.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kfloppy.categories DESTINATION ${KDE_INSTALL_CONFDIR})

# The mimetypes used in this servicemenu (e.g. floppy_unmounted) don't exist anymore.
# This needs to be done differently.
#install( FILES floppy_format.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

ecm_install_icons(ICONS
    128-apps-kfloppy.png
    16-apps-kfloppy.png
    22-apps-kfloppy.png
    32-apps-kfloppy.png
    48-apps-kfloppy.png
    64-apps-kfloppy.png
    DESTINATION ${ICON_INSTALL_DIR}
    THEME hicolor
)

add_subdirectory(doc)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

