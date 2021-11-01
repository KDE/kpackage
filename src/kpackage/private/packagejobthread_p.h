/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGE_PACKAGEJOBTHREAD_P_H
#define KPACKAGE_PACKAGEJOBTHREAD_P_H

#include "package.h"
#include <QThread>

namespace KPackage
{
class PackageJobThreadPrivate;

bool indexDirectory(const QString &dir, const QString &dest);

// true if version2 is more recent than version1
// TODO: replace with QVersionNumber when we will be able to depend from Qt 5.6
bool isVersionNewer(const QString &version1, const QString &version2);

class PackageJobThread : public QThread
{
    Q_OBJECT

public:
    enum OperationType {
        Install,
        Update,
    };

    explicit PackageJobThread(QObject *parent = nullptr);
    ~PackageJobThread() override;

    bool install(const QString &src, const QString &dest);
    bool update(const QString &src, const QString &dest);
    bool uninstall(const QString &packagePath);

    Package::JobError errorCode() const;

Q_SIGNALS:
    void jobThreadFinished(bool success, const QString &errorMessage = QString());
    void percentChanged(int percent);
    void error(const QString &errorMessage);
    void installPathChanged(const QString &installPath);

private:
    // OperationType says whether we want to install, update or any
    // new similar operation it will be expanded
    bool installDependency(const QUrl &src);
    bool installPackage(const QString &src, const QString &dest, OperationType operation);
    bool uninstallPackage(const QString &packagePath);
    PackageJobThreadPrivate *d;
};

}

#endif
