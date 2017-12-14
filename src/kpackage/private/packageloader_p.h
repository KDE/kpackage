/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KPACKAGE_PACKAGELOADER_P_H
#define KPACKAGE_PACKAGELOADER_P_H

#include "packagestructure.h"
#include <KPluginMetaData>

namespace KPackage
{

class PackageLoaderPrivate
{
public:
    PackageLoaderPrivate()
        : isDefaultLoader(false),
          packageStructurePluginDir(QStringLiteral("kpackage/packagestructure"))
    {
    }

    static QSet<QString> knownCategories();
    static QString parentAppConstraint(const QString &parentApp = QString());

    static QSet<QString> s_customCategories;

    QHash<QString, QPointer<PackageStructure> > structures;
    bool isDefaultLoader;
    QString packageStructurePluginDir;
    // We only use this cache during start of the process to speed up many consecutive calls
    // After that, we're too afraid to produce race conditions and it's not that time-critical anyway
    // the 20 seconds here means that the cache is only used within 20sec during startup, after that,
    // complexity goes up and we'd have to update the cache in order to avoid subtle bugs
    // just not using the cache is way easier then, since it doesn't make *that* much of a difference,
    // anyway
    int maxCacheAge = 20;
    qint64 pluginCacheAge = 0;
    QHash<QString, QList<KPluginMetaData>> pluginCache;
};

}

#endif

