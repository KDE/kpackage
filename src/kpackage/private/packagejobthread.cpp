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
#include <kdesktopfile.h>
#include <klocalizedstring.h>
#include <ktar.h>
#include <kzip.h>
#include <KConfigGroup>

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

// Qt5 TODO: use QDir::removeRecursively() instead
bool removeFolder(QString folderPath)
{
    QDir folder(folderPath);
    if (!folder.exists()) {
        return false;
    }

    foreach (const QString &fileName, folder.entryList(QDir::Files)) {
        if (!QFile::remove(folderPath + QDir::separator() + fileName)) {
            return false;
        }
    }

    foreach (const QString &subFolderName, folder.entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        if (!removeFolder(folderPath + QDir::separator() + subFolderName)) {
            return false;
        }
    }

    QString folderName = folder.dirName();
    folder.cdUp();
    return folder.rmdir(folderName);
}


QVariantMap convert(const QString &src)
{
    KDesktopFile df(src);
    KConfigGroup c = df.desktopGroup();

    static const QSet<QString> boolkeys = QSet<QString>()
                                          << QStringLiteral("Hidden") << QStringLiteral("X-KDE-PluginInfo-EnabledByDefault");
    static const QSet<QString> stringlistkeys = QSet<QString>()
            << QStringLiteral("X-KDE-ServiceTypes") << QStringLiteral("X-KDE-PluginInfo-Depends")
            << QStringLiteral("X-Plasma-Provides");

    QVariantMap vm;
    foreach (const QString &k, c.keyList()) {
        if (boolkeys.contains(k)) {
            vm[k] = c.readEntry(k, false);
        } else if (stringlistkeys.contains(k)) {
            vm[k] = c.readEntry(k, QStringList());
        } else {
            vm[k] = c.readEntry(k, QString());
        }
    }

    return vm;
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
                //qDebug() << "Deleted index: " << fileInfo.absoluteFilePath();
            }
        } else {
            qWarning() << "Cannot remove kplugin index file (not writable): " << fileInfo.absoluteFilePath();
            ok = false;
        }
    }
    return ok;
}

bool indexDirectory(const QString& dir, const QString& dest)
{
    QVariantMap vm;
    QVariantMap pluginsVm;
    vm[QStringLiteral("Version")] = QStringLiteral("1.0");
    vm[QStringLiteral("Timestamp")] = QDateTime::currentMSecsSinceEpoch();

    QJsonArray plugins;

    int i = 0;
    QDirIterator it(dir, QStringList()<<QStringLiteral("*.desktop"), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QString _f = it.fileInfo().absoluteFilePath();

        QJsonObject obj;
        obj["FileName"] = _f;

        if (_f.endsWith(".desktop")) {
            obj["KPlugin"] = QJsonObject::fromVariantMap(convert(_f));
        }
        plugins.insert(i, obj);
        i++;
    }


    // Less than two plugin means it's not worth indexing
    if (plugins.count() < 2) {
        removeIndex(dir);
        return true;
    }

    QJsonDocument jdoc;
    jdoc.setArray(plugins);

    QString destfile = dest;
    const QFileInfo fi(dest);
    if (!fi.isAbsolute()) {
        destfile = dir + dest;
    }
    QFile file(destfile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open " << destfile;
        return false;
    }

//     file.write(jdoc.toJson());
    file.write(jdoc.toBinaryData());
    qWarning() << "Generated " << destfile << " (" << i << " plugins)";

    return true;
}



class PackageJobThreadPrivate
{
public:
    QString installPath;
    QString errorMessage;
    //TODO: remove
    QString servicePrefix;
};

PackageJobThread::PackageJobThread(const QString &servicePrefix, QObject *parent) :
    QThread(parent)
{
    d = new PackageJobThreadPrivate;
    d->servicePrefix = servicePrefix;
}

PackageJobThread::~PackageJobThread()
{
    delete d;
}

bool PackageJobThread::install(const QString &src, const QString &dest)
{
    bool ok = installPackage(src, dest);
    emit installPathChanged(d->installPath);
    emit finished(ok, d->errorMessage);
    return ok;
}

