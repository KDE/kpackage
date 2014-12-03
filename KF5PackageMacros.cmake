
find_package(ECM 0.0.9 CONFIG REQUIRED)
include(KDEInstallDirs)

set(KPACKAGE_RELATIVE_DATA_INSTALL_DIR "kpackage")
set(KPACKAGE_DATA_INSTALL_DIR "${DATA_INSTALL_DIR}/${KPACKAGE_RELATIVE_DATA_INSTALL_DIR}")

# kpackage_install_package(path componentname [root] [install_dir])
#
# Installs a package to the system path
# @arg path The source path to install from, location of metadata.desktop
# @arg componentname The plugin name of the component, corresponding to the
#       X-KDE-PluginInfo-Name key in metadata.desktop
# @arg root The subdirectory to install to, default: packages
# @arg install_dir the path where to install packages, 
#       such as myapp, that would go under prefix/share/myapp:
#       default ${KPACKAGE_DATA_INSTALL_DIR}
#
# Examples:
# kpackage_install_package(mywidget org.kde.plasma.mywidget plasmoids) # installs an applet
# kpackage_install_package(declarativetoolbox org.kde.toolbox packages) # installs a generic package
# kpackage_install_package(declarativetoolbox org.kde.toolbox) # installs a generic package
#
macro(kpackage_install_package dir component)
   set(root ${ARGV2})
   set(install_dir ${ARGV3})
   if(NOT root)
      set(root packages)
   endif()
   if(NOT install_dir)
      set(install_dir ${KPACKAGE_RELATIVE_DATA_INSTALL_DIR})
   endif()
   install(DIRECTORY ${dir}/ DESTINATION ${DATA_INSTALL_DIR}/${install_dir}/${root}/${component}
           PATTERN .svn EXCLUDE
           PATTERN CMakeLists.txt EXCLUDE
           PATTERN Messages.sh EXCLUDE
           PATTERN dummydata EXCLUDE)

   set(kpackagetool_cmd ${CMAKE_INSTALL_PREFIX}/bin/kpackagetool)

   execute_process(COMMAND ${kpackagetool_cmd} --generate-index -g -p ${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/${install_dir}/${root})

   file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/regenerateindex.sh "${kpackagetool_cmd} --generate-index -g -p ${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/${install_dir}/${root}\n")
endmacro()



