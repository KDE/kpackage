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
    packagetrader.cpp
    private/packagejob.cpp
    private/packagejobthread.cpp
    version.cpp
)




add_library(KF5Package ${Package_LIB_SRCS})
add_library(KF5::Package ALIAS KF5Package)

target_link_libraries(KF5Package
PUBLIC
     KF5::Service # For kplugininfo.h and kservice.h
PRIVATE
     KF5::Archive
     KF5::I18n
     Qt5::DBus
)

target_include_directories(KF5Package PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}/Package>"
)
target_include_directories(KF5Package INTERFACE
  "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR};${KF5_INCLUDE_INSTALL_DIR}/Package>"
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
        PackageTrader
    REQUIRED_HEADERS Package_HEADERS
    PREFIX KPackage
)


install(FILES
        ${Package_HEADERS}
        ${CMAKE_CURRENT_BINARY_DIR}/kpackage/package_export.h
        version.h
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/kpackage COMPONENT Devel)

install(DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/KPackage
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR} COMPONENT Devel)



install(FILES
   data/servicetypes/kpackage-packagestructure.desktop
   DESTINATION ${SERVICETYPES_INSTALL_DIR})


install(TARGETS KF5Package EXPORT KF5PackageTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