bool PackageJobThread::installPackage(const QString &src, const QString &dest)
{
    QString packageRoot = dest;

    QDir root(dest);
    if (!root.exists()) {
        QDir().mkpath(dest);
        if (!root.exists()) {
            d->errorMessage = i18n("Could not create package root directory: %1", dest);
            //qWarning() << "Could not create package root directory: " << dest;
            return false;
        }
    }

    QFileInfo fileInfo(src);
    if (!fileInfo.exists()) {
        d->errorMessage = i18n("No such file: %1", src);
        return false;
    }

    QString path;
    QTemporaryDir tempdir;
    bool archivedPackage = false;

    if (fileInfo.isDir()) {
        // we have a directory, so let's just install what is in there
        path = src;
        // make sure we end in a slash!
        if (path[path.size() - 1] != '/') {
            path.append('/');
        }
    } else {
        KArchive *archive = 0;
        QMimeDatabase db;
        QMimeType mimetype = db.mimeTypeForFile(src);
        if (mimetype.inherits("application/zip")) {
            archive = new KZip(src);
        } else if (mimetype.inherits("application/x-compressed-tar") ||
                   mimetype.inherits("application/x-tar") || mimetype.inherits("application/x-bzip-compressed-tar") ||
                   mimetype.inherits("application/x-xz") || mimetype.inherits("application/x-lzma")) {
            archive = new KTar(src);
        } else {
            //qWarning() << "Could not open package file, unsupported archive format:" << src << mimetype.name();
            d->errorMessage = i18n("Could not open package file, unsupported archive format: %1 %2", src, mimetype.name());
            return false;
        }

        if (!archive->open(QIODevice::ReadOnly)) {
            //qWarning() << "Could not open package file:" << src;
            delete archive;
            d->errorMessage = i18n("Could not open package file: %1", src);
            return false;
        }

        archivedPackage = true;
        path = tempdir.path() + '/';

        d->installPath = path;

        const KArchiveDirectory *source = archive->directory();
        source->copyTo(path);

        QStringList entries = source->entries();
        if (entries.count() == 1) {
            const KArchiveEntry *entry = source->entry(entries[0]);
            if (entry->isDirectory()) {
                path.append(entry->name()).append("/");
            }
        }

        delete archive;
    }

    QString metadataPath = path + "metadata.desktop";
    if (!QFile::exists(metadataPath)) {
        qDebug() << "No metadata file in package" << src << metadataPath;
        d->errorMessage = i18n("No metadata file in package: %1", src);
        return false;
    }

    KPluginMetaData meta(metadataPath);


    QString pluginName = meta.pluginId();
    qDebug() << "pluginname: " << meta.pluginId();
    if (pluginName.isEmpty()) {
        //qWarning() << "Package plugin name not specified";
        d->errorMessage = i18n("Package plugin name not specified: %1", src);
        return false;
    }

    // Ensure that package names are safe so package uninstall can't inject
    // bad characters into the paths used for removal.
    QRegExp validatePluginName("^[\\w-\\.]+$"); // Only allow letters, numbers, underscore and period.
    if (!validatePluginName.exactMatch(pluginName)) {
        //qDebug() << "Package plugin name " << pluginName << "contains invalid characters";
        d->errorMessage = i18n("Package plugin name %1 contains invalid characters", pluginName);
        return false;
    }

    QString targetName = dest;
    if (targetName[targetName.size() - 1] != '/') {
        targetName.append('/');
    }
    targetName.append(pluginName);

    if (QFile::exists(targetName)) {
        d->errorMessage = i18n("%1 already exists", targetName);
        return false;
    }

    if (archivedPackage) {
        // it's in a temp dir, so just move it over.
        const bool ok = copyFolder(path, targetName);
        removeFolder(path);
        if (!ok) {
            //qWarning() << "Could not move package to destination:" << targetName;
            d->errorMessage = i18n("Could not move package to destination: %1", targetName);
            return false;
        }
    } else {
        // it's a directory containing the stuff, so copy the contents rather
        // than move them
        const bool ok = copyFolder(path, targetName);
        if (!ok) {
            //qWarning() << "Could not copy package to destination:" << targetName;
            d->errorMessage = i18n("Could not copy package to destination: %1", targetName);
            return false;
        }
    }

    if (archivedPackage) {
        // no need to remove the temp dir (which has been successfully moved if it's an archive)
        tempdir.setAutoRemove(false);
    }


    indexDirectory(dest, QStringLiteral("kpluginindex.json"));


    d->installPath = targetName;

    //qWarning() << "Not updating kbuildsycoca4, since that will go away. Do it yourself for now if needed.";
    return true;

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
        return false;
    }
    QString pkg;
    QString root;
    {
        // FIXME: remove, pass in packageroot, type and pluginName separately?
        QStringList ps = packagePath.split('/');
        int ix = ps.count() - 1;
        if (packagePath.endsWith('/')) {
            ix = ps.count() - 2;
        }
        pkg = ps[ix];
        ps.pop_back();
        root = ps.join('/');
    }

    bool ok = removeFolder(packagePath);
    if (!ok) {
        d->errorMessage = i18n("Could not delete package from: %1", packagePath);
        return false;
    }

    indexDirectory(root, QStringLiteral("kpluginindex.json"));

    return true;
}

} // namespace KPackage

#include "moc_packagejobthread_p.cpp"

