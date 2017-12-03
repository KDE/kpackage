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

#include "private/packagejobthread_p.h"

#include "package.h"
#include "config-package.h"

#include <karchive.h>
#include <klocalizedstring.h>
#include <ktar.h>
#include <kzip.h>


#include <kcompressiondevice.h>

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QMimeType>
#include <QMimeDatabase>
#include <QRegExp>
#include <QJsonArray>
#include <QDirIterator>
#include <QJsonDocument>
#include <qtemporarydir.h>
#include <QProcess>
#include <QUrl>
#include <QStandardPaths>
#include <QDebug>

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

    foreach (const QString &fileName, source.entryList(QDir::Files)) {
        QString sourceFilePath = sourcePath + QDir::separator() + fileName;
        QString targetFilePath = targetPath + QDir::separator() + fileName;

        if (!QFile::copy(sourceFilePath, targetFilePath)) {
            return false;
        }
    }

    foreach (const QString &subFolderName, source.entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
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

bool removeIndex(const QString& dir)
{
    bool ok = true;
    QFileInfo fileInfo(dir, QStringLiteral("kpluginindex.json"));
    if (fileInfo.exists()) {
        if (fileInfo.isWritable()) {
            QFile f(fileInfo.absoluteFilePath());
            if (!f.remove()) {
                ok = false;
                qWarning() << "Cannot remove kplugin index file: " << fileInfo.absoluteFilePath();
            } else {
                qDebug() << "Deleted index: " << fileInfo.absoluteFilePath();
            }
        } else {
            qWarning() << "Cannot remove kplugin index file (not writable): " << fileInfo.absoluteFilePath();
            ok = false;
        }
    }
    return ok;
}

Q_GLOBAL_STATIC_WITH_ARGS(QStringList, metaDataFiles, (QStringList(QLatin1String("metadata.desktop")) << QLatin1String("metadata.json")))

bool indexDirectory(const QString& dir, const QString& dest)
{
    QVariantMap vm;
    QVariantMap pluginsVm;
    vm[QStringLiteral("Version")] = QStringLiteral("1.0");
    vm[QStringLiteral("Timestamp")] = QDateTime::currentMSecsSinceEpoch();

    QJsonArray plugins;

    QDirIterator it(dir, *metaDataFiles, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QString path = it.fileInfo().absoluteFilePath();
        QJsonObject obj = KPluginMetaData(path).rawData();
        obj.insert(QStringLiteral("FileName"), path);

        plugins.append(obj);
    }

    // Less than two plugin means it's not worth indexing
    if (plugins.count() < 2) {
        removeIndex(dir);
        return true;
    }

    QString destfile = dest;
    if (!QDir::isAbsolutePath(dest)) {
        destfile = dir + QLatin1Char('/') + dest;
    }

    QDir().mkpath(QFileInfo(destfile).dir().absolutePath());
    //QFile file(destfile);
    KCompressionDevice file(destfile, KCompressionDevice::BZip2);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open " << destfile;
        return false;
    }

    QJsonDocument jdoc;
    jdoc.setArray(plugins);
//     file.write(jdoc.toJson());
    file.write(jdoc.toBinaryData());
    qWarning() << "Generated " << destfile << " (" << plugins.count() << " plugins)";

    return true;
}

class PackageJobThreadPrivate
{
public:
    QString installPath;
    QString errorMessage;
    int errorCode;
};

PackageJobThread::PackageJobThread(QObject *parent) :
    QThread(parent)
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
    emit installPathChanged(d->installPath);
    emit finished(ok, d->errorMessage);
    return ok;
}

static QString resolveHandler(const QString &scheme)
{
    QString candidatePath = QStringLiteral(CMAKE_INSTALL_FULL_LIBEXECDIR_KF5 "/kpackagehandlers/%1handler").arg(scheme);
    if (qEnvironmentVariableIsSet("KPACKAGE_DEP_RESOLVERS_PATH")) {
        candidatePath = QStringLiteral("%1/%2handler").arg(QString::fromUtf8(qgetenv("KPACKAGE_DEP_RESOLVERS_PATH")), scheme);
    }
    return QFile::exists(candidatePath) ? candidatePath : QString();
}

