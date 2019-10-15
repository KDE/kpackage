/******************************************************************************
*   Copyright 2007-2011 by Aaron Seigo <aseigo@kde.org>                       *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#ifndef KPACKAGE_PACKAGE_H
#define KPACKAGE_PACKAGE_H

#include <QStringList>
#include <QMetaType>
#include <QCryptographicHash>
#include <QUrl>

#include <kpluginmetadata.h>

#include <kpackage/package_export.h>

#include <kjob.h>

namespace KPackage
{

/**
 * @class Package kpackage/package.h <KPackage/Package>
 *
 * @short object representing an installed package
 *
 * Package defines what is in a package and provides easy access to the contents.
 *
 * To define a package, one might write the following code:
 *
 @code
    Package package;

    package.addDirectoryDefinition("images", "pics/", i18n("Images"));
    QStringList mimeTypes;
    mimeTypes << "image/svg" << "image/png" << "image/jpeg";
    package.setMimeTypes("images", mimeTypes);

    package.addDirectoryDefinition("scripts", "code/", i18n("Executable Scripts"));
    mimeTypes.clear();
    mimeTypes << "text/\*";
    package.setMimeTypes("scripts", mimeTypes);

    package.addFileDefinition("mainscript", "code/main.js", i18n("Main Script File"));
    package.setRequired("mainscript", true);
 @endcode
 * One may also choose to create a subclass of PackageStructure and include the setup
 * in the constructor.
 *
 * Either way, Package creates a self-documenting contract between the packager and
 * the application without exposing package internals such as actual on-disk structure
 * of the package or requiring that all contents be explicitly known ahead of time.
 *
 * Subclassing PackageStructure does have provide a number of potential const benefits:
 *    * the package can be notified of path changes via the virtual pathChanged() method
 *    * the subclass may implement mechanisms to install and remove packages using the
 *      virtual install and uninstall methods
 *    * subclasses can be compiled as plugins for easy re-use
 **/
//TODO: write documentation on USING a package

class PackagePrivate;
class PackageStructure;

class KPACKAGE_EXPORT Package
{
public:
    /**
     * Error codes for the install/update/remove jobs
     * @since 5.17
     */
    enum JobError {
        RootCreationError = KJob::UserDefinedError + 1, /**< Cannot create package root directory */
        PackageFileNotFoundError, /**< The package file does not exist */
        UnsupportedArchiveFormatError, /**< The archive format of the package is not supported */
        PackageOpenError, /**< Can't open the package file for reading */
        MetadataFileMissingError, /**< The package doesn't have a metadata.desktop file */
        PluginNameMissingError, /**< The metadata.desktop file doesn't specify a plugin name */
        PluginNameInvalidError, /**< The plugin name contains characters different from letters, digits, dots and underscores */
        UpdatePackageTypeMismatchError, /**< A package with this plugin name was already installed, but has a different type in the metadata.desktop file */
        OldVersionRemovalError, /**< Failed to remove the old version of the package during an upgrade */
        NewerVersionAlreadyInstalledError, /**< We tried to update, but the same version or a newer one is already installed */
        PackageAlreadyInstalledError, /**< The package is already installed and a normal install (not update) was performed */
        PackageMoveError, /**< Failure to move a package from the system temporary folder to its final destination */
        PackageCopyError, /**< Failure to copy a package folder from somewhere in the filesystem to its final destination */
        PackageUninstallError /**< Failure to uninstall a package */
    };

    /**
     * Default constructor
     *
     * @param structure if a null pointer is passed in, this will creates an empty (invalid) Package;
     * otherwise the structure is allowed to set up the Package's initial layout
     * @since 4.6
     */
    explicit Package(PackageStructure *structure = nullptr);

    /**
     * Copy constructor
     * @since 4.6
     */
    Package(const Package &other);

    virtual ~Package();

    /**
     * Assignment operator
     * @since 4.6
     */
    Package &operator=(const Package &rhs);

