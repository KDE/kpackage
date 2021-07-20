/*
    SPDX-FileCopyrightText: 2010 Ryan Rix <ry@n.rix.si>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packageloader.h"
#include "private/packageloader_p.h"
#include "private/utils.h"

#include "kpackage_debug.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDirIterator>
#include <QStandardPaths>
#include <QVector>

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>

#include "config-package.h"

#include "package.h"
#include "packagestructure.h"
#include "private/packagejobthread_p.h"
#include "private/packages_p.h"

namespace KPackage
{
static PackageLoader *s_packageTrader = nullptr;

QSet<QString> PackageLoaderPrivate::s_customCategories;

QSet<QString> PackageLoaderPrivate::knownCategories()
{
    // this is to trick the translation tools into making the correct
    // strings for translation
    QSet<QString> categories = s_customCategories;
    // clang-format off
    categories << QStringLiteral(I18N_NOOP("Accessibility")).toLower()
               << QStringLiteral(I18N_NOOP("Application Launchers")).toLower()
               << QStringLiteral(I18N_NOOP("Astronomy")).toLower()
               << QStringLiteral(I18N_NOOP("Date and Time")).toLower()
               << QStringLiteral(I18N_NOOP("Development Tools")).toLower()
               << QStringLiteral(I18N_NOOP("Education")).toLower()
               << QStringLiteral(I18N_NOOP("Environment and Weather")).toLower()
               << QStringLiteral(I18N_NOOP("Examples")).toLower()
               << QStringLiteral(I18N_NOOP("File System")).toLower()
               << QStringLiteral(I18N_NOOP("Fun and Games")).toLower()
               << QStringLiteral(I18N_NOOP("Graphics")).toLower()
               << QStringLiteral(I18N_NOOP("Language")).toLower()
               << QStringLiteral(I18N_NOOP("Mapping")).toLower()
               << QStringLiteral(I18N_NOOP("Miscellaneous")).toLower()
               << QStringLiteral(I18N_NOOP("Multimedia")).toLower()
               << QStringLiteral(I18N_NOOP("Online Services")).toLower()
               << QStringLiteral(I18N_NOOP("Productivity")).toLower()
               << QStringLiteral(I18N_NOOP("System Information")).toLower()
               << QStringLiteral(I18N_NOOP("Utilities")).toLower()
               << QStringLiteral(I18N_NOOP("Windows and Tasks")).toLower();
    // clang-format on
    return categories;
}

PackageLoader::PackageLoader()
    : d(new PackageLoaderPrivate)
{
}

PackageLoader::~PackageLoader()
{
    for (auto wp : qAsConst(d->structures)) {
        delete wp.data();
    }
    delete d;
}

void PackageLoader::setPackageLoader(PackageLoader *loader)
{
    if (!s_packageTrader) {
        s_packageTrader = loader;
    } else {
#ifndef NDEBUG
        // qCDebug(KPACKAGE_LOG) << "Cannot set packageTrader, already set!" << s_packageTrader;
#endif
    }
}

PackageLoader *PackageLoader::self()
{
    if (!s_packageTrader) {
        // we have been called before any PackageLoader was set, so just use the default
        // implementation. this prevents plugins from nefariously injecting their own
        // plugin loader if the app doesn't
        s_packageTrader = new PackageLoader;
        s_packageTrader->d->isDefaultLoader = true;
    }

    return s_packageTrader;
}

Package PackageLoader::loadPackage(const QString &packageFormat, const QString &packagePath)
{
    if (!d->isDefaultLoader) {
        Package p = internalLoadPackage(packageFormat);
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

    PackageStructure *structure = loadPackageStructure(packageFormat);

    if (structure) {
        Package p(structure);
        if (!packagePath.isEmpty()) {
            p.setPath(packagePath);
        }
        return p;
    }

#ifndef NDEBUG
    // qCDebug(KPACKAGE_LOG) << "Couldn't load Package for" << packageFormat << "! reason given: " << error;
#endif

    return Package();
}

QList<KPluginMetaData> PackageLoader::listPackages(const QString &packageFormat, const QString &packageRoot)
{
    // Note: Use QDateTime::currentSecsSinceEpoch() once we can depend on Qt 5.8
    const qint64 now = qRound64(QDateTime::currentMSecsSinceEpoch() / 1000.0);
    bool useRuntimeCache = true;
    if (now - d->pluginCacheAge > d->maxCacheAge && d->pluginCacheAge != 0) {
        // cache is old and we're not within a few seconds of startup anymore
        useRuntimeCache = false;
        d->pluginCache.clear();
    }

    const QString cacheKey = QStringLiteral("%1.%2").arg(packageFormat, packageRoot);
    if (useRuntimeCache) {
        auto it = d->pluginCache.constFind(cacheKey);
        if (it != d->pluginCache.constEnd()) {
            return *it;
        }
    }
    if (d->pluginCacheAge == 0) {
        d->pluginCacheAge = now;
    }

    QList<KPluginMetaData> lst;

    // has been a root specified?
    QString actualRoot = packageRoot;

    // try to take it from the package structure
    if (actualRoot.isEmpty()) {
        PackageStructure *structure = d->structures.value(packageFormat).data();
        if (!structure) {
            if (packageFormat == QStringLiteral("KPackage/Generic")) {
                structure = new GenericPackage();
            } else if (packageFormat == QStringLiteral("KPackage/GenericQML")) {
                structure = new GenericQMLPackage();
            }
        }

        if (!structure) {
            structure = loadPackageStructure(packageFormat);
        }

        if (structure) {
            d->structures.insert(packageFormat, structure);
            Package p(structure);
            actualRoot = p.defaultPackageRoot();
        }
    }

    if (actualRoot.isEmpty()) {
        actualRoot = packageFormat;
    }

    QSet<QString> uniqueIds;
    QStringList paths;
    if (QDir::isAbsolutePath(actualRoot)) {
        paths = QStringList(actualRoot);
    } else {
        const auto listPath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        for (const QString &path : listPath) {
            paths += path + QLatin1Char('/') + actualRoot;
        }
    }

    for (auto const &plugindir : qAsConst(paths)) {
        const QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
        const QStringList nameFilters = {QStringLiteral("metadata.json"), QStringLiteral("metadata.desktop")};

        QDirIterator it(plugindir, nameFilters, QDir::Files, flags);
        QSet<QString> dirs;
        while (it.hasNext()) {
            it.next();

            const QString dir = it.fileInfo().absoluteDir().path();

            if (dirs.contains(dir)) {
                continue;
            }
            dirs << dir;

            const QString metadataPath = it.fileInfo().absoluteFilePath();
            const KPluginMetaData info(metadataPath);

            if (!info.isValid() || uniqueIds.contains(info.pluginId())) {
                continue;
            }

            const QStringList kpackageTypes = readKPackageTypes(info);
            if (packageFormat.isEmpty() || kpackageTypes.isEmpty() || kpackageTypes.contains(packageFormat)) {
                uniqueIds << info.pluginId();
                lst << info;
            }
        }
    }

    if (useRuntimeCache) {
        d->pluginCache.insert(cacheKey, lst);
    }
    return lst;
}

QList<KPluginMetaData>
PackageLoader::findPackages(const QString &packageFormat, const QString &packageRoot, std::function<bool(const KPluginMetaData &)> filter)
{
    QList<KPluginMetaData> lst;
    const auto lstPlugins = listPackages(packageFormat, packageRoot);
    for (auto const &plugin : lstPlugins) {
        if (!filter || filter(plugin)) {
            lst << plugin;
        }
    }
    return lst;
}

KPackage::PackageStructure *PackageLoader::loadPackageStructure(const QString &packageFormat)
{
    PackageStructure *structure = d->structures.value(packageFormat).data();
    if (!structure) {
        if (packageFormat == QStringLiteral("KPackage/Generic")) {
            structure = new GenericPackage();
            d->structures.insert(packageFormat, structure);
        } else if (packageFormat == QStringLiteral("KPackage/GenericQML")) {
            structure = new GenericQMLPackage();
            d->structures.insert(packageFormat, structure);
        }
    }

    if (structure) {
        return structure;
    }

    KPluginMetaData metaData;
    QVector<KPluginMetaData> plugins =
        KPluginLoader::findPlugins(QStringLiteral("kpackage/packagestructure"), [packageFormat](const KPluginMetaData &metaData) {
            return readKPackageTypes(metaData).contains(packageFormat);
        });
    if (!plugins.isEmpty()) {
        metaData = plugins.constFirst();
    }

    QString error;
    if (metaData.isValid()) {
        KPluginLoader loader(metaData.fileName());
        const QVariantList argsWithMetaData = QVariantList() << loader.metaData().toVariantMap();
        KPluginFactory *factory = loader.factory();
        if (factory) {
            structure = factory->create<PackageStructure>(nullptr, argsWithMetaData);
            if (!structure) {
                error = QCoreApplication::translate("", "No service matching the requirements was found");
            }
        }
    }

    if (structure && !error.isEmpty()) {
        qCWarning(KPACKAGE_LOG) << i18n("Could not load installer for package of type %1. Error reported was: %2", packageFormat, error);
    }

    if (structure) {
        d->structures.insert(packageFormat, structure);
    }

    return structure;
}

void PackageLoader::addKnownPackageStructure(const QString &packageFormat, KPackage::PackageStructure *structure)
{
    d->structures.insert(packageFormat, structure);
}

Package PackageLoader::internalLoadPackage(const QString &name)
{
    Q_UNUSED(name);
    return Package();
}

} // KPackage Namespace
