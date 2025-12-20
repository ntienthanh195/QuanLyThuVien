#ifndef TST_MUONTRA_H
#define TST_MUONTRA_H

#include <QtTest>
#include "models/MuonTra.h"
#include "models/DocGia.h"
#include "models/DauSach.h"
#include "utils/SettingsManager.h" // Mocked

class TestMuonTra : public QObject
{
    Q_OBJECT

private:
   PTRDG docGia = nullptr;

private slots:
    void init() {
        docGia = taoDocGia("Test", "User", true, 1, 123);
    }
    
    void cleanup() {
        if(docGia) {
            delete docGia;
            docGia = nullptr;
        }
    }

    void testThemMuonTra_Limit() {
        // Mock limit is default 5 (based on mock_SettingsManager values? Check implementation)
        // In mock_SettingsManager.cpp we set:
        // SO_NGAY_MUON_TOI_DA = 7
        // SO_SACH_MUON_TOI_DA is NOT set in constructor, so it returns defaultValue of getInt(key, 0)
        // Oops, real code uses getInt("SO_SACH_MUON_TOI_DA", 3). 
        // Mock default for integer is 0 if not found.
        // Let's update test logic or rely on code using default value 3.
        
        // Assuming default 3 from code logic if settings missing
        // themMuonTra logic:
        // int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
        
        // Simulate borrowing 3 books
        PTRDMS b1 = new NodeDMS; b1->maSach = "B001"; b1->trangThai = 0;
        PTRDMS b2 = new NodeDMS; b2->maSach = "B002"; b2->trangThai = 0;
        PTRDMS b3 = new NodeDMS; b3->maSach = "B003"; b3->trangThai = 0;
        PTRDMS b4 = new NodeDMS; b4->maSach = "B004"; b4->trangThai = 0;
        
        themMuonTra(docGia, b1);
        themMuonTra(docGia, b2);
        themMuonTra(docGia, b3);
        
        QCOMPARE(docGia->data.soSachDangMuon, 3);
        
        // Try adding 4th => Should fail (console error) and count remains 3
        themMuonTra(docGia, b4);
        QCOMPARE(docGia->data.soSachDangMuon, 3);
        
        // Verify list structure (LIFO or FIFO? Implementation is LIFO: node->next = head)
        QCOMPARE(docGia->data.dsmt->data.maSachCached, std::string("B003"));
    }
    
    void testTinhSoNgayQuaHanLonNhat() {
        // Mock current time is "Now".
        // We inject a transaction with "Yesterday" -> Not overdue
        // We inject a transaction with "10 days ago" -> Overdue (Limit 7)
        
        MuonTra mt;
        mt.NgayMuon = "10/01/2020"; // Very old date
        mt.TrangThai = 0; // Borrowing
        mt.banSaoSach = nullptr;
        mt.maSachCached = "OLD_BOOK";
        
        themMuonTra(docGia, mt);
        // Note: themMuonTra(docGia, MuonTra) (overload 2) does NOT check limit or increment counter automatically?
        // Let's check source code... 
        // themMuonTra(docgia, const MuonTra &mt) -> Just adds node. Does NOT check limit.
        // But TinhSoNgayQuaHanLonNhat iterates list.
        
        int days = TinhSoNgayQuaHanLonNhat(docGia);
        QVERIFY(days > 100); // Definitely overdue per 2020 date
    }
};

#endif // TST_MUONTRA_H
