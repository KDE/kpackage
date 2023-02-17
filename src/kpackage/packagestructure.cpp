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
PackageStructure::PackageStructure(QObject *parent, const QVariantList & /*args*/)
    : QObject(parent)
    , d(nullptr)
{
}

PackageStructure::~PackageStructure()
{
}

void PackageStructure::initPackage(Package * /*package*/)
{
}

void PackageStructure::pathChanged(Package * /*package*/)
{
}

KJob *PackageStructure::install(Package *package, const QString &archivePath, const QString &packageRoot)
{
    return PackageJob::install(package, archivePath, packageRoot);
}

KJob *PackageStructure::update(Package *package, const QString &archivePath, const QString &packageRoot)
{
    return PackageJob::update(package, archivePath, packageRoot);
}

KJob *PackageStructure::uninstall(Package *package, const QString &packageRoot)
{
    const QString pluginID = package->metadata().pluginId();
    QString uninstallPath;
    // We handle the empty path when uninstalling the package
    // If the dir already got deleted the pluginId is an empty string, without this
    // check we would delete the package root, BUG: 410682
    if (!pluginID.isEmpty()) {
        uninstallPath = packageRoot + QLatin1Char('/') + pluginID;
    }
    return PackageJob::uninstall(package, uninstallPath);
}

}
