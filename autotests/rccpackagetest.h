/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RCCPACKAGETEST_H
#define RCCPACKAGETEST_H

#include <QTest>

#include "kpackage/package.h"

class RccPackageTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void accessFiles();
    void validate();
    void testResourceRefCount();

private:
    KPackage::Package *m_pkg;
    KPackage::Package *m_pkg2;
    QString m_packagePath;
};

#endif
