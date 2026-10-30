/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packagestructure.h"
#include "packagejob.h"

namespace KPackage
{
PackageStructure::PackageStructure(QObject *parent, const QVariantList & /*args*/)
    : QObject(parent)
{
    Q_UNUSED(d)
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

}

#include "moc_packagestructure.cpp"
