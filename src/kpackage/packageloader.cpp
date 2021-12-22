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

#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KPluginFactory>

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
    categories << kli18n("Accessibility").toString().toLower()
               << kli18n("Application Launchers").toString().toLower()
               << kli18n("Astronomy").toString().toLower()
               << kli18n("Date and Time").toString().toLower()
               << kli18n("Development Tools").toString().toLower()
               << kli18n("Education").toString().toLower()
               << kli18n("Environment and Weather").toString().toLower()
               << kli18n("Examples").toString().toLower()
               << kli18n("File System").toString().toLower()
               << kli18n("Fun and Games").toString().toLower()
               << kli18n("Graphics").toString().toLower()
               << kli18n("Language").toString().toLower()
               << kli18n("Mapping").toString().toLower()
               << kli18n("Miscellaneous").toString().toLower()
               << kli18n("Multimedia").toString().toLower()
               << kli18n("Online Services").toString().toLower()
               << kli18n("Productivity").toString().toLower()
               << kli18n("System Information").toString().toLower()
               << kli18n("Utilities").toString().toLower()
               << kli18n("Windows and Tasks").toString().toLower();
    // clang-format on
    return categories;
}

PackageLoader::PackageLoader()
    : d(new PackageLoaderPrivate)
{
}

PackageLoader::~PackageLoader()
{
    for (auto wp : std::as_const(d->structures)) {
        delete wp.data();
    }
    delete d;
}

#if KPACKAGE_BUILD_DEPRECATED_SINCE(5, 86)
void PackageLoader::setPackageLoader(PackageLoader *loader)
{
    if (!s_packageTrader) {
        s_packageTrader = loader;
    }
}
#endif

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
#if KPACKAGE_BUILD_DEPRECATED_SINCE(5, 86)
    if (!d->isDefaultLoader) {
        Package p = internalLoadPackage(packageFormat);
        if (p.hasValidStructure()) {
            if (!packagePath.isEmpty()) {
                p.setPath(packagePath);
            }
            return p;
        }
    }
#endif

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

    for (auto const &plugindir : std::as_const(paths)) {
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

    const KPluginMetaData metaData = structureForKPackageType(packageFormat);

    QString error;
    if (!metaData.isValid()) {
        qCWarning(KPACKAGE_LOG) << "Invalid metadata for package structure" << packageFormat;
        return nullptr;
    }

    auto result = KPluginFactory::instantiatePlugin<PackageStructure>(metaData, nullptr, {metaData.rawData().toVariantMap()});

    if (!result) {
        qCWarning(KPACKAGE_LOG) << i18n("Could not load installer for package of type %1. Error reported was: %2", packageFormat, result.errorString);
        return nullptr;
    }

    structure = result.plugin;

    d->structures.insert(packageFormat, structure);

    return structure;
}

void PackageLoader::addKnownPackageStructure(const QString &packageFormat, KPackage::PackageStructure *structure)
{
    d->structures.insert(packageFormat, structure);
}

#if KPACKAGE_BUILD_DEPRECATED_SINCE(5, 86)
Package PackageLoader::internalLoadPackage(const QString &name)
{
    Q_UNUSED(name);
    return Package();
}
#endif

} // KPackage Namespace
