/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGE_PACKAGEJOB_H
#define KPACKAGE_PACKAGEJOB_H

#include <kpackage/package_export.h>

#include <KJob>
#include <memory>

namespace KPackage
{
class PackageJobPrivate;
class Package;
class PackageStructure;

/*!
 * \class KPackage::PackageJob
 * \inheaderfile KPackage/PackageJob
 * \inmodule KPackage
 *
 * \brief KJob subclass that allows async install/update/uninstall operations for packages.
 */
class KPACKAGE_EXPORT PackageJob : public KJob
{
    Q_OBJECT

public:
    /*!
     * Error codes for the install/update/remove jobs
     *
     * \value InvalidPackageStructure Could not find/load the given package structure
     * \value RootCreationError Cannot create package root directory
     * \value PackageFileNotFoundError The package file does not exist
     * \value UnsupportedArchiveFormatError The archive format of the package is not supported
     * \value PackageOpenError Can't open the package file for reading
     * \value PluginIdInvalidError The plugin id is not specified in the metadata.json file or contains characters different from letters, digits, dots and
     * underscores
     * \value UpdatePackageTypeMismatchError A package with this plugin id was already installed, but has a different type in the metadata.json file
     * \value OldVersionRemovalError Failed to remove the old version of the package during an upgrade
     * \value NewerVersionAlreadyInstalledError We tried to update, but the same version or a newer one is already installed
     * \value PackageAlreadyInstalledError The package is already installed and a normal install (not update) was performed
     * \value PackageMoveError Failure to move a package from the system temporary folder to its final destination
     * \value PackageCopyError Failure to copy a package folder from somewhere in the filesystem to its final destination
     * \value PackageUninstallError Failure to uninstall a package
     */
    enum JobError {
        InvalidPackageStructure = KJob::UserDefinedError + 1,
        RootCreationError,
        PackageFileNotFoundError,
        UnsupportedArchiveFormatError,
        PackageOpenError,
        PluginIdInvalidError,
        UpdatePackageTypeMismatchError,
        OldVersionRemovalError,
        NewerVersionAlreadyInstalledError,
        PackageAlreadyInstalledError,
        PackageMoveError,
        PackageCopyError,
        PackageUninstallError,
    };

    ~PackageJob() override;
    /*!
     * Installs the given package. The returned job is already started
     */
    static PackageJob *install(const QString &packageFormat, const QString &sourcePackage, const QString &packageRoot = QString());
    /*!
     * Installs the given package. The returned job is already started
     */
    static PackageJob *update(const QString &packageFormat, const QString &sourcePackage, const QString &packageRoot = QString());
    /*!
     * Installs the given package. The returned job is already started
     */
    static PackageJob *uninstall(const QString &packageFormat, const QString &pluginId, const QString &packageRoot = QString());

    /*!
     *
     */
    KPackage::Package package() const;

private:
    friend class PackageJobThread;
    enum OperationType {
        Install,
        Update,
        Uninstall,
    };
    void start() override;

    KPACKAGE_NO_EXPORT explicit PackageJob(OperationType type, const Package &package, const QString &src, const QString &dest);
    KPACKAGE_NO_EXPORT void setupNotificationsOnJobFinished(const QString &messageName);

    const std::unique_ptr<PackageJobPrivate> d;
    friend PackageJobPrivate;
};

}

#endif
