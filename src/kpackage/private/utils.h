#pragma once

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
    if (metaData.serviceTypes() == ignoreStringList) {
        // If the service type is set to this value we have a structure plugin, consequently we want the pluginId
        types << metaData.pluginId();
    } else {
        // We have a package, read the service types
        types << metaData.serviceTypes();
    }
    // while most package structure plugins do, they don't need to set the service types,
    // if we haven't found anything so far we use the plugin id
    if (types.isEmpty()) {
        types << metaData.pluginId();
    }
    return types;
}
