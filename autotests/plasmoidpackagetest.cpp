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

#include "plasmoidpackagetest.h"
#include "../src/kpackage/config-package.h"
#include "../src/kpackage/private/versionparser.cpp"

#include <QDir>
#include <QFile>
#include <kzip.h>
#include <kjob.h>
#include <QStandardPaths>

#include <QDebug>

#include "packageloader.h"
#include "plasmoidstructure.h"


void PlasmoidPackageTest::initTestCase()
{
    QStandardPaths::enableTestMode(true);
}

void PlasmoidPackageTest::init()
{
    qDebug() << "PlasmoidPackage::init()";
    m_package = QStringLiteral("Package");
    m_packageRoot = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/packageRoot";
    m_defaultPackage = KPackage::Package(new KPackage::PlasmoidPackage);
    cleanup(); // to prevent previous runs from interfering with this one
}

void PlasmoidPackageTest::cleanup()
{
    qDebug() << "cleaning up";
    // Clean things up.
   QDir(m_packageRoot).removeRecursively();
}

void PlasmoidPackageTest::createTestPackage(const QString &packageName, const QString &version)
{
    qDebug() << "Create test package" << m_packageRoot;
    QDir pRoot(m_packageRoot);
    // Create the root and package dir.
    if (!pRoot.exists()) {
        QVERIFY(QDir().mkpath(m_packageRoot));
    }

    // Create the package dir
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + packageName));
    qDebug() << "Created" << (m_packageRoot + "/" + packageName);

    // Create the metadata.desktop file
    QFile file(m_packageRoot + "/" + packageName + "/metadata.desktop");

    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << "[Desktop Entry]\n";
    out << "Name=" << packageName << "\n";
    out << "X-KDE-PluginInfo-Name=" << packageName << "\n";
    out << "X-KDE-PluginInfo-Version=" << version << "\n";
    file.flush();
    file.close();

    qDebug() << "OUT: " << packageName;

    // Create the ui dir.
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + packageName + "/contents/ui"));

    // Create the main file.
    file.setFileName(m_packageRoot + "/" + packageName + "/contents/ui/main.qml");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out << "THIS IS A PLASMOID SCRIPT.....";
    file.flush();
    file.close();

    qDebug() << "THIS IS A PLASMOID SCRIPT THING";
    // Now we have a minimal plasmoid package which is valid. Let's add some
    // files to it for test purposes.

    // Create the images dir.
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + packageName + "/contents/images"));
    file.setFileName(m_packageRoot + "/" + packageName + "/contents/images/image-1.svg");

    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out << "<svg>This is a test image</svg>";
    file.flush();
    file.close();

    file.setFileName(m_packageRoot + "/" + packageName + "/contents/images/image-2.svg");

    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out.setDevice(&file);
    out << "<svg>This is another test image</svg>";
    file.flush();
    file.close();

    // Create the scripts dir.
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + packageName + "/contents/code"));

    // Create 2 js files
    file.setFileName(m_packageRoot + "/" + packageName + "/contents/code/script.js");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out << "THIS IS A SCRIPT.....";
    file.flush();
    file.close();
}

void PlasmoidPackageTest::isValid()
{
    KPackage::Package *p = new KPackage::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
#ifndef NDEBUG
    qDebug() << "package path is" << p->path();
#endif

    // A PlasmoidPackage is valid when:
    // - The package root exists.
    // - The package root consists an file named "ui/main.qml"
    QVERIFY(!p->isValid());

    // Create the root and package dir.
    QVERIFY(QDir().mkpath(m_packageRoot));
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + m_package));

    // Should still be invalid.
    delete p;
    p = new KPackage::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(!p->isValid());

    // Create the metadata.desktop file.
    QFile file(m_packageRoot + "/" + m_package + "/metadata.desktop");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << "[Desktop Entry]\n";
    out << "Name=test\n";
    out << "Description=Just a test desktop file";
    file.flush();
    file.close();

    // Create the ui dir.
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + m_package + "/contents/ui"));

    // No main file yet so should still be invalid.
    delete p;
    p = new KPackage::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(!p->isValid());

    // Create the main file.
    file.setFileName(m_packageRoot + "/" + m_package + "/contents/ui/main.qml");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out.setDevice(&file);
    out << "THIS IS A PLASMOID SCRIPT.....\n";
    file.flush();
    file.close();

    file.setPermissions(QFile::ReadUser | QFile::WriteUser);
    // Main file exists so should be valid now.
    delete p;
    p = new KPackage::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(p->isValid());
    QCOMPARE(p->contentsHash(), QStringLiteral("a41160c6a763ea505c95bee12a7fc87952a61cf1"));
    delete p;
}

