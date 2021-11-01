/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PACKAGETOOL_H
#define PACKAGETOOL_H

#include <QCoreApplication>

class QCommandLineParser;
class KJob;

namespace KPackage
{
class PackageToolPrivate;

class PackageTool : public QCoreApplication
{
    Q_OBJECT

public:
    PackageTool(int &argc, char **argv, QCommandLineParser *parser);
    ~PackageTool() override;

    void listPackages(const QStringList &types, const QString &path = QString());
    void showPackageInfo(const QString &pluginName);
    void showAppstreamInfo(const QString &pluginName);
    QString findPackageRoot(const QString &pluginName, const QString &prefix);

private Q_SLOTS:
    void runMain();
    void packageInstalled(KJob *job);
    void packageUninstalled(KJob *job);

private:
    PackageToolPrivate *d;
};

}

#endif
