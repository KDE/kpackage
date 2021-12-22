/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "private/packagejobthread_p.h"
#include "private/utils.h"

#include "config-package.h"
#include "package.h"

#include <KArchive>
#include <KLocalizedString>
#include <KTar>
#include <kzip.h>


#include "kpackage_debug.h"
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QMimeDatabase>
#include <QMimeType>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include <qtemporarydir.h>

namespace KPackage
{
bool copyFolder(QString sourcePath, QString targetPath)
{
    QDir source(sourcePath);
    if (!source.exists()) {
        return false;
    }

    QDir target(targetPath);
    if (!target.exists()) {
        QString targetName = target.dirName();
        target.cdUp();
        target.mkdir(targetName);
        target = QDir(targetPath);
    }

    const auto lstEntries = source.entryList(QDir::Files);
    for (const QString &fileName : lstEntries) {
        QString sourceFilePath = sourcePath + QDir::separator() + fileName;
        QString targetFilePath = targetPath + QDir::separator() + fileName;

        if (!QFile::copy(sourceFilePath, targetFilePath)) {
            return false;
        }
    }
    const auto lstEntries2 = source.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for (const QString &subFolderName : lstEntries2) {
        QString sourceSubFolderPath = sourcePath + QDir::separator() + subFolderName;
        QString targetSubFolderPath = targetPath + QDir::separator() + subFolderName;

        if (!copyFolder(sourceSubFolderPath, targetSubFolderPath)) {
            return false;
        }
    }

    return true;
}

bool removeFolder(QString folderPath)
{
    QDir folder(folderPath);
    return folder.removeRecursively();
}

Q_GLOBAL_STATIC_WITH_ARGS(QStringList, metaDataFiles, (QStringList(QLatin1String("metadata.desktop")) << QLatin1String("metadata.json")))


class PackageJobThreadPrivate
{
public:
    QString installPath;
    QString errorMessage;
    int errorCode;
};

PackageJobThread::PackageJobThread(QObject *parent)
    : QThread(parent)
{
    d = new PackageJobThreadPrivate;
    d->errorCode = KJob::NoError;
}

PackageJobThread::~PackageJobThread()
{
    delete d;
}

bool PackageJobThread::install(const QString &src, const QString &dest)
{
    bool ok = installPackage(src, dest, Install);
    Q_EMIT installPathChanged(d->installPath);
    Q_EMIT jobThreadFinished(ok, d->errorMessage);
    return ok;
}

static QString resolveHandler(const QString &scheme)
{
    QString candidatePath = QStringLiteral(KDE_INSTALL_FULL_LIBEXECDIR_KF "/kpackagehandlers/%1handler").arg(scheme);
    if (qEnvironmentVariableIsSet("KPACKAGE_DEP_RESOLVERS_PATH")) {
        candidatePath = QStringLiteral("%1/%2handler").arg(QString::fromUtf8(qgetenv("KPACKAGE_DEP_RESOLVERS_PATH")), scheme);
    }
    return QFile::exists(candidatePath) ? candidatePath : QString();
}

bool PackageJobThread::installDependency(const QUrl &destUrl)
{
    auto handler = resolveHandler(destUrl.scheme());
    if (handler.isEmpty()) {
        return false;
    }

    QProcess process;
    process.setProgram(handler);
    process.setArguments({destUrl.toString()});
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    process.start();
    process.waitForFinished();

    return process.exitCode() == 0;
}

bool PackageJobThread::installPackage(const QString &src, const QString &dest, OperationType operation)
{
    QDir root(dest);
    if (!root.exists()) {
        QDir().mkpath(dest);
        if (!root.exists()) {
            d->errorMessage = i18n("Could not create package root directory: %1", dest);
            d->errorCode = Package::JobError::RootCreationError;
            // qCWarning(KPACKAGE_LOG) << "Could not create package root directory: " << dest;
            return false;
        }
    }

    QFileInfo fileInfo(src);
    if (!fileInfo.exists()) {
        d->errorMessage = i18n("No such file: %1", src);
        d->errorCode = Package::JobError::PackageFileNotFoundError;
        return false;
    }

    QString path;
    QTemporaryDir tempdir;
    bool archivedPackage = false;

    if (fileInfo.isDir()) {
        // we have a directory, so let's just install what is in there
        path = src;
        // make sure we end in a slash!
        if (!path.endsWith(QLatin1Char('/'))) {
            path.append(QLatin1Char('/'));
        }
    } else {
        KArchive *archive = nullptr;
        QMimeDatabase db;
        QMimeType mimetype = db.mimeTypeForFile(src);
        if (mimetype.inherits(QStringLiteral("application/zip"))) {
            archive = new KZip(src);
        } else if (mimetype.inherits(QStringLiteral("application/x-compressed-tar")) || //
                   mimetype.inherits(QStringLiteral("application/x-tar")) || //
                   mimetype.inherits(QStringLiteral("application/x-bzip-compressed-tar")) || //
                   mimetype.inherits(QStringLiteral("application/x-xz")) || //
                   mimetype.inherits(QStringLiteral("application/x-lzma"))) {
            archive = new KTar(src);
        } else {
            // qCWarning(KPACKAGE_LOG) << "Could not open package file, unsupported archive format:" << src << mimetype.name();
            d->errorMessage = i18n("Could not open package file, unsupported archive format: %1 %2", src, mimetype.name());
            d->errorCode = Package::JobError::UnsupportedArchiveFormatError;
            return false;
        }

        if (!archive->open(QIODevice::ReadOnly)) {
            // qCWarning(KPACKAGE_LOG) << "Could not open package file:" << src;
            delete archive;
            d->errorMessage = i18n("Could not open package file: %1", src);
            d->errorCode = Package::JobError::PackageOpenError;
            return false;
        }

        archivedPackage = true;
        path = tempdir.path() + QLatin1Char('/');

        d->installPath = path;

        const KArchiveDirectory *source = archive->directory();
        source->copyTo(path);

        QStringList entries = source->entries();
        if (entries.count() == 1) {
            const KArchiveEntry *entry = source->entry(entries[0]);
            if (entry->isDirectory()) {
                path = path + entry->name() + QLatin1Char('/');
            }
        }

        delete archive;
    }

    QDir packageDir(path);
    QFileInfoList entries = packageDir.entryInfoList(*metaDataFiles);
    KPluginMetaData meta;
    if (!entries.isEmpty()) {
        const QString metadataFilePath = entries.first().filePath();
        if (metadataFilePath.endsWith(QLatin1String(".desktop"))) {
            meta = KPluginMetaData::fromDesktopFile(metadataFilePath, {QStringLiteral(":/kservicetypes5/kpackage-generic.desktop")});
        } else {
            QFile f(metadataFilePath);
            if (!f.open(QIODevice::ReadOnly)) {
                qCWarning(KPACKAGE_LOG) << "Couldn't open metadata file" << src << path;
                d->errorMessage = i18n("Could not open metadata file: %1", src);
                d->errorCode = Package::JobError::MetadataFileMissingError;
                return false;
            }
            QJsonObject metadataObject = QJsonDocument::fromJson(f.readAll()).object();
            meta = KPluginMetaData(metadataObject, QString(), metadataFilePath);
        }
    }

    if (!meta.isValid()) {
        qCDebug(KPACKAGE_LOG) << "No metadata file in package" << src << path;
        d->errorMessage = i18n("No metadata file in package: %1", src);
        d->errorCode = Package::JobError::MetadataFileMissingError;
        return false;
    }

    QString pluginName = meta.pluginId();
    qCDebug(KPACKAGE_LOG) << "pluginname: " << meta.pluginId();
    if (pluginName.isEmpty()) {
        // qCWarning(KPACKAGE_LOG) << "Package plugin name not specified";
        d->errorMessage = i18n("Package plugin name not specified: %1", src);
        d->errorCode = Package::JobError::PluginNameMissingError;
        return false;
    }

    // Ensure that package names are safe so package uninstall can't inject
    // bad characters into the paths used for removal.
    const QRegularExpression validatePluginName(QStringLiteral("^[\\w\\-\\.]+$")); // Only allow letters, numbers, underscore and period.
    if (!validatePluginName.match(pluginName).hasMatch()) {
        // qCDebug(KPACKAGE_LOG) << "Package plugin name " << pluginName << "contains invalid characters";
        d->errorMessage = i18n("Package plugin name %1 contains invalid characters", pluginName);
        d->errorCode = Package::JobError::PluginNameInvalidError;
        return false;
    }

    QString targetName = dest;
    if (targetName[targetName.size() - 1] != QLatin1Char('/')) {
        targetName.append(QLatin1Char('/'));
    }
    targetName.append(pluginName);

    if (QFile::exists(targetName)) {
        if (operation == Update) {
            KPluginMetaData oldMeta(targetName + QLatin1String("/metadata.desktop"));

            if (readKPackageTypes(oldMeta) != readKPackageTypes(meta)) {
                d->errorMessage = i18n("The new package has a different type from the old version already installed.");
                d->errorCode = Package::JobError::UpdatePackageTypeMismatchError;
            } else if (isVersionNewer(oldMeta.version(), meta.version())) {
                const bool ok = uninstallPackage(targetName);
                if (!ok) {
                    d->errorMessage = i18n("Impossible to remove the old installation of %1 located at %2. error: %3", pluginName, targetName, d->errorMessage);
                    d->errorCode = Package::JobError::OldVersionRemovalError;
                }
            } else {
                d->errorMessage = i18n("Not installing version %1 of %2. Version %3 already installed.", meta.version(), meta.pluginId(), oldMeta.version());
                d->errorCode = Package::JobError::NewerVersionAlreadyInstalledError;
            }
        } else {
            d->errorMessage = i18n("%1 already exists", targetName);
            d->errorCode = Package::JobError::PackageAlreadyInstalledError;
        }

        if (d->errorCode != KJob::NoError) {
            d->installPath = targetName;
            return false;
        }
    }

    // install dependencies
    const QStringList dependencies = meta.value(QStringLiteral("X-KPackage-Dependencies"), QStringList());
    for (const QString &dep : dependencies) {
        QUrl depUrl(dep);
        if (!installDependency(depUrl)) {
            d->errorMessage = i18n("Could not install dependency: '%1'", dep);
            d->errorCode = Package::JobError::PackageCopyError;
            return false;
        }
    }

    if (archivedPackage) {
        // it's in a temp dir, so just move it over.
        const bool ok = copyFolder(path, targetName);
        removeFolder(path);
        if (!ok) {
            // qCWarning(KPACKAGE_LOG) << "Could not move package to destination:" << targetName;
            d->errorMessage = i18n("Could not move package to destination: %1", targetName);
            d->errorCode = Package::JobError::PackageMoveError;
            return false;
        }
    } else {
        // it's a directory containing the stuff, so copy the contents rather
        // than move them
        const bool ok = copyFolder(path, targetName);
        if (!ok) {
            // qCWarning(KPACKAGE_LOG) << "Could not copy package to destination:" << targetName;
            d->errorMessage = i18n("Could not copy package to destination: %1", targetName);
            d->errorCode = Package::JobError::PackageCopyError;
            return false;
        }
    }

    if (archivedPackage) {
        // no need to remove the temp dir (which has been successfully moved if it's an archive)
        tempdir.setAutoRemove(false);
    }

    d->installPath = targetName;
    return true;
}

bool PackageJobThread::update(const QString &src, const QString &dest)
{
    bool ok = installPackage(src, dest, Update);
    Q_EMIT installPathChanged(d->installPath);
    Q_EMIT jobThreadFinished(ok, d->errorMessage);
    return ok;
}

bool PackageJobThread::uninstall(const QString &packagePath)
{
    bool ok = uninstallPackage(packagePath);
    // qCDebug(KPACKAGE_LOG) << "emit installPathChanged " << d->installPath;
    Q_EMIT installPathChanged(QString());
    // qCDebug(KPACKAGE_LOG) << "Thread: installFinished" << ok;
    Q_EMIT jobThreadFinished(ok, d->errorMessage);
    return ok;
}

bool PackageJobThread::uninstallPackage(const QString &packagePath)
{
    if (!QFile::exists(packagePath)) {
        d->errorMessage = packagePath.isEmpty() ? i18n("package path was deleted manually") : i18n("%1 does not exist", packagePath);
        d->errorCode = Package::JobError::PackageFileNotFoundError;
        return false;
    }
    QString pkg;
    QString root;
    {
        // TODO KF6 remove, pass in packageroot, type and pluginName separately?
        QStringList ps = packagePath.split(QLatin1Char('/'));
        int ix = ps.count() - 1;
        if (packagePath.endsWith(QLatin1Char('/'))) {
            ix = ps.count() - 2;
        }
        pkg = ps[ix];
        ps.pop_back();
        root = ps.join(QLatin1Char('/'));
    }

    bool ok = removeFolder(packagePath);
    if (!ok) {
        d->errorMessage = i18n("Could not delete package from: %1", packagePath);
        d->errorCode = Package::JobError::PackageUninstallError;
        return false;
    }

    return true;
}

Package::JobError PackageJobThread::errorCode() const
{
    return static_cast<Package::JobError>(d->errorCode);
}

} // namespace KPackage

#include "moc_packagejobthread_p.cpp"
