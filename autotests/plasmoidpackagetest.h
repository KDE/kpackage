/*
    SPDX-FileCopyrightText: 2007 Bertjan Broeksema <b.broeksema@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PACKAGETEST_H

#include <QTest>

#include "kpackage/package.h"
#include "kpackage/packagestructure.h"

class PlasmoidPackageTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void createAndInstallPackage();
    void createAndUpdatePackage();
    void uncompressPackageWithSubFolder();
    void isValid();
    void filePath();
    void entryList();
    void noCrashOnAsyncInstall();

    void packageInstalled(KJob *j);
    void packageUninstalled(KJob *j);

private:
    void createTestPackage(const QString &packageName, const QString &version);
    void cleanupPackage(const QString &packageName);

    QString m_packageRoot;
    QString m_package;
    KJob *m_packageJob;
    KPackage::Package m_defaultPackage;
    KPackage::PackageStructure *m_defaultPackageStructure;
};

#endif
