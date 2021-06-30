/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PACKAGESTRUCTURETEST_H

#include <QTest>

#include "kpackage/package.h"

class QueryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void installAndQuery();

    void queryCustomPlugin();

private:
    KPackage::Package ps;
    QDir m_dataDir;
};

#endif
