/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGE_PACKAGEJOB_H
#define KPACKAGE_PACKAGEJOB_H

#include <kpackage/package_export.h>

#include <KJob>
#include <memory>

namespace KPackage
{
class PackageJobPrivate;
class Package;

class KPACKAGE_EXPORT PackageJob : public KJob
{
    Q_OBJECT

public:
    ~PackageJob() override;
    static PackageJob *install(Package *package, const QString &src, const QString &dest);
    static PackageJob *update(Package *package, const QString &src, const QString &dest);
    static PackageJob *uninstall(Package *package, const QString &packagePath);

Q_SIGNALS:
    void installPathChanged(const QString &path);

private:
    friend class PackageJobThread;
    enum OperationType {
        Install,
        Update,
        Uninstall,
    };
    void start() override;
    explicit PackageJob(OperationType type, Package *package, const QString &src, const QString &dest, const QString &packagePath = {});
    void setupNotificationsOnJobFinished(const QString &messageName);

    const std::unique_ptr<PackageJobPrivate> d;
};

}

#endif
