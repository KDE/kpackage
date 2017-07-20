execute_process(COMMAND ${kpackagetool} --appstream-metainfo . WORKING_DIRECTORY ${input} OUTPUT_FILE "${generated}" ERROR_VARIABLE error)
if (error)
    message(FATAL_ERROR "couldn't generate metadata: ${error}")
endif()

execute_process(COMMAND cmake -E compare_files ${output} ${generated} ERROR_VARIABLE error_compare)
if (error_compare)
    message(FATAL_ERROR "error on compare: ${error_compare}")
endif()

# Make sure the standard test passes appstream validation.
get_filename_component(generated_name ${generated} NAME)
if(${generated_name} STREQUAL "testpackage.appdata.xml")
    find_program(APPSTREAMCLI appstreamcli)
    if(APPSTREAMCLI)
        execute_process(COMMAND ${APPSTREAMCLI} validate ${generated}
            ERROR_VARIABLE appstream_stderr
            OUTPUT_VARIABLE appstream_stdout
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "appstream data seems to be imperfect: ${appstream_stderr} ${appstream_stdout}")
        endif()
    else()
        message(WARNING "skipping appstream validation as no appstreamcli binary was found")
    endif()
endif()
