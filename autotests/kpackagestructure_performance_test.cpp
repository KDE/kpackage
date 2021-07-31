/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QTest>

#include "../src/kpackage/private/utils.h"
Q_LOGGING_CATEGORY(KPACKAGE_LOG, "kf.package", QtInfoMsg)

class PerformanceTest : public QObject
{
    Q_OBJECT

    const QStringList m_availablePackageTypes = {
        QStringLiteral("Plasma/Applet"),
        QStringLiteral("Plasma/ContainmentActions"),
        QStringLiteral("Plasma/ContainmentActions"),
        QStringLiteral("Plasma/Generic"),
        QStringLiteral("Plasma/Theme"),
        QStringLiteral("KWin/Aurorae"),
        QStringLiteral("KWin/Decoration"),
        QStringLiteral("KWin/Effect"),
        QStringLiteral("KWin/Script"),
        QStringLiteral("KWin/WindowSwitcher"),
    };

private Q_SLOTS:
    void testOldStyleLoading()
    {
        QBENCHMARK {
            for (const auto &packageFormat : m_availablePackageTypes) {
                QVector<KPluginMetaData> plugins =
                    KPluginLoader::findPlugins(QStringLiteral("kpackage/packagestructure"), [packageFormat](const KPluginMetaData &metaData) {
                        return readKPackageTypes(metaData).contains(packageFormat);
                    });
                Q_UNUSED(plugins)
            }
        }
    }

    void testHelperFunction()
    {
        QBENCHMARK {
            for (const auto &packageFormat : m_availablePackageTypes) {
                const KPluginMetaData &metaData = structureForKPackageType(packageFormat);
                QVERIFY(metaData.isValid());
            }
        }
    }
};

QTEST_MAIN(PerformanceTest);

#include "kpackagestructure_performance_test.moc"
