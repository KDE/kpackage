find_package(Qt5Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
set_package_properties(Qt5Test PROPERTIES PURPOSE "Required for tests")

include(ECMMarkAsTest)
include(ECMAddTests)

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} plasmoidstructure.cpp ${_testname}.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/kpackage/private/packages.cpp)
               target_link_libraries(${_testname} Qt5::Test
                                     KF5::Package KF5::Archive KF5::CoreAddons
                                     KF5::I18n)
               add_test(plasma-${_testname} ${_testname})
               ecm_mark_as_test(${_testname})
       ENDFOREACH(_testname)
ENDMACRO(PLASMA_UNIT_TESTS)

PLASMA_UNIT_TESTS(
    fallbackpackagetest
    packagestructuretest
    plasmoidpackagetest
    querytest
)

function(kpackagetool5test testname)
    add_test(NAME ${testname}-appstream COMMAND cmake -Dkpackagetool=$<TARGET_FILE:kpackagetool5>
                                                      -Dgenerated=${CMAKE_CURRENT_BINARY_DIR}/${testname}.appdata.xml
                                                      -Dinput=${CMAKE_CURRENT_LIST_DIR}/data/${testname}/
                                                      -Doutput=${CMAKE_CURRENT_SOURCE_DIR}/data/${testname}/${testname}.testappdataxml
                                                      -P ${CMAKE_CURRENT_SOURCE_DIR}/kpackagetoolappstreamtest.cmake
    )
endfunction()

foreach(var "testfallbackpackage" "testpackage" "testjsonmetadatapackage" "testpackagesdep") # "customcontent" "simplecontent"
    kpackagetool5test(${var})
endforeach()
