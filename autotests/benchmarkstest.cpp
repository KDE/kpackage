#include <QTest>
#include <qbenchmark.h>

#include "kpackage/package.h"
#include "kpackage/packageloader.h"
#include "kpackagetool/kpackagetool.h"

class BenchmarksTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void benchmarkPackageListing();
    void benchmarkPackageStructoreLoading();
};

void BenchmarksTest::benchmarkPackageListing()
{
    QBENCHMARK {
        KPackage::PackageLoader::self()->listPackages("Plasma/LookAndFeel");
    }
}

void BenchmarksTest::benchmarkPackageStructoreLoading()
{
    QBENCHMARK {
        KPackage::PackageLoader::self()->loadPackageStructure("Plasma/LookAndFeel");
    }
}

QTEST_MAIN(BenchmarksTest)

#include "benchmarkstest.moc"