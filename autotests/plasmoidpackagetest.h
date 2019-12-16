/******************************************************************************
*   Copyright 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>             *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#ifndef PACKAGETEST_H

#include <QTest>

#include "kpackage/package.h"
#include "kpackage/packagestructure.h"

class PlasmoidPackageTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void createAndInstallPackage();
    void createAndUpdatePackage();
    void uncompressPackageWithSubFolder();
    void isValid();
    void filePath();
    void entryList();
    void noCrashOnAsyncInstall();

    void packageInstalled(KJob *j);
    void packageUninstalled(KJob *j);

private:
    void createTestPackage(const QString &packageName, const QString &version);
    void cleanupPackage(const QString &packageName);

    QString m_packageRoot;
    QString m_package;
    KJob *m_packageJob;
    KPackage::Package m_defaultPackage;
    KPackage::PackageStructure *m_defaultPackageStructure;
};

#endif

