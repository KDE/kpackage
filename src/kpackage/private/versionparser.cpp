/******************************************************************************
*   Copyright 2015 Marco Martin <mart@kde.org>                                *
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


