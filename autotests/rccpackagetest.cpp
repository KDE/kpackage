/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                            *
*   Copyright 2014 Marco Martin <mart@kde.org>                                *
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

#include "rccpackagetest.h"


#include <QDebug>

#include <klocalizedstring.h>
#include "packagestructure.h"
#include "plasmoidstructure.h"
#include "packageloader.h"

void RccPackageTest::initTestCase()
{
    QStandardPaths::enableTestMode(true);

    QString destination = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/plasma/plasmoids/testpackage-rcc/");
    QDir dir;
    dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/plasma/"));
    dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/plasma/plasmoids/"));
    dir.mkpath(destination);
    QVERIFY(dir.exists(destination));

    //verify the source files exist
    QVERIFY(QFile::exists(QStringLiteral("./autotests/testpackage-rcc/metadata.json")));
    QVERIFY(QFile::exists(QStringLiteral("./autotests/testpackage-rcc/contents.rcc")));

    QFile::copy(QStringLiteral("./autotests/testpackage-rcc/metadata.json"), destination + QStringLiteral("metadata.json"));
    QFile::copy(QStringLiteral("./autotests/testpackage-rcc/contents.rcc"), destination + QStringLiteral("contents.rcc"));

    //verify they have been copied correctly
    QVERIFY(QFile::exists(destination + QStringLiteral("metadata.json")));
    QVERIFY(QFile::exists(destination + QStringLiteral("contents.rcc")));

    m_packagePath = "testpackage-rcc";
    m_pkg = new KPackage::Package(new KPackage::PlasmoidPackage);
    m_pkg->setPath(m_packagePath);
    //Two package instances with the same resource
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

    //check for existence of all files
    QVERIFY(!m_pkg->filePath("ui", QStringLiteral("main.qml")).isEmpty());
    QVERIFY(!m_pkg->filePath("ui", QStringLiteral("otherfile.qml")).isEmpty());
    QVERIFY(!m_pkg->filePath("images", QStringLiteral("empty.png")).isEmpty());

    QVERIFY(!m_pkg2->filePath("ui", QStringLiteral("main.qml")).isEmpty());
    QVERIFY(!m_pkg2->filePath("ui", QStringLiteral("otherfile.qml")).isEmpty());
    QVERIFY(!m_pkg2->filePath("images", QStringLiteral("empty.png")).isEmpty());
}

void RccPackageTest::validate()
{
    QCOMPARE(m_pkg->cryptographicHash(QCryptographicHash::Sha1), QByteArrayLiteral("ed0959c062b7ef7eaab5243e83e2f9afe5b3b15f"));
    QCOMPARE(m_pkg2->cryptographicHash(QCryptographicHash::Sha1), QByteArrayLiteral("ed0959c062b7ef7eaab5243e83e2f9afe5b3b15f"));
}

void RccPackageTest::testResourceRefCount()
{
    delete m_pkg;
    m_pkg = nullptr;

    QVERIFY(m_pkg2->isValid());
    QVERIFY(QFile::exists(QStringLiteral(":/plasma/plasmoids/testpackage-rcc/contents/ui/main.qml")));

    //no reference from this package anymore
    delete m_pkg2;
    m_pkg2 = nullptr;
    QVERIFY(!QFile::exists(QStringLiteral(":/plasma/plasmoids/testpackage-rcc/contents/ui/main.qml")));
}

QTEST_MAIN(RccPackageTest)

