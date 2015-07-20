/******************************************************************************
*   Copyright 2008 Aaron Seigo <aseigo@kde.org>                               *
*   Copyright 2012-2013 Sebastian Kügler <sebas@kde.org>                      *
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

#include "kpackagetool.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <kshell.h>
#include <kconfig.h>
#include <klocalizedstring.h>
#include <kaboutdata.h>
#include <KPluginLoader>

#include <kpackage/packagestructure.h>
#include <kpackage/package.h>
#include <kpackage/packageloader.h>
#include <kjob.h>

#include <qcommandlineparser.h>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>

#include <iostream>
#include <iomanip>

#include "../kpackage/config-package.h"
//for the index creation function
#include "../kpackage/private/packagejobthread_p.h"

static QTextStream cout(stdout);

namespace KPackage
{
class PackageToolPrivate
{
public:
    QString packageRoot;
    QString packageFile;
    QString package;
    QStringList pluginTypes;
    KPackage::Package installer;
    KPluginMetaData metadata;
    QString installPath;
    void output(const QString &msg);
    QStringList packages(const QStringList &types, const QString &path = QString());
    void renderTypeTable(const QMap<QString, QStringList> &plugins);
    void listTypes();
    void coutput(const QString &msg);
    QCommandLineParser *parser;
};

PackageTool::PackageTool(int &argc, char **argv, QCommandLineParser *parser) :
    QCoreApplication(argc, argv)
{
    d = new PackageToolPrivate;
    d->parser = parser;
    QTimer::singleShot(0, this, SLOT(runMain()));
}

PackageTool::~PackageTool()
{
    delete d;
}

void PackageTool::runMain()
{
    KPackage::PackageStructure *structure = new KPackage::PackageStructure;
    if (d->parser->isSet("hash")) {
        const QString path = d->parser->value("hash");
        KPackage::Package package(structure);
        package.setPath(path);
        const QString hash = package.contentsHash();
        if (hash.isEmpty()) {
            d->coutput(i18n("Failed to generate a Package hash for %1", path));
            exit(9);
        } else {
            d->coutput(i18n("SHA1 hash for Package at %1: '%2'", package.path(), hash));
            exit(0);
        }
        return;
    }

    if (d->parser->isSet("list-types")) {
        d->listTypes();
        exit(0);
        return;
    }

    QString type = d->parser->value("type");
    QString packageRoot = type;
    d->pluginTypes.clear();
    d->installer = Package();

    if (d->parser->isSet("remove")) {
        d->package = d->parser->value("remove");
    } else if (d->parser->isSet("upgrade")) {
        d->package = d->parser->value("upgrade");
    } else if (d->parser->isSet("install")) {
        d->package = d->parser->value("install");
    } else if (d->parser->isSet("show")) {
        d->package = d->parser->value("show");
    }

    if (!QDir::isAbsolutePath(d->package)) {
        d->packageFile = QDir(QDir::currentPath() + '/' + d->package).absolutePath();
        d->packageFile = QFileInfo(d->packageFile).canonicalFilePath();
        if (d->parser->isSet("upgrade")) {
            d->package = d->packageFile;
        }
    } else {
        d->packageFile = d->package;
    }

    if (!d->packageFile.isEmpty() && (!d->parser->isSet("type") ||
                                      type.compare(i18nc("package type", "wallpaper"), Qt::CaseInsensitive) == 0 ||
                                      type.compare("wallpaper", Qt::CaseInsensitive) == 0)) {
        // Check type for common plasma packages
        KPackage::Package package(structure);
        QString serviceType;
        if (d->parser->isSet("remove")) {
            package.setPath(d->package);
        } else {
            package.setPath(d->packageFile);
        }
        if (package.isValid() && package.metadata().isValid()) {
            serviceType = package.metadata().value("X-Plasma-ServiceType");
        }

        if (!serviceType.isEmpty()) {
            if (serviceType == "KPackage/Generic") {
                type = "KPackage/Generic";
            } else {
                type = serviceType;
                //qDebug() << "fallthrough type is" << serviceType;
            }
        }
    }

    {
        PackageStructure *structure = PackageLoader::self()->loadPackageStructure(type);

        if (structure) {
            d->installer = Package(structure);
        }

        if (!d->installer.hasValidStructure()) {
            qWarning() << "Package type" << type << "not found";
        }

        d->packageRoot = d->installer.defaultPackageRoot();
        d->pluginTypes << type;
    }
    if (d->parser->isSet("show")) {
        const QString pluginName = d->package;
        showPackageInfo(pluginName);
        exit(0);
        return;
    }

    if (d->parser->isSet("list")) {
        d->packageRoot = findPackageRoot(d->package, d->packageRoot);
        d->coutput(i18n("Listing service types: %1 in %2", d->pluginTypes.join(", "), d->packageRoot));
        listPackages(d->pluginTypes, d->packageRoot);
        exit(0);

    } else if (d->parser->isSet("generate-index")) {
        recreateIndex();
        exit(0);

    } else {
        // install, remove or upgrade
        if (!d->installer.isValid()) {

            d->installer = KPackage::Package(new KPackage::PackageStructure());
        }

        d->packageRoot = findPackageRoot(d->package, d->packageRoot);

        if (d->parser->isSet("remove") || d->parser->isSet("upgrade")) {
            QString pkgPath;
            foreach (const QString &t, d->pluginTypes) {
                KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(t);
                pkg.setPath(d->package);
                if (pkg.isValid()) {
                    pkgPath = pkg.path();
                    if (pkgPath.isEmpty() && !d->packageFile.isEmpty()) {
                        pkgPath = d->packageFile;
                    }
                    continue;
                }
            }
            if (pkgPath.isEmpty()) {
                pkgPath = d->package;
            }

            if (d->parser->isSet("upgrade")) {
                d->installer.setPath(d->package);
            }
            QString _p = d->packageRoot;
            if (!_p.endsWith('/')) {
                _p.append('/');
            }
            _p.append(d->package);
            d->installer.setDefaultPackageRoot(d->packageRoot);
            d->installer.setPath(pkgPath);
            QString pluginName;
            if (d->installer.isValid()) {
                d->metadata = d->installer.metadata();
                if (!d->metadata.isValid()) {
                    pluginName = d->package;
                } else if (!d->metadata.isValid() && d->metadata.pluginId().isEmpty()) {
                    // plugin name given in command line
                    pluginName = d->package;
                } else {
                    // Parameter was a plasma package, get plugin name from the package
                    pluginName = d->metadata.pluginId();
                }
            }
            QStringList installed = d->packages(d->pluginTypes);

            const QString file = QDir::currentPath() + '/' + pluginName;

            if (QFile::exists(d->packageFile)) {
                const QString file = QDir::currentPath() + '/' + d->package;
                d->installer.setPath(d->packageFile);
                if (d->installer.isValid()) {
                    if (d->installer.metadata().isValid()) {
                        pluginName = d->installer.metadata().pluginId();
                    }
                }
            }
            // Uninstalling ...
            if (installed.contains(pluginName)) { // Assume it's a plugin name
                d->installer.setPath(pluginName);
                KJob *uninstallJob = d->installer.uninstall(pluginName, d->packageRoot);
                connect(uninstallJob, SIGNAL(result(KJob*)), SLOT(packageUninstalled(KJob*)));
                return;
            } else {
                d->coutput(i18n("Error: Plugin %1 is not installed.", pluginName));
                exit(2);
            }
        }
        if (d->parser->isSet("install")) {
            KJob *installJob = d->installer.install(d->packageFile, d->packageRoot);
            connect(installJob, SIGNAL(result(KJob*)), SLOT(packageInstalled(KJob*)));
            return;
        }
        if (d->package.isEmpty()) {
            qWarning() << i18nc("No option was given, this is the error message telling the user he needs at least one, do not translate install, remove, upgrade nor list", "One of install, remove, upgrade or list is required.");
            exit(6);
        }
    }
}

void PackageToolPrivate::coutput(const QString &msg)
{
    cout << msg << endl;
}

QStringList PackageToolPrivate::packages(const QStringList &types, const QString &path)
{
    QStringList result;

    foreach (const QString &type, types) {

        const QList<KPluginMetaData> services = KPackage::PackageLoader::self()->listPackages(type, path);
        foreach (const KPluginMetaData &service, services) {
            const QString _plugin = service.pluginId();
            if (!result.contains(_plugin)) {
                result << _plugin;
            }
        }
    }

    return result;
}

void PackageTool::showPackageInfo(const QString &pluginName)
{
    QString type = QStringLiteral("KPackage/Generic");
    if (!d->pluginTypes.contains(type) && d->pluginTypes.count() > 0) {
        type = d->pluginTypes.at(0);
    }
    KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(type);

    pkg.setDefaultPackageRoot(d->packageRoot);

    if (QFile::exists(d->packageFile)) {
        pkg.setPath(d->packageFile);
    } else {
        pkg.setPath(pluginName);
    }

    KPluginMetaData i = pkg.metadata();
    if (!i.isValid()) {
        d->coutput(i18n("Error: Can't find plugin metadata: %1", pluginName));
        exit(3);
        return;
    }
    d->coutput(i18n("Showing info for package: %1", pluginName));
    d->coutput(i18n("      Name : %1", i.name()));
    d->coutput(i18n("   Comment : %1", i.value("Comment")));
    d->coutput(i18n("    Plugin : %1", i.pluginId()));
    d->coutput(i18n("    Author : %1", i.authors().first().name()));
    d->coutput(i18n("      Path : %1", pkg.path()));

    exit(0);
}

QString PackageTool::findPackageRoot(const QString &pluginName, const QString &prefix)
{
    QString packageRoot;
    if (d->parser->isSet("packageroot") && d->parser->isSet("global") && !d->parser->isSet("generate-index")) {
        qWarning() << i18nc("The user entered conflicting options packageroot and global, this is the error message telling the user he can use only one", "The packageroot and global options conflict with each other, please select only one.");
        ::exit(7);
    } else if (d->parser->isSet("packageroot")) {
        packageRoot = d->parser->value("packageroot");
        //qDebug() << "(set via arg) d->packageRoot is: " << d->packageRoot;
    } else if (d->parser->isSet("global")) {
        packageRoot = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, d->packageRoot, QStandardPaths::LocateDirectory).last();
    } else {
        packageRoot = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + d->packageRoot;
    }
    return packageRoot;
}

void PackageTool::listPackages(const QStringList &types, const QString &path)
{
    QStringList list = d->packages(types, path);
    list.sort();
    foreach (const QString &package, list) {
        d->coutput(package);
    }
    exit(0);
}

void PackageToolPrivate::renderTypeTable(const QMap<QString, QStringList> &plugins)
{
    const QString nameHeader = i18n("Addon Name");
    const QString pluginHeader = i18n("Service Type");
    const QString pathHeader = i18n("Path");
    int nameWidth = nameHeader.length();
    int pluginWidth = pluginHeader.length();
    int pathWidth = pathHeader.length();

    QMapIterator<QString, QStringList> pluginIt(plugins);
    while (pluginIt.hasNext()) {
        pluginIt.next();
        if (pluginIt.key().length() > nameWidth) {
            nameWidth = pluginIt.key().length();
        }

        if (pluginIt.value()[0].length() > pluginWidth) {
            pluginWidth = pluginIt.value()[0].length();
        }

        if (pluginIt.value()[1].length() > pathWidth) {
            pathWidth = pluginIt.value()[1].length();
        }

    }

    std::cout << nameHeader.toLocal8Bit().constData() << std::setw(nameWidth - nameHeader.length() + 2) << ' '
              << pluginHeader.toLocal8Bit().constData() << std::setw(pluginWidth - pluginHeader.length() + 2) << ' '
              << pathHeader.toLocal8Bit().constData() << std::setw(pathWidth - pathHeader.length() + 2) << ' '
              << std::endl;
    std::cout << std::setfill('-') << std::setw(nameWidth) << '-' << "  "
              << std::setw(pluginWidth) << '-' << "  "
              << std::setw(pathWidth) << '-' << "  "
              << std::endl;
    std::cout << std::setfill(' ');

    pluginIt.toFront();
    while (pluginIt.hasNext()) {
        pluginIt.next();
        std::cout << pluginIt.key().toLocal8Bit().constData() << std::setw(nameWidth - pluginIt.key().length() + 2) << ' '
                  << pluginIt.value()[0].toLocal8Bit().constData() << std::setw(pluginWidth - pluginIt.value()[0].length() + 2) << ' '
                  << pluginIt.value()[1].toLocal8Bit().constData() << std::setw(pathWidth - pluginIt.value()[1].length() + 2) << std::endl;
    }
}

void PackageToolPrivate::listTypes()
{
    coutput(i18n("Package types that are installable with this tool:"));
    coutput(i18n("Built in:"));

    QMap<QString, QStringList> builtIns;
    builtIns.insert(i18n("KPackage/Generic"), QStringList() << QStringLiteral("KPackage/Generic") << KPACKAGE_RELATIVE_DATA_INSTALL_DIR "/packages/");
    builtIns.insert(i18n("KPackage/GenericQML"), QStringList() << QStringLiteral("KPackage/GenericQML") << KPACKAGE_RELATIVE_DATA_INSTALL_DIR "/packagesqml/");

    renderTypeTable(builtIns);

    QStringList libraryPaths;

    const QString subDirectory = "kpackage/packagestructure";

    Q_FOREACH (const QString &dir, QCoreApplication::libraryPaths()) {
        QString d = dir + QDir::separator() + subDirectory;
        if (!d.endsWith(QDir::separator())) {
            d += QDir::separator();
        }
        libraryPaths << d;
    }


    QString pluginFileName;

    QList<KPluginMetaData> offers;

    Q_FOREACH (const QString &plugindir, libraryPaths) {
        const QString &_ixfile = plugindir + QStringLiteral("kpluginindex.json");
        QFile indexFile(_ixfile);
        if (indexFile.exists()) {
            indexFile.open(QIODevice::ReadOnly);
            QJsonDocument jdoc = QJsonDocument::fromBinaryData(indexFile.readAll());
            indexFile.close();


            QJsonArray plugins = jdoc.array();

            for (QJsonArray::const_iterator iter = plugins.constBegin(); iter != plugins.constEnd(); ++iter) {
                const QJsonObject &obj = QJsonValue(*iter).toObject();
                const QString &candidate = obj.value(QStringLiteral("FileName")).toString();
                const KPluginMetaData m(obj, candidate);
                offers << m;
            }
        } else {
            QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(plugindir);
            QVectorIterator<KPluginMetaData> iter(plugins);
            while (iter.hasNext()) {
                auto md = iter.next();
                offers << md;
            }
        }
    }



    if (!offers.isEmpty()) {
        std::cout << std::endl;
        coutput(i18n("Provided by plugins:"));

        QMap<QString, QStringList> plugins;
        foreach (const KPluginMetaData &info, offers) {
            //const QString proot = "";
            KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(info.pluginId());
            QString name = info.name();
            QString comment = info.value("Comment");
            QString plugin = info.pluginId();
            QString path = pkg.defaultPackageRoot();
            QString types = info.serviceTypes().join(",");
            //QString path = defaultPackageRoot;
            plugins.insert(name, QStringList() << plugin << path);
            //qDebug() << "KService stuff:" << name << plugin << comment << path;
        }

        renderTypeTable(plugins);
    }

    QStringList desktopFiles = QStandardPaths::locateAll(QStandardPaths::DataLocation, KPACKAGE_RELATIVE_DATA_INSTALL_DIR + QStringLiteral("/packageformats/*rc"), QStandardPaths::LocateFile);

    if (!desktopFiles.isEmpty()) {
        coutput(i18n("Provided by .desktop files:"));
        KPackage::PackageStructure structure;
        QMap<QString, QStringList> plugins;
        foreach (const QString &file, desktopFiles) {
            // extract the type
            KConfig config(file, KConfig::SimpleConfig);
#pragma message("read config here")
            // structure.read(&config);
            // get the name based on the rc file name, just as KPackage::PackageStructure does
            const QString name = file.left(file.length() - 2);
            //plugins.insert(name, QStringList() << structure.type() << structure.defaultPackageRoot());
        }
    }
}

void PackageTool::recreateIndex()
{
    d->packageRoot = findPackageRoot(d->package, d->packageRoot);

    if (!QDir::isAbsolutePath(d->packageRoot)) {
        if (d->parser->isSet("global")) {
            for (auto p : QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, d->packageRoot, QStandardPaths::LocateDirectory)) {
                d->coutput(i18n("Generating %1/kpluginindex.json", p));
                KPackage::indexDirectory(p, QStringLiteral("kpluginindex.json"));
            }
            return;
        } else {
            d->packageRoot = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + d->packageRoot;
        }
    }

    d->coutput(i18n("Generating %1/kpluginindex.json", d->packageRoot));
    KPackage::indexDirectory(d->packageRoot, QStringLiteral("kpluginindex.json"));
}

void PackageTool::packageInstalled(KJob *job)
{
    bool success = (job->error() == KJob::NoError);
    int exitcode = 0;
    if (success) {
        if (d->parser->isSet("upgrade")) {
            d->coutput(i18n("Successfully upgraded %1", d->packageFile));
        } else {
            d->coutput(i18n("Successfully installed %1", d->packageFile));
        }
    } else {
        d->coutput(i18n("Error: Installation of %1 failed: %2", d->packageFile, job->errorText()));
        exitcode = 4;
    }
    exit(exitcode);
}

void PackageTool::packageUninstalled(KJob *job)
{
    bool success = (job->error() == KJob::NoError);
    int exitcode = 0;
    if (success) {
        if (d->parser->isSet("upgrade")) {
            d->coutput(i18n("Upgrading package from file: %1", d->packageFile));
            KJob *installJob = d->installer.install(d->packageFile, d->packageRoot);
            connect(installJob, SIGNAL(result(KJob*)), SLOT(packageInstalled(KJob*)));
            return;
        }
        d->coutput(i18n("Successfully uninstalled %1", d->packageFile));
    } else {
        d->coutput(i18n("Error: Uninstallation of %1 failed: %2", d->packageFile, job->errorText()));
        exitcode = 7;
    }
    exit(exitcode);
}

} // namespace KPackage

#include "moc_kpackagetool.cpp"

