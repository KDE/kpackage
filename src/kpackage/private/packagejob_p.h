/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGE_PACKAGEJOB_P_H
#define KPACKAGE_PACKAGEJOB_P_H

#include <KJob>

namespace KPackage
{
class PackageJobPrivate;
class Package;

class PackageJob : public KJob
{
    Q_OBJECT

public:
    PackageJob(Package *package, QObject *parent = nullptr);
    ~PackageJob() override;

    void start() override;

    void install(const QString &src, const QString &dest);
    void update(const QString &src, const QString &dest);
    void uninstall(const QString &installationPath);

Q_SIGNALS:
    void installPathChanged(const QString &path);

private Q_SLOTS:
    void slotFinished(bool ok, const QString &err);

private:
    void setupNotificationsOnJobFinished(const QString &messageName);

    PackageJobPrivate *const d;
};

}

#endif