    /**
     * @return true if this package has a valid PackageStructure associatedw it with it.
     * A package may not be valid, but have a valid structure. Useful when dealing with
     * Package objects in a semi-initialized state (e.g. before calling setPath())
     * @since 5.1
     */
    bool hasValidStructure() const;

    /**
     * @return true if all the required components exist
     **/
    bool isValid() const;

    /**
     * Sets the path to the root of this package
     * @param path an absolute path, or a relative path to the default package root
     * @since 4.3
     */
    void setPath(const QString &path);

    /**
     * @return the path to the root of this particular package
     */
    const QString path() const;

    /**
     * Get the path to a given file based on the key and an optional filename.
     * Example: finding the main script in a scripting package:
     *      filePath("mainscript")
     *
     * Example: finding a specific image in the images directory:
     *      filePath("images", "myimage.png")
     *
     * @param key the key of the file type to look for,
     * @param filename optional name of the file to locate within the package
     * @return path to the file on disk. QString() if not found.
     **/
    QString filePath(const QByteArray &key, const QString &filename = QString()) const;

    /**
     * Get the url to a given file based on the key and an optional filename, is the file:// or qrc:// format
     * Example: finding the main script in a scripting package:
     *      filePath("mainscript")
     *
     * Example: finding a specific image in the images directory:
     *      filePath("images", "myimage.png")
     *
     * @param key the key of the file type to look for,
     * @param filename optional name of the file to locate within the package
     * @return path to the file on disk. QString() if not found.
     * @since 5.41
     **/
    QUrl fileUrl(const QByteArray &key, const QString &filename = QString()) const;

    /**
     * Get the list of files of a given type.
     *
     * @param fileType the type of file to look for, as defined in the
     *               package structure.
     * @return list of files by name, suitable for passing to filePath
     **/
    QStringList entryList(const QByteArray &key) const;

    /**
     * @return user visible name for the given entry
     **/
    QString name(const QByteArray &key) const;

    /**
     * @return true if the item at path exists and is required
     **/
    bool isRequired(const QByteArray &key) const;

    /**
     * @return the mimeTypes associated with the path, if any
     **/
    QStringList mimeTypes(const QByteArray &key) const;

    /**
     * @return the prefix paths inserted between the base path and content entries, in order of priority.
     *         When searching for a file, all paths will be tried in order.
     * @since 4.6
     */
    QStringList contentsPrefixPaths() const;

    /**
     * @return preferred package root. This defaults to kpackage/generic/
     */
    QString defaultPackageRoot() const;

    /**
     * @return true if paths/symlinks outside the package itself should be followed.
     * By default this is set to false for security reasons.
     */
    bool allowExternalPaths() const;

    /**
      * @return the package metadata object.
      */
    KPluginMetaData metadata() const;

#if KPACKAGE_ENABLE_DEPRECATED_SINCE(5, 21)
    /**
     * @return a SHA1 hash digest of the contents of the package in hexadecimal form
     * @since 4.4
     * @deprecated Since 5.21 use cryptographicHash
     */
    KPACKAGE_DEPRECATED_VERSION(5, 21, "Use Package::cryptographicHash(QCryptographicHash::Algorithm)")
    QString contentsHash() const;
#endif

    /**
     * @return a hash digest of the contents of the package in hexadecimal form
     * @since 5.21
     */
    QByteArray cryptographicHash(QCryptographicHash::Algorithm algorithm) const;

    /**
     * Adds a directory to the structure of the package. It is added as
     * a not-required element with no associated mimeTypes.
     *
     * Starting in 4.6, if an entry with the given key
     * already exists, the path is added to it as a search alternative.
     *
     * @param key  used as an internal label for this directory
     * @param path the path within the package for this directory
     * @param name the user visible (translated) name for the directory
     **/
    void addDirectoryDefinition(const QByteArray &key, const QString &path, const QString &name);

