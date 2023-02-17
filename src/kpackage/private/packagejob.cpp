/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-package.h"
#include "packagejob_p.h"
#include "packagejobthread_p.h"

#include "package_p.h"
#include "utils.h"

#include "kpackage_debug.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QThreadPool>
#include <QTimer>

namespace KPackage
{
class PackageJobPrivate
{
public:
    PackageJobThread *thread = nullptr;
    Package *package = nullptr;
    QString installPath;
};

PackageJob::PackageJob(OperationType type, Package *package, const QString &src, const QString &dest, const QString &packagePath)
    : KJob()
    , d(new PackageJobPrivate)
{
    d->thread = new PackageJobThread(type, src, dest, packagePath);
    d->package = package;

    if (type == Install) {
        setupNotificationsOnJobFinished(QStringLiteral("packageInstalled"));
    } else if (type == Update) {
        setupNotificationsOnJobFinished(QStringLiteral("packageUpdated"));
        d->thread->update(src, dest);
    } else if (type == Uninstall) {
        setupNotificationsOnJobFinished(QStringLiteral("packageUninstalled"));
    } else {
        Q_UNREACHABLE();
    }

    connect(PackageDeletionNotifier::self(), &PackageDeletionNotifier::packageDeleted, this, [this](Package *package) {
        if (package == d->package) {
            d->package = nullptr;
        }
    });

    connect(
        d->thread,
        &PackageJobThread::installPathChanged,
        this,
        [this](const QString &installPath) {
            if (d->package) {
                d->package->setPath(installPath);
            }
            Q_EMIT installPathChanged(installPath);
        },
        Qt::QueuedConnection);
}

PackageJob::~PackageJob()
{
    delete d;
}

void PackageJob::start()
{
    Q_ASSERT(d->thread);
    QThreadPool::globalInstance()->start(d->thread);
    d->thread = nullptr;
}

PackageJob *PackageJob::install(Package *package, const QString &src, const QString &dest)
{
    auto job = new PackageJob(Install, package, src, dest);
    job->start();
    return job;
}

PackageJob *PackageJob::update(Package *package, const QString &src, const QString &dest)
{
    auto job = new PackageJob(Update, package, src, dest);
    job->start();
    return job;
}

PackageJob *PackageJob::uninstall(Package *package, const QString &packagePath)
{
    auto job = new PackageJob(Uninstall, package, QString(), QString(), packagePath);
    job->start();
    return job;
}

void PackageJob::setupNotificationsOnJobFinished(const QString &messageName)
{
    // capture first as uninstalling wipes d->package
    // or d-package can become dangling during the job if deleted externally
    const QString pluginId = d->package->metadata().pluginId();
    const QString kpackageType = readKPackageType(d->package->metadata());

    auto onJobFinished = [=](bool ok, Package::JobError errorCode, const QString &error) {
        if (ok) {
            auto msg = QDBusMessage::createSignal(QStringLiteral("/KPackage/") + kpackageType, QStringLiteral("org.kde.plasma.kpackage"), messageName);
            msg.setArguments({pluginId});
            QDBusConnection::sessionBus().send(msg);
        }

        if (ok) {
            setError(NoError);
        } else {
            setError(errorCode);
            setErrorText(error);
        }
        emitResult();
    };
    connect(d->thread, &PackageJobThread::jobThreadFinished, this, onJobFinished, Qt::QueuedConnection);
}

} // namespace KPackage