bool PackageJobThread::installDependency(const QUrl &destUrl)
{
    auto handler = resolveHandler(destUrl.scheme());
    if (handler.isEmpty())
        return false;

    QProcess process;
    process.setProgram(handler);
    process.setArguments({ destUrl.toString() });
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
            //qWarning() << "Could not create package root directory: " << dest;
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
        } else if (mimetype.inherits(QStringLiteral("application/x-compressed-tar")) ||
                   mimetype.inherits(QStringLiteral("application/x-tar")) ||
                   mimetype.inherits(QStringLiteral("application/x-bzip-compressed-tar")) ||
                   mimetype.inherits(QStringLiteral("application/x-xz")) ||
                   mimetype.inherits(QStringLiteral("application/x-lzma"))) {
            archive = new KTar(src);
        } else {
            //qWarning() << "Could not open package file, unsupported archive format:" << src << mimetype.name();
            d->errorMessage = i18n("Could not open package file, unsupported archive format: %1 %2", src, mimetype.name());
            d->errorCode = Package::JobError::UnsupportedArchiveFormatError;
            return false;
        }

        if (!archive->open(QIODevice::ReadOnly)) {
            //qWarning() << "Could not open package file:" << src;
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
            if(!f.open(QIODevice::ReadOnly)){
                qWarning() << "Couldn't open metadata file" << src << path;
                d->errorMessage = i18n("Could not open metadata file: %1", src);
                d->errorCode = Package::JobError::MetadataFileMissingError;
                return false;
            }
            QJsonObject metadataObject = QJsonDocument::fromJson(f.readAll()).object();
            meta = KPluginMetaData(metadataObject, QString(), metadataFilePath);
        }
    }

    if (!meta.isValid()) {
        qDebug() << "No metadata file in package" << src << path;
        d->errorMessage = i18n("No metadata file in package: %1", src);
        d->errorCode = Package::JobError::MetadataFileMissingError;
        return false;
    }


    QString pluginName = meta.pluginId();
    qDebug() << "pluginname: " << meta.pluginId();
    if (pluginName.isEmpty()) {
        //qWarning() << "Package plugin name not specified";
        d->errorMessage = i18n("Package plugin name not specified: %1", src);
        d->errorCode = Package::JobError::PluginNameMissingError;
        return false;
    }

    // Ensure that package names are safe so package uninstall can't inject
    // bad characters into the paths used for removal.
    QRegExp validatePluginName(QStringLiteral("^[\\w-\\.]+$")); // Only allow letters, numbers, underscore and period.
    if (!validatePluginName.exactMatch(pluginName)) {
        //qDebug() << "Package plugin name " << pluginName << "contains invalid characters";
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

            if (oldMeta.serviceTypes() != meta.serviceTypes()) {
                d->errorMessage = i18n("The new package has a different type from the old version already installed.", meta.version(), meta.pluginId(), oldMeta.version());
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

    //install dependencies
    const QStringList dependencies = KPluginMetaData::readStringList(meta.rawData(), QStringLiteral("X-KPackage-Dependencies"));
    for(const QString &dep : dependencies) {
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
            //qWarning() << "Could not move package to destination:" << targetName;
            d->errorMessage = i18n("Could not move package to destination: %1", targetName);
            d->errorCode = Package::JobError::PackageMoveError;
            return false;
        }
    } else {
        // it's a directory containing the stuff, so copy the contents rather
        // than move them
        const bool ok = copyFolder(path, targetName);
        if (!ok) {
            //qWarning() << "Could not copy package to destination:" << targetName;
            d->errorMessage = i18n("Could not copy package to destination: %1", targetName);
            d->errorCode = Package::JobError::PackageCopyError;
            return false;
        }
    }

    if (archivedPackage) {
        // no need to remove the temp dir (which has been successfully moved if it's an archive)
        tempdir.setAutoRemove(false);
    }

    indexDirectory(dest, QStringLiteral("kpluginindex.json"));
    d->installPath = targetName;
    return true;
}

bool PackageJobThread::update(const QString &src, const QString &dest)
{
    bool ok = installPackage(src, dest, Update);
    emit installPathChanged(d->installPath);
    emit finished(ok, d->errorMessage);
    return ok;
}

bool PackageJobThread::uninstall(const QString &packagePath)
{
    bool ok = uninstallPackage(packagePath);
    //qDebug() << "emit installPathChanged " << d->installPath;
    emit installPathChanged(QString());
    //qDebug() << "Thread: installFinished" << ok;
    emit finished(ok, d->errorMessage);
    return ok;
}

bool PackageJobThread::uninstallPackage(const QString &packagePath)
{
    if (!QFile::exists(packagePath)) {
        d->errorMessage = i18n("%1 does not exist", packagePath);
        d->errorCode = Package::JobError::PackageFileNotFoundError;
        return false;
    }
    QString pkg;
    QString root;
    {
        // FIXME: remove, pass in packageroot, type and pluginName separately?
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

    indexDirectory(root, QStringLiteral("kpluginindex.json"));

    return true;
}

Package::JobError PackageJobThread::errorCode() const
{
    return (Package::JobError)d->errorCode;
}

} // namespace KPackage

#include "moc_packagejobthread_p.cpp"

