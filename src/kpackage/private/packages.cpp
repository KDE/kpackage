/******************************************************************************
*   Copyright 2007-2009 by Aaron Seigo <aseigo@kde.org>                       *
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

#include "packages_p.h"

#include <math.h>

#include <QCoreApplication>
#include <QFileInfo>

#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <klocalizedstring.h>

#include "kpackage/package.h"


void ChangeableMainScriptPackage::initPackage(KPackage::Package *package)
{
    package->addFileDefinition("mainscript", "scripts/main.js", i18n("Main Script File"));
    package->setRequired("mainscript", true);
}

QString ChangeableMainScriptPackage::mainScriptConfigKey() const
{
    return QLatin1String("X-KPackage-MainScript");
}

void ChangeableMainScriptPackage::pathChanged(KPackage::Package *package)
{
    if (package->path().isEmpty()) {
        return;
    }

    KDesktopFile config(package->path() + "/metadata.desktop");
    KConfigGroup cg = config.desktopGroup();
    QString mainScript = cg.readEntry(mainScriptConfigKey(), QString());
    if (mainScript.isEmpty()) {
        mainScript = cg.readEntry("X-Plasma-MainScript", QString());
    }

    if (!mainScript.isEmpty()) {
        package->addFileDefinition("mainscript", mainScript, i18n("Main Script File"));
    }
}

void GenericPackage::initPackage(KPackage::Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);

    package->setDefaultPackageRoot("kpackage" "/generic/");

    package->addDirectoryDefinition("images", "images", i18n("Images"));
    package->addDirectoryDefinition("theme", "theme", i18n("Themed Images"));
    QStringList mimetypes;
    mimetypes << "image/svg+xml" << "image/png" << "image/jpeg";
    package->setMimeTypes("images", mimetypes);
    package->setMimeTypes("theme", mimetypes);

    package->addDirectoryDefinition("config", "config", i18n("Configuration Definitions"));
    mimetypes.clear();
    mimetypes << "text/xml";
    package->setMimeTypes("config", mimetypes);

    package->addDirectoryDefinition("ui", "ui", i18n("User Interface"));

    package->addDirectoryDefinition("data", "data", i18n("Data Files"));

    package->addDirectoryDefinition("scripts", "code", i18n("Executable Scripts"));
    mimetypes.clear();
    mimetypes << "text/plain";
    package->setMimeTypes("scripts", mimetypes);

    package->addDirectoryDefinition("translations", "locale", i18n("Translations"));
}

void GenericQMLPackage::initPackage(KPackage::Package *package)
{
    GenericPackage::initPackage(package);

    package->addFileDefinition("mainscript", "ui/main.qml", i18n("Main UI File"));
    package->setRequired("mainscript", true);
    package->setDefaultPackageRoot("kpackage" "/genericqml/");
}

