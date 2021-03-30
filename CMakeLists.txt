cmake_minimum_required(VERSION 3.5)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "21")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kfloppy VERSION ${RELEASE_SERVICE_VERSION})

if(NOT UNIX)
  message(FATAL_ERROR "KFloppy is only supposed to be built on Unix platforms.")
endif(NOT UNIX)

set(KF5_MIN_VERSION "5.80.0")
set(REQUIRED_QT_VERSION "5.14.0")

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

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050e00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055100)

set(kfloppy_SRCS main.cpp format.cpp floppy.cpp )

set_property(SOURCE main.cpp APPEND PROPERTY COMPILE_DEFINITIONS "KFLOPPY_VERSION=\"${RELEASE_SERVICE_VERSION}\"")

ecm_qt_declare_logging_category(kfloppy_SRCS HEADER kfloppy_debug.h IDENTIFIER KFLOPPY_LOG CATEGORY_NAME org.kde.kfloppy DESCRIPTION "KFloppy" EXPORT KFLOPPY)

add_executable(kfloppy ${kfloppy_SRCS})

target_link_libraries(kfloppy KF5::CoreAddons KF5::I18n KF5::Completion KF5::XmlGui Qt5::DBus KF5::Crash)

install(TARGETS kfloppy  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install(PROGRAMS org.kde.kfloppy.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kfloppy.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# The mimetypes used in this servicemenu (e.g. floppy_unmounted) don't exist anymore.
# This needs to be done differently.
#install( FILES floppy_format.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/ServiceMenus)

ecm_install_icons(ICONS
    128-apps-kfloppy.png
    16-apps-kfloppy.png
    22-apps-kfloppy.png
    32-apps-kfloppy.png
    48-apps-kfloppy.png
    64-apps-kfloppy.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)

add_subdirectory(doc)

ecm_qt_install_logging_categories(
	EXPORT KFLOPPY
        FILE kfloppy.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )
ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
