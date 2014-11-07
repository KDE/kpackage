/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "packagetrader.h"

#include <QStandardPaths>
#include <QDirIterator>

#include <QDebug>
#include <KService>
#include <KServiceTypeTrader>
#include <KPluginTrader>
#include <KLocalizedString>

#include "config-package.h"



#include "private/packages_p.h"
#include "package.h"
#include "packagestructure.h"

namespace KPackage
{

static PackageTrader *s_packageTrader = 0;

class PackageTraderPrivate
{
public:
    PackageTraderPrivate()
        : isDefaultLoader(false),
          packageStructurePluginDir("kpackage/packagestructure"),
          packageRE("[^a-zA-Z0-9\\-_]")
    {
    }

    static QSet<QString> knownCategories();
    static QString parentAppConstraint(const QString &parentApp = QString());

    static QSet<QString> s_customCategories;
    QHash<QString, QWeakPointer<PackageStructure> > structures;
    bool isDefaultLoader;
    QString packageStructurePluginDir;
    QRegExp packageRE;
};

QSet<QString> PackageTraderPrivate::s_customCategories;

QSet<QString> PackageTraderPrivate::knownCategories()
{
    // this is to trick the tranlsation tools into making the correct
    // strings for translation
    QSet<QString> categories = s_customCategories;
    categories << QString(I18N_NOOP("Accessibility")).toLower()
               << QString(I18N_NOOP("Application Launchers")).toLower()
               << QString(I18N_NOOP("Astronomy")).toLower()
               << QString(I18N_NOOP("Date and Time")).toLower()
               << QString(I18N_NOOP("Development Tools")).toLower()
               << QString(I18N_NOOP("Education")).toLower()
               << QString(I18N_NOOP("Environment and Weather")).toLower()
               << QString(I18N_NOOP("Examples")).toLower()
               << QString(I18N_NOOP("File System")).toLower()
               << QString(I18N_NOOP("Fun and Games")).toLower()
               << QString(I18N_NOOP("Graphics")).toLower()
               << QString(I18N_NOOP("Language")).toLower()
               << QString(I18N_NOOP("Mapping")).toLower()
               << QString(I18N_NOOP("Miscellaneous")).toLower()
               << QString(I18N_NOOP("Multimedia")).toLower()
               << QString(I18N_NOOP("Online Services")).toLower()
               << QString(I18N_NOOP("Productivity")).toLower()
               << QString(I18N_NOOP("System Information")).toLower()
               << QString(I18N_NOOP("Utilities")).toLower()
               << QString(I18N_NOOP("Windows and Tasks")).toLower();
    return categories;
}

QString PackageTraderPrivate::parentAppConstraint(const QString &parentApp)
{
    if (parentApp.isEmpty()) {
        const QCoreApplication *app = QCoreApplication::instance();
        if (!app) {
            return QString();
        }

        return QString("((not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '') or [X-KDE-ParentApp] == '%1')")
               .arg(app->applicationName());
    }

    return QString("[X-KDE-ParentApp] == '%1'").arg(parentApp);
}

PackageTrader::PackageTrader()
    : d(new PackageTraderPrivate)
{
}

PackageTrader::~PackageTrader()
{
    foreach (auto wp, d->structures) {
        delete wp.data();
    }
    delete d;
}

void PackageTrader::setPackageTrader(PackageTrader *loader)
{
    if (!s_packageTrader) {
        s_packageTrader = loader;
    } else {
#ifndef NDEBUG
        // qDebug() << "Cannot set packageTrader, already set!" << s_packageTrader;
#endif
    }
}

PackageTrader *PackageTrader::self()
{
    if (!s_packageTrader) {
        // we have been called before any PackageTrader was set, so just use the default
        // implementation. this prevents plugins from nefariously injecting their own
        // plugin loader if the app doesn't
        s_packageTrader = new PackageTrader;
        s_packageTrader->d->isDefaultLoader = true;
    }

    return s_packageTrader;
}

Package PackageTrader::loadPackage(const QString &packageFormat, const QString &packagePath, const QString &specialization)
{
    if (!d->isDefaultLoader) {
        Package p = internalLoadPackage(packageFormat, specialization);
        if (p.hasValidStructure()) {
            if (!packagePath.isEmpty()) {
                p.setPath(packagePath);
            }
            return p;
        }
    }

    if (packageFormat.isEmpty()) {
        return Package();
    }

    const QString hashkey = packageFormat + '%' + specialization;
    PackageStructure *structure = d->structures.value(hashkey).data();
    if (structure) {
        Package p(structure);
        if (!packagePath.isEmpty()) {
            p.setPath(packagePath);
        }
        return p;
    }

    if (!specialization.isEmpty()) {
        // check that the provided strings are safe to use in a ServiceType query
        if (d->packageRE.indexIn(specialization) == -1 && d->packageRE.indexIn(packageFormat) == -1) {
            const KPluginInfo::List offers = KPluginTrader::self()->query(packageFormat, QString(), specialization);

            if (!offers.isEmpty()) {
                return loadPackage(packageFormat, packagePath);
            }
        }
    }

    if (packageFormat == QStringLiteral("KPackage/Generic")) {
        structure = new GenericPackage();
        d->structures.insert(hashkey, structure);
        Package p(structure);
        if (!packagePath.isEmpty()) {
            p.setPath(packagePath);
        }
        return p;
    }

    // first we check for plugins in sycoca
    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(packageFormat);
    structure = KPluginTrader::createInstanceFromQuery<KPackage::PackageStructure>(d->packageStructurePluginDir, "KPackage/PackageStructure", constraint, 0);
    if (structure) {
        d->structures.insert(hashkey, structure);
        Package p(structure);
        if (!packagePath.isEmpty()) {
            p.setPath(packagePath);
        }
        return p;
    }

#ifndef NDEBUG
        // qDebug() << "Couldn't load Package for" << packageFormat << "! reason given: " << error;
#endif

    return Package();
}

KPluginInfo::List PackageTrader::query(const QString &packageFormat,
                                    const QString &constraint)
{
    KPluginInfo::List lst;

    QString packageRoot = packageFormat;

    PackageStructure *structure = d->structures.value(packageFormat).data();
    if (!structure) {
        if (packageFormat == QStringLiteral("KPackage/Generic")) {
            structure = new GenericPackage();
        }
    }

    if (!structure) {
        const QString structConstraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(packageFormat);
        structure = KPluginTrader::createInstanceFromQuery<KPackage::PackageStructure>(d->packageStructurePluginDir,
                                                           QStringLiteral("KPackage/PackageStructure"), structConstraint, 0);
    }

    if (structure) {
        d->structures.insert(packageFormat, structure);
        Package p(structure);
        packageRoot = p.defaultPackageRoot();

    }

    //TODO: case in which defaultpackageroot is absolute
    for (auto datadir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const QString plugindir = datadir + '/' + packageRoot;
        //qDebug() << "Not cached";
        // If there's no cache file, fall back to listing the directory
        const QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
        const QStringList nameFilters = QStringList(QStringLiteral("metadata.desktop"));

        QDirIterator it(plugindir, nameFilters, QDir::Files, flags);
        while (it.hasNext()) {
            it.next();
            const QString file = it.fileInfo().absoluteFilePath();

            const KPluginInfo info(file);
            if (!info.isValid()) {
                continue;
            }

            if (packageFormat.isEmpty() || info.serviceTypes().contains(packageFormat)) {
                lst << info;
            }
        }
    }

    KPluginTrader::applyConstraints(lst, constraint);
    return lst;
}

QList<Package> PackageTrader::packagesFromQuery(const QString &packageFormat,
                                    const QString &constraint,
                                    const QString &requiredKey,
                                    const QString &requiredFilename)
{
    QList<Package> list;
    KPluginInfo::List plugins = query(packageFormat, constraint);

    foreach (const KPluginInfo &info, plugins) {
        Package p = loadPackage(packageFormat, info.pluginName());

        if (!requiredKey.isEmpty() && p.filePath(requiredKey.toLatin1(), requiredFilename).isEmpty()) {
            continue;
        }

        list << p;
    }
    return list;
}

Package PackageTrader::internalLoadPackage(const QString &name, const QString &specialization)
{
    Q_UNUSED(name);
    Q_UNUSED(specialization);
    return Package();
}

} // Plasma Namespace

