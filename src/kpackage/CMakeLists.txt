configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-package.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-package.h)

add_library(KF5Package)
add_library(KF5::Package ALIAS KF5Package)

set_target_properties(KF5Package PROPERTIES
    VERSION     ${PACKAGE_VERSION}
    SOVERSION   ${PACKAGE_SOVERSION}
    EXPORT_NAME Package
)

target_sources(KF5Package PRIVATE
    package.cpp
    packagestructure.cpp
    packageloader.cpp
    private/packages.cpp
    private/packagejob.cpp
    version.cpp
    kpackage.qrc
)

ecm_qt_declare_logging_category(KF5Package
    HEADER kpackage_debug.h
    IDENTIFIER KPACKAGE_LOG
    CATEGORY_NAME kf.package
    OLD_CATEGORY_NAMES kf5.kpackage
    DESCRIPTION "kpackage (lib)"
    EXPORT KPACKAGE
)



ecm_generate_export_header(KF5Package
    EXPORT_FILE_NAME kpackage/package_export.h
    BASE_NAME KPackage
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.21 5.84 5.85 5.86
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

add_library(kpackage_common_STATIC STATIC private/packagejobthread.cpp private/versionparser.cpp)
target_link_libraries(kpackage_common_STATIC PRIVATE Qt${QT_VERSION_MAJOR}::Core KF5::CoreAddons KF5::Archive KF5::I18n)
set_property(TARGET kpackage_common_STATIC PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(KF5Package
PUBLIC
     KF5::CoreAddons
PRIVATE
     Qt${QT_MAJOR_VERSION}::DBus
     KF5::Archive
     KF5::I18n
     kpackage_common_STATIC
)

target_include_directories(KF5Package PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}/KPackage>"
)
target_include_directories(KF5Package INTERFACE
  "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF};${KDE_INSTALL_INCLUDEDIR_KF}/KPackage>"
)

########### install files ###############
ecm_generate_headers(Package_CamelCase_HEADERS
    HEADER_NAMES
        Package
        PackageStructure
        PackageLoader
    REQUIRED_HEADERS Package_HEADERS
    PREFIX KPackage
)

set(Package_HEADERS
    ${Package_HEADERS}
    version.h
)

install(FILES
        ${Package_HEADERS}
        ${CMAKE_CURRENT_BINARY_DIR}/kpackage/package_export.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPackage/kpackage COMPONENT Devel)

install(FILES
        ${Package_CamelCase_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPackage/KPackage COMPONENT Devel)

install(FILES
   data/kservicetypes5/kpackage-packagestructure.desktop
   data/kservicetypes5/kpackage-generic.desktop
   data/kservicetypes5/kpackage-genericqml.desktop
   DESTINATION ${KDE_INSTALL_KSERVICETYPESDIR})


install(TARGETS KF5Package EXPORT KF5PackageTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
if (NOT BUILD_SHARED_LIBS)
    install(TARGETS kpackage_common_STATIC EXPORT KF5PackageTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

if(BUILD_QCH)
    ecm_add_qch(
        KF5Package_QCH
        NAME KPackage
        BASE_NAME KF5Package
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${Package_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            KF5CoreAddons_QCH
        BLANK_MACROS
            KPACKAGE_EXPORT
            "KPACKAGE_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
