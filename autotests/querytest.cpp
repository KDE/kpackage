/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                            *
*   Copyright 2014 by Marco Martin <mart@kde.org>                             *
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

#include "querytest.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <QDebug>
#include <klocalizedstring.h>

#include "packagestructure.h"
#include "packagetrader.h"

void QueryTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    m_dataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    m_dataDir.removeRecursively();

    QVERIFY(m_dataDir.mkpath("."));

    ps = KPackage::PackageTrader::self()->loadPackage("KPackage/Generic");
    ps.addFileDefinition("mainscript", "ui/main.qml", i18n("Main Script File"));
}

void QueryTest::cleanupTestCase()
{
    m_dataDir.removeRecursively();
}

void QueryTest::install()
{
    ps.install(QFINDTESTDATA("data/testpackage"));
    ps.install(QFINDTESTDATA("data/testfallbackpackage"));
}

void QueryTest::query()
{
    //QEXPECT_FAIL("", "queries don't work yet", Continue);
    QCOMPARE(KPackage::PackageTrader::self()->listPackages("KPackage/Generic").count(), 2);
}

QTEST_MAIN(QueryTest)

