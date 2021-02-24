/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packages_p.h"

#include <math.h>

#include <KLocalizedString>

#include "kpackage/package.h"

void ChangeableMainScriptPackage::initPackage(KPackage::Package *package)
{
    package->addFileDefinition("mainscript", QStringLiteral("scripts/main.js"), i18n("Main Script File"));
    package->setRequired("mainscript", true);
}

QString ChangeableMainScriptPackage::mainScriptConfigKey() const
{
    return QStringLiteral("X-KPackage-MainScript");
}

void ChangeableMainScriptPackage::pathChanged(KPackage::Package *package)
{
    if (package->path().isEmpty()) {
        return;
    }

    QString mainScript = package->metadata().value(mainScriptConfigKey());
    if (mainScript.isEmpty()) {
        mainScript = package->metadata().value(QStringLiteral("X-Plasma-MainScript"));
    }

    if (!mainScript.isEmpty()) {
        package->addFileDefinition("mainscript", mainScript, i18n("Main Script File"));
    }
}

void GenericPackage::initPackage(KPackage::Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);

    package->setDefaultPackageRoot(QStringLiteral("kpackage/generic/"));

    package->addDirectoryDefinition("images", QStringLiteral("images"), i18n("Images"));
    package->addDirectoryDefinition("theme", QStringLiteral("theme"), i18n("Themed Images"));
    QStringList mimetypes;
    mimetypes << QStringLiteral("image/svg+xml") << QStringLiteral("image/png") << QStringLiteral("image/jpeg");
    package->setMimeTypes("images", mimetypes);
    package->setMimeTypes("theme", mimetypes);

    package->addDirectoryDefinition("config", QStringLiteral("config"), i18n("Configuration Definitions"));
    mimetypes.clear();
    mimetypes << QStringLiteral("text/xml");
    package->setMimeTypes("config", mimetypes);

    package->addDirectoryDefinition("ui", QStringLiteral("ui"), i18n("User Interface"));

    package->addDirectoryDefinition("data", QStringLiteral("data"), i18n("Data Files"));

    package->addDirectoryDefinition("scripts", QStringLiteral("code"), i18n("Executable Scripts"));
    mimetypes.clear();
    mimetypes << QStringLiteral("text/plain");
    package->setMimeTypes("scripts", mimetypes);

    package->addDirectoryDefinition("translations", QStringLiteral("locale"), i18n("Translations"));
}

void GenericQMLPackage::initPackage(KPackage::Package *package)
{
    GenericPackage::initPackage(package);

    package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"), i18n("Main UI File"));
    package->setRequired("mainscript", true);
    package->setDefaultPackageRoot(
        QStringLiteral("kpackage"
                       "/genericqml/"));
}
