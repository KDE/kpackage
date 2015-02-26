/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KPACKAGE_PACKAGE_P_H
#define KPACKAGE_PACKAGE_P_H

#include "../package.h"

#include <QCryptographicHash>
#include <QDir>
#include <QString>
#include <QSharedData>
#include <QPointer>

namespace KPackage
{

class ContentStructure
{
public:
    ContentStructure()
        : directory(false),
          required(false)
    {
    }

    ContentStructure(const ContentStructure &other)
    {
        paths = other.paths;
        name = other.name;
        mimeTypes = other.mimeTypes;
        directory = other.directory;
        required = other.required;
    }

    QString found;
    QStringList paths;
    QString name;
    QStringList mimeTypes;
    bool directory : 1;
    bool required : 1;
};

class PackagePrivate : public QSharedData
{
public:
    PackagePrivate();
    PackagePrivate(const PackagePrivate &other);
    ~PackagePrivate();

    PackagePrivate &operator=(const PackagePrivate &rhs);

    void createPackageMetadata(const QString &path);
    QString unpack(const QString &filePath);
    void updateHash(const QString &basePath, const QString &subPath, const QDir &dir, QCryptographicHash &hash);
    QString fallbackFilePath(const QByteArray &key, const QString &filename = QString()) const;
    bool hasCycle(const KPackage::Package &package);
    bool isInsidePackageDir(const QString& canonicalPath) const;

    QPointer<PackageStructure> structure;
    QString path;
    QString tempRoot;
    QStringList contentsPrefixPaths;
    QString defaultPackageRoot;
    QHash<QString, QString> discoveries;
    QHash<QByteArray, ContentStructure> contents;
    Package *fallbackPackage;
    QStringList mimeTypes;
    KPluginMetaData *metadata;
    bool externalPaths : 1;
    bool valid : 1;
    bool checkedValid : 1;
};

}

#endif
