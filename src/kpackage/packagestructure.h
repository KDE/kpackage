/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGE_PACKAGESTRUCTURE_H
#define KPACKAGE_PACKAGESTRUCTURE_H

#include <QStringList>

#include <KPluginFactory>

#include <kpackage/package.h>
#include <kpackage/package_export.h>

namespace KPackage
{
class PackageStructurePrivate;

/**
 * @class PackageStructure kpackage/packagestructure.h <KPackage/PackageStructure>
 *
 * This class is used to define the filesystem structure of a package type.
 * A PackageStructure is implemented as a dynamically loaded plugin, in the reimplementation
 * of initPackage the allowed fines and directories in the package are set into the package,
 * for instance:
 *
 * @code
 * package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"), i18n("Main Script File"));
 * package->setDefaultPackageRoot(QStringLiteral("plasma/wallpapers/"));
 * package->addDirectoryDefinition("images", QStringLiteral("images"), i18n("Images"));
 * package->addDirectoryDefinition("theme", QStringLiteral("theme"), i18n("Themed Images"));
 * QStringList mimetypes;
 * mimetypes << QStringLiteral("image/svg+xml") << QStringLiteral("image/png") << QStringLiteral("image/jpeg");
 * package->setMimeTypes("images", mimetypes);
 * @endcode
 *
 * @author Aaron Seigo
 */
class KPACKAGE_EXPORT PackageStructure : public QObject
{
    Q_OBJECT

public:
    explicit PackageStructure(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    ~PackageStructure() override;

    /**
     * Called when a the PackageStructure should initialize a Package with the initial
     * structure. This allows setting paths before setPath is called.
     *
     * Note: one special value is "metadata" which can be set to the location of KPluginMetaData
     * compatible .desktop file within the package. If not defined, it is assumed that this file
     * exists under the top level directory of the package.
     *
     * @param package the Package to set up. The object is empty of all definition when
     *      first passed in.
     */
    virtual void initPackage(Package *package);

    /**
     * Called whenever the path changes so that subclasses may take
     * package specific actions.
     */
    virtual void pathChanged(Package *package);

    /**
     * Installs a package matching this package structure. By default installs a
     * native KPackage::Package.
     *
     * @param package the instance of Package that is being used for the install; useful for
     * accessing file paths
     * @param archivePath path to the package archive file
     * @param packageRoot path to the directory where the package should be
     *                    installed to
     * @return KJob* to track the installation status
     **/
    virtual KJob *install(Package *package, const QString &archivePath, const QString &packageRoot);

    /**
     * Updates a package matching this package structure. By default installs a
     * native KPackage::Package. If an older version is already installed,
     * it removes the old one. If the installed one is newer,
     * an error will occur.
     *
     * @param package the instance of Package that is being used for the install; useful for
     * accessing file paths
     * @param archivePath path to the package archive file
     * @param packageRoot path to the directory where the package should be
     *                    installed to
     * @return KJob* to track the installation status
     * @since 5.17
     **/
    virtual KJob *update(Package *package, const QString &archivePath, const QString &packageRoot);

    /**
     * Uninstalls a package matching this package structure.
     *
     * @param package the instance of Package that is being used for the install; useful for
     * accessing file paths
     * @param packageName the name of the package to remove
     * @param packageRoot path to the directory where the package should be installed to
     * @return KJob* to track the installation status
     */
    virtual KJob *uninstall(Package *package, const QString &packageRoot);

private:
    PackageStructurePrivate *d;
};

} // KPackage namespace

/**
 * Register a Package class when it is contained in a loadable module
 */

#define K_EXPORT_KPACKAGE_PACKAGE_WITH_JSON(classname, jsonFile) K_PLUGIN_FACTORY_WITH_JSON(factory, jsonFile, registerPlugin<classname>();)

#endif
