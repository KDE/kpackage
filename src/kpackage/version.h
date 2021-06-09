/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGEVERSION_H
#define KPACKAGEVERSION_H

/**
 *@file kpackage/version.h <KPackage/Version>
 *@deprecated Deprecated for lack of usage, use equivalents from plasma-framework instead
 */

#include <kpackage/package_export.h>
#include <kpackage_version.h>

#if KPACKAGE_ENABLE_DEPRECATED_SINCE(5, 84)

#define PACKAGE_MAKE_VERSION(a, b, c) (((a) << 16) | ((b) << 8) | (c))

/**
 * Compile-time macro for checking the kpackage version. Not useful for
 * detecting the version of kpackage at runtime.
 */
#define PACKAGE_IS_VERSION(a, b, c) (PACKAGE_VERSION >= PACKAGE_MAKE_VERSION(a, b, c))

/**
 * Namespace for everything in kpackage
 */
namespace KPackage
{
/**
 * The runtime version of libkpackage
 */
KPACKAGE_EXPORT unsigned int version();

/**
 * The runtime major version of libkpackage
 */
KPACKAGE_EXPORT unsigned int versionMajor();

/**
 * The runtime major version of libkpackage
 */
KPACKAGE_EXPORT unsigned int versionMinor();

/**
 * The runtime major version of libkpackage
 */
KPACKAGE_EXPORT unsigned int versionRelease();

/**
 * The runtime version string of libkpackage
 */
KPACKAGE_EXPORT const char *versionString();

/**
 * Verifies that a plugin is compatible with plasma
 */
KPACKAGE_EXPORT bool isPluginVersionCompatible(unsigned int version);

} // Plasma namespace

#endif

#endif // multiple inclusion guard
