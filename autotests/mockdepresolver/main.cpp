/******************************************************************************
*   Copyright 2016 Bhushan Shah <bshah@kde.org>                               *
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

#include <QCoreApplication>
#include <QDebug>
#include <QUrl>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    Q_ASSERT(app.arguments().count() == 2);

    const QUrl url(app.arguments().last());
    Q_ASSERT(url.isValid());
    Q_ASSERT(url.scheme() == QLatin1String("mock"));

    // This is very basic dep resolver used for mocking in tests
    // if asked to install invalidapp, will fail
    // if asked to install validdep, will pass
	const QString componentName = url.host();
	if (componentName.isEmpty()) {
		qWarning() << "wrongly formatted URI" << url;
        return 1;
	}

    if (componentName == QStringLiteral("invaliddep")) {
        qWarning() << "package asked to install invalid dep, bailing out";
        return 1;
    }

    if (componentName == QStringLiteral("validdep")) {
        qWarning() << "asked to install valid dep, success!";
        return 0;
    }

    qWarning() << "Assuming provided package is not available";
    return 1;

}

