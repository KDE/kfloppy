cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "80")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kfloppy VERSION ${RELEASE_SERVICE_VERSION})

if(NOT UNIX)
  message(FATAL_ERROR "KFloppy is only supposed to be built on Unix platforms.")
endif(NOT UNIX)

set(KF_MIN_VERSION "5.102.0")
set(REQUIRED_QT_VERSION "5.15.2")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(KDEGitCommitHooks)
include(KDEClangFormat)
include(ECMDeprecationSettings)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_MIN_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core DBus Widgets)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat)
endif()
find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS I18n CoreAddons Completion XmlGui)
find_package(KF5DocTools ${KF_MIN_VERSION})
set_package_properties(KF${KF_MAJOR_VERSION}DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)
find_package(KF${KF_MAJOR_VERSION}Crash ${KF_MIN_VERSION} CONFIG REQUIRED)

ecm_set_disabled_deprecation_versions(QT 6.4
    KF 5.101
)


add_executable(kfloppy)
target_sources(kfloppy PRIVATE main.cpp format.cpp floppy.cpp format.h floppy.h)

set_property(SOURCE main.cpp APPEND PROPERTY COMPILE_DEFINITIONS "KFLOPPY_VERSION=\"${RELEASE_SERVICE_VERSION}\"")

ecm_qt_declare_logging_category(kfloppy HEADER kfloppy_debug.h IDENTIFIER KFLOPPY_LOG CATEGORY_NAME org.kde.kfloppy DESCRIPTION "KFloppy" EXPORT KFLOPPY)


target_link_libraries(kfloppy KF${KF_MAJOR_VERSION}::CoreAddons KF${KF_MAJOR_VERSION}::I18n KF${KF_MAJOR_VERSION}::Completion
    KF${KF_MAJOR_VERSION}::XmlGui Qt${QT_MAJOR_VERSION}::DBus KF${KF_MAJOR_VERSION}::Crash)

if (TARGET Qt::Core5Compat)
    target_link_libraries(kfloppy Qt::Core5Compat)
endif()

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

ecm_qt_install_logging_categories(
	EXPORT KFLOPPY
        FILE kfloppy.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )
ki18n_install(po)
if (KF5DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
