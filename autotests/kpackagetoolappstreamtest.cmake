execute_process(COMMAND ${kpackagetool} --appstream-metainfo . WORKING_DIRECTORY ${input} OUTPUT_FILE "${generated}" ERROR_VARIABLE error)
if (error)
    message(FATAL_ERROR "couldn't generate metadata: ${error}")
endif()

execute_process(COMMAND cmake -E compare_files ${output} ${generated} ERROR_VARIABLE error_compare)
if (error_compare)
    message(FATAL_ERROR "error on compare: ${error_compare}")
endif()
