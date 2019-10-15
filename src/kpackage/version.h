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

#ifndef KPACKAGEVERSION_H
#define KPACKAGEVERSION_H

/** @file kpackage/version.h <KPackage/Version> */

#include <kpackage/package_export.h>
#include <kpackage_version.h>

#define PACKAGE_MAKE_VERSION(a,b,c) (((a) << 16) | ((b) << 8) | (c))

/**
 * Compile-time macro for checking the kpackage version. Not useful for
 * detecting the version of kpackage at runtime.
 */
#define PACKAGE_IS_VERSION(a,b,c) (PACKAGE_VERSION >= PACKAGE_MAKE_VERSION(a,b,c))

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

#endif // multiple inclusion guard
