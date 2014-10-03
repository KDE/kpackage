/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
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

#include <kpackage/version.h>

#include <QDebug>

#define PACKAGE_MAKE_VERSION(MAJOR, MINOR, PATCH) (MAJOR << 16) | (MINOR << 8) | (PATCH << 0)

namespace KPackage
{

unsigned int version()
{
    return PACKAGE_VERSION;
}

unsigned int versionMajor()
{
    return PACKAGE_VERSION_MAJOR;
}

unsigned int versionMinor()
{
    return PACKAGE_VERSION_MINOR;
}

unsigned int versionRelease()
{
    return PACKAGE_VERSION_PATCH;
}

const char *versionString()
{
    return PACKAGE_VERSION_STRING;
}

bool isPluginVersionCompatible(unsigned int version)
{
    if (version == quint32(-1)) {
        // unversioned, just let it through
        qWarning() << "unversioned plugin detected, may result in instability";
        return true;
    }

    // we require PACKAGE_VERSION_MAJOR and PACKAGE_VERSION_MINOR
    const quint32 minVersion = PACKAGE_MAKE_VERSION(PACKAGE_VERSION_MAJOR, 0, 0);
    const quint32 maxVersion = PACKAGE_MAKE_VERSION(PACKAGE_VERSION_MAJOR, PACKAGE_VERSION_MINOR, 60);

    if (version < minVersion || version > maxVersion) {
#ifndef NDEBUG
        // qDebug() << "plugin is compiled against incompatible Plasma version  " << version
        //         << "This build is compatible with" << PACKAGE_VERSION_MAJOR << ".0.0 (" << minVersion
        //         << ") to" << PACKAGE_VERSION_STRING << "(" << maxVersion << ")";
#endif
        return false;
    }

    return true;
}

} // namespace

