/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                            *
*   Copyright 2014 by Marco Martin <mart@kde.org>                             *
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

#include "querytest.h"

#include <QDebug>
#include <QStandardPaths>
#include <klocalizedstring.h>

#include "packagestructure.h"
#include "packageloader.h"

#include "config.h"

void QueryTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    //Remove any eventual installed package globally on the system
    qputenv("XDG_DATA_DIRS", "/not/valid");
    qputenv("KPACKAGE_DEP_RESOLVERS_PATH", KPACKAGE_DEP_RESOLVERS);

    m_dataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    m_dataDir.removeRecursively();

    QVERIFY(m_dataDir.mkpath(QStringLiteral(".")));

    ps = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KPackage/Generic"));
    ps.addFileDefinition("mainscript", QStringLiteral("ui/main.qml"), i18n("Main Script File"));
}

void QueryTest::cleanupTestCase()
{
    m_dataDir.removeRecursively();
}

void QueryTest::installAndQuery()
{
    // verify that no packages are installed
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 0);

    // install some packages
    ps.install(QFINDTESTDATA("data/testpackage"));
    ps.install(QFINDTESTDATA("data/testfallbackpackage"));
    ps.install(QFINDTESTDATA("data/testjsonmetadatapackage"));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 3);

    // installing package with invalid metadata should not be possible
    ps.install(QFINDTESTDATA("data/testinvalidmetadata"));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 3);

    // package with valid dep information should be installed
    ps.install(QFINDTESTDATA("data/testpackagesdep"));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 4);

    // package with invalid dep information should not be installed
    ps.install(QFINDTESTDATA("data/testpackagesdepinvalid"));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 4);
}

QTEST_MAIN(QueryTest)