void PlasmoidPackageTest::filePath()
{
    return;
    // Package::filePath() returns
    // - {package_root}/{package_name}/path/to/file if the file exists
    // - QString() otherwise.
    KPackage::Package *p = new KPackage::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);

    QCOMPARE(p->filePath("scripts", QStringLiteral("main")), QString());

    QVERIFY(QDir().mkpath(m_packageRoot + "/" + m_package + "/contents/ui/main.qml"));
    QFile file(m_packageRoot + "/" + m_package + "/contents/ui/main.qml");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << "THIS IS A PLASMOID SCRIPT.....";
    file.flush();
    file.close();

    // The package is valid by now so a path for code/main should get returned.
    delete p;
    p = new KPackage::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);

    const QString path = QFileInfo(m_packageRoot + "/" + m_package + "/contents/ui/main.qml").canonicalFilePath();

    // Two ways to get the same info.
    // 1. Give the file type which refers to a class of files (a directory) in
    //    the package structure and the file name.
    // 2. Give the file type which refers to a file in the package structure.
    //
    // NOTE: scripts, main and mainscript are defined in packages.cpp and are
    //       specific for a PlasmoidPackage.
    QCOMPARE(p->filePath("scripts", QStringLiteral("main")), path);
    QCOMPARE(p->filePath("mainscript"), path);
    delete p;
}

void PlasmoidPackageTest::entryList()
{
    // Create a package named @p packageName which is valid and has some images.
    createTestPackage(m_package, QStringLiteral("1.1"));

    // Create a package object and verify that it is valid.
    KPackage::Package *p = new KPackage::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(p->isValid());

    // Now we have a valid package that should contain the following files in
    // given filetypes:
    // fileTye - Files
    // scripts - {"script.js"}
    // images - {"image-1.svg", "image-2.svg"}
    QStringList files = p->entryList("scripts");
    QCOMPARE(files.size(), 1);
    QVERIFY(files.contains(QStringLiteral("script.js")));

    files = p->entryList("images");
    QCOMPARE(files.size(), 2);
    QVERIFY(files.contains(QStringLiteral("image-1.svg")));
    QVERIFY(files.contains(QStringLiteral("image-2.svg")));
    delete p;
}

void PlasmoidPackageTest::createAndInstallPackage()
{
    qDebug() << "                   ";
    qDebug() << "   CreateAndInstall ";
    createTestPackage(QStringLiteral("plasmoid_to_package"), QStringLiteral("1.1"));
    const QString packagePath = m_packageRoot + '/' + "testpackage.plasmoid";

    KZip creator(packagePath);
    QVERIFY(creator.open(QIODevice::WriteOnly));
    creator.addLocalDirectory(m_packageRoot + '/' + "plasmoid_to_package", QStringLiteral("."));
    creator.close();
    QDir rootDir(m_packageRoot + "/plasmoid_to_package");
    rootDir.removeRecursively();

    QVERIFY(QFile::exists(packagePath));

    KZip package(packagePath);
    QVERIFY(package.open(QIODevice::ReadOnly));
    const KArchiveDirectory *dir = package.directory();
    QVERIFY(dir);//
    QVERIFY(dir->entry(QStringLiteral("metadata.desktop")));
    const KArchiveEntry *contentsEntry = dir->entry(QStringLiteral("contents"));
    QVERIFY(contentsEntry);
    QVERIFY(contentsEntry->isDirectory());
    const KArchiveDirectory *contents = static_cast<const KArchiveDirectory *>(contentsEntry);
    QVERIFY(contents->entry(QStringLiteral("ui")));
    QVERIFY(contents->entry(QStringLiteral("images")));

    m_defaultPackageStructure = new KPackage::PackageStructure(this);
    KPackage::Package *p = new KPackage::Package(m_defaultPackageStructure);
    qDebug() << "Installing " << packagePath;
    //const QString packageRoot = "plasma/plasmoids/";
    //const QString servicePrefix = "plasma-applet-";
    KJob *job = p->install(packagePath, m_packageRoot);
    connect(job, SIGNAL(finished(KJob*)), SLOT(packageInstalled(KJob*)));
    QSignalSpy spy(job, SIGNAL(finished(KJob*)));
    QVERIFY(spy.wait(1000));
    cleanupPackage(QStringLiteral("plasmoid_to_package"));

    //QVERIFY(p->isValid());
    delete p;
}

