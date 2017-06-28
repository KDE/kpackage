# This option should be removed, or moved down as far as possible.
# That means porting the existing frameworks to the CMake automoc
# feature. Porting is mostly removing explicit moc includes, and
# leaving the ones which are truly needed (ie, if you remove
# them, the build fails).
set(CMAKE_AUTOMOC_RELAXED_MODE ON)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-package.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-package.h)


add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1209)

########### next target ###############

set(Package_LIB_SRCS
    package.cpp
    packagestructure.cpp
    packageloader.cpp
    private/packages.cpp
    private/packagejob.cpp
    private/packagejobthread.cpp
    private/versionparser.cpp
    version.cpp
)




add_library(KF5Package ${Package_LIB_SRCS})
add_library(KF5::Package ALIAS KF5Package)

target_link_libraries(KF5Package
PUBLIC
     #KF5::Service # For kplugininfo.h and kservice.h
     KF5::CoreAddons
PRIVATE
     KF5::Archive
     KF5::I18n
)

target_include_directories(KF5Package PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}/KPackage>"
)
target_include_directories(KF5Package INTERFACE
  "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5};${KF5_INCLUDE_INSTALL_DIR}/KPackage>"
)

set_target_properties(KF5Package PROPERTIES
                                VERSION ${PACKAGE_VERSION_STRING}
                                SOVERSION ${PACKAGE_SOVERSION}
                                EXPORT_NAME Package
                      )

########### install files ###############
generate_export_header(KF5Package
    BASE_NAME Package
    EXPORT_FILE_NAME kpackage/package_export.h)
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
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KPackage/kpackage COMPONENT Devel)

install(FILES
        ${Package_CamelCase_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KPackage/KPackage COMPONENT Devel)

install(FILES
   data/servicetypes/kpackage-packagestructure.desktop
   data/servicetypes/kpackage-generic.desktop
   data/servicetypes/kpackage-genericqml.desktop
   DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})


install(TARGETS KF5Package EXPORT KF5PackageTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

if(BUILD_QCH)
    ecm_add_qch(
        KF5Package_QCH
        NAME KPackage
        BASE_NAME KF5Package
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${Package_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            KF5CoreAddons_QCH
        BLANK_MACROS
            PACKAGE_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
