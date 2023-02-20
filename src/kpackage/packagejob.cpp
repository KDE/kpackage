/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packagejob.h"

#include "config-package.h"
#include "packageloader.h"
#include "packagestructure.h"
#include "private/package_p.h"
#include "private/packagejobthread_p.h"
#include "private/utils.h"

#include "kpackage_debug.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QStandardPaths>
#include <QThreadPool>
#include <QTimer>

namespace KPackage
{
class PackageJobPrivate
{
public:
    PackageJobThread *thread = nullptr;
    Package package;
    QString installPath;
};

PackageJob::PackageJob(OperationType type, Package &package, const QString &src, const QString &dest)
    : KJob()
    , d(new PackageJobPrivate)
{
    d->thread = new PackageJobThread(type, src, dest, package.path());
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
    connect(d->thread, &PackageJobThread::installPathChanged, this, [this](const QString &installPath) {
        d->package.setPath(installPath);
    });
    connect(d->thread, &PackageJobThread::jobThreadFinished, this, [this]() {
        Q_EMIT operationFinished(d->package);
    });
}

PackageJob::~PackageJob() = default;

void PackageJob::start()
{
    Q_ASSERT(d->thread);
    QThreadPool::globalInstance()->start(d->thread);
    d->thread = nullptr;
}

PackageJob *PackageJob::install(PackageStructure *structure, const QString &sourcePackage, const QString &packageRoot)
{
    Package package(structure);
    package.setPath(sourcePackage);
    QString dest = packageRoot.isEmpty() ? package.defaultPackageRoot() : packageRoot;
    PackageLoader::invalidateCache();

    // use absolute paths if passed, otherwise go under share
    if (!QDir::isAbsolutePath(dest)) {
        dest = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + dest;
    }
    auto job = new PackageJob(Install, package, sourcePackage, dest);
    job->start();
    return job;
}

PackageJob *PackageJob::update(PackageStructure *structure, const QString &sourcePackage, const QString &packageRoot)
{
    Package package(structure);
    package.setPath(sourcePackage);
    QString dest = packageRoot.isEmpty() ? package.defaultPackageRoot() : packageRoot;
    PackageLoader::invalidateCache();

    // use absolute paths if passed, otherwise go under share
    if (!QDir::isAbsolutePath(dest)) {
        dest = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + dest;
    }
    auto job = new PackageJob(Update, package, sourcePackage, dest);
    job->start();
    return job;
}

PackageJob *PackageJob::uninstall(PackageStructure *structure, const QString &pluginId, const QString &packageRoot)
{
    Package package(structure);
    QString uninstallPath;
    // We handle the empty path when uninstalling the package
    // If the dir already got deleted the pluginId is an empty string, without this
    // check we would delete the package root, BUG: 410682
    if (!pluginId.isEmpty()) {
        uninstallPath = packageRoot + QLatin1Char('/') + pluginId;
    }
    package.setPath(uninstallPath);

    PackageLoader::invalidateCache();
    auto job = new PackageJob(Uninstall, package, QString(), QString());
    job->start();
    return job;
}

void PackageJob::setupNotificationsOnJobFinished(const QString &messageName)
{
    // capture first as uninstalling wipes d->package
    // or d-package can become dangling during the job if deleted externally
    const QString pluginId = d->package.metadata().pluginId();
    const QString kpackageType = readKPackageType(d->package.metadata());

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
