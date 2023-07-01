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

namespace KPackage
{
class PackageJobPrivate
{
public:
    PackageJobThread *thread = nullptr;
    Package *package = nullptr;
    QString installPath;
};

PackageJob::PackageJob(Package *package, QObject *parent)
    : KJob(parent)
    , d(new PackageJobPrivate)
{
    d->thread = new PackageJobThread(this);
    d->package = package;

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

void PackageJob::slotFinished(bool ok, const QString &err)
{
    if (ok) {
        setError(NoError);
    } else {
        setError(d->thread->errorCode());
        setErrorText(err);
    }
    d->thread->exit(0);
    emitResult();
}

void PackageJob::start()
{
    d->thread->start();
}

void PackageJob::install(const QString &src, const QString &dest)
{
    setupNotificationsOnJobFinished(QStringLiteral("packageInstalled"));
    d->thread->install(src, dest);
}

void PackageJob::update(const QString &src, const QString &dest)
{
    setupNotificationsOnJobFinished(QStringLiteral("packageUpdated"));
    d->thread->update(src, dest);
}

void PackageJob::uninstall(const QString &installationPath)
{
    setupNotificationsOnJobFinished(QStringLiteral("packageUninstalled"));
    d->thread->uninstall(installationPath);
}

void PackageJob::setupNotificationsOnJobFinished(const QString &messageName)
{
    // capture first as uninstalling wipes d->package
    // or d-package can become dangling during the job if deleted externally
    const QString pluginId = d->package->metadata().pluginId();
    const QStringList serviceTypes = readKPackageTypes(d->package->metadata());

    auto onJobFinished = [=](bool ok, const QString &error) {
        if (ok) {
            for (auto &packageType : serviceTypes) {
                auto msg = QDBusMessage::createSignal(QStringLiteral("/KPackage/") + packageType, QStringLiteral("org.kde.plasma.kpackage"), messageName);
                msg.setArguments({pluginId});
                QDBusConnection::sessionBus().send(msg);
            }
        }
        slotFinished(ok, error);
    };
    connect(d->thread, &PackageJobThread::jobThreadFinished, this, onJobFinished, Qt::QueuedConnection);
}

} // namespace KPackage

#include "moc_packagejob_p.cpp"
