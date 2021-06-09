/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kpackage/version.h>

#if KPACKAGE_BUILD_DEPRECATED_SINCE(5, 84)

#include "kpackage_debug.h"

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
        qCWarning(KPACKAGE_LOG) << "unversioned plugin detected, may result in instability";
        return true;
    }

    // we require PACKAGE_VERSION_MAJOR and PACKAGE_VERSION_MINOR
    const quint32 minVersion = PACKAGE_MAKE_VERSION(PACKAGE_VERSION_MAJOR, 0, 0);
    const quint32 maxVersion = PACKAGE_MAKE_VERSION(PACKAGE_VERSION_MAJOR, PACKAGE_VERSION_MINOR, 60);

    if (version < minVersion || version > maxVersion) {
#ifndef NDEBUG
        // qCDebug(KPACKAGE_LOG) << "plugin is compiled against incompatible KPackage version  " << version
        //         << "This build is compatible with" << PACKAGE_VERSION_MAJOR << ".0.0 (" << minVersion
        //         << ") to" << PACKAGE_VERSION_STRING << "(" << maxVersion << ")";
#endif
        return false;
    }

    return true;
}

} // namespace
#endif
