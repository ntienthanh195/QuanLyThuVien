/**
 * @file MuonTra.cpp
 * @path src/models/MuonTra.cpp
 * @description Triển khai logic nghiệp vụ mượn sách và trả sách.
 * Xử lý các giao dịch (Transactions) lưu giữ trong danh sách liên kết kép của mỗi học viên.
 * @project Library Management System (QuanLyThuVien)
 */
#include "models/MuonTra.h"

#include "models/DocGia.h"
#include "models/DauSach.h"
#include "models/QuanLySach.h"
#include "utils/EventBus.h"
#include "utils/DatabaseManager.h"
#include "utils/AuditLogger.h"
#include "utils/SettingsManager.h"
#include "utils/NgayThang.h"
#include "utils/ThongBao.h"
#include "utils/XuLyChuoi.h"
#include "Constants.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QString>
#include <QMessageBox>
#include <QLocale>
#include <sstream>
#include <iostream>

// External global variables
extern bool duLieuDaThayDoi;

// Forward declarations for helper functions
// thongBao is now available from utils/ThongBao.h
// LOI, CANH_BAO are defined in Constants.h

// ============================================
// HELPER FUNCTIONS - Ban Sao (Book Copies)
// ============================================

// tim ban sao co the muon cua 1 dau sach (O(M) - M: so ban sao)
PTRDMS TimBanSaoCoTheMuon(PTRDS dauSach) {
    if (dauSach == nullptr) return nullptr;
    
    PTRDMS p = dauSach->dms;
    int dem = 0;
    
    while (p != nullptr && dem < SO_VONG_LAP_DMS_MAX) {
        // chi lay ban sao co trang thai CHO_MUON_DUOC (0)
        if (p->trangThai == CHO_MUON_DUOC) {
            return p;  // tra ve ban sao dau tien co the muon
        }
        p = p->next;
        dem++;
    }
    
    if (dem >= SO_VONG_LAP_DMS_MAX) {
        thongBao(std::cout, "Vong lap vo han DMS!", LOI);
    }
    
    return nullptr;  // khong co ban sao nao san sang
}

// tim ban sao theo ma sach trong 1 dau sach (O(M))
PTRDMS TimBanSaoTheoMa(PTRDS dauSach, const std::string& maSach) {
    if (dauSach == nullptr) return nullptr;
    if (maSach.empty()) return nullptr;
    if (maSach.length() > DO_DAI_MA_SACH_MAX) return nullptr; // guard insane input

    PTRDMS p = dauSach->dms;
    if (!p) return nullptr; // no copies

    int dem = 0;

    while (p != nullptr && dem < SO_VONG_LAP_DMS_MAX) {
        // defensive: ensure p->maSach is reasonable before comparing
        if (!p->maSach.empty() && p->maSach == maSach) {
            return p;  // tim thay
        }
        p = p->next;
        dem++;
    }

    if (dem >= SO_VONG_LAP_DMS_MAX) {
        thongBao(std::cout, "Vong lap vo han DMS!", LOI);
    }

    return nullptr;  // khong tim thay
}

// dem so ban sao co the muon (O(M))
int DemBanSaoCoTheMuon(PTRDS dauSach) {
    if (dauSach == nullptr) return 0;

    int dem = 0;
    PTRDMS p = dauSach->dms;
    int vong = 0;

    // Use both logical and defensive caps to avoid infinite loops
    int limit = SO_VONG_LAP_DMS_MAX;
    if (limit <= 0) limit = MAX_BAN_SAO;

    while (p != nullptr && vong < limit) {
        // protect against malformed nodes
        if (p->trangThai == CHO_MUON_DUOC) {
            dem++;
        }

        // detect trivial self-loop
        if (p->next == p) {
            thongBao(std::cout, "Phat hien node DMS self-loop trong DemBanSaoCoTheMuon.", CANH_BAO);
            break;
        }

        p = p->next;
        vong++;
    }
    
    if (vong >= SO_VONG_LAP_DMS_MAX) {
        thongBao(std::cout, "Vong lap vo han DMS!", LOI);
    }
    
    return dem;
}

// ============================================
// MAIN TRANSACTION FUNCTIONS
// ============================================

