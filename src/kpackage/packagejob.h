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

/**
 * @class PackageJob kpackage/packagejob.h <KPackage/PackageJob>
 * @short KJob subclass that allows async install/update/uninstall operations for packages
 */
class KPACKAGE_EXPORT PackageJob : public KJob
{
    Q_OBJECT

public:
    ~PackageJob() override;
    static PackageJob *install(PackageStructure *structure, const QString &sourcePackage, const QString &packageRoot);
    static PackageJob *update(PackageStructure *structure, const QString &sourcePackage, const QString &packageRoot);
    static PackageJob *uninstall(PackageStructure *structure, const QString &pluginId, const QString &packageRoot);

    Q_SIGNAL void finished(KJob *job, const KPackage::Package &package);

private:
    friend class PackageJobThread;
    enum OperationType {
        Install,
        Update,
        Uninstall,
    };
    void start() override;
    explicit PackageJob(OperationType type, const Package &package, const QString &src, const QString &dest);
    void setupNotificationsOnJobFinished(const QString &messageName);

    const std::unique_ptr<PackageJobPrivate> d;
};

}

#endif
