/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packagestructuretest.h"

#include <KLocalizedString>
#include <QDebug>

#include "packageloader.h"
#include "packagestructure.h"
#include "private/utils.h"

class NoPrefixes : public KPackage::Package
{
public:
    explicit NoPrefixes()
        : KPackage::Package(new KPackage::PackageStructure)
    {
        setContentsPrefixPaths(QStringList());
        addDirectoryDefinition("bin", QStringLiteral("bin"), QStringLiteral("bin"));
        addFileDefinition("MultiplePaths", QStringLiteral("first"), QStringLiteral("Description proper"));
        addFileDefinition("MultiplePaths", QStringLiteral("second"), QStringLiteral("Description proper"));
        setPath(QStringLiteral("/"));
    }
};

class Wallpaper : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    void initPackage(KPackage::Package *package) override
    {
        package->addDirectoryDefinition("images", QStringLiteral("images/"), i18n("Images"));

        QStringList mimetypes;
        mimetypes << QStringLiteral("image/svg") << QStringLiteral("image/png") << QStringLiteral("image/jpeg") << QStringLiteral("image/jpg");
        package->setMimeTypes("images", mimetypes);

        package->setRequired("images", true);
        package->addFileDefinition("screenshot", QStringLiteral("screenshot.png"), i18n("Screenshot"));
        package->setAllowExternalPaths(true);
    }
    void pathChanged(KPackage::Package *package) override
    {
        static bool guard = false;

        if (guard) {
            return;
        }

        guard = true;
        QString ppath = package->path();
        if (ppath.endsWith('/')) {
            ppath.chop(1);
            if (!QFile::exists(ppath)) {
                ppath = package->path();
            }
        }

        QFileInfo info(ppath);
        const bool isFullPackage = info.isDir();
        package->removeDefinition("preferred");
        package->setRequired("images", isFullPackage);

        if (isFullPackage) {
            package->setContentsPrefixPaths(QStringList() << QStringLiteral("contents/"));
        } else {
            package->addFileDefinition("screenshot", info.fileName(), i18n("Preview"));
            package->addFileDefinition("preferred", info.fileName(), QString());
            package->setContentsPrefixPaths(QStringList());
            package->setPath(info.path());
        }

        guard = false;
    }
};

class SimpleContent : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    void initPackage(KPackage::Package *package) override
    {
        package->addDirectoryDefinition("ui", QStringLiteral("ui/"), i18n("User interface"));
    }
    void pathChanged(KPackage::Package *package) override
    {
        if (!package->metadata().isValid()) {
            return;
        }
        if (readKPackageTypes(package->metadata()).contains(QStringLiteral("CustomContent"))) {
            package->addFileDefinition("customcontentfile",
                                       QStringLiteral("customcontent/CustomContentFile.qml"),
                                       i18n("Custom file only for packages that contain CustomContent in their servicetypes"));
        } else {
            package->removeDefinition("customcontentfile");
        }
    }
};

void PackageStructureTest::initTestCase()
{
    m_packagePath = QFINDTESTDATA("data/testpackage");
    ps = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KPackage/Generic"));
    ps.addFileDefinition("mainscript", QStringLiteral("ui/main.qml"), i18n("Main Script File"));
    ps.setPath(m_packagePath);
}

void PackageStructureTest::validStructures()
{
    QVERIFY(ps.hasValidStructure());
    QVERIFY(!KPackage::Package().hasValidStructure());
    QVERIFY(!KPackage::PackageLoader::self()->loadPackage(QStringLiteral("doesNotExist")).hasValidStructure());
}

void PackageStructureTest::validPackages()
{
    QVERIFY(ps.isValid());
    QVERIFY(!KPackage::Package().isValid());
    QVERIFY(!KPackage::PackageLoader::self()->loadPackage(QStringLiteral("doesNotExist")).isValid());
    QVERIFY(NoPrefixes().isValid());

    KPackage::Package p = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KPackage/Generic"));
    p.addFileDefinition("mainscript", QStringLiteral("ui/main.qml"), i18n("Main Script File"));
    QVERIFY(!p.isValid());
    p.setPath(QStringLiteral("/does/not/exist"));
    QVERIFY(!p.isValid());
    p.setPath(ps.path());
    QVERIFY(p.isValid());
}

void PackageStructureTest::wallpaperPackage()
{
    KPackage::Package p(new Wallpaper);
    p.setPath(m_packagePath);
    QVERIFY(p.isValid());

    KPackage::Package p2(new Wallpaper);
    p2.setPath(m_packagePath + "/contents/images/empty.png");
    QVERIFY(p2.isValid());
}

void PackageStructureTest::copyPerformance()
{
    // seed the cache first
    ps.filePath("mainscript");

    QBENCHMARK {
        KPackage::Package foo(ps);
        const QString bar = foo.filePath("mainscript");
        Q_UNUSED(bar);
    }
}

