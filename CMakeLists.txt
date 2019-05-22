project(kfloppy)
cmake_minimum_required(VERSION 3.5)

if(NOT UNIX)
  message(FATAL_ERROR "KFloppy is only supposed to be built on Unix platforms.")
endif(NOT UNIX)

set(KF5_MIN_VERSION "5.44.0")
set(REQUIRED_QT_VERSION "5.9.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)


find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core DBus Widgets)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n CoreAddons Completion DocTools XmlGui)
find_package(KF5Crash ${KF5_MIN_VERSION} CONFIG REQUIRED)

if (${KF5Config_VERSION} STRGREATER "5.56.0")
        add_definitions(-DQT_NO_FOREACH)
        MESSAGE(STATUS "compile without foreach")
endif()

set(kfloppy_SRCS main.cpp format.cpp floppy.cpp )

ecm_qt_declare_logging_category(kfloppy_SRCS HEADER kfloppy_debug.h IDENTIFIER KFLOPPY_LOG CATEGORY_NAME org.kde.kfloppy)


add_executable(kfloppy ${kfloppy_SRCS})

target_link_libraries(kfloppy KF5::CoreAddons KF5::I18n KF5::Completion KF5::XmlGui Qt5::DBus KF5::Crash)

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

