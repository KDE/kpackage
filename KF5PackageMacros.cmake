# To not mess up the ECM_FIND_VERSION we only include ECM if the required variables are not set
if (NOT KDE_INSTALL_DATADIR OR NOT KDE_INSTALL_METAINFODIR)
    find_package(ECM 5.83.0 CONFIG REQUIRED)
    include(${ECM_KDE_MODULE_DIR}/KDEInstallDirs.cmake)
endif()
set(KPACKAGE_RELATIVE_DATA_INSTALL_DIR "kpackage")

# kpackage_install_package(path componentname [root] [install_dir])
#
# Installs a package to the system path
# @arg path The source path to install from, location of metadata.desktop
# @arg componentname The plugin name of the component, corresponding to the
#       X-KDE-PluginInfo-Name key in metadata.desktop
# @arg root The subdirectory to install to, default: packages
# @arg install_dir the path where to install packages,
#       such as myapp, that would go under prefix/share/myapp:
#       default ${KPACKAGE_RELATIVE_DATA_INSTALL_DIR}
#
# Examples:
# kpackage_install_package(mywidget org.kde.plasma.mywidget plasmoids) # installs an applet
# kpackage_install_package(declarativetoolbox org.kde.toolbox packages) # installs a generic package
# kpackage_install_package(declarativetoolbox org.kde.toolbox) # installs a generic package
#

set(kpackagedir ${CMAKE_CURRENT_LIST_DIR})
function(kpackage_install_package dir component)
   set(root ${ARGV2})
   set(install_dir ${ARGV3})
   if(NOT root)
      set(root packages)
   endif()
   if(NOT install_dir)
      set(install_dir ${KPACKAGE_RELATIVE_DATA_INSTALL_DIR})
   endif()

   install(DIRECTORY ${dir}/ DESTINATION ${KDE_INSTALL_DATADIR}/${install_dir}/${root}/${component}
            PATTERN .svn EXCLUDE
            PATTERN *.qmlc EXCLUDE
            PATTERN CMakeLists.txt EXCLUDE
            PATTERN Messages.sh EXCLUDE
            PATTERN dummydata EXCLUDE)

   set(metadatajson)
   set(ORIGINAL_METADATA "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/metadata.desktop")
   if(NOT EXISTS ${component}-${root}-metadata.json AND EXISTS ${ORIGINAL_METADATA})
        message(WARNING "Providing metadata with .desktop files is deprecated. Please run \"desktoptojson -i ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/metadata.desktop\" to convert the file to json and remove the desktop file")
        set(GENERATED_METADATA "${CMAKE_CURRENT_BINARY_DIR}/${component}-${root}-metadata.json")
        add_custom_target(${component}-${root}-metadata-json ALL DEPENDS ${GENERATED_METADATA})
        kcoreaddons_desktop_to_json(${component}-${root}-metadata-json ${ORIGINAL_METADATA}
                                    OUTPUT_FILE ${GENERATED_METADATA})
        install(FILES ${GENERATED_METADATA} DESTINATION ${KDE_INSTALL_DATADIR}/${install_dir}/${root}/${component} RENAME metadata.json)
        set(metadatajson ${GENERATED_METADATA})
    elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/metadata.json")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/metadata.json" DESTINATION ${KDE_INSTALL_DATADIR}/${install_dir}/${root}/${component})
    endif()

   get_target_property(kpackagetool_cmd KF5::kpackagetool5 LOCATION)
   if (${component} MATCHES "^.+\\..+\\.") #we make sure there's at least 2 dots
        set(APPDATAFILE "${CMAKE_CURRENT_BINARY_DIR}/${component}.appdata.xml")

        execute_process(
            COMMAND ${kpackagetool_cmd} --appstream-metainfo ${CMAKE_CURRENT_SOURCE_DIR}/${dir} --appstream-metainfo-output ${APPDATAFILE}
            ERROR_VARIABLE appstreamerror
            RESULT_VARIABLE result)
        if (NOT result EQUAL 0)
            message(WARNING "couldn't generate metainfo for ${component}: ${appstreamerror}")
        else()
            if(appstreamerror)
                message(WARNING "warnings during generation of metainfo for ${component}: ${appstreamerror}")
            endif()

            # OPTIONAL because desktop files can be NoDisplay so they render no XML.
            install(FILES ${APPDATAFILE} DESTINATION ${KDE_INSTALL_METAINFODIR} OPTIONAL)
        endif()
   else()
        message(DEBUG "KPackage components should be specified in reverse domain notation. Appstream information won't be generated for ${component}.")
   endif()
