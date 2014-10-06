
find_package(ECM 0.0.9 CONFIG REQUIRED)
include(KDEInstallDirs)

set(KPACKAGE_RELATIVE_DATA_INSTALL_DIR "kpackage")
set(KPACKAGE_DATA_INSTALL_DIR "${DATA_INSTALL_DIR}/${KPACKAGE_RELATIVE_DATA_INSTALL_DIR}")

# kpackage_install_package(path componentname [root] [type])
#
# Installs a package to the system path
# @arg path The source path to install from, location of metadata.desktop
# @arg componentname The plugin name of the component, corresponding to the
#       X-KDE-PluginInfo-Name key in metadata.desktop
# @arg root The subdirectory to install to, default: plasmoids
# @arg type The type, default to applet, or applet, package, containment,
#       wallpaper, shell, lookandfeel, etc.
# @see Types column in plasmapkg --list-types
#
# Examples:
# kpackage_install_package(mywidget org.kde.plasma.mywidget) # installs an applet
# kpackage_install_package(declarativetoolbox org.kde.toolbox packages package) # installs a generic package
#
macro(kpackage_install_package dir component)
   set(root ${ARGV2})
   set(type ${ARGV3})
   if(NOT root)
      set(root plasmoids)
   endif()
   if(NOT type)
      set(type applet)
   endif()
   install(DIRECTORY ${dir}/ DESTINATION ${KPACKAGE_DATA_INSTALL_DIR}/${root}/${component}
           PATTERN .svn EXCLUDE
           PATTERN CMakeLists.txt EXCLUDE
           PATTERN Messages.sh EXCLUDE
           PATTERN dummydata EXCLUDE)

   install(FILES ${dir}/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME kpackage-${type}-${component}.desktop)
endmacro()



