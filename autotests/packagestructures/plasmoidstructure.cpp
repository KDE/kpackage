/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasmoidstructure.h"

#include "config-package.h"
#include "package.h"
#include <KLocalizedString>

namespace KPackage
{
void PlasmoidPackage::initPackage(Package *package)
{
    KPackage::PackageStructure::initPackage(package);
    package->setDefaultPackageRoot(QStringLiteral("plasma/plasmoids/"));

    package->addDirectoryDefinition("ui", QStringLiteral("ui/"), i18n("User interface"));
    package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"), i18n("Main Script File"));
    package->setRequired("mainscript", true);

    package->addFileDefinition("configmodel", QStringLiteral("config/config.qml"), i18n("Configuration UI pages model"));
    package->addFileDefinition("mainconfigxml", QStringLiteral("config/main.xml"), i18n("Configuration XML file"));

    package->addDirectoryDefinition("images", QStringLiteral("images"), i18n("Images"));
    package->setMimeTypes("images", {QStringLiteral("image/svg+xml"), QStringLiteral("image/png"), QStringLiteral("image/jpeg")});

    package->addDirectoryDefinition("scripts", QStringLiteral("code"), i18n("Executable Scripts"));
    package->setMimeTypes("scripts", {QStringLiteral("text/plain")});
}

} // namespace KPackage
