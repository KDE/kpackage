/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "private/packagejobthread_p.h"

#include "package.h"

#include <QVector>

namespace KPackage
{
bool isVersionNewer(const QString &version1, const QString &version2)
{
    if (version1 == version2) {
        return false;
    }

    const auto versionChunks = QVector<QString>::fromList(version2.split(QLatin1Char('.')));
    const auto oldVersionChunks = QVector<QString>::fromList(version1.split(QLatin1Char('.')));
    const int length = qMin(versionChunks.size(), oldVersionChunks.size());

    for (int i = 0; i < length; ++i) {
        if (versionChunks[i] != oldVersionChunks[i]) {
            return versionChunks[i] > oldVersionChunks[i];
        }
    }

    return versionChunks.size() > oldVersionChunks.size();
}

} // namespace KPackage
