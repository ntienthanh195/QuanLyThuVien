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

// ham backend: muon sach (O(log N) + O(M))
std::string MuonSach(PTRDG docGia, const std::string& isbnOrMaSach, PTRDS dsDauSach[], int soLuongDauSach) {
    if (docGia == nullptr) return "Loi: Doc gia khong hop le!";
    
    // --- BƯỚC 1: KIỂM TRA QUÁ HẠN ---
    // Duyệt qua danh sách sách đang mượn để kiểm tra xem có cuốn nào quá hạn không
    MUONTRA p = docGia->data.dsmt;
    std::stringstream dummyStream; 
    int safety = 0;
    while (p != nullptr && safety < SO_VONG_LAP_MT_MAX) {
        if (p->next == p) break; // Error handling
        if (p->data.TrangThai == 0) {
            int soNgayQua = tinhSoNgayQuaHan(p->data.NgayMuon, dummyStream);
            if (soNgayQua > 0) {
                return "Loi: Doc gia co sach qua han " + std::to_string(soNgayQua) + " ngay! Vui long tra sach truoc.";
            }
        }
        p = p->next;
        ++safety;
    }
    // ----------------------------------------

    // --- BƯỚC 2: KIỂM TRA SỐ LƯỢNG MƯỢN ---
    int maxSach = SettingsManager::instance().getInt("SO_SACH_MUON_TOI_DA", 3);
    if (docGia->data.soSachDangMuon >= maxSach) {
        return "Loi: Doc gia da muon toi da " + std::to_string(maxSach) + " cuon!";
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
                if (banSaoCanMuon->trangThai == DANG_MUON) return "Loi: Cuon sach nay (" + isbnOrMaSach + ") dang co nguoi muon!";
                if (banSaoCanMuon->trangThai == THANH_LY) return "Loi: Cuon sach nay da thanh ly!";
                if (banSaoCanMuon->trangThai != CHO_MUON_DUOC) return "Loi: Cuon sach nay khong the muon duoc!";
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
    
    // --- BƯỚC 4: THỰC HIỆN MƯỢN ---
    
    // Cập nhật trạng thái sách
    banSaoCanMuon->trangThai = DANG_MUON;
    
    // Them vao lich su muon tra
    MuonTra mt;
    mt.banSaoSach = banSaoCanMuon;
    mt.maSachCached = banSaoCanMuon->maSach;
    mt.NgayMuon = layNgayHienTai(dummyStream); 
    mt.NgayTra = "";
    mt.TrangThai = 0;  // dang muon
    
    themMuonTra(docGia, mt);
    
    duLieuDaThayDoi = true;
    dauSach->soLuotMuon++;
    
    // Notify listeners
    try {
        if (!mt.maSachCached.empty()) {
            EventBus::instance().muonTraAdded(docGia->data.MaThe, QString::fromStdString(mt.maSachCached));
        }
        EventBus::instance().docGiaUpdated(docGia->data.MaThe);
    } catch(...) {}
    
    // --- DB SYNC ---
    // Resolve ban_sao_id because muon_tra table requires it, and ma_sach column doesn't exist
    QSqlQuery qId(DatabaseManager::instance().database());
    qId.prepare("SELECT id FROM ban_sao WHERE ma_sach = ?");
    qId.addBindValue(QString::fromStdString(mt.maSachCached));
    
    if (qId.exec() && qId.next()) {
        int banSaoId = qId.value(0).toInt();
        
        QSqlQuery q(DatabaseManager::instance().database());
        q.prepare("INSERT INTO muon_tra (ma_the, ban_sao_id, ngay_muon, ngay_tra, trang_thai) VALUES (?, ?, ?, ?, ?)");
        q.addBindValue(docGia->data.MaThe);
        q.addBindValue(banSaoId);
        q.addBindValue(QString::fromStdString(mt.NgayMuon));
        q.addBindValue("");
        q.addBindValue(0); // Dang muon
        
        if (!q.exec()) {
            std::cerr << "[DB ERROR] Insert muon_tra failed: " << q.lastError().text().toStdString() << "\n";
            return "Loi DB: Khong the luu giao dich muon!";
        } else {
             std::cout << "[DB] Insert muon_tra OK. MaThe=" << docGia->data.MaThe << " BanSaoId=" << banSaoId << "\n";
             
             // Also update book status in ban_sao table ONLY if insert succeeded
             QSqlQuery qUpd(DatabaseManager::instance().database());
             qUpd.prepare("UPDATE ban_sao SET trang_thai=? WHERE id=?");
             qUpd.addBindValue(1); // DANG_MUON
             qUpd.addBindValue(banSaoId);
             qUpd.exec();
        }
    } else {
        std::cerr << "[DB ERROR] Khong tim thay ban_sao_id cho ma_sach: " << mt.maSachCached << "\n";
        return "Loi DB: Sach khong ton tai trong CSDL (Ma Barcode loi?)!";
    }
    // --- END DB SYNC ---
    
    // [AUDIT LOG]
    AuditLogger::log("MUON_SACH", "MUON_TRA", QString::number(docGia->data.MaThe), 
                     "Muon sach: " + QString::fromStdString(mt.maSachCached));

    // [EVENT BUS] Realtime Update
    try {
        EventBus::instance().muonTraAdded(docGia->data.MaThe, QString::fromStdString(mt.maSachCached));
    } catch(...) {}
    
    return "";  // Thanh cong (empty string means success)
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
        // Prefer cached maSach if present (safer than dereferencing possibly-dangling PTRDMS)
        std::string currMaSach = !curr->data.maSachCached.empty() ? curr->data.maSachCached : (curr->data.banSaoSach ? curr->data.banSaoSach->maSach : std::string());
        if (!currMaSach.empty() && currMaSach == maSach && curr->data.TrangThai == 0) {
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
    
    // B2: Cap nhat trang thai sach (thong qua con tro)
    if (found->data.banSaoSach != nullptr) {
        found->data.banSaoSach->trangThai = CHO_MUON_DUOC;
    } else {
        // Fallback: tim theo ma sach neu con tro bi null
        std::string isbn = TachISBNTuMaSach(maSach);
        PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
        if (dauSach != nullptr) {
            PTRDMS banSao = TimBanSaoTheoMa(dauSach, maSach);
            if (banSao != nullptr) {
                banSao->trangThai = CHO_MUON_DUOC;
            }
        }
    }
    
    // B3: Cap nhat lich su muon tra
    found->data.NgayTra = layNgayHienTai();
    found->data.TrangThai = 1;  // da tra
    
    docGia->data.soSachDangMuon--;
    
    duLieuDaThayDoi = true;
    // Emit events: muonTraUpdated + docGiaUpdated
    try {
        EventBus::instance().muonTraUpdated(docGia->data.MaThe, QString::fromStdString(maSach));
        EventBus::instance().docGiaUpdated(docGia->data.MaThe);
    } catch(...) {}
    
    // --- DB SYNC ---
    QSqlQuery q(DatabaseManager::instance().database());
    // Use subquery to match ban_sao_id from ma_sach
    q.prepare("UPDATE muon_tra SET ngay_tra=?, trang_thai=? WHERE ma_the=? AND ban_sao_id=(SELECT id FROM ban_sao WHERE ma_sach=?) AND trang_thai=0");
    q.addBindValue(QString::fromStdString(found->data.NgayTra));
    q.addBindValue(1); // Da tra
    q.addBindValue(docGia->data.MaThe);
    q.addBindValue(QString::fromStdString(maSach));
    
    if (!q.exec()) {
        std::cerr << "[DB ERROR] Update muon_tra failed: " << q.lastError().text().toStdString() << "\n";
        return "Loi DB: Khong the cap nhat giao dich tra!";
    } else {
        if (q.numRowsAffected() == 0) {
             std::cerr << "[DB WARNING] Update muon_tra thanh cong nhung KHONG co dong nao thay doi (Zero rows)!\n";
             std::cerr << "   -> Kiem tra: MaThe=" << docGia->data.MaThe << " MaSach=" << maSach << "\n";
        } else {
             std::cout << "[DB] Update muon_tra OK (Rows=" << q.numRowsAffected() << ")\n";
        }
    }
    
    // Update ban_sao status back to Available (0)
    QSqlQuery qUpd(DatabaseManager::instance().database());
    qUpd.prepare("UPDATE ban_sao SET trang_thai=0 WHERE ma_sach=?");
    qUpd.addBindValue(QString::fromStdString(maSach));
    qUpd.exec();
    // --- END DB SYNC ---
    
    // [AUDIT LOG]
    AuditLogger::log("TRA_SACH", "MUON_TRA", QString::number(docGia->data.MaThe), 
                     "Tra sach Ma: " + QString::fromStdString(maSach));

    // [EVENT BUS] Realtime Update
    try {
        EventBus::instance().muonTraUpdated(docGia->data.MaThe, QString::fromStdString(maSach));
    } catch(...) {}
    
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
        if (p->data.banSaoSach == banSao && (p->data.TrangThai == 0 || (p->data.TrangThai == 1 && !p->data.daThanhToan))) {
            // Cap nhat trang thai = 2 (Mat)
            p->data.TrangThai = 2;
            p->data.NgayTra = layNgayHienTai();
            
            // Tinh tien phat (Gia sach + Phi phat)
            long long giaSach = dauSach->giaTien;
            int phiPhat = SettingsManager::instance().getInt("PHI_MAT_SACH", 20000);
            p->data.tienPhat = giaSach + phiPhat;
            
            // Thanh ly sach
            banSao->trangThai = THANH_LY;
            duLieuDaThayDoi = true;
            
             // --- DB SYNC ---
            QSqlQuery q(DatabaseManager::instance().database());
            // Update transaction: lost, return date, and FINE
            q.prepare("UPDATE muon_tra SET ngay_tra=?, trang_thai=2, tien_phat=? WHERE ma_the=? AND ban_sao_id=(SELECT id FROM ban_sao WHERE ma_sach=?) AND (trang_thai=0 OR (trang_thai=1 AND da_thanh_toan=0))");
            q.addBindValue(QString::fromStdString(p->data.NgayTra));
            q.addBindValue((qint64)p->data.tienPhat); // Save calculated fine
            q.addBindValue(docGia->data.MaThe);
            q.addBindValue(QString::fromStdString(maSach));
            if (!q.exec()) {
                std::cerr << "[DB ERROR] Update muon_tra failed: " << q.lastError().text().toStdString() << "\n";
            }
            
            QSqlQuery qUpd(DatabaseManager::instance().database());
            qUpd.prepare("UPDATE ban_sao SET trang_thai=2 WHERE ma_sach=?");
            qUpd.addBindValue(QString::fromStdString(maSach));
            qUpd.exec();
            // --- END DB SYNC ---
            
            // [AUDIT LOG]
            AuditLogger::log("BAO_MAT", "MUON_TRA", QString::number(docGia->data.MaThe), 
                             QString("Bao mat sach %1. Phat: %2").arg(QString::fromStdString(maSach)).arg(p->data.tienPhat));

            // [EVENT BUS]
            try {
                EventBus::instance().docGiaUpdated(docGia->data.MaThe);
            } catch(...) {}
            
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

        // Update: Include Borrowing (0) OR Returned-Unpaid (1 && !daThanhToan)
        if (curr->data.TrangThai == 0 || (curr->data.TrangThai == 1 && !curr->data.daThanhToan)) {
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
                    // Calculate soNgayGiu
                    std::stringstream dummyStream;
                    int soNgay = tinhSoNgayQuaHan(curr->data.NgayMuon, dummyStream);
                    ketQua[dem].soNgayGiu = (soNgay >= 0) ? soNgay : 0;
                    // Feature 4: Pass status to UI (0=Borrowing, 1=Returned/Debt)
                    ketQua[dem].trangThai = curr->data.TrangThai; 
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
