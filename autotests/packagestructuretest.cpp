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

#include "packagestructuretest.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <QDebug>
#include <klocalizedstring.h>

#include "packagestructure.h"
#include "packageloader.h"

class NoPrefixes : public KPackage::Package
{
public:
    explicit NoPrefixes()
        : KPackage::Package(new KPackage::PackageStructure)
    {
        setContentsPrefixPaths(QStringList());
        addDirectoryDefinition("bin", "bin", "bin");
        addFileDefinition("MultiplePaths", "first", "Description proper");
        addFileDefinition("MultiplePaths", "second", "Description proper");
        setPath("/");
    }
};

void PackageStructureTest::initTestCase()
{
    m_packagePath = QFINDTESTDATA("data/testpackage");
    ps = KPackage::PackageLoader::self()->loadPackage("KPackage/Generic");
    ps.addFileDefinition("mainscript", "ui/main.qml", i18n("Main Script File"));
    ps.setPath(m_packagePath);
}

void PackageStructureTest::validStructures()
{
    QVERIFY(ps.hasValidStructure());
    QVERIFY(!KPackage::Package().hasValidStructure());
    QVERIFY(!KPackage::PackageLoader::self()->loadPackage("doesNotExist").hasValidStructure());
}

void PackageStructureTest::validPackages()
{
    QVERIFY(ps.isValid());
    QVERIFY(!KPackage::Package().isValid());
    QVERIFY(!KPackage::PackageLoader::self()->loadPackage("doesNotExist").isValid());
    QVERIFY(NoPrefixes().isValid());

    KPackage::Package p = KPackage::PackageLoader::self()->loadPackage("KPackage/Generic");
    p.addFileDefinition("mainscript", "ui/main.qml", i18n("Main Script File"));
    QVERIFY(!p.isValid());
    p.setPath("/does/not/exist");
    QVERIFY(!p.isValid());
    p.setPath(ps.path());
    QVERIFY(p.isValid());
}

void PackageStructureTest::copyPerformance()
{
    // seed the cache first
    ps.filePath("mainscript");

    QTime t;
    t.start();

    for (int i = 0; i < 100000; ++i) {
        KPackage::Package foo(ps);
        const QString bar = foo.filePath("mainscript");
    }

    QVERIFY(t.elapsed() < 400);
}

void PackageStructureTest::mutateAfterCopy()
{
    const bool mainscriptRequired = ps.isRequired("mainscript");
    const QStringList imageMimeTypes = ps.mimeTypes("images");
    const QStringList defaultMimeTypes = ps.mimeTypes("translations");
    const QString packageRoot = ps.defaultPackageRoot();
    const bool externalPaths = ps.allowExternalPaths();
    const QString servicePrefix = ps.servicePrefix();
    const QStringList contentsPrefixPaths = ps.contentsPrefixPaths();
    const QList<const char *> files = ps.files();
    const QList<const char *> dirs = ps.directories();

    KPackage::Package copy(ps);

    copy.setRequired("mainscript", !mainscriptRequired);
    QCOMPARE(ps.isRequired("mainscript"), mainscriptRequired);
    QCOMPARE(copy.isRequired("mainscript"), !mainscriptRequired);

    copy = ps;
    const QString copyPackageRoot = packageRoot + "more/";
    copy.setDefaultPackageRoot(copyPackageRoot);
    QCOMPARE(ps.defaultPackageRoot(), packageRoot);
    QCOMPARE(copy.defaultPackageRoot(), copyPackageRoot);

    copy = ps;
    copy.setAllowExternalPaths(!externalPaths);
    QCOMPARE(ps.allowExternalPaths(), externalPaths);
    QCOMPARE(copy.allowExternalPaths(), !externalPaths);

    copy = ps;
    const QString copyServicePrefix = packageRoot + "more/";
    copy.setServicePrefix(copyServicePrefix);
    QCOMPARE(ps.servicePrefix(), servicePrefix);
    QCOMPARE(copy.servicePrefix(), copyServicePrefix);

    copy = ps;
    QStringList copyContentsPrefixPaths = contentsPrefixPaths;
    copyContentsPrefixPaths << "more/";
    copy.setContentsPrefixPaths(copyContentsPrefixPaths);
    QCOMPARE(ps.contentsPrefixPaths(), contentsPrefixPaths);
    QCOMPARE(copy.contentsPrefixPaths(), copyContentsPrefixPaths);

    copy = ps;
    copy.addFileDefinition("nonsense", "foobar", QString());
    QCOMPARE(ps.files(), files);
    QVERIFY(ps.files() != copy.files());

    copy = ps;
    copy.addDirectoryDefinition("nonsense", "foobar", QString());
    QCOMPARE(ps.directories(), dirs);
    QVERIFY(ps.directories() != copy.directories());

    copy = ps;
    copy.removeDefinition("mainscript");
    QCOMPARE(ps.files(), files);
    QVERIFY(ps.files() != copy.files());

#ifndef PLASMA_NO_PACKAGE_EXTRADATA
    copy = ps;
    QVERIFY(!imageMimeTypes.isEmpty());
    QStringList copyMimeTypes;
    copyMimeTypes << imageMimeTypes.first();
    copy.setMimeTypes("images", copyMimeTypes);
    QCOMPARE(ps.mimeTypes("images"), imageMimeTypes);
    QCOMPARE(copy.mimeTypes("images"), copyMimeTypes);


    copy = ps;
    QStringList copyDefaultMimeTypes = defaultMimeTypes;
    copyDefaultMimeTypes << "rubbish";
    copy.setDefaultMimeTypes(copyDefaultMimeTypes);
    QCOMPARE(ps.mimeTypes("translations"), defaultMimeTypes);
    QCOMPARE(copy.mimeTypes("translations"), copyDefaultMimeTypes);
#endif
}

