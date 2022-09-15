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

static bool checkedInstall(KPackage::Package &ps, const QString &source, int expectedError)
{
    auto job = ps.install(source);
    job->exec();
    if (job->error() == expectedError) {
        return true;
    }
    qWarning() << "Unexpected error" << job->error() << "while installing" << source << job->errorString();
    return false;
}

void QueryTest::installAndQuery()
{
    // verify that no packages are installed
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 0);

    // install some packages
    QVERIFY(checkedInstall(ps, QFINDTESTDATA("data/testpackage"), KJob::NoError));
    QVERIFY(checkedInstall(ps, QFINDTESTDATA("data/testfallbackpackage"), KJob::NoError));
    QVERIFY(checkedInstall(ps, QFINDTESTDATA("data/testjsonmetadatapackage"), KJob::NoError));
    system("find '/Users/alex/.qttest/Library/Application Support'");
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 3);

    // installing package with invalid metadata should not be possible
    QVERIFY(checkedInstall(ps, QFINDTESTDATA("data/testinvalidmetadata"), KPackage::Package::MetadataFileMissingError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 3);

    // package with valid dep information should be installed
    QVERIFY(checkedInstall(ps, QFINDTESTDATA("data/testpackagesdep"), KJob::NoError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 4);

    // package with invalid dep information should not be installed
    QVERIFY(checkedInstall(ps, QFINDTESTDATA("data/testpackagesdepinvalid"), KPackage::Package::JobError::PackageCopyError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("KPackage/Generic")).count(), 4);
}

void QueryTest::queryCustomPlugin()
{
    // verify that the test plugin if found
    auto packageStructure = KPackage::PackageLoader::self()->loadPackageStructure("Plasma/TestKPackageInternalPlasmoid");
    QVERIFY(packageStructure);

    // verify that no packages are installed
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 0);

    auto testPackageStructure = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/TestKPackageInternalPlasmoid"));
    // install some packages
    QVERIFY(checkedInstall(testPackageStructure, QFINDTESTDATA("data/testpackage"), KJob::NoError));
    QVERIFY(checkedInstall(testPackageStructure, QFINDTESTDATA("data/testfallbackpackage"), KJob::NoError));
    QVERIFY(checkedInstall(testPackageStructure, QFINDTESTDATA("data/testjsonmetadatapackage"), KJob::NoError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 3);

    // installing package with invalid metadata should not be possible
    QVERIFY(checkedInstall(testPackageStructure, QFINDTESTDATA("data/testinvalidmetadata"), KPackage::Package::MetadataFileMissingError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 3);

    // package with valid dep information should be installed
    QVERIFY(checkedInstall(testPackageStructure, QFINDTESTDATA("data/testpackagesdep"), KJob::NoError));
    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 4);

    // package with invalid dep information should not be installed
    QVERIFY(checkedInstall(testPackageStructure, QFINDTESTDATA("data/testpackagesdepinvalid"), KPackage::Package::JobError::PackageCopyError));

    QCOMPARE(KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/TestKPackageInternalPlasmoid")).count(), 4);
}

QTEST_MAIN(QueryTest)
