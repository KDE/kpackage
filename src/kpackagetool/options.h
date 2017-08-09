#ifndef OPTIONS_H
#define OPTIONS_H

#include <QCommandLineOption>

namespace Options
{
    static QCommandLineOption hash {
        QStringLiteral("hash"), i18nc("Do not translate <path>", "Generate a SHA1 hash for the package at <path>"), QStringLiteral("path")
    };
    static QCommandLineOption global {
        QStringList { QStringLiteral("g"), QStringLiteral("global") },
        i18n("For install or remove, operates on packages installed for all users.")
    };
    static QCommandLineOption type {
        QStringList { QStringLiteral("t"), QStringLiteral("type") },
        i18nc("theme, wallpaper, etc. are keywords, but they may be translated, as both versions "
              "are recognized by the application "
              "(if translated, should be same as messages with 'package type' context below)",
              "The type of package, corresponding to the service type of the package plugin, e.g. KPackage/Generic, Plasma/Theme, Plasma/Wallpaper, Plasma/Applet, etc."),
        QStringLiteral("type"), QStringLiteral("KPackage/Generic")
    };
    static QCommandLineOption install {
        QStringList { QStringLiteral("i"), QStringLiteral("install") },
        i18nc("Do not translate <path>", "Install the package at <path>"),
        QStringLiteral("path")
    };
    static QCommandLineOption show {
        QStringList { QStringLiteral("s"), QStringLiteral("show") },
        i18nc("Do not translate <name>", "Show information of package <name>"),
        QStringLiteral("name")
    };
    static QCommandLineOption upgrade {
        QStringList { QStringLiteral("u"), QStringLiteral("upgrade") },
        i18nc("Do not translate <path>", "Upgrade the package at <path>"),
        QStringLiteral("path")
    };
    static QCommandLineOption list {
        QStringList { QStringLiteral("l"), QStringLiteral("list") },
        i18n("List installed packages")
    };
    static QCommandLineOption listTypes {
        QStringList { QStringLiteral("list-types") },
        i18n("List all known package types that can be installed")
    };
    static QCommandLineOption remove {
        QStringList { QStringLiteral("r"), QStringLiteral("remove") },
        i18nc("Do not translate <name>", "Remove the package named <name>"),
        QStringLiteral("name")
    };
    static QCommandLineOption packageRoot {
        QStringList { QStringLiteral("p"), QStringLiteral("packageroot") },
        i18n("Absolute path to the package root. If not supplied, then the standard data"
             " directories for this KDE session will be searched instead."),
        QStringLiteral("path")
    };
    static QCommandLineOption generateIndex {
        QStringLiteral("generate-index"),
        i18n("Recreate the plugin index. To be used in: conjunction with either the option -t or"
             " -g. Recreates the index for the given type or package root. Operates in the user"
             " directory, unless -g is used")
    };
    static QCommandLineOption appstream {
        QStringLiteral("appstream-metainfo"),
        i18nc("Do not translate <path>", "Outputs the metadata for the package <path>"),
        QStringLiteral("path")
    };
    static QCommandLineOption appstreamOutput {
        QStringLiteral("appstream-metainfo-output"),
        i18nc("Do not translate <path>", "Outputs the metadata for the package into <path>"),
        QStringLiteral("path")
    };
}

#endif // OPTIONS_H
