# Package Framework

Installation and loading of additional content (ex: scripts, images...) as packages

## Introduction
The Package framework lets the user install and load packages of non binary content such as scripted extensions or graphic assets, as if they were traditional plugins.

# using KPackage
The frameworks consists of 3 classes: PackageStructure, PackageLoader and Package
The central class is Package, that represents an installed package and is used to access its file, install, update or uninstall it.

In order to use the framework the developer has to define a package structure. (the package type, that will be depending on the particular application, and the type of content that will be provided, may be a sound theme, graphics assets, qml files etc).

The package loader is used to load instances of Package of a given type.

### Package class
Package is the central class of the framework, and it represents a package of a certain type, it can be either "abstract", not pointing to a particular package on disk (in which case the only things allowed will be installing and uninstalling) or it can point to a particular package installed on disk: its path() not being empty and valid.
A Package defines a particular filesystem structure for installed addons, made up of allowed subdirectories in which content will be organized and several optional named files, if you want to mandate a particular named file in every package (for instance if your addons need a file named "main.qml" in order to be considered valid)
A fixed structure is encouraged by the api of Package in order to encourage plugin creators to distribute packages made with a standard and expected structure, encouraging a common path installation prefix, and strongly discouraging cross references between differnt packages such as ../ relative paths and symlinks across packages.
An example filesystem structure (that in the end, depends from the PackageStructure) may be:
(root)
    metadata.desktop
    contents/
        code/
            main.js
        config/
            config.xml
        images/
            background.png
The special, main and always required for every packagestructure file is the "metadata" file, which describes the package with values such as name, description, pluginname etc. It is in any format accepted by KPluginMetadata, meaning at the moment either a .desktop or json files. The metadata is accessible with Package::metadata()
All the other files are under the contents/ subdirectory: a folder under addDirectoryDefinition will be registered under contents/.
If the developer wants that those extension package require to have a particular file or folder, he will use setRequired() on a particular key: in that case if a package misses that file or folder, isValid() will return false and all the filePath resolution won't work.
To access a file within the package, the filePath(key, filename) method is used. In order to use that the caller doesn't have to care where the package is installed and absolute paths of any kind, it will pass a relative one, and the prober absolute path will be resolved and returned.
The key parameter is one of those that has been registered by the structure beforehand by addFileDefinition or addDirectoryDefinition. Files asked with an unknown keys won't be resolved.
Accessing a file under a directory will look like package.filePath("ui", "main.qml") while accessing a particular named file will look like package.filePath("mainscript")
If as file path is passed a relative path with ".." elements in it or an absolute path, the resolution will fail as well, in order to discourage cross references (same thing will happen if the resolved file is a symlink), unless the package structure explicitly allowed it with setAllowExternalPaths()


### Package structures
Package structures are instance of PackageStructure and are shipeed as plugins.
PackageStructure::initPackage will be executed once when any package is created, and this initializes the Package instance on what are the allowed subfolders and named files for this type. The most important thing to do in initPackage is setting a packageRoot path, that will be a common prefix under which all packages will be installed, relative to the xdg data paths. For instance the packages of type "plasmoid" will be under "plasma/plasmoids" which means searching unser ~/.local/share/plasma/plasmoids and /usr/share/plasma/plasmoids
This is the only required function to be reimplmented for a given PackageStructure, the other ones being optional only when particular needs ensue.
PackageStructure::pathChanged gets executed when the base path of the Package changes (that means, what package on the filesystem the class is pointing to, as this can change at runetime with Package::setPath). Reimplement this only if you need to have extra check/operations when such a thing happens.

### Package loader
The PackageLoader is used to list, find and load Package instances.
The most important functions of this class are listPackages/findPackages to list and search packages
of a certain type (package structure), for example listing all plasmoids or all plasmoids that have "clock" in their description
PackageLoader::loadPackage loads a "blank" package of a certain type. The returned package will be
a Package instance that has been initialized to that given structure, pointing to a particular
package in the filesystem only if the optional parameter packagePath has been passed. If not,
it will be an invalid package, still usable to install and uninstall packages of this type or
it can always load a package on the filesystem with the setPath Package method, relative to the packageroot. Upon setPath, subdirectories of the packageroot will be searched both in the global system installation and in the user home, preferring the home, such as ~/.local/share/plasma/plasmoids and /usr/share/plasma/plasmoids. If one has to iterate trough packages, creating a single Package instance then loading different ones with subsequnt calls of setPath is preferrable over creating multiple instances for performance reasons.
Example of code loading a package:
    KPackage::Package p = KPackageLoader::self()->loadPackage("Plasma/Applet", "org.kde.plasma.analogclock");
    if (p.isValid()) {
        qDebug() << p.filePath("mainscript");
    }


## Note for packagers
After a package gets installed, its index should be regenerated. Index files CANNOT be shipped directly in packages. However they should be regenerated as post install script for the package itself.
In the cmake build directory of applications with packages there will be the file regenerateindex.sh that will contain all the commands that have to be executed in post install scripts to regenerate the indexes.