void PackageStructureTest::emptyContentsPrefix()
{
    NoPrefixes package;
    QString path(package.filePath("bin", "ls"));
    //qDebug() << path;
    QCOMPARE(path, QString("/bin/ls"));
}

void PackageStructureTest::multiplePaths()
{
    NoPrefixes package;
    QCOMPARE(package.name("MultiplePaths"), QString("Description proper"));
}

void PackageStructureTest::directories()
{
    QList<const char *> dirs;
    dirs << "config" << "data" << "images" << "theme" << "scripts" << "translations" << "ui";

    QList<const char *> psDirs = ps.directories();

    QCOMPARE(dirs.count(), psDirs.count());

    foreach (const char *dir, psDirs) {
        bool found = false;
        foreach (const char *check, dirs) {
            if (qstrcmp(dir, check)) {
                found = true;
                break;
            }
        }
        QVERIFY(found);
    }

    foreach (const char *dir, dirs) {
        bool found = false;
        foreach (const char *check, psDirs) {
            if (qstrcmp(dir, check)) {
                found = true;
                break;
            }
        }
        QVERIFY(found);
    }
}

void PackageStructureTest::requiredDirectories()
{
    QList<const char *> dirs;
    QCOMPARE(ps.requiredDirectories(), dirs);
}

void PackageStructureTest::files()
{
    QList<const char *> files;
    files << "mainconfigui" << "mainconfigxml" << "mainscript";

    QList<const char *> psFiles = ps.files();

    //for (int i = 0; i < psFiles.count(); ++i) {
    //    qDebug() << psFiles[i];
    //}
    foreach (const char *file, psFiles) {
        bool found = false;
        foreach (const char *check, files) {
            if (qstrcmp(file, check)) {
                found = true;
                break;
            }
        }
        QVERIFY(found);
    }
}

void PackageStructureTest::requiredFiles()
{
    QList<const char *> files;
    files << "mainscript";

    QList<const char *> psFiles = ps.requiredFiles();

    QCOMPARE(files.count(), psFiles.count());
    for (int i = 0; i < files.count(); ++i) {
        QCOMPARE(files[i], psFiles[i]);
    }
}

void PackageStructureTest::path()
{
    QCOMPARE(ps.filePath("images"), QDir(m_packagePath + QString("/contents/images")).canonicalPath());
    QCOMPARE(ps.filePath("theme"), QDir(m_packagePath + QString("/contents/theme")).canonicalPath());
    QCOMPARE(ps.filePath("mainscript"), QFileInfo(m_packagePath + QString("/contents/ui/main.qml")).canonicalFilePath());
}

void PackageStructureTest::name()
{
    QCOMPARE(ps.name("config"), i18n("Configuration Definitions"));
    QCOMPARE(ps.name("mainscript"), i18n("Main Script File"));
}

void PackageStructureTest::required()
{
    QVERIFY(ps.isRequired("mainscript"));
}

void PackageStructureTest::mimeTypes()
{
    QStringList mimeTypes;
    mimeTypes << "image/svg+xml" << "image/png" << "image/jpeg";
    QCOMPARE(ps.mimeTypes("images"), mimeTypes);
    QCOMPARE(ps.mimeTypes("theme"), mimeTypes);
}

QTEST_MAIN(PackageStructureTest)