// [HELPER] Recalculate cached earliest due date (O(K), K=active loans)
void CapNhatNgayHetHanSomNhat(PTRDG docGia) {
     if (!docGia) return;
     docGia->data.ngayHetHanSomNhat = 0;
     if (docGia->data.soSachDangMuon <= 0) return;

     time_t minDate = 0;
     MUONTRA p = docGia->data.dsmt;
     int count = 0;
     // Scan until we find all active loans or hit safety limit
     while (p != nullptr && count < docGia->data.soSachDangMuon + 10) { 
         if (p->data.TrangThai == 0) { // Active loan
             time_t borrowTime = ChuyenNgaySangTimeT(p->data.NgayMuon);
             // Use same logic as check
             int maxDays = SettingsManager::instance().getInt("SO_NGAY_MUON_TOI_DA", 7);
             time_t dueDate = borrowTime + maxDays * 24 * 3600;

             if (minDate == 0 || dueDate < minDate) {
                 minDate = dueDate;
             }
             count++; 
         }
         p = p->next;
     }
     docGia->data.ngayHetHanSomNhat = minDate;
}

// ham backend: muon sach (O(log N) + O(M))
std::string MuonSach(PTRDG docGia, const std::string& isbnOrMaSach, PTRDS dsDauSach[], int soLuongDauSach) {
    if (docGia == nullptr) return "Loi: Doc gia khong hop le!";
    
    // --- BƯỚC 1: CẬP NHẬT TRẠNG THÁI QUYỀN MƯỢN (REALTIME) ---
    // Đảm bảo coTheMuon luôn chính xác (kiểm tra nợ, ngày hết hạn mới nhất từ DB/Memory)
    CapNhatQuyenMuon(docGia);
    
    // --- BƯỚC 2: KIỂM TRA QUYỀN MƯỢN TỔNG QUÁT ---
    if (!docGia->data.coTheMuon) {
        if (docGia->data.TrangThai == 0) {
            return "Loi: The doc gia dang BI KHOA (Vo hieu hoa) boi Admin!";
        }
        
        // Detailed check to explain WHY it's blocked (Professional feedback)
        time_t hanDung = ChuyenNgaySangTimeT(docGia->data.NgayHetHan);
        if (hanDung != -1 && time(nullptr) > hanDung) {
            return "Loi: The doc gia da HET HAN (" + docGia->data.NgayHetHan + ")! Vui long gia han the.";
        }
        
        long long totalDebt = TinhTongNo(docGia);
        int debtBuffer = SettingsManager::instance().getInt("MAX_DEBT_BUFFER", 10000);
        if (totalDebt > debtBuffer) {
            return "Loi: Doc gia dang NO qua han muc cho phep (" + std::to_string(totalDebt) + " > " + std::to_string(debtBuffer) + " VNĐ)!";
        }

        if (TinhSoNgayQuaHanLonNhat(docGia) > 0) {
            return "Loi: Doc gia co SACH QUA HAN! Vui long tra sach truoc khi muon moi.";
        }

        return "Loi: Doc gia dang BI CHAN MUON (Vi pham quy dinh thu vien)!";
    }

    // --- BƯỚC 3: KIỂM TRA SỐ LƯỢNG MƯỢN ---
    int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
    if (docGia->data.soSachDangMuon >= maxSach) {
        return "Loi: Doc gia da muon toi da " + std::to_string(maxSach) + " cuon!";
    }

    // --- BƯỚC 4: KIỂM TRA QUÁ HẠN CACHE (OPTIMIZED CHECK) ---
    time_t now = time(nullptr);
    if (docGia->data.soSachDangMuon > 0 && docGia->data.ngayHetHanSomNhat == 0) {
        CapNhatNgayHetHanSomNhat(docGia);
    }
    if (docGia->data.ngayHetHanSomNhat != 0 && docGia->data.ngayHetHanSomNhat < now) {
         int daysOver = (now - docGia->data.ngayHetHanSomNhat) / (24*3600);
         return "Loi: Doc gia co sach qua han (khoang " + std::to_string(daysOver) + " ngay)! Vui long tra sach truoc.";
    }

    // --- BƯỚC 3: XỬ LÝ LOGIC MƯỢN THÔNG MINH (Smart Borrow) ---
    // Input 'isbnOrMaSach' có thể là ISBN (978-...) hoặc Mã Sách Cụ Thể (ISBN-1)
    
    PTRDS dauSach = nullptr;
    PTRDMS banSaoCanMuon = nullptr;
    std::string maSachThucTe = "";
    
    // 3a. Thử tìm theo Mã Sách Cụ Thể trước
    std::string isbnTuMa = TachISBNTuMaSach(isbnOrMaSach);
    if (!isbnTuMa.empty()) {
        PTRDS dsTam = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbnTuMa);
        if (dsTam) {
            PTRDMS bs = TimBanSaoTheoMa(dsTam, isbnOrMaSach);
            if (bs) {
                // TÌM THẤY MÃ SÁCH CỤ THỂ!
                dauSach = dsTam;
                banSaoCanMuon = bs;
                
                // Kiểm tra trạng thái của bản sao cụ thể này
                if (banSaoCanMuon->trangThai == SACH_DANG_MUON) return "Loi: Cuon sach nay (" + isbnOrMaSach + ") dang co nguoi muon!";
                if (banSaoCanMuon->trangThai == SACH_THANH_LY) return "Loi: Cuon sach nay da thanh ly!";
                if (banSaoCanMuon->trangThai != SACH_TRONG_KHO) return "Loi: Cuon sach nay khong thể mượn được (Trạng thái: " + std::to_string(banSaoCanMuon->trangThai) + ")!";
            }
        }
    }
    
    // 3b. Nếu không phải mã sách cụ thể (hoặc không tìm thấy), coi nó là ISBN và Auto-Pick
    if (banSaoCanMuon == nullptr) {
        dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbnOrMaSach);
        if (dauSach == nullptr) {
            return "Loi: Khong tim thay dau sach hoac ma sach!";
        }
        
        // Auto-pick bản sao đầu tiên rảnh
        banSaoCanMuon = TimBanSaoCoTheMuon(dauSach);
        if (banSaoCanMuon == nullptr) {
            return "Loi: Tat ca ban sao dang duoc muon!";
        }
    }
    
    // --- BƯỚC 4: THỰC HIỆN MƯỢN (TRANSACTIONAL) ---
    QSqlDatabase db = DatabaseManager::instance().database();
    
    // Resolve ban_sao_id first
    QSqlQuery qId(db);
    qId.prepare("SELECT id FROM ban_sao WHERE ma_sach = ?");
    qId.addBindValue(QString::fromStdString(banSaoCanMuon->maSach));
    
    int banSaoId = -1;
    if (qId.exec() && qId.next()) {
        banSaoId = qId.value(0).toInt();
    } else {
        return "Loi DB: Sach khong ton tai trong CSDL ban_sao (Ma Barcode loi?)!";
    }

    // Start Transaction
    if (!db.transaction()) {
        std::cerr << "[DB ERROR] Failed to start transaction.\n";
        return "Loi DB: Khong the khoi tao giao dich!";
    }

    bool success = true;
    std::string errorMsg = "";

    // 4.1 Insert muon_tra
    std::stringstream dummyStream;
    std::string ngayMuon = layNgayHienTai(dummyStream);
    QSqlQuery q(db);
    q.prepare("INSERT INTO muon_tra (ma_the, ban_sao_id, ma_sach, ngay_muon, ngay_tra, trang_thai) VALUES (?, ?, ?, ?, ?, ?)");
    q.addBindValue(docGia->data.MaThe);
    q.addBindValue(banSaoId);
    q.addBindValue(QString::fromStdString(banSaoCanMuon->maSach));
    q.addBindValue(QString::fromStdString(ngayMuon));
    q.addBindValue("");
    q.addBindValue(GD_DANG_MUON); // Dang muon
    
    if (!q.exec()) {
        success = false;
        errorMsg = "Loi DB: Khong the luu giao dich muon: " + q.lastError().text().toStdString();
    }

    // 4.2 Update ban_sao status
    if (success) {
        QSqlQuery qUpd(db);
        qUpd.prepare("UPDATE ban_sao SET trang_thai=? WHERE id=?");
        qUpd.addBindValue(SACH_DANG_MUON); 
        qUpd.addBindValue(banSaoId);
        if (!qUpd.exec()) {
            success = false;
            errorMsg = "Loi DB: Khong the cap nhat trang thai sach: " + qUpd.lastError().text().toStdString();
        }
    }

    // 4.3 Commit or Rollback
    if (success) {
        if (!db.commit()) {
             success = false;
             errorMsg = "Loi DB: Commit giao dich that bai!";
             db.rollback();
        }
    } else {
        db.rollback();
        std::cerr << "[DB ROLLBACK] " << errorMsg << "\n";
        return errorMsg;
    }

    // --- SUCCESS: UPDATE MEMORY ---
    
    // Cập nhật trạng thái sách (Memory)
    banSaoCanMuon->trangThai = SACH_DANG_MUON;
    dauSach->soLuotMuon++; 
    
    // Them vao lich su muon tra (Memory)
    MuonTra mt;
    mt.banSaoSach = banSaoCanMuon;
    mt.maSachCached = banSaoCanMuon->maSach;
    mt.NgayMuon = ngayMuon; 
    mt.NgayTra = "";
    mt.TrangThai = GD_DANG_MUON;  // dang muon
    
    themMuonTra(docGia, mt);
    
    // Update cache statistics
    docGia->data.tongSachDaMuonHistory++;
    docGia->data.soSachDangMuon++; // FIX: Tang so sach dang muon
    CapNhatNgayHetHanSomNhat(docGia);
    
    duLieuDaThayDoi = true;
    
    // Notify listeners (REALTIME UPDATE)
    try {
        CapNhatQuyenMuon(docGia);
        EventBus::instance().muonTraAdded(docGia->data.MaThe, QString::fromStdString(mt.maSachCached));
        EventBus::instance().docGiaUpdated(docGia->data.MaThe);
    } catch(...) {}
    
    // [AUDIT LOG]
    AuditLogger::log("MUON_SACH", "MUON_TRA", QString::number(docGia->data.MaThe), 
                     QString("%1 (Mã sách: %2)")
                     .arg(QString::fromStdString(dauSach->tenSach))
                     .arg(QString::fromStdString(mt.maSachCached)));

    return "";  // Thanh cong
}

