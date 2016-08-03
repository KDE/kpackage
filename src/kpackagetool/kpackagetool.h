/******************************************************************************
*   Copyright 2008 Aaron Seigo <aseigo@kde.org>                               *
*   Copyright 2012 Sebastian Kügler <sebas@kde.org>                           *
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
    virtual ~PackageTool();

    void listPackages(const QStringList &types, const QString &path = QString());
    void showPackageInfo(const QString &pluginName);
    void showAppstreamInfo(const QString &pluginName);
    QString findPackageRoot(const QString &pluginName, const QString &prefix);
    void recreateIndex();

private Q_SLOTS:
    void runMain();
    void packageInstalled(KJob *job);
    void packageUninstalled(KJob *job);

private:
    PackageToolPrivate *d;
};

}

#endif