void PackageStructureTest::mutateAfterCopy()
{
    const bool mainscriptRequired = ps.isRequired("mainscript");
    const QStringList imageMimeTypes = ps.mimeTypes("images");
    const QStringList defaultMimeTypes = ps.mimeTypes("translations");
    const QString packageRoot = ps.defaultPackageRoot();
    const bool externalPaths = ps.allowExternalPaths();
    const QStringList contentsPrefixPaths = ps.contentsPrefixPaths();
    const QList<QByteArray> files = ps.files();
    const QList<QByteArray> dirs = ps.directories();

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
    QStringList copyContentsPrefixPaths = contentsPrefixPaths;
    copyContentsPrefixPaths << QStringLiteral("more/");
    copy.setContentsPrefixPaths(copyContentsPrefixPaths);
    QCOMPARE(ps.contentsPrefixPaths(), contentsPrefixPaths);
    QCOMPARE(copy.contentsPrefixPaths(), copyContentsPrefixPaths);

    copy = ps;
    copy.addFileDefinition("nonsense", QStringLiteral("foobar"), QString());
    QCOMPARE(ps.files(), files);
    QVERIFY(ps.files() != copy.files());

    copy = ps;
    copy.addDirectoryDefinition("nonsense", QStringLiteral("foobar"), QString());
    QCOMPARE(ps.directories(), dirs);
    QVERIFY(ps.directories() != copy.directories());

    copy = ps;
    copy.removeDefinition("mainscript");
    QCOMPARE(ps.files(), files);
    QVERIFY(ps.files() != copy.files());

    copy = ps;
    QVERIFY(!imageMimeTypes.isEmpty());
    QStringList copyMimeTypes;
    copyMimeTypes << imageMimeTypes.first();
    copy.setMimeTypes("images", copyMimeTypes);
    QCOMPARE(ps.mimeTypes("images"), imageMimeTypes);
    QCOMPARE(copy.mimeTypes("images"), copyMimeTypes);

    copy = ps;
    QStringList copyDefaultMimeTypes = defaultMimeTypes;
    copyDefaultMimeTypes << QStringLiteral("rubbish");
    copy.setDefaultMimeTypes(copyDefaultMimeTypes);
    QCOMPARE(ps.mimeTypes("translations"), defaultMimeTypes);
    QCOMPARE(copy.mimeTypes("translations"), copyDefaultMimeTypes);
}

void PackageStructureTest::emptyContentsPrefix()
{
    NoPrefixes package;
    QString path(package.filePath("bin", QStringLiteral("ls")));
    // qDebug() << path;
    if (QFileInfo::exists(QStringLiteral("/bin/ls"))) { // not Windows
        QCOMPARE(path, QStringLiteral("/bin/ls"));
    }
}

void PackageStructureTest::multiplePaths()
{
    NoPrefixes package;
    QCOMPARE(package.name("MultiplePaths"), QStringLiteral("Description proper"));
}

void PackageStructureTest::directories()
{
    QList<QByteArray> dirs;
    dirs << "config"
         << "data"
         << "images"
         << "theme"
         << "scripts"
         << "translations"
         << "ui";

    const QList<QByteArray> psDirs = ps.directories();

    QCOMPARE(dirs.count(), psDirs.count());

    for (const char *dir : psDirs) {
        bool found = false;
        for (const char *check : std::as_const(dirs)) {
            if (qstrcmp(dir, check)) {
                found = true;
                break;
            }
        }
        QVERIFY(found);
    }

    for (const char *dir : std::as_const(dirs)) {
        bool found = false;
        for (const char *check : psDirs) {
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
    QList<QByteArray> dirs;
    QCOMPARE(ps.requiredDirectories(), dirs);
}

void PackageStructureTest::files()
{
    QList<QByteArray> files;
    files << "mainconfigui"
          << "mainconfigxml"
          << "mainscript";

    const QList<QByteArray> psFiles = ps.files();

    // for (int i = 0; i < psFiles.count(); ++i) {
    //    qDebug() << psFiles[i];
    //}
    for (const char *file : psFiles) {
        bool found = false;
        for (const char *check : std::as_const(files)) {
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
    QList<QByteArray> files;
    files << "mainscript";

    QList<QByteArray> psFiles = ps.requiredFiles();

    QCOMPARE(files.count(), psFiles.count());
    for (int i = 0; i < files.count(); ++i) {
        QCOMPARE(files[i], psFiles[i]);
    }
}

void PackageStructureTest::path()
{
    QCOMPARE(ps.filePath("images"), QDir(m_packagePath + QLatin1String("/contents/images")).canonicalPath());
    QCOMPARE(ps.filePath("theme"), QString());
    QCOMPARE(ps.filePath("mainscript"), QFileInfo(m_packagePath + QLatin1String("/contents/ui/main.qml")).canonicalFilePath());
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
    mimeTypes << QStringLiteral("image/svg+xml") << QStringLiteral("image/png") << QStringLiteral("image/jpeg");
    QCOMPARE(ps.mimeTypes("images"), mimeTypes);
    QCOMPARE(ps.mimeTypes("theme"), mimeTypes);
}

void PackageStructureTest::customContent()
{
    KPackage::Package p(new SimpleContent);
    p.setPath(QFINDTESTDATA("data/simplecontent"));
    QVERIFY(p.isValid());
    QCOMPARE(p.filePath("customcontentfile"), QString());

    p.setPath(QFINDTESTDATA("data/customcontent"));
    const QString expected = QFINDTESTDATA("data/customcontent") + "/contents/customcontent/CustomContentFile.qml";
    QCOMPARE(p.filePath("customcontentfile"), expected);
    QVERIFY(p.isValid());

    p.setPath(QFINDTESTDATA("data/simplecontent"));
    QVERIFY(p.isValid());
    QCOMPARE(p.filePath("customcontentfile"), QString());
}

QTEST_MAIN(PackageStructureTest)

#include "packagestructuretest.moc"
