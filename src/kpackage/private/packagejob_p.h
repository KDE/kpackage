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
    enum OperationType {
        Install,
        Update,
        Uninstall,
    };
    ~PackageJob() override;

    void start() override;

    static PackageJob *install(Package *package, const QString &src, const QString &dest);
    static PackageJob *update(Package *package, const QString &src, const QString &dest);
    static PackageJob *uninstall(Package *package, const QString &packagePath);

Q_SIGNALS:
    void installPathChanged(const QString &path);

private:
    explicit PackageJob(OperationType type, Package *package, const QString &src, const QString &dest, const QString &packagePath = {});
    void setupNotificationsOnJobFinished(const QString &messageName);

    PackageJobPrivate *const d;
};

}

#endif