void PlasmoidPackageTest::createAndUpdatePackage()
{
    //does the version number parsing work?
    QVERIFY(KPackage::isVersionNewer(QStringLiteral("1.1"), QStringLiteral("1.1.1")));
    QVERIFY(!KPackage::isVersionNewer(QStringLiteral("1.1.1"), QStringLiteral("1.1")));
    QVERIFY(KPackage::isVersionNewer(QStringLiteral("1.1.1"), QStringLiteral("1.1.2")));
    QVERIFY(KPackage::isVersionNewer(QStringLiteral("1.1.2"), QStringLiteral("2.1")));
    QVERIFY(KPackage::isVersionNewer(QStringLiteral("0.1.2"), QStringLiteral("2")));
    QVERIFY(!KPackage::isVersionNewer(QStringLiteral("1"), QStringLiteral("0.1.2")));

    qDebug() << "                   ";
    qDebug() << "   CreateAndUpdate ";
    createTestPackage(QStringLiteral("plasmoid_to_package"), QStringLiteral("1.1"));
    const QString packagePath = m_packageRoot + '/' + "testpackage.plasmoid";

    KZip creator(packagePath);
    QVERIFY(creator.open(QIODevice::WriteOnly));
    creator.addLocalDirectory(m_packageRoot + '/' + "plasmoid_to_package", QStringLiteral("."));
    creator.close();
    QDir rootDir(m_packageRoot + "/plasmoid_to_package");
    rootDir.removeRecursively();

    QVERIFY(QFile::exists(packagePath));

    KZip package(packagePath);
    QVERIFY(package.open(QIODevice::ReadOnly));
    const KArchiveDirectory *dir = package.directory();
    QVERIFY(dir);//
    QVERIFY(dir->entry(QStringLiteral("metadata.desktop")));
    const KArchiveEntry *contentsEntry = dir->entry(QStringLiteral("contents"));
    QVERIFY(contentsEntry);
    QVERIFY(contentsEntry->isDirectory());
    const KArchiveDirectory *contents = static_cast<const KArchiveDirectory *>(contentsEntry);
    QVERIFY(contents->entry(QStringLiteral("ui")));
    QVERIFY(contents->entry(QStringLiteral("images")));

    m_defaultPackageStructure = new KPackage::PackageStructure(this);
    KPackage::Package *p = new KPackage::Package(m_defaultPackageStructure);
    qDebug() << "Installing " << packagePath;
    //const QString packageRoot = "plasma/plasmoids/";
    //const QString servicePrefix = "plasma-applet-";

    KJob *job = p->update(packagePath, m_packageRoot);
    connect(job, SIGNAL(finished(KJob*)), SLOT(packageInstalled(KJob*)));
    QSignalSpy spy(job, SIGNAL(finished(KJob*)));
    QVERIFY(spy.wait(1000));

    //same version, should fail
    job = p->update(packagePath, m_packageRoot);
    QSignalSpy spyFail(job, SIGNAL(finished(KJob*)));
    QVERIFY(spyFail.wait(1000));
    QVERIFY(job->error() == KPackage::Package::JobError::NewerVersionAlreadyInstalledError);
    qDebug()<<job->errorText();


    //create a new package with higher version
    createTestPackage(QStringLiteral("plasmoid_to_package"), QStringLiteral("1.2"));

    KZip creator2(packagePath);
    QVERIFY(creator2.open(QIODevice::WriteOnly));
    creator2.addLocalDirectory(m_packageRoot + '/' + "plasmoid_to_package", QStringLiteral("."));
    creator2.close();
    QDir rootDir2(m_packageRoot + "/plasmoid_to_package");
    rootDir2.removeRecursively();


    KJob *job2 = p->update(packagePath, m_packageRoot);
    connect(job2, SIGNAL(finished(KJob*)), SLOT(packageInstalled(KJob*)));
    QSignalSpy spy2(job2, SIGNAL(finished(KJob*)));
    QVERIFY(spy2.wait(1000));

    cleanupPackage(QStringLiteral("plasmoid_to_package"));

    //QVERIFY(p->isValid());
    delete p;
}

void PlasmoidPackageTest::cleanupPackage(const QString &packageName)
{
    KPackage::Package *p = new KPackage::Package(m_defaultPackageStructure);
    KJob *jj = p->uninstall(packageName, m_packageRoot);
    connect(jj, SIGNAL(finished(KJob*)), SLOT(packageInstalled(KJob*)));
}

void PlasmoidPackageTest::packageInstalled(KJob *j)
{
    qDebug() << "!!!!!!!!!!!!!!!!!!!! package installed" << (j->error() == KJob::NoError);
    QVERIFY(j->error() == KJob::NoError);
}

void PlasmoidPackageTest::packageUninstalled(KJob *j)
{
    qDebug() << "!!!!!!!!!!!!!!!!!!!!! package uninstalled";
    QVERIFY(j->error() == KJob::NoError);
}

QTEST_MAIN(PlasmoidPackageTest)
