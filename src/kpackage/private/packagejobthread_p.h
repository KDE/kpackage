/******************************************************************************
*   Copyright 2007-2009 by Aaron Seigo <aseigo@kde.org>                       *
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

#ifndef KPACKAGE_PACKAGEJOBTHREAD_P_H
#define KPACKAGE_PACKAGEJOBTHREAD_P_H

#include "kjob.h"
#include "package.h"
#include <QThread>

const static auto s_kpluginindex = QStringLiteral("kpluginindex.json");

namespace KPackage
{

class PackageJobThreadPrivate;

bool indexDirectory(const QString& dir, const QString& dest);

//true if version2 is more recent than version1
//TODO: replace with QVersionNumber when we will be able to depend from Qt 5.6
bool isVersionNewer(const QString &version1, const QString &version2);


class PackageJobThread : public QThread
{
    Q_OBJECT

public:
    enum OperationType {
        Install,
        Update
    };

    explicit PackageJobThread(QObject *parent = nullptr);
    virtual ~PackageJobThread();

    bool install(const QString &src, const QString &dest);
    bool update(const QString &src, const QString &dest);
    bool uninstall(const QString &packagePath);

    Package::JobError errorCode() const;

Q_SIGNALS:
    void finished(bool success, const QString &errorMessage = QString());
    void percentChanged(int percent);
    void error(const QString &errorMessage);
    void installPathChanged(const QString &installPath);

private:
    //OperationType says wether we want to install, update or any
    //new similar operation it will be expanded
    bool installDependency(const QUrl &src);
    bool installPackage(const QString &src, const QString &dest, OperationType operation);
    bool uninstallPackage(const QString &packagePath);
    PackageJobThreadPrivate *d;
};

}

#endif
