# To not mess up the ECM_FIND_VERSION we only include ECM if the required variables are not set
if (NOT KDE_INSTALL_DATADIR OR NOT KDE_INSTALL_METAINFODIR)
    find_package(ECM 5.83.0 CONFIG REQUIRED)
    include(${ECM_KDE_MODULE_DIR}/KDEInstallDirs.cmake)
endif()
set(KPACKAGE_RELATIVE_DATA_INSTALL_DIR "kpackage")

# kpackage_install_package(path componentname [root] [install_dir])
#
# Installs a package to the system path
# @arg path The source path to install from, location of metadata.json
# @arg componentname The plugin id of the component, corresponding to the
#       Id key of the KPlugin object in metadata.json
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
endfunction()
