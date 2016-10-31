cmake_minimum_required(VERSION 2.8.12)

project(Package)

# ECM setup
include(FeatureSummary)
find_package(ECM 5.27.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})


include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

set(KF5_VERSION "5.28.0") # handled by release scripts
set(KF5_DEP_VERSION "5.27.0") # handled by release scripts

ecm_setup_version(${KF5_VERSION}
                  VARIABLE_PREFIX PACKAGE
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kpackage_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5PackageConfigVersion.cmake"
                  SOVERSION 5)

################# Enable C++0x (still too early for -std=c++11) features for clang and gcc #################

if(UNIX)
   #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++0x")
   add_definitions("-Wall -std=c++0x")
endif()

################# now find all used packages #################

set (REQUIRED_QT_VERSION "5.3.0")

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core)

find_package(KF5Archive ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5I18n ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Config ${KF5_DEP_VERSION} REQUIRED)

find_package(KF5DocTools ${KF5_DEP_VERSION})
set_package_properties(KF5DocTools PROPERTIES DESCRIPTION "Tools to generate documentation"
                       TYPE OPTIONAL
                      )

#########################################################################

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

#add_definitions(-Wno-deprecated)
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

include(KF5PackageMacros.cmake)

################ create PackageConfig.cmake and install it ####################

# create a Config.cmake and a ConfigVersion.cmake file and install them

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Package")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5PackageConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PackageConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  KF5_INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES
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

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kpackage_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel )

# make package_version.h available
include_directories(${CMAKE_CURRENT_BINARY_DIR})

################# list the subdirectories #################
if (KF5DocTools_FOUND)
    add_subdirectory(docs)
endif()
add_definitions(-DTRANSLATION_DOMAIN=\"libkpackage5\")
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install(po)
    if (KF5DocTools_FOUND)
        kdoctools_install(po)
    endif()
endif()

add_subdirectory(src)
add_subdirectory(autotests)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
