#ifndef TST_NGAYTHANG_H
#define TST_NGAYTHANG_H

#include <QtTest>
#include "utils/NgayThang.h"

class TestNgayThang : public QObject
{
    Q_OBJECT

private slots:
    void testPhanTichNgay_data() {
        QTest::addColumn<QString>("input");
        QTest::addColumn<bool>("expectedResult");
        QTest::addColumn<int>("expectedDay");
        QTest::addColumn<int>("expectedMonth");
        QTest::addColumn<int>("expectedYear");

        QTest::newRow("Valid Date") << "15/05/2025" << true << 15 << 5 << 2025;
        QTest::newRow("Invalid Format") << "2025-05-15" << false << 0 << 0 << 0;
        QTest::newRow("Invalid Day") << "32/01/2025" << false << 0 << 0 << 0;
        QTest::newRow("Leap Day Valid") << "29/02/2024" << true << 29 << 2 << 2024;
        QTest::newRow("Leap Day Invalid") << "29/02/2023" << false << 0 << 0 << 0;
    }

    void testPhanTichNgay() {
        QFETCH(QString, input);
        QFETCH(bool, expectedResult);
        QFETCH(int, expectedDay);
        QFETCH(int, expectedMonth);
        QFETCH(int, expectedYear);

        int d, m, y;
        bool result = phanTichNgay(input.toStdString(), d, m, y);

        QCOMPARE(result, expectedResult);
        if (expectedResult) {
            QCOMPARE(d, expectedDay);
            QCOMPARE(m, expectedMonth);
            QCOMPARE(y, expectedYear);
        }
    }

    void testTinhSoNgayQuaHan_data() {
        QTest::addColumn<QString>("borrowDate");
        QTest::addColumn<int>("expectedDaysOverdue"); 
        // Note: tinhSoNgayQuaHan logic depends on "Current Date" which is dynamic.
        // Ideally we should mock "Current Date" or use relative dates.
        // However, the function `tinhSoNgayQuaHan` uses `layNgayHienTai`.
        // We will test `ChuyenNgaySangTimeT` difference instead for deterministic testing,
        // OR rely on the fact that today is known.
        // BUT, `ChuyenNgaySangTimeT` is strictly deterministic.
    }
    
    void testChuyenNgaySangTimeT() {
        // Test deterministic conversion
        std::string s1 = "01/01/2024";
        std::string s2 = "02/01/2024"; // +1 day
        
        time_t t1 = ChuyenNgaySangTimeT(s1);
        time_t t2 = ChuyenNgaySangTimeT(s2);
        
        QVERIFY(t1 > 0);
        QVERIFY(t2 > t1);
        double diff = difftime(t2, t1);
        QCOMPARE(diff, 24 * 3600.0);
    }

};

#endif // TST_NGAYTHANG_H