    /**
     * Adds a file to the structure of the package. It is added as
     * a not-required element with no associated mimeTypes.
     *
     * Starting in 4.6, if an entry with the given key
     * already exists, the path is added to it as a search alternative.
     *
     * @param key  used as an internal label for this file
     * @param path the path within the package for this file
     * @param name the user visible (translated) name for the file
     **/
    void addFileDefinition(const QByteArray &key, const QString &path, const QString &name);

    /**
     * Removes a definition from the structure of the package.
     * @since 4.6
     * @param key the internal label of the file or directory to remove
     */
    void removeDefinition(const QByteArray &key);

    /**
     * Sets whether or not a given part of the structure is required or not.
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     *
     * @param key the entry within the package
     * @param required true if this entry is required, false if not
     */
    void setRequired(const QByteArray &key, bool required);

    /**
     * Defines the default mimeTypes for any definitions that do not have
     * associated mimeTypes. Handy for packages with only one or predominantly
     * one file type.
     *
     * @param mimeTypes a list of mimeTypes
     **/
    void setDefaultMimeTypes(const QStringList &mimeTypes);

    /**
     * Define mimeTypes for a given part of the structure
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     *
     * @param key the entry within the package
     * @param mimeTypes a list of mimeTypes
     **/
    void setMimeTypes(const QByteArray &key, const QStringList &mimeTypes);

    /**
     * Sets the prefixes that all the contents in this package should
     * appear under. This defaults to "contents/" and is added automatically
     * between the base path and the entries as defined by the package
     * structure. Multiple entries can be added.
     * In this case each file request will be searched in all prefixes in order,
     * and the first found will be returned.
     *
     * @param prefix paths the directory prefix to use
     * @since 4.6
     */
    void setContentsPrefixPaths(const QStringList &prefixPaths);

    /**
     * Sets whether or not external paths/symlinks can be followed by a package
     * @param allow true if paths/symlinks outside of the package should be followed,
     *             false if they should be rejected.
     */
    void setAllowExternalPaths(bool allow);

    /**
     * Sets preferred package root.
     */
    void setDefaultPackageRoot(const QString &packageRoot);

    /**
     * Sets the fallback package root path
     * If a file won't be found in this package, it will search it in the package
     * with the same structure identified by path
     * It is intended to be used by the packageStructure
     * @param path package root path @see setPath
     */
    void setFallbackPackage(const KPackage::Package &package);

    /**
     * @return The fallback package root path
     */
    KPackage::Package fallbackPackage() const;

    // Content structure description methods
    /**
     * @return all directories registered as part of this Package's structure
     */
    QList<QByteArray> directories() const;

    /**
     * @return all directories registered as part of this Package's required structure
     */
    QList<QByteArray> requiredDirectories() const;

    /**
     * @return all files registered as part of this Package's structure
     */
    QList<QByteArray> files() const;

    /**
     * @return all files registered as part of this Package's required structure
     */
    QList<QByteArray> requiredFiles() const;

    /**
     * Installs a package matching this package structure. By default installs a
     * native KPackage::Package.
     * After the KJob will emitted finished(), if the install went well
     * the Package instance will be guaranteed to have loaded the package just
     * installed, and be valid and usable.
     *
     * @return KJob to track installation progress and result
     **/
    KJob *install(const QString &sourcePackage, const QString &packageRoot = QString());

    /**
     * Updates a package matching this package structure. By default installs a
     * native KPackage::Package. If an older version is already installed,
     * it removes the old one. If the installed one is newer,
     * an error will occur.
     * After the KJob will emitted finished(), if the install went well
     * the Package instance will be guaranteed to have loaded the package just
     * updated, and be valid and usable.
     *
     * @return KJob to track installation progress and result
     * @since 5.17
     **/
    KJob *update(const QString &sourcePackage, const QString &packageRoot = QString());

    /**
     * Uninstalls a package matching this package structure.
     *
     * @return KJob to track removal progress and result
     */
    KJob *uninstall(const QString &packageName, const QString &packageRoot);

private:
    QExplicitlySharedDataPointer<PackagePrivate> d;
    friend class PackagePrivate;
};

}

Q_DECLARE_METATYPE(KPackage::Package)
#endif