// ham backend: tra sach (O(log N) + O(L))
std::string TraSach(PTRDG docGia, const std::string& maSach, PTRDS dsDauSach[], int soLuongDauSach) {
    if (docGia == nullptr) return "Loi: Doc gia khong hop le!";
    
    // B1: Tim giao dich muon trong lich su
    MUONTRA curr = docGia->data.dsmt;
    MUONTRA found = nullptr;
    int safety = 0;
    while (curr != nullptr && safety < SO_VONG_LAP_MT_MAX) {
        if (curr->next == curr) {
            thongBao(std::cout, "Phat hien node MUONTRA self-loop trong TraSach. Dung duyet.", CANH_BAO);
            break;
        }
        // Prefer cached maSach if present
        std::string currMaSach = !curr->data.maSachCached.empty() ? curr->data.maSachCached : (curr->data.banSaoSach ? curr->data.banSaoSach->maSach : std::string());
        
        // Match exact MaSach AND (Active Loan OR Lost Book awaiting settlement OR Settled Lost Book)
        if (!currMaSach.empty() && currMaSach == maSach && 
           (curr->data.TrangThai == GD_DANG_MUON || curr->data.TrangThai == GD_MAT_CHO_THANH_TOAN || curr->data.TrangThai == GD_MAT_DA_THANH_TOAN)) {
            found = curr;
            break;
        }
        curr = curr->next;
        ++safety;
    }
    if (safety >= SO_VONG_LAP_MT_MAX) {
        thongBao(std::cout, "Phat hien vong lap vo han MUONTRA trong TraSach.", LOI);
    }
    
    if (found == nullptr) {
        return "Loi: Doc gia khong muon sach nay!";
    }
    
    // START TRANSACTION (Transactional Refactor)
    QSqlDatabase db = DatabaseManager::instance().database();
    if (!db.transaction()) {
        std::cerr << "[DB ERROR] Failed to start transaction.\n";
        return "Loi DB: Khong the khoi tao giao dich!";
    }

    bool success = true;
    std::string errorMsg = "";
    std::string ngayTra = layNgayHienTai();
    
    // Calculate Fine or Adjustment
    long long tienPhat = 0;
    bool daThanhToanNoCu = found->data.daThanhToan;
    long long refundAmount = 0;
    QString thongBaoGiaoDich = "";

    std::stringstream dummy;
    int soNgayQuaHan = tinhSoNgayQuaHan(found->data.NgayMuon, dummy);

    if (found->data.TrangThai == GD_MAT_CHO_THANH_TOAN || found->data.TrangThai == GD_MAT_DA_THANH_TOAN) {
        // --- CASE: FOUND LOST BOOK ---
        int phiPhatMat = SettingsManager::instance().getInt("PHI_MAT_SACH", 20000);
        
        if (daThanhToanNoCu) {
            // Student already paid (Price + Fee). Now found: Refund Price, keep Fee.
            // Adjust tienHoanTra
            long long tongDaDong = found->data.tienPhat; 
            tienPhat = phiPhatMat; 
            refundAmount = tongDaDong - phiPhatMat;
            found->data.tienHoanTra = refundAmount; // Track refund
            
            thongBaoGiaoDich = QString("Sách báo mất đã tìm thấy! Học viên đã nộp đền bù: %1 VNĐ. \n"
                                     "Hệ thống giữ lại Phí phạt: %2 VNĐ. \n"
                                     "Vui lòng HOÀN LẠI cho học viên: %3 VNĐ (Tiền gốc sách).")
                                     .arg(QLocale(QLocale::Vietnamese).toString((qint64)tongDaDong))
                                     .arg(QLocale(QLocale::Vietnamese).toString((qint64)phiPhatMat))
                                     .arg(QLocale(QLocale::Vietnamese).toString((qint64)refundAmount));
        } else {
            // Student hasn't paid yet. Adjust debt: from (Price + Fee) down to (Fee).
            tienPhat = phiPhatMat;
            thongBaoGiaoDich = QString("Sách báo mất đã tìm thấy! \n"
                                     "Giảm nợ cho học viên: Chỉ còn nộp Phí phạt %1 VNĐ.")
                                     .arg(QLocale(QLocale::Vietnamese).toString((qint64)phiPhatMat));
        }
    } else {
        // --- CASE: NORMAL RETURN ---
        if (soNgayQuaHan > 0) {
            int phiMoiNgay = SettingsManager::instance().getInt("TIEN_PHAT_MOI_NGAY", 5000);
            tienPhat = (long long)soNgayQuaHan * phiMoiNgay;
        }
    }
    
    found->data.tienPhat = (daThanhToanNoCu && (found->data.TrangThai == GD_MAT_CHO_THANH_TOAN)) ? found->data.tienPhat : tienPhat; 
    // If already paid lost book, we keep the 'paid' amount as is in record but notify refund.
    if (found->data.TrangThai == GD_MAT_CHO_THANH_TOAN || found->data.TrangThai == GD_MAT_DA_THANH_TOAN) found->data.tienPhat = tienPhat;

    // DB Update muon_tra
    QSqlQuery q(db);
    // [UPGRADE] Transaction Status: 
    // Nếu sách đang mất mà đã đền -> Chờ hoàn tiền. Nếu chưa đền hoặc đang mượn -> Đã trả.
    int nextStatus = (found->data.TrangThai == GD_MAT_DA_THANH_TOAN) ? GD_MAT_CHO_HOAN_TIEN : GD_DA_TRA;
    
    q.prepare("UPDATE muon_tra SET ngay_tra=?, trang_thai=?, tien_phat=?, tien_hoan_tra=? "
              "WHERE ma_the=? AND ban_sao_id=(SELECT id FROM ban_sao WHERE ma_sach=?) "
              "AND (trang_thai=0 OR trang_thai=2 OR trang_thai=3)");
    q.addBindValue(QString::fromStdString(ngayTra));
    q.addBindValue(nextStatus); 
    q.addBindValue((qint64)tienPhat);
    q.addBindValue((qint64)found->data.tienHoanTra);
    q.addBindValue(docGia->data.MaThe);
    q.addBindValue(QString::fromStdString(maSach));
    
    if (!q.exec()) {
        success = false;
        errorMsg = "Lơi DB: Khong the cap nhat giao dich tra: " + q.lastError().text().toStdString();
    }
    
    // Permission Logic handled via CapNhatQuyenMuon at the end

    if (success) {
        // DB Update ban_sao
        QSqlQuery qUpd(db);
        qUpd.prepare("UPDATE ban_sao SET trang_thai=? WHERE ma_sach=?");
        qUpd.addBindValue(SACH_TRONG_KHO); 
        qUpd.addBindValue(QString::fromStdString(maSach));
        if (!qUpd.exec()) {
            success = false;
            errorMsg = "Loi DB: Khong the cap nhat trang thai sach: " + qUpd.lastError().text().toStdString();
        }
    }

    // Commit
    if (success) {
        if (!db.commit()) {
             success = false;
             errorMsg = "Loi DB: Commit giao dich that bai!";
             db.rollback();
        }
    } else {
        db.rollback();
        return errorMsg;
    }

    // --- SUCCESS: UPDATE MEMORY ---

    // B2: Cap nhat trang thai sách (thông qua con trỏ)
    if (found->data.banSaoSach != nullptr) {
        found->data.banSaoSach->trangThai = SACH_TRONG_KHO;
    } else {
        // Fallback: tim theo ma sach neu con tro bi null
        std::string isbn = TachISBNTuMaSach(maSach);
        PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
        if (dauSach != nullptr) {
            PTRDMS banSao = TimBanSaoTheoMa(dauSach, maSach);
            if (banSao != nullptr) {
                banSao->trangThai = SACH_TRONG_KHO;
            }
        }
    }
    
    // B3: Cap nhat lich su muon tra (Financial State Machine)
    int oldStatus = found->data.TrangThai;
    found->data.NgayTra = ngayTra;
    found->data.TrangThai = (oldStatus == GD_MAT_DA_THANH_TOAN) ? GD_MAT_CHO_HOAN_TIEN : GD_DA_TRA;  
    
    // Only decrement if it was an active loan. 
    // If it was already marked LOST (GD_MAT_...), soSachDangMuon was decremented in BaoMatSach already.
    if (oldStatus == GD_DANG_MUON) {
        docGia->data.soSachDangMuon--;
        if (docGia->data.soSachDangMuon < 0) docGia->data.soSachDangMuon = 0; 
    }
    
    duLieuDaThayDoi = true;
    
    // Notify listeners (REALTIME UPDATE)
    try {
        CapNhatQuyenMuon(docGia);
        if (!thongBaoGiaoDich.isEmpty()) {
            QMessageBox::information(nullptr, "Thông Tin Giao Dịch", thongBaoGiaoDich);
        }
        EventBus::instance().muonTraUpdated(docGia->data.MaThe, QString::fromStdString(maSach));
        EventBus::instance().docGiaUpdated(docGia->data.MaThe);
    } catch(...) {}

    // Update Cache
    CapNhatNgayHetHanSomNhat(docGia);
    
    // [AUDIT LOG]
    QString detailStr = QString("Mã sách: %1").arg(QString::fromStdString(maSach));
    if (found && found->data.banSaoSach) {
        // Find title via ISBN
        std::string isbn = TachISBNTuMaSach(maSach);
        PTRDS ds = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
        if (ds) {
            detailStr = QString("%1 (Mã sách: %2)").arg(QString::fromStdString(ds->tenSach)).arg(QString::fromStdString(maSach));
        }
    }
    AuditLogger::log("TRA_SACH", "MUON_TRA", QString::number(docGia->data.MaThe), detailStr);

    return "";  // thanh cong
}

