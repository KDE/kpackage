cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.103.0") # handled by release scripts
set(KF_DEP_VERSION "5.103.0") # handled by release scripts
project(KPackage VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.103.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupQtPluginMacroNames)
include(ECMDeprecationSettings)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX PACKAGE
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kpackage_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5PackageConfigVersion.cmake"
                  SOVERSION 5)

################# now find all used packages #################

set (REQUIRED_QT_VERSION 5.15.2)

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core DBus)

find_package(KF5Archive ${KF_DEP_VERSION} REQUIRED)
find_package(KF5I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF_DEP_VERSION} REQUIRED)

find_package(KF5DocTools ${KF_DEP_VERSION})
set_package_properties(KF5DocTools PROPERTIES DESCRIPTION "Tools to generate documentation"
                       TYPE OPTIONAL
                      )

#########################################################################

ecm_setup_qtplugin_macro_names(
    JSON_ARG2
        "K_EXPORT_KPACKAGE_PACKAGE_WITH_JSON"
    CONFIG_CODE_VARIABLE
        PACKAGE_SETUP_AUTOMOC_VARIABLES
)
ecm_set_disabled_deprecation_versions(
    QT 5.15.2
    KF 5.97
    KCOREADDONS 5.91 # KPluginMetaData::fromDesktopFile
)

include(KF5PackageMacros.cmake)

# make package_version.h available
include_directories(${CMAKE_CURRENT_BINARY_DIR})

################# list the subdirectories #################
if (KF5DocTools_FOUND)
    add_subdirectory(docs)
endif()
add_definitions(-DTRANSLATION_DOMAIN=\"libkpackage5\")
ki18n_install(po)
if (KF5DocTools_FOUND)
    kdoctools_install(po)
endif()
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

################ create PackageConfig.cmake and install it ####################

# create a Config.cmake and a ConfigVersion.cmake file and install them

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Package")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5Package_QCH
        FILE KF5PackageQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5PackageQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5PackageConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PackageConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  KDE_INSTALL_INCLUDEDIR_KF CMAKE_INSTALL_PREFIX
)

install(FILES
  qrc.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PackageConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PackageConfigVersion.cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5PackageMacros.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5PackageTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5PackageTargets.cmake
        NAMESPACE KF5::
        COMPONENT Devel)
install(EXPORT KF5PackageToolsTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5PackageToolsTargets.cmake
        NAMESPACE KF5::
        COMPONENT Devel)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kpackage_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPackage COMPONENT Devel )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
