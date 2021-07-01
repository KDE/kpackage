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
    GenericPackage::initPackage(package);
    package->setDefaultPackageRoot(QStringLiteral("plasma/plasmoids/"));

    package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"), i18n("Main Script File"));
    package->addFileDefinition("configmodel", QStringLiteral("config/config.qml"), i18n("Configuration UI pages model"));
    package->addFileDefinition("mainconfigxml", QStringLiteral("config/main.xml"), i18n("Configuration XML file"));
}

} // namespace KPackage
