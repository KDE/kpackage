find_package(Qt5Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
set_package_properties(Qt5Test PROPERTIES PURPOSE "Required for tests")

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

include(ECMMarkAsTest)
include(ECMAddTests)

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               ecm_add_test(
                       plasmoidstructure.cpp ${_testname}.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/kpackage/private/packages.cpp
                       LINK_LIBRARIES Qt5::Test KF5::Package KF5::Archive KF5::CoreAddons KF5::I18n
                       TEST_NAME ${_testname}
                       NAME_PREFIX "plasma-")
       ENDFOREACH(_testname)
ENDMACRO(PLASMA_UNIT_TESTS)

PLASMA_UNIT_TESTS(
    fallbackpackagetest
    packagestructuretest
    plasmoidpackagetest
    rccpackagetest
    querytest
    benchmarkstest
)

set_tests_properties(plasma-querytest PROPERTIES RUN_SERIAL TRUE) # it wipes out ~/.qttest/share
set_tests_properties(plasma-plasmoidpackagetest PROPERTIES RUN_SERIAL TRUE)

function(kpackagetool5test testname)
    add_test(NAME ${testname}-appstream COMMAND cmake -Dkpackagetool=$<TARGET_FILE:kpackagetool5>
                                                      -Dgenerated=${CMAKE_CURRENT_BINARY_DIR}/${testname}.appdata.xml
                                                      -Dinput=${CMAKE_CURRENT_LIST_DIR}/data/${testname}/
                                                      -Doutput=${CMAKE_CURRENT_SOURCE_DIR}/data/${testname}/${testname}.testappdataxml
                                                      -P ${CMAKE_CURRENT_SOURCE_DIR}/kpackagetoolappstreamtest.cmake
    )
    set(XDG_DATA_DIRS "$ENV{XDG_DATA_DIRS}")
    if(NOT XDG_DATA_DIRS)
        set(XDG_DATA_DIRS "/usr/local/share:/usr/share")
    endif()
    set_property(TEST ${testname}-appstream PROPERTY ENVIRONMENT "XDG_DATA_DIRS=${CMAKE_SOURCE_DIR}/src/kpackage/data:${XDG_DATA_DIRS}")
endfunction()

foreach(var "testfallbackpackage" "testpackage" "testpackage-nodisplay" "testjsonmetadatapackage" "testpackagesdep" "testpackagesdepinvalid") # "customcontent" "simplecontent"
    kpackagetool5test(${var})
endforeach()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/testpackage-rcc")
set(kpkgqrc "${CMAKE_CURRENT_SOURCE_DIR}/data/testpackage-rcc/resources.qrc")
set(kpkgrcc "${CMAKE_CURRENT_BINARY_DIR}/testpackage-rcc/contents.rcc")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/data/testpackage-rcc/metadata.json"  ${CMAKE_CURRENT_BINARY_DIR}/testpackage-rcc COPYONLY)
add_custom_command(OUTPUT ${kpkgrcc}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND Qt5::rcc ${kpkgqrc} --binary -o ${kpkgrcc})
add_custom_target(testpackage-rcc ALL DEPENDS ${kpkgrcc})


add_subdirectory(mockdepresolver)
