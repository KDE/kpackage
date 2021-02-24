/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packagestructure.h"
#include "kpackage_debug.h"
#include "private/package_p.h"
#include <private/packagejob_p.h>

namespace KPackage
{
PackageStructure::PackageStructure(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(nullptr)
{
    Q_UNUSED(args)
}

PackageStructure::~PackageStructure()
{
}

void PackageStructure::initPackage(Package *package)
{
    Q_UNUSED(package)
}

void PackageStructure::pathChanged(Package *package)
{
    Q_UNUSED(package)
}

KJob *PackageStructure::install(Package *package, const QString &archivePath, const QString &packageRoot)
{
    PackageJob *j = new PackageJob(package);
    j->install(archivePath, packageRoot);
    return j;
}

KJob *PackageStructure::update(Package *package, const QString &archivePath, const QString &packageRoot)
{
    PackageJob *j = new PackageJob(package);
    j->update(archivePath, packageRoot);
    return j;
}

KJob *PackageStructure::uninstall(Package *package, const QString &packageRoot)
{
    PackageJob *j = new PackageJob(package);
    const QString pluginID = package->metadata().pluginId();
    QString uninstallPath;
    // We handle the empty path when uninstalling the package
    // If the dir already got deleted the pluginId is an empty string, without this
    // check we would delete the package root, BUG: 410682
    if (!pluginID.isEmpty()) {
        uninstallPath = packageRoot + QLatin1Char('/') + pluginID;
    }
    j->uninstall(uninstallPath);
    return j;
}

}

#include "moc_packagestructure.cpp"
