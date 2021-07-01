/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "rccpackagetest.h"

#include <QDebug>
#include <QStandardPaths>

#include "packageloader.h"
#include "packagestructure.h"
#include "packagestructures/plasmoidstructure.h"

void RccPackageTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    QString destination = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/plasma/plasmoids/testpackage-rcc/");
    QDir dir;
    dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/plasma/"));
    dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/plasma/plasmoids/"));
    dir.mkpath(destination);
    QVERIFY(dir.exists(destination));

    // verify the source files exist
    QVERIFY(QFile::exists(QStringLiteral("../autotests/testpackage-rcc/metadata.json")));
    QVERIFY(QFile::exists(QStringLiteral("../autotests/testpackage-rcc/contents.rcc")));

    QFile::copy(QStringLiteral("../autotests/testpackage-rcc/metadata.json"), destination + QStringLiteral("metadata.json"));
    QFile::copy(QStringLiteral("../autotests/testpackage-rcc/contents.rcc"), destination + QStringLiteral("contents.rcc"));

    // verify they have been copied correctly
    QVERIFY(QFile::exists(destination + QStringLiteral("metadata.json")));
    QVERIFY(QFile::exists(destination + QStringLiteral("contents.rcc")));

    m_packagePath = "testpackage-rcc";
    m_pkg = new KPackage::Package(new KPackage::PlasmoidPackage);
    m_pkg->setPath(m_packagePath);
    // Two package instances with the same resource
    m_pkg2 = new KPackage::Package(new KPackage::PlasmoidPackage);
    m_pkg2->setPath(m_packagePath);
}

void RccPackageTest::cleanupTestCase()
{
    qDebug() << "cleaning up";
    // Clean things up.
    QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)).removeRecursively();
}

void RccPackageTest::accessFiles()
{
    QVERIFY(m_pkg->hasValidStructure());
    QVERIFY(m_pkg->hasValidStructure());

    // check for existence of all files
    QVERIFY(!m_pkg->filePath("ui", QStringLiteral("main.qml")).isEmpty());
    QVERIFY(!m_pkg->filePath("ui", QStringLiteral("otherfile.qml")).isEmpty());
    QVERIFY(!m_pkg->filePath("images", QStringLiteral("empty.png")).isEmpty());

    QVERIFY(!m_pkg2->filePath("ui", QStringLiteral("main.qml")).isEmpty());
    QVERIFY(!m_pkg2->filePath("ui", QStringLiteral("otherfile.qml")).isEmpty());
    QVERIFY(!m_pkg2->filePath("images", QStringLiteral("empty.png")).isEmpty());
}

void RccPackageTest::validate()
{
#ifdef Q_OS_WIN
    QEXPECT_FAIL("", "Windows gets a different sha1 here, why?", Abort);
#endif
    QCOMPARE(m_pkg->cryptographicHash(QCryptographicHash::Sha1), QByteArrayLiteral("ed0959c062b7ef7eaab5243e83e2f9afe5b3b15f"));
    QCOMPARE(m_pkg2->cryptographicHash(QCryptographicHash::Sha1), QByteArrayLiteral("ed0959c062b7ef7eaab5243e83e2f9afe5b3b15f"));
}

void RccPackageTest::testResourceRefCount()
{
    delete m_pkg;
    m_pkg = nullptr;

    QVERIFY(m_pkg2->isValid());
    QVERIFY(QFile::exists(QStringLiteral(":/plasma/plasmoids/testpackage-rcc/contents/ui/main.qml")));

    // no reference from this package anymore
    delete m_pkg2;
    m_pkg2 = nullptr;
    QVERIFY(!QFile::exists(QStringLiteral(":/plasma/plasmoids/testpackage-rcc/contents/ui/main.qml")));
}

QTEST_MAIN(RccPackageTest)
