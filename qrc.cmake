set(OUTPUT "<!DOCTYPE RCC><RCC version='1.0'>\n
<qresource prefix='/${INSTALL_DIR}/${ROOT}/${COMPONENT}/'>
")

file(GLOB_RECURSE files LIST_DIRECTORIES FALSE RELATIVE ${DIRECTORY} ${DIRECTORY}/*)
foreach(v IN LISTS files)
    set(OUTPUT "${OUTPUT}   <file alias='${v}'>${DIRECTORY}/${v}</file>\n")
endforeach()

if (metadatajson)
    set(OUTPUT "${OUTPUT}   <file alias='metadata.json'>${metadatajson}</file>\n")
endif()

set(OUTPUT "${OUTPUT}</qresource>
</RCC>
")
file(WRITE "${OUTPUTFILE}" "${OUTPUT}")