// Ham bao mat sach (TrangThai = 2)
std::string BaoMatSach(PTRDG docGia, const std::string& maSach, PTRDS dsDauSach[], int soLuongDauSach) {
    if (!docGia) return "Loi: Doc gia khong ton tai!";
    if (maSach.empty()) return "Loi: Ma sach khong hop le!";
    
    // Tim ISBN tu ma sach
    std::string isbn = TachISBNTuMaSach(maSach);
    if (isbn.empty()) return "Loi: Ma sach khong hop le!";
    
    // Tim dau sach
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if (!dauSach) return "Loi: Khong tim thay dau sach!";
    
    // Tim ban sao
    PTRDMS banSao = TimBanSaoTheoMa(dauSach, maSach);
    if (!banSao) return "Loi: Khong tim thay ma sach: " + maSach;
    
    // Tim giao dich muon dang hoat dong
    MUONTRA p = docGia->data.dsmt;
    while (p) {
        if (p->data.banSaoSach == banSao && (p->data.TrangThai == GD_DANG_MUON || (p->data.TrangThai == GD_DA_TRA && !p->data.daThanhToan))) {
            // Financial: Chờ thanh toán đền bù (Nợ)
            p->data.TrangThai = GD_MAT_CHO_THANH_TOAN;
            p->data.NgayTra = layNgayHienTai();
            
            // Tinh tien phat (Gia sach + Phi phat)
            long long giaSach = dauSach->giaTien;
            int phiPhat = SettingsManager::instance().getInt("PHI_MAT_SACH", 20000); // Updated Key
            p->data.tienPhat = giaSach + phiPhat;
            
            // Thanh ly sach
            banSao->trangThai = SACH_MAT;
            duLieuDaThayDoi = true;
            
             // --- DB SYNC (TRANSACTIONAL) ---
            QSqlDatabase db = DatabaseManager::instance().database();
            if (!db.transaction()) {
                std::cerr << "[DB ERROR] Failed to start transaction in BaoMatSach.\n";
            }

            bool sqlSuccess = true;
            // Update transaction: lost, return date, and FINE
            QSqlQuery q(db);
            q.prepare("UPDATE muon_tra SET ngay_tra=?, trang_thai=?, tien_phat=? "
                      "WHERE ma_the=? AND ban_sao_id=(SELECT id FROM ban_sao WHERE ma_sach=?)"
                      " AND (trang_thai=0 OR (trang_thai=1 AND da_thanh_toan=0))");
            q.addBindValue(QString::fromStdString(p->data.NgayTra));
            q.addBindValue(GD_MAT_CHO_THANH_TOAN);
            q.addBindValue((qint64)p->data.tienPhat); // Save calculated fine
            q.addBindValue(docGia->data.MaThe);
            q.addBindValue(QString::fromStdString(maSach));
            if (!q.exec()) {
                sqlSuccess = false;
                std::cerr << "[DB ERROR] Update muon_tra failed: " << q.lastError().text().toStdString() << "\n";
            }
            
            if (sqlSuccess) {
                QSqlQuery qUpd(db);
                qUpd.prepare("UPDATE ban_sao SET trang_thai=? WHERE ma_sach=?");
                qUpd.addBindValue(SACH_MAT);
                qUpd.addBindValue(QString::fromStdString(maSach));
                if (!qUpd.exec()) {
                    sqlSuccess = false;
                    std::cerr << "[DB ERROR] Update ban_sao failed: " << qUpd.lastError().text().toStdString() << "\n";
                }
            }

            if (sqlSuccess) {
                db.commit();
            } else {
                db.rollback();
                return "Loi DB: Khong the cap nhat trang thai mat sach!";
            }
            // --- END DB SYNC ---
            
            // Permission Logic handled via CapNhatQuyenMuon
            
            // [AUDIT LOG]
            QString detailStr = QString("%1 (Mã sách: %2). Phạt: %3")
                                .arg(QString::fromStdString(dauSach->tenSach))
                                .arg(QString::fromStdString(maSach))
                                .arg(p->data.tienPhat);
            AuditLogger::log(AuditLogger::ACTION_LOST, "MUON_TRA", QString::number(docGia->data.MaThe), detailStr);

            // [EVENT BUS]
            try {
                CapNhatQuyenMuon(docGia);
                EventBus::instance().docGiaUpdated(docGia->data.MaThe);
            } catch(...) {}

            // Update Cache
            docGia->data.soSachDangMuon--; // Lost book counts as returned logic (no longer 'borrowing')
            if (docGia->data.soSachDangMuon < 0) docGia->data.soSachDangMuon = 0;
            CapNhatNgayHetHanSomNhat(docGia);
            
            return ""; // Thanh cong
        }
        p = p->next;
    }
    
    return "Loi: Khong tim thay giao dich muon sach nay!";
}

