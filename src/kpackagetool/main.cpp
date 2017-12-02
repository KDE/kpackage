/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

/**
 * kpackagetool5 exit codes used in this program

    0 No error

    1 Unspecified error
    2 Plugin is not installed
    3 Plugin or package invalid
    4 Installation failed, see stderr for reason
    5 Could not find a suitable installer for package type
    6 No install option given
    7 Conflicting arguments supplied
    8 Uninstallation failed, see stderr for reason
    9 Failed to generate package hash

*/

#include <klocalizedstring.h>
#include <qcommandlineparser.h>

#include "kpackagetool.h"
#include "options.h"

int main(int argc, char **argv)
{
    QCommandLineParser parser;
    KPackage::PackageTool app(argc, argv, &parser);

    const QString description = i18n("KPackage Manager");
    const auto version = QStringLiteral("2.0");

    app.setApplicationVersion(version);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(description);
    parser.addOptions({
                          Options::hash,
                          Options::global,
                          Options::type,
                          Options::install,
                          Options::show,
                          Options::upgrade,
                          Options::list,
                          Options::listTypes,
                          Options::remove,
                          Options::packageRoot,
                          Options::generateIndex,
                          Options::removeIndex,
                          Options::appstream,
                          Options::appstreamOutput
                      });
    parser.process(app);

    //at least one operation should be specified
    if (!parser.isSet(QStringLiteral("hash")) && !parser.isSet(QStringLiteral("g")) &&
        !parser.isSet(QStringLiteral("i")) && !parser.isSet(QStringLiteral("s")) && !parser.isSet(QStringLiteral("appstream-metainfo")) &&
        !parser.isSet(QStringLiteral("u")) && !parser.isSet(QStringLiteral("l")) &&
        !parser.isSet(QStringLiteral("list-types")) && !parser.isSet(QStringLiteral("r")) &&
        !parser.isSet(QStringLiteral("generate-index")) && !parser.isSet(QStringLiteral("remove-index"))) {
        parser.showHelp(0);
    }
    return app.exec();
}

