#pragma once

#include "kpackage_debug.h"
#include <KPluginMetaData>
#include <QString>

inline QString readKPackageType(const KPluginMetaData &metaData)
{
    return metaData.value(QStringLiteral("KPackageStructure"));
}

inline KPluginMetaData structureForKPackageType(const QString &packageFormat)
{
    const QString guessedPath = QStringLiteral("kpackage/packagestructure/") + QString(packageFormat).toLower().replace(QLatin1Char('/'), QLatin1Char('_'));
    KPluginMetaData guessedData(guessedPath);
    if (guessedData.isValid() && readKPackageType(guessedData) == packageFormat) {
        return guessedData;
    }
    qCDebug(KPACKAGE_LOG) << "Could not find package structure for" << packageFormat << "by plugin path. The guessed path was" << guessedPath;

    const QVector<KPluginMetaData> plugins =
        KPluginMetaData::findPlugins(QStringLiteral("kpackage/packagestructure"), [packageFormat](const KPluginMetaData &metaData) {
            return readKPackageType(metaData) == packageFormat;
        });
    return plugins.isEmpty() ? KPluginMetaData() : plugins.first();
}