//dung cho MaSach_to_PTRDMS
void themMuonTra(PTRDG docgia, PTRDMS banSaoSach) {
    if (docgia == nullptr) return;
    // Lay gioi han tu Settings
    int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
    if (docgia->data.soSachDangMuon >= maxSach) { 
        std::cerr << "Lỗi: Độc giả đã mượn tối đa " << maxSach << " cuốn sách." << std::endl;
        return;
    }

    MUONTRA node = new NodeMT;

    // 1. Gán con trỏ trực tiếp (Fix lỗi crash)
    node->data.banSaoSach = banSaoSach; 
    node->data.maSachCached = (banSaoSach != nullptr) ? banSaoSach->maSach : std::string();
    
    // 2. Gán dữ liệu giao dịch
    node->data.NgayMuon = layNgayHienTai();
    node->data.NgayTra = "";
    
    // 3. Gán trạng thái GIAO DỊCH, không phải trạng thái SÁCH (Fix lỗi logic)
    // 0 = Đang mượn
    node->data.TrangThai = 0; 
    
    // Feature 4 Defaults
    node->data.soLanGiaHan = 0;
    node->data.daThanhToan = false; 

    node->next = docgia->data.dsmt;
    docgia->data.dsmt = node;
    
    // Cập nhật số lượng sách đang mượn
    docgia->data.soSachDangMuon++;
    // Phát sự kiện realtime
    try {
        if (!node->data.maSachCached.empty()) {
            EventBus::instance().muonTraAdded(docgia->data.MaThe, QString::fromStdString(node->data.maSachCached));
        } else if (node->data.banSaoSach != nullptr) {
            EventBus::instance().muonTraAdded(docgia->data.MaThe, QString::fromStdString(node->data.banSaoSach->maSach));
        }
        EventBus::instance().docGiaUpdated(docgia->data.MaThe);
    } catch(...) {}
   
}

