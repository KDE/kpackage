#pragma once

#include "kpackage_debug.h"
#include <KPluginMetaData>
#include <QString>

inline QStringList readKPackageTypes(const KPluginMetaData &metaData)
{
    const QString structureValue = metaData.value(QStringLiteral("KPackageStructure"));

    // If we have the new property we can immediately return it's value, no matter if it is a structure plugin or package
    if (!structureValue.isEmpty()) {
        return QStringList(structureValue);
    }

    // Compatibility block for old keys
    QStringList types;
    const static QStringList ignoreStringList = QStringList{QStringLiteral("KPackage/PackageStructure")};
    const auto serviceTypes = metaData.rawData().value(QStringLiteral("KPlugin")).toObject().value(QStringLiteral("ServiceTypes")).toVariant().toStringList();
    if (serviceTypes == ignoreStringList) {
        // If the service type is set to this value we have a structure plugin, consequently we want the pluginId
        types << metaData.pluginId();
    } else {
        // We have a package, read the service types
        types << serviceTypes;
    }
    // while most package structure plugins do, they don't need to set the service types,
    // if we haven't found anything so far we use the plugin id
    if (types.isEmpty() && !metaData.fileName().endsWith(QLatin1String(".json"))) {
        types << metaData.pluginId();
    }
    return types;
}

inline KPluginMetaData structureForKPackageType(const QString &packageFormat)
{
    const QString guessedPath = QStringLiteral("kpackage/packagestructure/") + QString(packageFormat).toLower().replace(QLatin1Char('/'), QLatin1Char('_'));
    KPluginMetaData guessedData(guessedPath);
    if (guessedData.isValid() && readKPackageTypes(guessedData).contains(packageFormat)) {
        return guessedData;
    }
    qCDebug(KPACKAGE_LOG) << "Could not find package structure for" << packageFormat << "by plugin path. The guessed path was" << guessedPath;
    QVector<KPluginMetaData> plugins =
        KPluginMetaData::findPlugins(QStringLiteral("kpackage/packagestructure"), [packageFormat](const KPluginMetaData &metaData) {
            return readKPackageTypes(metaData).contains(packageFormat);
        });
    if (!plugins.isEmpty()) {
        return plugins.constFirst();
    }
    return KPluginMetaData();
}
