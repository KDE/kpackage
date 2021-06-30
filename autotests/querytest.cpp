/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "querytest.h"

#include <KLocalizedString>
#include <QCoreApplication>
#include <QStandardPaths>

#include "packageloader.h"
#include "packagestructure.h"

#include "config.h"

void QueryTest::initTestCase()
{
    QCoreApplication::addLibraryPath(TEST_PLUGIN_DIR);
    QStandardPaths::setTestModeEnabled(true);
    // Remove any eventual installed package globally on the system
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
#ifdef Q_OS_WIN
    QEXPECT_FAIL("", "This returns 3 on Windows, why?", Abort);
#endif
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 4);

    // package with invalid dep information should not be installed
    ps.install(QFINDTESTDATA("data/testpackagesdepinvalid"));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 4);
}

void QueryTest::queryCustomPlugin()
{
    // verify that the test plugin if found
    QVERIFY(KPackage::PackageLoader::self()->loadPackageStructure("Plasma/TestKPackageInternalPlasmoid"));

    // verify that no packages are installed
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 0);

    auto testPackageStructure = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/TestKPackageInternalPlasmoid"));
    // install some packages
    testPackageStructure.install(QFINDTESTDATA("data/testpackage"));
    testPackageStructure.install(QFINDTESTDATA("data/testfallbackpackage"));
    testPackageStructure.install(QFINDTESTDATA("data/testjsonmetadatapackage"));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 3);

    // installing package with invalid metadata should not be possible
    testPackageStructure.install(QFINDTESTDATA("data/testinvalidmetadata"));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 3);

    // package with valid dep information should be installed
    testPackageStructure.install(QFINDTESTDATA("data/testpackagesdep"));
#ifdef Q_OS_WIN
    QEXPECT_FAIL("", "This returns 3 on Windows, why?", Abort);
#endif
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 4);

    // package with invalid dep information should not be installed
    ps.install(QFINDTESTDATA("data/testpackagesdepinvalid"));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 4);
}

QTEST_MAIN(QueryTest)