//dùng cho (NapGiaoDich)
void themMuonTra(PTRDG docgia, const MuonTra &mt) {
    if (docgia == nullptr) return;

    MUONTRA node = new NodeMT;
    node->data = mt;
    node->next = docgia->data.dsmt;
    docgia->data.dsmt = node;
    // NOTE: This overload is used when loading existing transactions from storage.
    // The caller (loader) is responsible for setting `docgia->data.soSachDangMuon`
    // from the persisted value. Do NOT modify the counter here to avoid double-counting.
}

// ============================================
// QUERY / REPORTING FUNCTIONS
// ============================================

// lay danh sach sach dang muon cua doc gia (O(L) - L: so giao dich)
int LayDSSachDangMuon(PTRDG docGia, ThongTinSachDangMuon_DTO ketQua[], int maxKetQua, PTRDS dsDauSach[], int soLuongDauSach) {
    if (docGia == nullptr || ketQua == nullptr) return -1;  // loi: khong tim thay doc gia
    if (maxKetQua <= 0) return 0; // nothing to fill
    if (maxKetQua > MAX_DAUSACH) maxKetQua = MAX_DAUSACH;
    if (soLuongDauSach < 0 || soLuongDauSach > MAX_DAUSACH) {
        // invalid catalog array - treat as empty so lookups return nullptr safely
        soLuongDauSach = 0;
        dsDauSach = nullptr;
    }
    
    MUONTRA curr = docGia->data.dsmt;
    if (curr == nullptr) return 0;  // doc gia chua muon cuon nao

    int dem = 0;
    int safety = 0;

    // duyet toan bo danh sach muon with safety checks
    while (curr != nullptr && safety < SO_VONG_LAP_MT_MAX) {
        // detect trivial self-loop to avoid infinite loops / corruption
        if (curr->next == curr) {
            thongBao(std::cout, "Phat hien node MUONTRA self-loop. Dung duyet.", CANH_BAO);
            break;
        }

        // Update: Include ALL books that librarian needs to see in "Reader Details"
        // 0: Borrowing, 1: Returned (with debt), 2: Lost (unpaid), 3: Lost (paid - hide?), 4: Lost (found, refund wait)
        if (curr->data.TrangThai == 0 || 
            curr->data.TrangThai == 1 || // Include returned to check for debt
            curr->data.TrangThai == 2 || 
            curr->data.TrangThai == 4) {
            
            // Determine MaSach: prefer cached, then pointer
            std::string maSach = curr->data.maSachCached;
            if (maSach.empty() && curr->data.banSaoSach != nullptr) {
                maSach = curr->data.banSaoSach->maSach;
            }
            
            if (!maSach.empty()) {
                std::string isbn = TachISBNTuMaSach(maSach);
                PTRDS dauSach = nullptr;
                if (dsDauSach != nullptr && soLuongDauSach > 0) {
                    dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
                }

                if (dem < maxKetQua) {
                    ketQua[dem].maSach = maSach;
                    ketQua[dem].tenSach = (dauSach != nullptr) ? dauSach->tenSach : "";
                    ketQua[dem].ngayMuon = curr->data.NgayMuon;
                    
                    // Populate Financials directly
                    ketQua[dem].tienPhat = curr->data.tienPhat;
                    ketQua[dem].tienHoanTra = curr->data.tienHoanTra;
                    ketQua[dem].daThanhToan = curr->data.daThanhToan;
                    ketQua[dem].soLanGiaHan = curr->data.soLanGiaHan;
                    ketQua[dem].trangThai = curr->data.TrangThai; 

                    // Calculate soNgayGiu (for current loans only)
                    if (curr->data.TrangThai == 0) {
                        std::stringstream dummyStream;
                        int soNgay = tinhSoNgayQuaHan(curr->data.NgayMuon, dummyStream);
                        ketQua[dem].soNgayGiu = (soNgay >= 0) ? soNgay : 0;
                    } else {
                        ketQua[dem].soNgayGiu = 0;
                    }

                    dem++;
                }
            }
        }

        curr = curr->next;
        ++safety;
    }

    if (safety >= SO_VONG_LAP_MT_MAX) {
        thongBao(std::cout, "Phat hien vong lap vo han (MUONTRA).", LOI);
    }

    return dem;  // tra ve so luong hien thi
}