endfunction()

# See kpackage_install_package documentation above.
# This function bundles the package into a Qt rcc file instead of installing
# the individual files. Great care must be taken that the way package files are
# accessed is actually rcc-compatible (e.g. package.fileUrl must be used and
# manually constructed paths shouldn't treat the resources as file:// URLs
# either).
# Deprecated since 5.85, use kpackage_install_package instead. 
function(kpackage_install_bundled_package dir component)
    message(WARNING "kpackage_install_bundled_package is deprecated, use kpackage_install_package instead")
    set(root ${ARGV2})
    set(install_dir ${ARGV3})
    if(NOT root)
        set(root packages)
    endif()
    if(NOT install_dir)
        set(install_dir ${KPACKAGE_RELATIVE_DATA_INSTALL_DIR})
    endif()

    set(metadatajson)
    set(ORIGINAL_METADATA "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/metadata.desktop")
    if(NOT EXISTS ${component}-${root}-metadata.json AND EXISTS ${ORIGINAL_METADATA})
            set(GENERATED_METADATA "${CMAKE_CURRENT_BINARY_DIR}/${component}-${root}-metadata.json")
            add_custom_target(${component}-${root}-metadata-json ALL DEPENDS ${GENERATED_METADATA})
            kcoreaddons_desktop_to_json(${component}-${root}-metadata-json ${ORIGINAL_METADATA}
                                        OUTPUT_FILE ${GENERATED_METADATA})
            install(FILES ${GENERATED_METADATA} DESTINATION ${KDE_INSTALL_DATADIR}/${install_dir}/${root}/${component} RENAME metadata.json)
            set(metadatajson ${GENERATED_METADATA})
    endif()


    get_target_property(kpackagetool_cmd KF5::kpackagetool5 LOCATION)
    if (${component} MATCHES "^.+\\..+\\.") #we make sure there's at least 2 dots
            set(APPDATAFILE "${CMAKE_CURRENT_BINARY_DIR}/${component}.appdata.xml")

            execute_process(
                COMMAND ${kpackagetool_cmd} --appstream-metainfo ${CMAKE_CURRENT_SOURCE_DIR}/${dir} --appstream-metainfo-output ${APPDATAFILE}
                ERROR_VARIABLE appstreamerror
                RESULT_VARIABLE result)
            if (NOT result EQUAL 0)
                message(WARNING "couldn't generate metainfo for ${component}: ${appstreamerror}")
            else()
                if(appstreamerror)
                    message(WARNING "warnings during generation of metainfo for ${component}: ${appstreamerror}")
                endif()

                # OPTIONAL because desktop files can be NoDisplay so they render no XML.
                install(FILES ${APPDATAFILE} DESTINATION ${KDE_INSTALL_METAINFODIR} OPTIONAL)
            endif()
    else()
            message(DEBUG "KPackage components should be specified in reverse domain notation. Appstream information won't be generated for ${component}.")
    endif()

    set(kpkgqrc "${CMAKE_CURRENT_BINARY_DIR}/${component}.qrc")
    set(metadatajson ${metadatajson})
    set(component ${component})
    set(root ${root})
    set(BINARYDIR ${CMAKE_CURRENT_BINARY_DIR})
    set(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
    set(OUTPUTFILE "${kpkgqrc}")
    add_custom_target(${component}-${root}-qrc ALL
                      COMMAND ${CMAKE_COMMAND} -DINSTALL_DIR=${install_dir} -DROOT=${root} -DCOMPONENT=${component} -DDIRECTORY=${DIRECTORY} -D OUTPUTFILE=${OUTPUTFILE} -P ${kpackagedir}/qrc.cmake
                      DEPENDS ${component}-${root}-metadata-json)
    set(GENERATED_RCC_CONTENTS "${CMAKE_CURRENT_BINARY_DIR}/${component}-contents.rcc")
    # add_custom_target depends on ALL target so qrc is run every time
    # it doesn't have OUTPUT property so it's considered out-of-date every build
    add_custom_target(${component}-${root}-contents-rcc ALL
                      COMMENT "Generating ${component}-contents.rcc"
                      COMMAND Qt${QT_MAJOR_VERSION}::rcc "${kpkgqrc}" --binary -o "${GENERATED_RCC_CONTENTS}"
                      DEPENDS ${component}-${root}-metadata-json ${component}-${root}-qrc)
    install(FILES ${GENERATED_RCC_CONTENTS} DESTINATION ${KDE_INSTALL_DATADIR}/${install_dir}/${root}/${component}/ RENAME contents.rcc)

endfunction()