// tinh so ngay qua han lon nhat cua 1 doc gia (O(L))
int TinhSoNgayQuaHanLonNhat(PTRDG docgia) {
    if (docgia == nullptr) return 0;
    
    int maxQuaHan = 0;
    int quaHan = 0;
    MUONTRA curr = docgia->data.dsmt;
    int safety = 0;
    while (curr != nullptr && safety < SO_VONG_LAP_MT_MAX) {
        if (curr->next == curr) {
            thongBao(std::cout, "Phat hien node MUONTRA self-loop trong TinhSoNgayQuaHanLonNhat. Dung duyet.", CANH_BAO);
            break;
        }
        if (curr->data.TrangThai == 0) {
            std::stringstream dummy;  // thay std::cout bang stream ao
            quaHan = tinhSoNgayQuaHan(curr->data.NgayMuon, dummy);

            if (quaHan > maxQuaHan)
                maxQuaHan = quaHan;
        }
        curr = curr->next;
        ++safety;
    }
    if (safety >= SO_VONG_LAP_MT_MAX) {
        thongBao(std::cout, "Phat hien vong lap vo han MUONTRA trong TinhSoNgayQuaHanLonNhat.", LOI);
    }
    return maxQuaHan;
}

// ============================================
// HELPER FUNCTION IMPLEMENTATION
// ============================================
// thongBao is provided by utils/ThongBao.h
