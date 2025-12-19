/**
 * @file DocGia.cpp
 * @path src/models/DocGia.cpp
 * @description Triển khai các hàm xử lý dữ liệu học viên (Độc giả).
 * Bao gồm: quản lý danh sách học viên (cây nhị phân search), tìm kiếm, và các thao tác liên quan đến thẻ học viên.
 * @project Library Management System (QuanLyThuVien)
 */
#include "models/DocGia.h"

#include "models/MuonTra.h"
#include "utils/FilePathManager.h"
#include "utils/BackupManager.h"
#include <iostream>
#include <stack>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include "models/ThaoTacFile.h"
#include "models/QuanLySach.h"
#include "Constants.h"
#include "utils/NgayThang.h"
// Cần thêm thư viện XuLyChuoi.h cho hàm BoDauVaThuong()
#include "utils/XuLyChuoi.h" 
// Database support
#include "utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QVariant>
#include <QDate>
#include <functional>
#include "utils/AuditLogger.h"
// Cần thêm thư viện QuanLySach.h nếu sử dụng TimDanhMucSachTuMaSach
// #include "QuanLySach.h" 

#include "utils/EventBus.h"
#include "utils/Perf.h"
#include "utils/SettingsManager.h" // Added SettingsManager

// helper thay the max/abs tu <algorithm>
static inline int my_max(int a, int b) { return (a > b) ? a : b; }
static inline int my_abs(int x) { return (x < 0) ? -x : x; }

// forward-declare static helpers
static PTRDG InsertDocGiaRec(PTRDG pavtree, PTRDG node);
static PTRDG xoaDocGiaRec(PTRDG root, int mathe);
// Lay lai prototype vi static helper dung
PTRDG timDocGia(PTRDG root, int mathe); 

// hang so cho sinh ma
static const int MA_THE_MIN = 10000;
static const int MA_THE_MAX = 99999;
static const int NUM_RANGES = 9; // chia range de phan tan

// bien static theo doi RNG
static bool daKhoiTao = false;
static int khoangHienTai = 0;
static int countTrongKhoang = 0;

int static sinhMaTheNgauNhien(PTRDG root) {
    if (!daKhoiTao) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        daKhoiTao = true;
    }

    const int sizeKhoang = (MA_THE_MAX - MA_THE_MIN + 1) / NUM_RANGES;
    for (int i = 0; i < MAX_LAN_THU_SINH_MA; ++i) {
        int rangeStart = MA_THE_MIN + khoangHienTai * sizeKhoang;
        int rangeEnd = rangeStart + sizeKhoang - 1;
        int ma = rangeStart + (std::rand() % (rangeEnd - rangeStart + 1));
        if (timDocGia(root, ma) == nullptr) {
            ++countTrongKhoang;
            if (countTrongKhoang >= sizeKhoang / NUM_RANGES) {
                countTrongKhoang = 0;
                khoangHienTai = (khoangHienTai + 1) % NUM_RANGES;
            }
            return ma;
        }
    }

    // fallback: linear search
    int ma = MA_THE_MIN;
    while (ma <= MA_THE_MAX && timDocGia(root, ma) != nullptr) ++ma;
    return (ma <= MA_THE_MAX) ? ma : -1;
}
//AVL
// get/update height and balance helpers
static inline int getHeight(PTRDG n) {
    return (n == nullptr) ? 0 : n->height;
}
static inline void updateHeight(PTRDG n) {
    if (n != nullptr) n->height = 1 + my_max(getHeight(n->left), getHeight(n->right));
}
static inline int getBalance(PTRDG n) {
    return (n == nullptr) ? 0 : getHeight(n->left) - getHeight(n->right);
}

// rotations update heights
PTRDG RotateLeft(PTRDG p) {
    PTRDG q = p->right;
    PTRDG B = q->left;

    q->left = p;
    p->right = B;

    updateHeight(p);
    updateHeight(q);
    return q;
}

PTRDG RotateRight(PTRDG p) {
    PTRDG q = p->left;
    PTRDG B = q->right;

    q->right = p;
    p->left = B;

    updateHeight(p);
    updateHeight(q);
    return q;
}

// Them AVL - dung height de tinh balance O(1)
static PTRDG InsertDocGiaRec(PTRDG pavltree, PTRDG node) {
    if (pavltree == nullptr) {
        return node;
    }

    if (node->data.MaThe < pavltree->data.MaThe)
        pavltree->left = InsertDocGiaRec(pavltree->left, node);
    else if (node->data.MaThe > pavltree->data.MaThe)
        pavltree->right = InsertDocGiaRec(pavltree->right, node);
    else {
        // duplicate id -> do nothing
        return pavltree;
    }

    // cap nhat height va rebalance
    updateHeight(pavltree);
    int balance = getBalance(pavltree);

    // LL (Left Left Case)
    if (balance > 1 && getBalance(pavltree->left) >= 0)
        return RotateRight(pavltree);
    // RR (Right Right Case)
    if (balance < -1 && getBalance(pavltree->right) <= 0)
        return RotateLeft(pavltree);
    // LR (Left Right Case)
    if (balance > 1 && getBalance(pavltree->left) < 0) {
        pavltree->left = RotateLeft(pavltree->left);
        return RotateRight(pavltree);
    }
    // RL (Right Left Case)
    if (balance < -1 && getBalance(pavltree->right) > 0) {
        pavltree->right = RotateRight(pavltree->right);
        return RotateLeft(pavltree);
    }

    return pavltree;
}

void InsertDocGia(PTRDG &root, PTRDG node, bool syncDB) {
    // ensure new node has height = 1
    if (node != nullptr) node->height = 1;
    root = InsertDocGiaRec(root, node);
    
    // Emit event that a new reader was added
    try {
        if (node) EventBus::instance().docGiaAdded(node->data.MaThe);
    } catch(...) {}
    
    // --- DB SYNC ---
    if (syncDB && node) {
     QSqlQuery q(DatabaseManager::instance().database());
        q.prepare("INSERT INTO doc_gia (ma_the, ho, ten, phai, trang_thai, so_dien_thoai, lop, email, ngay_sinh, loai_doc_gia, so_sach_dang_muon) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        q.addBindValue(node->data.MaThe);
        q.addBindValue(QString::fromStdString(node->data.Ho));
        q.addBindValue(QString::fromStdString(node->data.Ten));
        q.addBindValue(node->data.Phai ? 1 : 0);
        q.addBindValue(node->data.TrangThai);
        // Phase 2 New Fields
        q.addBindValue(QString::fromStdString(node->data.SDT));
        q.addBindValue(QString::fromStdString(node->data.Lop));
        q.addBindValue(QString::fromStdString(node->data.Email));
        q.addBindValue(QString::fromStdString(node->data.NgaySinh));
        q.addBindValue(node->data.LoaiDocGia);
        q.addBindValue(0); // so_sach_dang_muon default
        q.exec();
        
        // [AUDIT LOG]
        if (q.lastError().type() == QSqlError::NoError) {
             AuditLogger::log(AuditLogger::ACTION_INSERT, "DOC_GIA", QString::number(node->data.MaThe), 
                              QString("Them doc gia moi: %1 %2").arg(QString::fromStdString(node->data.Ho)).arg(QString::fromStdString(node->data.Ten)));
        }
    }
    // --- END DB SYNC ---
}

// Feature 4: Thanh Toan Phat
std::string ThanhToanPhat(PTRDG docGia, const std::string& maSach) {
    if (!docGia) return "Lỗi: Độc giả null";
    
    MUONTRA p = docGia->data.dsmt;
    while (p) {
        std::string currentMa = p->data.maSachCached;
        if (currentMa.empty() && p->data.banSaoSach) currentMa = p->data.banSaoSach->maSach;
        
        if (currentMa == maSach) {
            // Found transaction (assuming latest or matching)
            // If multiple same books borrowed? (Rare/Impossible due to constraints? Actually possible if returned and borrowed again).
            // We should pick the one that is !daThanhToan and (TrangThai==1 or HasFine).
            // Prefer the one with TrangThai == 1 (Returned) first?
            
            if (!p->data.daThanhToan) {
                p->data.daThanhToan = true;
                
                // Sync DB
                QSqlQuery q(DatabaseManager::instance().database());
                q.prepare("UPDATE muon_tra SET da_thanh_toan=1 WHERE ma_the=? AND ma_sach=? AND ngay_muon=?");
                q.addBindValue(docGia->data.MaThe);
                q.addBindValue(QString::fromStdString(currentMa));
                q.addBindValue(QString::fromStdString(p->data.NgayMuon));
                
                if (q.exec()) {
                    AuditLogger::log(AuditLogger::ACTION_UPDATE, "MUON_TRA", QString::number(docGia->data.MaThe), 
                                     QString("Thanh toan phat: %1").arg(QString::fromStdString(currentMa)));
                    
                    // [NEW] Feature: Auto-Unlock Reader if All Debts Cleared
                    // Use helper TinhTongNo (Memory Based) for immediate check
                    long long totalDebt = TinhTongNo(docGia);
                    if (totalDebt == 0 && docGia->data.TrangThai == 0) {
                        // 1. Unlock in Memory
                        docGia->data.TrangThai = 1; // Hoat dong
                        
                        // 2. Unlock in DB
                        QSqlQuery qUnlock(DatabaseManager::instance().database());
                        qUnlock.prepare("UPDATE doc_gia SET trang_thai=1 WHERE ma_the=?");
                        qUnlock.addBindValue(docGia->data.MaThe);
                        if (qUnlock.exec()) {
                             AuditLogger::log(AuditLogger::ACTION_UPDATE, "DOC_GIA", QString::number(docGia->data.MaThe), 
                                              "Tu dong MO KHOA the (da thanh toan het no)");
                        }
                    }

                    return "";
                } else {
                    return "Lỗi DB: " + q.lastError().text().toStdString();
                }
            }
        }
        p = p->next;
    }
    return "Không tìm thấy giao dịch nợ để thanh toán.";
}

// Tinh tong no phat cua doc gia (Feature 4)
long long TinhTongNo(PTRDG docGia) {
    if (!docGia) return 0;
    long long tong = 0;
    MUONTRA p = docGia->data.dsmt;
    while (p) {
        // Chi tinh sach "Da Tra" nhung "Chua Thanh Toan"
        // Hoac sach dang muon neu muon tinh ca so tien du kien (hien tai ta chi tinh no thuc te)
        if (p->data.TrangThai == 1 && !p->data.daThanhToan) {
             tong += p->data.tienPhat;
        }
        p = p->next;
    }
    return tong;
}

// ==========================================
// DATABASE INTEGRATION
// ==========================================

void NapDocGiaTuDB(PTRDG &root, PTRDS dsDauSach[], int soLuongDS) {
    GiaiPhongCay(root);
    
    QSqlDatabase db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;
    
    // 1. Load Doc Gia
    QSqlQuery q(db);
    if (q.exec("SELECT ma_the, ho, ten, phai, trang_thai FROM doc_gia")) {
        while (q.next()) {
            int ma = q.value(0).toInt();
            std::string ho = q.value(1).toString().toStdString();
            std::string ten = q.value(2).toString().toStdString();
            int phai = q.value(3).toInt();
            int tt = q.value(4).toInt();
            
            PTRDG node = taoDocGia(ho, ten, phai, tt, ma);
            InsertDocGia(root, node, false); // false = no db sync
        }
    }
    
    // 2. Load Muon Tra
    if (q.exec("SELECT ma_the, ma_sach, ngay_muon, ngay_tra, trang_thai, so_lan_gia_han, da_thanh_toan, tien_phat FROM muon_tra")) {
        while (q.next()) {
            int maThe = q.value(0).toInt();
            std::string maSach = q.value(1).toString().toStdString();
            std::string ngayMuon = q.value(2).toString().toStdString();
            std::string ngayTra = q.value(3).toString().toStdString();
            int tt = q.value(4).toInt();
            int giaHan = q.value(5).toInt();
            bool thanhToan = q.value(6).toBool();
            long long tienPhat = q.value(7).toLongLong(); // Load tien_phat
            
            PTRDG dg = timDocGia(root, maThe);
            if (dg) {
                 MuonTra mt;
                 mt.banSaoSach = nullptr; // Load later or use cached logic
                 // Try to resolve pointer if we have book list
                 if (dsDauSach && soLuongDS > 0) {
                      std::string isbn = TachISBNTuMaSach(maSach);
                      PTRDS ds = TimDauSachTheoISBN(dsDauSach, soLuongDS, isbn);
                      if (ds) mt.banSaoSach = TimBanSaoTheoMa(ds, maSach);
                 }
                 
                 mt.maSachCached = maSach;
                 mt.NgayMuon = ngayMuon;
                 mt.NgayTra = ngayTra;
                 mt.TrangThai = tt;

                 // Feature 4
                 mt.soLanGiaHan = giaHan;
                 mt.daThanhToan = thanhToan;
                 mt.tienPhat = tienPhat; // Assign loaded fine

                 themMuonTra(dg, mt);
                 
                 // Fix counter logic: themMuonTra(overload) does NOT inc counter?
                 // Checking code: yes it didn't. But we need to maintain it.
                 if (tt == 0) // Dang muon
                 {
                     dg->data.soSachDangMuon++;
                 }
            }
        }
    }
}

// quan ly cay doc gia
// tao PTRDG docgia
PTRDG taoDocGia(std::string ho, std::string ten, bool phai, int trangthai, PTRDG root) {
    PTRDG dg = new NodeDG;
    dg->data.MaThe = sinhMaTheNgauNhien(root);
    // Normalize name fields
    dg->data.Ho = ChuanHoaHoTen(ho);
    dg->data.Ten = ChuanHoaHoTen(ten);
    dg->data.Phai = phai;
    dg->data.TrangThai = trangthai;
    // START PHASE 2 INIT
    dg->data.SDT = "";
    dg->data.Lop = "";
    dg->data.Email = "";
    dg->data.NgaySinh = "";
    dg->data.LoaiDocGia = 0; // Default Student
    // END PHASE 2 INIT
    dg->data.dsmt = nullptr;
    dg->data.soSachDangMuon = 0; 
    dg->data.soSachDangMuon = 0; 

    // Auto-set Dates
    std::string today = layNgayHienTai();
    dg->data.NgayLapThe = today;

    // Calculate Expiry
    int validityMonths = SettingsManager::instance().getInt("THOI_HAN_THE_THANG", 6);
    int d, m, y;
    if (phanTichNgay(today, d, m, y)) {
        m += validityMonths;
        while (m > 12) {
            m -= 12;
            y++;
        }
        // Handle end of month (simple clamp, e.g. 31/02 -> 28/02)
        // For simplicity in this C++98/Legacy context:
        if (d > 28) {
             // Basic check for Feb
             if (m == 2) {
                 bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
                 d = leap ? 29 : 28;
             } else if (m == 4 || m == 6 || m == 9 || m == 11) {
                 if (d > 30) d = 30;
             }
        }
        char buffer[20];
        sprintf(buffer, "%02d/%02d/%04d", d, m, y);
        dg->data.NgayHetHan = std::string(buffer);
    } else {
        dg->data.NgayHetHan = today; // Fallback
    }

    dg->left = dg->right = nullptr;
    dg->height = 1;
    return dg;
}
PTRDG taoDocGia(std::string ho, std::string ten, bool phai, int trangthai, int mathe) {
    PTRDG dg = new NodeDG;
    dg->data.MaThe = mathe;
    // Normalize name fields
    dg->data.Ho = ChuanHoaHoTen(ho);
    dg->data.Ten = ChuanHoaHoTen(ten);
    dg->data.Phai = phai;
    dg->data.TrangThai = trangthai;
     // START PHASE 2 INIT
    dg->data.SDT = "";
    dg->data.Lop = "";
    dg->data.Email = "";
    dg->data.NgaySinh = "";
    dg->data.LoaiDocGia = 0;
    // END PHASE 2 INIT
    dg->data.dsmt = nullptr;
    dg->data.soSachDangMuon = 0; 
    dg->data.soSachDangMuon = 0; 
    
    // Auto-set Dates (Similar logic)
    std::string today = layNgayHienTai();
    dg->data.NgayLapThe = today;
    
    int validityMonths = SettingsManager::instance().getInt("THOI_HAN_THE_THANG", 6);
    int d, m, y;
    if (phanTichNgay(today, d, m, y)) {
        m += validityMonths;
        while (m > 12) {
            m -= 12;
            y++;
        }
        if (d > 28) {
             if (m == 2) {
                 bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
                 d = leap ? 29 : 28;
             } else if (m == 4 || m == 6 || m == 9 || m == 11) {
                 if (d > 30) d = 30;
             }
        }
        char buffer[20];
        sprintf(buffer, "%02d/%02d/%04d", d, m, y);
        dg->data.NgayHetHan = std::string(buffer);
    } else {
        dg->data.NgayHetHan = today;
    }

    dg->left = dg->right = nullptr;
    dg->height = 1;
    return dg;
}

// giai phong dsmt
void giaiPhongDsmt(MUONTRA& dsmt) {
    // Defensive delete: handle possible self-loop corruption and avoid infinite loops
    while (dsmt != nullptr) {
        MUONTRA next = dsmt->next;
        // If node points to itself, delete and break
        if (next == dsmt) {
            delete dsmt;
            dsmt = nullptr;
            break;
        }
        delete dsmt;
        dsmt = next;
    }
}

// them ham giai phong toan bo cay
void GiaiPhongCay(PTRDG &root) {
    if (root == nullptr) return;
    GiaiPhongCay(root->left);
    GiaiPhongCay(root->right);
    giaiPhongDsmt(root->data.dsmt);
    delete root;
    root = nullptr;
}

// xoa node - cap nhat height va rebalance
static PTRDG xoaDocGiaRec(PTRDG root, int mathe) {
    if (root == nullptr) return root;

    if (mathe < root->data.MaThe) {
        root->left = xoaDocGiaRec(root->left, mathe);
    } else if (mathe > root->data.MaThe) {
        root->right = xoaDocGiaRec(root->right, mathe);
    } else {
        // found node
        if (root->left == nullptr || root->right == nullptr) {
            // Trường hợp 0 hoặc 1 con
            giaiPhongDsmt(root->data.dsmt);
            PTRDG temp = root->left ? root->left : root->right;
            delete root;
            return temp;
        } else {
            // Trường hợp 2 con: lấy inorder successor (succ)
            PTRDG succ = root->right;
            while (succ->left != nullptr) succ = succ->left;

            // CẢI TIẾN: Sửa lỗi bộ nhớ và cập nhật trường mới
            giaiPhongDsmt(root->data.dsmt); 
            
            // Sao chép dữ liệu của succ sang root
            root->data.MaThe = succ->data.MaThe;
            root->data.Ho = succ->data.Ho;
            root->data.Ten = succ->data.Ten;
            root->data.Phai = succ->data.Phai;
            root->data.TrangThai = succ->data.TrangThai;
            root->data.soSachDangMuon = succ->data.soSachDangMuon; // Cập nhật trường mới
            
            // Chuyển quyền sở hữu con trỏ dsmt từ succ sang root
            root->data.dsmt = succ->data.dsmt;
            succ->data.dsmt = nullptr;

            // Đệ quy xóa nút successor
            root->right = xoaDocGiaRec(root->right, root->data.MaThe);
        }
    }

    // cap nhat height va rebalance 
    if (root == nullptr) return root;
    updateHeight(root);
    int balance = getBalance(root);
    int leftBalance = getBalance(root->left);
    int rightBalance = getBalance(root->right);

    // LL
    if (balance > 1 && leftBalance >= 0)
        return RotateRight(root);
    // LR
    if (balance > 1 && leftBalance < 0) {
        root->left = RotateLeft(root->left);
        return RotateRight(root);
    }
    // RR
    if (balance < -1 && rightBalance <= 0)
        return RotateLeft(root);
    // RL
    if (balance < -1 && rightBalance > 0) {
        root->right = RotateRight(root->right);
        return RotateLeft(root);
    }

    return root;
}
void xoaDocGia(PTRDG& root, int mathe) {
    // check existence before deletion
    PTRDG found = timDocGia(root, mathe);
    root = xoaDocGiaRec(root, mathe);
    if (found) {
        try {
            EventBus::instance().docGiaRemoved(mathe);
        } catch(...) {}
        
        // --- DB SYNC ---
        QSqlQuery q(DatabaseManager::instance().database());
        q.prepare("DELETE FROM doc_gia WHERE ma_the = ?");
        q.addBindValue(mathe);
        q.exec();
        
        // Also delete transactions? Or keep for history?
        // Usually keeping history is better, but this app might want clean delete.
        // Let's delete transactions for now to match memory behavior (memory is gone).
        QSqlQuery q2(DatabaseManager::instance().database());
        q2.prepare("DELETE FROM muon_tra WHERE ma_the = ?");
        q2.addBindValue(mathe);
        q2.exec();
        q2.addBindValue(mathe);
        q2.exec();
        // --- END DB SYNC ---
        
        // [AUDIT LOG]
        AuditLogger::log(AuditLogger::ACTION_DELETE, "DOC_GIA", QString::number(mathe), "Xoa doc gia");
    }
}

// tim doc gia
PTRDG timDocGia(PTRDG root, int mathe) {
    while (root != nullptr && mathe != root->data.MaThe) {
        if (mathe < root->data.MaThe) root = root->left;
        else root = root->right;
    }
    return root;
}

// hieu chinh doc gia
void hieuChinhDocGia(PTRDG root, const TheDocGia& new_data) {
    PTRDG p = timDocGia(root, new_data.MaThe);
    if (p == nullptr) {
        std::cerr << "Khong tim thay doc gia co ma the " << new_data.MaThe << std::endl;
        return;
    }

    // Chỉ cập nhật các trường được phép thay đổi
    std::string hoChuan = ChuanHoaHoTen(new_data.Ho);
    std::string tenChuan = ChuanHoaHoTen(new_data.Ten);
    if (p->data.Ho != hoChuan) p->data.Ho = hoChuan;
    if (p->data.Ten != tenChuan) p->data.Ten = tenChuan;
    if (p->data.Phai != new_data.Phai) p->data.Phai = new_data.Phai;
    if (p->data.TrangThai != new_data.TrangThai) p->data.TrangThai = new_data.TrangThai;
    // Updated: Copy date fields
    if (p->data.NgayLapThe != new_data.NgayLapThe) p->data.NgayLapThe = new_data.NgayLapThe;
    if (p->data.NgayHetHan != new_data.NgayHetHan) p->data.NgayHetHan = new_data.NgayHetHan;
    
    // Phase 2 Updates: Copy new fields
    p->data.SDT = new_data.SDT;
    p->data.Lop = new_data.Lop;
    p->data.Email = new_data.Email;
    p->data.NgaySinh = new_data.NgaySinh;
    p->data.LoaiDocGia = new_data.LoaiDocGia;
    // Emit update event
    try {
        EventBus::instance().docGiaUpdated(new_data.MaThe);
    } catch(...) {}
    
    // --- DB SYNC ---
    // --- DB SYNC ---
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("UPDATE doc_gia SET ho=?, ten=?, phai=?, trang_thai=?, so_dien_thoai=?, lop=?, email=?, ngay_sinh=?, loai_doc_gia=? WHERE ma_the=?");
    q.addBindValue(QString::fromStdString(p->data.Ho));
    q.addBindValue(QString::fromStdString(p->data.Ten));
    q.addBindValue(p->data.Phai ? 1 : 0);
    q.addBindValue(p->data.TrangThai);
    // Phase 2 updates
    // Assume p was updated in memory before (Wait, logic above lines 573 only updated basic fields)
    // We need to update MEMORY p first! See correction below.
    q.addBindValue(QString::fromStdString(p->data.SDT));
    q.addBindValue(QString::fromStdString(p->data.Lop));
    q.addBindValue(QString::fromStdString(p->data.Email));
    q.addBindValue(QString::fromStdString(p->data.NgaySinh));
    q.addBindValue(p->data.LoaiDocGia);
    
    q.addBindValue(new_data.MaThe);
    q.exec();
    // --- END DB SYNC ---
    
    // [AUDIT LOG]
    AuditLogger::log(AuditLogger::ACTION_UPDATE, "DOC_GIA", QString::number(new_data.MaThe), "Hieu chinh thong tin doc gia");
}

// hien thi
void display(PTRDG root) {
    if (root == nullptr) {
        std::cerr << "Khong the hien thi: con tro doc gia null!" << std::endl;
        return;
    }

    std::cout << "MaThe: " << root->data.MaThe
         << " | HoTen: " << root->data.Ho << " " << root->data.Ten
         << " | Phai: " << (root->data.Phai ? "Nu" : "Nam")
         << " | TrangThai: " << (root->data.TrangThai ? "Hoat dong" : "Khoa")
         << " | Sach Dang Muon: " << root->data.soSachDangMuon
         << " | AVL Height: " << root->height
         << std::endl;
}

void inDocGiaInOrder(PTRDG root) {
    if (root != nullptr) {
        inDocGiaInOrder(root->left);
        display(root);
        inDocGiaInOrder(root->right);
    }
}

//B.In danh sach doc gia
// Helper: So sánh hai chuỗi theo Tên + Họ (sử dụng BoDauVaThuong)
static int SoSanhTenHo(PTRDG dg1, PTRDG dg2) {
    // 1. So sánh theo Tên (không dấu, không phân biệt hoa/thường)
    std::string ten1 = ChuanHoaChuoiTimKiem(dg1->data.Ten);
    std::string ten2 = ChuanHoaChuoiTimKiem(dg2->data.Ten);
    if (ten1 != ten2) {
        return (ten1 < ten2) ? -1 : 1;
    }
    
    // 2. Nếu Tên bằng nhau, so sánh theo Họ
    std::string ho1 = ChuanHoaChuoiTimKiem(dg1->data.Ho);
    std::string ho2 = ChuanHoaChuoiTimKiem(dg2->data.Ho);
    if (ho1 != ho2) {
        return (ho1 < ho2) ? -1 : 1;
    }
    
    // 3. Nếu Tên và Họ giống nhau, so sánh theo Mã thẻ để đảm bảo thứ tự ổn định
    if (dg1->data.MaThe < dg2->data.MaThe) return -1;
    if (dg1->data.MaThe > dg2->data.MaThe) return 1;
    
    return 0; 
}

// SwapDG removed (unused)

// Wrapper using std::sort instead of recursive QuickSort
#include <algorithm>
void QuickSortDocGia(PTRDG arr[], int low, int high) {
    if (low >= high) return;
    std::sort(arr + low, arr + high + 1, [](PTRDG a, PTRDG b) {
        return SoSanhTenHo(a, b) < 0; // Use existing comparator
    });
}

// ... (DuyetCayRaMang omitted, keeping it conceptually) ...


// Duyệt cây InOrder ra mảng
// Duyệt cây InOrder ra mảng (Iterative - Fix Stack Overflow)
void DuyetCayRaMang(PTRDG root, PTRDG arr[], int &count) {
    if (root == nullptr) return;

    std::stack<PTRDG> s;
    PTRDG curr = root;

    while (curr != nullptr || !s.empty()) {
        // Reach the left most Node of the curr Node
        while (curr != nullptr) {
            s.push(curr);
            curr = curr->left;
        }

        // Current must be NULL at this point
        curr = s.top();
        s.pop();

        // Add the node to the array if within bounds
        if (count < MAX_DAUSACH) { // Using MAX_DAUSACH as a safety limit (assumed to be large enough or should use passed size)
             arr[count++] = curr;
        } else {
             // Break if array is full to avoid overflow
             break; 
        }

        // We have visited the node and its left subtree. Now, it's right subtree's turn
        curr = curr->right;
    }
}
// --- HẾT LOGIC SẮP XẾP ---

//F.Muon Sach
// ==============================================================
// FUNCTIONS MOVED TO src/models/MuonTra.cpp
// ==============================================================
// The following functions have been refactored and moved:
// - TimBanSaoCoTheMuon()
// - TimBanSaoTheoMa()
// - DemBanSaoCoTheMuon()
// - MuonSach()
// - TraSach()
// - BaoMatSach()
// - themMuonTra() [both overloads]
// - LayDSSachDangMuon()
// - TinhSoNgayQuaHanLonNhat()
//
// Please include "models/MuonTra.h" to use these functions.
// ==============================================================


// ============================================
// HAM BAO MAT VA CAC CHUC NANG KHAC
// ============================================
// Ham BaoMatSach da duoc chuyen sang MuonTra.cpp



// ham backend: muon sach (O(log N) + O(M))


string PTRDMS_to_String(PTRDMS p){
    return (p != nullptr)? p->maSach : "";
}

PTRDMS MaSach_to_PTRDMS(const string& maSach, PTRDS dsDauSach[], int soLuongDauSach){
    std::stringstream dummyStream;
    return timDanhMucTheoMaSach(maSach, dsDauSach,soLuongDauSach,dummyStream, true);
}



// so sanh qua han (giam dan)
static int SoSanhQuaHan(const ThongTinQuaHan& a, const ThongTinQuaHan& b) {
    if (a.soNgayQuaHanMax != b.soNgayQuaHanMax) {
        return (a.soNgayQuaHanMax > b.soNgayQuaHanMax) ? -1 : 1;
    }
    return SoSanhTenHo(a.docGia, b.docGia);
}

// thu thap doc gia qua han (duyet inorder)
static void ThuThapDocGiaQuaHanRec(PTRDG root, ThongTinQuaHan arr[], int& count) {
    if (root == nullptr) return;
    
    // duyet InOrder (LNR)
    ThuThapDocGiaQuaHanRec(root->left, arr, count);
    
    // xu ly node hien tai
    int maxQuaHan = TinhSoNgayQuaHanLonNhat(root);  // O(L)
    
    if (maxQuaHan > 0) {
        if (count < MAX_DAUSACH) {
            arr[count].docGia = root;
            arr[count].soNgayQuaHanMax = maxQuaHan;
            count++;
        }
    }
    
    ThuThapDocGiaQuaHanRec(root->right, arr, count);
}

// ham ho tro sap xep
static void SwapQH(ThongTinQuaHan& a, ThongTinQuaHan& b) {
    ThongTinQuaHan temp = a; a = b; b = temp;
}

static int PartitionQH(ThongTinQuaHan arr[], int low, int high) {
    ThongTinQuaHan pivot = arr[high];
    int i = low - 1;
    
    for (int j = low; j <= high - 1; j++) {
        if (SoSanhQuaHan(arr[j], pivot) < 0) {
            i++;
            SwapQH(arr[j], arr[i]);
        }
    }
    SwapQH(arr[i + 1], arr[high]);
    return i + 1;
}

static void QuickSortQuaHan(ThongTinQuaHan arr[], int low, int high) {
    if (low < high) {
        int pi = PartitionQH(arr, low, high);
        QuickSortQuaHan(arr, low, pi - 1);
        QuickSortQuaHan(arr, pi + 1, high);
    }
}

// ham backend chinh: lay ds doc gia qua han (cap nhat dung DocGiaQuaHanDTO)
int LayDSDocGiaQuaHan(PTRDG root, DocGiaQuaHanDTO arr[], int maxKetQua) {
    if (root == nullptr || arr == nullptr || maxKetQua <= 0) return 0;
    
    // Tao mang tam ThongTinQuaHan de thu thap
    ThongTinQuaHan temp[1000];
    int count = 0;
    
    // 1. thu thap du lieu
    ThuThapDocGiaQuaHanRec(root, temp, count);  // O(N * L)
    
    // 2. sap xep mang quicksort
    if (count > 1) {
        QuickSortQuaHan(temp, 0, count - 1);  // O(N log N)
    }
    
    // 3. chuyen sang DocGiaQuaHanDTO
    int result = (count < maxKetQua) ? count : maxKetQua;
    for (int i = 0; i < result; ++i) {
        arr[i].docGia = temp[i].docGia;
        arr[i].soNgayQuaHanMax = temp[i].soNgayQuaHanMax;
    }
    
    return result;
}

//ham backend chinh: lay top 10 sach dang muon
static int SoSanhLM(const TopSachDTO& a, const TopSachDTO& b){
    //Sap xep soLuotMuon (giam dan)
    int LuotMuonA = (a.dauSach != nullptr) ? a.dauSach->soLuotMuon : 0;
    int LuotMuonB = (b.dauSach != nullptr) ? b.dauSach->soLuotMuon : 0;

    if(LuotMuonA != LuotMuonB){
        return (LuotMuonA > LuotMuonB) ? -1 : 1;
    }

    //Sap xep tenSach (tang dan)
    std::string tenA = (a.dauSach != nullptr) ? ChuanHoaChuoiTimKiem(a.dauSach->tenSach) : "";
    std::string tenB = (b.dauSach != nullptr) ? ChuanHoaChuoiTimKiem(b.dauSach->tenSach) : "";
    
    if (tenA != tenB) {
        return (tenA < tenB) ? -1 : 1;
    }
    return 0;//trung nhau
}

static void SwapTS(TopSachDTO& a, TopSachDTO& b){
    TopSachDTO temp = a; a = b; b = temp;
}

static int PartitionTS(TopSachDTO arr[], int low, int high){
    TopSachDTO pivot = arr[high];
    int i = (low - 1);

    for(int j = low; j < high - 1; j++){
        if (SoSanhLM(arr[j], pivot) < 0){
            i++;
            SwapTS(arr[i], arr[j]);
        }
    }
    SwapTS(arr[i + 1], arr[high]);
    return (i + 1);
}

static void QuickSortTS(TopSachDTO arr[], int low, int high){
    if(low < high){
        int pi = PartitionTS(arr, low, high);
        QuickSortTS(arr, low, pi - 1);
        QuickSortTS(arr, pi + 1, high);
    }
}
int LayTopSach(PTRDS dsDauSach[], int soLuongDS, TopSachDTO arr[]){
    // --- ĐOẠN DEBUG---
    std::cout << "\n--- DEBUG TOP 10 ---\n";
    std::cout << "So luong dau sach nhan duoc: " << soLuongDS << "\n";
    
    if(dsDauSach == nullptr) {
        std::cout << "LOI: Mang dsDauSach bi NULL!\n";
        return 0;
    }
    // -----------------

    if(arr == nullptr || soLuongDS == 0)  return 0;

    int count = 0;
    
    for(int i = 0; i < soLuongDS; i++){
        // ĐK QUAN TRỌNG: Lấy cả sách có lượt mượn >= 0 (thay vì > 0)
        if(dsDauSach[i] != nullptr && dsDauSach[i]->soLuotMuon >= 0){ 
            if(count < MAX_DAUSACH){
                arr[count].dauSach = dsDauSach[i];
                count++;
            }
        }
    }
    
    std::cout << "So luong sach lay duoc vao Top: " << count << "\n"; // In kết quả đếm
    
    if(count == 0) return 0; 

    if(count > 1){
        QuickSortTS(arr, 0, count - 1);
    }

    return (count < 10) ? count : 10;
}

//Save
//ham de quy
// LuuNodeVaoFile removed (unused)

void saveDocGia(PTRDG root) {
    Q_UNUSED(root);
    std::cout << "[DEPRECATED] saveDocGia (Text) calling GhiDocGiaFile (Stub). Skipped.\n";
}
//Load
PTRDG loadDocGia(PTRDS dsDauSach[], int soLuongDS) {
    Q_UNUSED(dsDauSach);
    Q_UNUSED(soLuongDS);
    std::cout << "[DEPRECATED] loadDocGia (Text) calling NapDocGiaFile (Stub). Skipped.\n";
    return nullptr;
}

// Hàm kiểm tra chuỗi fullString có BẮT ĐẦU bằng prefix hay không
bool KiemTraBatDauBang(const std::string& fullString, const std::string& prefix) {
    if(prefix.empty()) return true;
    if(prefix.length() > fullString.length()) return false;

    for(size_t i = 0; i < prefix.length(); i++){
        if(std::tolower(fullString[i]) != std::tolower(prefix[i])){
            return false;
        }
    }
    // DEBUG
    // std::cout << "Match: [" << prefix << "] in [" << fullString << "] -> TRUE\n";
    return true;
}


//  THONG KE - STATISTICS FUNCTIONS FOR DOC GIA 

// helper de duyet cay va dem
// helper de duyet cay va dem (Iterative version to prevent Stack Overflow)
static void DemDocGiaIterative(PTRDG root, int& tong, int& hoatDong, int& khoa, 
                               int& dangMuon, int& tongSachMuon, int& coQuaHan_Out) {
                               
    if (!root) return;

    std::stack<PTRDG> s;
    std::set<PTRDG> visited; // Detect cycles
    s.push(root);

    time_t now = time(nullptr); // Check time once per batch

    while (!s.empty()) {
        PTRDG node = s.top();
        s.pop();

        // Cycle/Duplicate safety
        if (visited.count(node)) {
            // thongBao(std::cout, "Phat hien cycle hoac duplicate node trong cay Doc Gia. Bo qua.", CANH_BAO);
            continue;
        }
        visited.insert(node);

        // Dem doc gia
        tong++;
        
        // Dem theo trang thai
        if (node->data.TrangThai == 1) {
            hoatDong++;
        } else {
            khoa++;
        }
        
        // Dem so sach dang muon (Using Cached Field O(1))
        int sachDangMuonCuaDocGia = node->data.soSachDangMuon;
        
        bool hasOverdue = false;
        if (sachDangMuonCuaDocGia > 0) {
            dangMuon++;
            tongSachMuon += sachDangMuonCuaDocGia;
            
            // Check Overdue (Using Cached Field O(1))
            if (node->data.ngayHetHanSomNhat == 0) {
                // Lazy Load
                CapNhatNgayHetHanSomNhat(node);
            }
            
            if (node->data.ngayHetHanSomNhat != 0 && node->data.ngayHetHanSomNhat < now) {
                hasOverdue = true;
            }
        }
        
        if (hasOverdue) {
            coQuaHan_Out++;
        }

        // Push children to stack
        if (node->right) s.push(node->right);
        if (node->left) s.push(node->left);
    }
}

// lay thong ke tong quan doc gia
ThongKeDocGia LayThongKeDocGia(PTRDG root, PTRDS /*dsDauSach*/[], int /*soLuongDauSach*/) {
    ThongKeDocGia thongKe = {};
    thongKe.tongSoDocGia = 0;
    thongKe.docGiaHoatDong = 0;
    thongKe.docGiaBiKhoa = 0;
    thongKe.docGiaDangMuonSach = 0;
    thongKe.tongSoSachDangMuon = 0;
    thongKe.docGiaCoSachQuaHan = 0;
    
    DemDocGiaIterative(root, thongKe.tongSoDocGia, thongKe.docGiaHoatDong, 
                       thongKe.docGiaBiKhoa, thongKe.docGiaDangMuonSach,
                      thongKe.tongSoSachDangMuon, thongKe.docGiaCoSachQuaHan);
    
    return thongKe;
}

// helper de dem doc gia theo trang thai
// helper de dem doc gia theo trang thai (Iterative)
static void DemTheoTrangThaiIterative(PTRDG root, int trangThai, int& dem) {
    if (!root) return;
    
    std::stack<PTRDG> s;
    s.push(root);
    
    while (!s.empty()) {
        PTRDG node = s.top();
        s.pop();
        
        if (node->data.TrangThai == trangThai) {
            dem++;
        }
        
        if (node->right) s.push(node->right);
        if (node->left) s.push(node->left);
    }
}

// dem doc gia theo trang thai (0: khoa, 1: hoat dong)
int DemDocGiaTheoTrangThai(PTRDG root, int trangThai) {
    int dem = 0;
    DemTheoTrangThaiIterative(root, trangThai, dem);
    return dem;
}

// helper de dem doc gia dang muon sach (Iterative)
static void DemDangMuonIterative(PTRDG root, int& dem) {
    if (!root) return;
    
    std::stack<PTRDG> s;
    s.push(root);
    
    while (!s.empty()) {
        PTRDG node = s.top();
        s.pop();
        
        // Count if currently borrowing (Using Cache O(1))
        if (node->data.soSachDangMuon > 0) {
            dem++;
        }
        
        if (node->right) s.push(node->right);
        if (node->left) s.push(node->left);
    }
}

// dem doc gia dang muon sach
int DemDocGiaDangMuonSach(PTRDG root) {
    int dem = 0;
    DemDangMuonIterative(root, dem);
    return dem;
}


//  TIM KIEM NANG CAO DOC GIA 

// helper de duyet cay va tim theo ten (Iterative)
static void TimTheoTenIterative(PTRDG root, const std::string& tenThuong, PTRDG ketQua[], int& count, int maxKetQua) {
    if (!root || count >= maxKetQua) return;
    
    std::stack<PTRDG> s;
    PTRDG curr = root;
    
    // In-order traversal iterative
    while (curr != nullptr || !s.empty()) {
        while (curr != nullptr) {
            s.push(curr);
            curr = curr->left;
        }
        
        curr = s.top();
        s.pop();
        
        // Process node
        if (count < maxKetQua) {
            std::string tenNodeThuong = BoDauVaThuong(curr->data.Ten);
            if (tenNodeThuong.find(tenThuong) != std::string::npos) {
                ketQua[count++] = curr;
            }
        } else {
            return; // Full buffer
        }
        
        curr = curr->right;
    }
}

// tim doc gia theo ten (khong phan biet hoa/thuong)
int TimDocGiaTheoTen(PTRDG root, const std::string& ten, PTRDG ketQua[], int maxKetQua) {
    if (!root || !ketQua || maxKetQua <= 0 || ten.empty()) return 0;
    
    std::string tenThuong = BoDauVaThuong(ten);
    int count = 0;
    
    TimTheoTenIterative(root, tenThuong, ketQua, count, maxKetQua);
    
    return count;
}

// helper de duyet cay va tim theo ho
// helper de duyet cay va tim theo ho (Iterative)
static void TimTheoHoIterative(PTRDG root, const std::string& hoThuong, PTRDG ketQua[], int& count, int maxKetQua) {
    if (!root || count >= maxKetQua) return;
    
    std::stack<PTRDG> s;
    PTRDG curr = root;
    
    while (curr != nullptr || !s.empty()) {
        while (curr != nullptr) {
            s.push(curr);
            curr = curr->left;
        }
        
        curr = s.top();
        s.pop();
        
        if (count < maxKetQua) {
            std::string hoNodeThuong = BoDauVaThuong(curr->data.Ho);
            if (hoNodeThuong.find(hoThuong) != std::string::npos) {
                ketQua[count++] = curr;
            }
        } else {
            return;
        }
        
        curr = curr->right;
    }
}

// tim doc gia theo ho (khong phan biet hoa/thuong)
int TimDocGiaTheoHo(PTRDG root, const std::string& ho, PTRDG ketQua[], int maxKetQua) {
    if (!root || !ketQua || maxKetQua <= 0 || ho.empty()) return 0;
    
    std::string hoThuong = BoDauVaThuong(ho);
    int count = 0;
    
    TimTheoHoIterative(root, hoThuong, ketQua, count, maxKetQua);
    
    return count;
}

// helper de lay danh sach theo trang thai
// helper de lay danh sach theo trang thai (Iterative)
static void LayTheoTrangThaiIterative(PTRDG root, int trangThai, PTRDG ketQua[], int& count, int maxKetQua) {
    if (!root || count >= maxKetQua) return;
    
    std::stack<PTRDG> s;
    PTRDG curr = root;
    
    while (curr != nullptr || !s.empty()) {
        while (curr != nullptr) {
            s.push(curr);
            curr = curr->left;
        }
        
        curr = s.top();
        s.pop();
        
        if (count < maxKetQua) {
            if (curr->data.TrangThai == trangThai) {
                ketQua[count++] = curr;
            }
        } else {
            return;
        }
        
        curr = curr->right;
    }
}

// lay danh sach doc gia theo trang thai
int TimDocGiaTheoTrangThai(PTRDG root, int trangThai, PTRDG ketQua[], int maxKetQua) {
    if (!root || !ketQua || maxKetQua <= 0) return 0;
    
    int count = 0;
    LayTheoTrangThaiIterative(root, trangThai, ketQua, count, maxKetQua);
    
    return count;
}

// helper de tim theo ho + ten
// helper de tim theo ho + ten (Iterative)
static void TimTheoHoTenIterative(PTRDG root, const std::string& hoTenThuong, PTRDG ketQua[], int& count, int maxKetQua) {
    if (!root || count >= maxKetQua) return;
    
    std::stack<PTRDG> s;
    PTRDG curr = root;
    
    while (curr != nullptr || !s.empty()) {
        while (curr != nullptr) {
            s.push(curr);
            curr = curr->left;
        }
        
        curr = s.top();
        s.pop();
        
        if (count < maxKetQua) {
            std::string hoTenNode = BoDauVaThuong(curr->data.Ho + " " + curr->data.Ten);
            if (hoTenNode.find(hoTenThuong) != std::string::npos) {
                ketQua[count++] = curr;
            }
        } else {
            return;
        }
        
        curr = curr->right;
    }
}

// tim doc gia theo ho + ten
int TimDocGiaTheoHoTen(PTRDG root, const std::string& hoTen, PTRDG ketQua[], int maxKetQua) {
    if (!root || !ketQua || maxKetQua <= 0 || hoTen.empty()) return 0;
    
    std::string hoTenThuong = BoDauVaThuong(hoTen);
    int count = 0;
    
    TimTheoHoTenIterative(root, hoTenThuong, ketQua, count, maxKetQua);
    
    return count;
}
// One-off migration: create backup of docgia file and re-save normalized data
// One-off migration: DEPRECATED - Database auto-migrates if needed
void MigrateNormalizeDocGiaFile(PTRDG root) {
    Q_UNUSED(root);
    std::cout << "[LEGACY] Skipping MigrateNormalizeDocGiaFile.\n";
}

// Gia han muon sach (them 7 ngay)
std::string GiaHanMuon(PTRDG docGia, const std::string& maSach) {
    if (!docGia) return "Lỗi: Thông tin độc giả không hợp lệ!";

    MUONTRA p = docGia->data.dsmt;
    while (p != nullptr) {
        // Tim sach trong danh sach muon (chua tra)
        bool match = false;
        if (p->data.banSaoSach && p->data.banSaoSach->maSach == maSach) match = true;
        else if (!p->data.maSachCached.empty() && p->data.maSachCached == maSach) match = true;

        if (match && p->data.TrangThai == 0) {
            // Parse ngay tra hien tai
            int d, m, y;
            if (!phanTichNgay(p->data.NgayTra, d, m, y)) {
                return "Lỗi: Ngày trả hiện tại không hợp lệ!";
            }
            
            // Tinh ngay moi (+7 ngay)
            QDate date(y, m, d);
            QDate newDate = date.addDays(7);
            
            // Cap nhat lai NgayTra
            p->data.NgayTra = newDate.toString("dd/MM/yyyy").toStdString();
            
            return ""; // Tra ve rong la thanh cong
        }
        p = p->next;
    }
    return "Lỗi: Không tìm thấy sách đang mượn với mã này!";
}

// ==========================================
// DATABASE LOADING (Corrected & Self-Healing)
// ==========================================
void NapDocGiaTuDB_Fixed(PTRDG &root, PTRDS dsDauSach[], int soLuongDS) {
    // 0. SELF-HEALING: Reset all ban_sao to CHO_MUON_DUOC first
    // This wipes out any "zombie" DANG_MUON statuses from the database table 'ban_sao'
    // and relies purely on the actual 'muon_tra' records to determine status.
    for (int i = 0; i < soLuongDS; i++) {
        if (!dsDauSach[i]) continue;
        PTRDMS p = dsDauSach[i]->dms;
        while (p) {
            // Force reset to Available
            p->trangThai = CHO_MUON_DUOC; // 0
            p = p->next;
        }
    }
    std::cout << "[DB FIXED] Reset all ban_sao statuses to Available. Re-applying from muon_tra...\n";

    // 1. Clean up old AVL tree
    GiaiPhongCay(root);
    root = nullptr;
    
    QSqlDatabase db = DatabaseManager::instance().database();
    if (!db.isOpen()) {
        std::cerr << "[DB ERROR] DB not open in NapDocGiaTuDB!\n";
        return;
    }
    
    // 2. Load Doc Gia
    QSqlQuery q(db);
    // Use low-level check
    // Phase 2: Add SDT, Lop... to select
    if (!q.exec("SELECT ma_the, ho, ten, phai, trang_thai, so_sach_dang_muon, so_dien_thoai, lop, email, ngay_sinh, loai_doc_gia FROM doc_gia ORDER BY ma_the")) {
         std::cerr << "[DB ERROR] SQL doc_gia: " << q.lastError().text().toStdString() << "\n";
         return;
    }
    
    while (q.next()) {
        int maThe = q.value(0).toInt();
        QString hoQ = q.value(1).toString();
        QString tenQ = q.value(2).toString();
        int phaiQ = q.value(3).toInt();
        int trangQ = q.value(4).toInt();
        int soSachDangMuon = q.value(5).toInt();
        // Phase 2 Columns
        QString sdt = q.value(6).toString();
        QString lop = q.value(7).toString();
        QString email = q.value(8).toString();
        QString ngaySinh = q.value(9).toString();
        int loaiDG = q.value(10).toInt();
        
        // Create node
        PTRDG node = taoDocGia(hoQ.toStdString(), tenQ.toStdString(), phaiQ != 0, trangQ, maThe);
        node->data.soSachDangMuon = soSachDangMuon;
        // Assign new fields
        node->data.SDT = sdt.toStdString();
        node->data.Lop = lop.toStdString();
        node->data.Email = email.toStdString();
        node->data.NgaySinh = ngaySinh.toStdString();
        node->data.LoaiDocGia = loaiDG;
        
        InsertDocGia(root, node, false); // false = no DB sync during load
    }
    
    // 3. Load Muon Tra
    QSqlQuery q2(db);
    if (q2.exec("SELECT m.ma_the, b.ma_sach, m.ngay_muon, m.ngay_tra, m.trang_thai, m.so_lan_gia_han, m.da_thanh_toan, m.tien_phat FROM muon_tra m JOIN ban_sao b ON m.ban_sao_id = b.id ORDER BY m.id")) {
        while (q2.next()) {
            int maThe = q2.value(0).toInt();
            QString maSachQ = q2.value(1).toString();
            QString ngayMuonQ = q2.value(2).toString();
            QString ngayTraQ = q2.value(3).toString();
            int trangQ = q2.value(4).toInt();
            int soLanGiaHan = q2.value(5).toInt();
            bool daThanhToan = q2.value(6).toBool();
            long long tienPhat = q2.value(7).toLongLong();
            
            PTRDG p = timDocGia(root, maThe);
            if (!p) continue;
            
            MuonTra mt;
            mt.NgayMuon = ngayMuonQ.toStdString();
            mt.NgayTra = ngayTraQ.toStdString();
            mt.TrangThai = trangQ;
            // Feature 4 fields
            mt.soLanGiaHan = soLanGiaHan;
            mt.daThanhToan = daThanhToan;
            mt.tienPhat = tienPhat;
            
            // Resolve ban_sao pointer
            mt.banSaoSach = MaSach_to_PTRDMS(maSachQ.toStdString(), dsDauSach, soLuongDS);
            mt.maSachCached = maSachQ.toStdString();
            
            // CRITICAL: Re-apply status if Active Loan
            if (mt.banSaoSach != nullptr) {
                if (trangQ == 0) { // Dang muon
                    mt.banSaoSach->trangThai = DANG_MUON; // 1
                } else if (trangQ == 2) { // Mat sach
                     mt.banSaoSach->trangThai = THANH_LY; // 2 (or equivalent)
                }
                // If returned (1), we leave it as CHO_MUON_DUOC (default reset)
            }
            
            // Update stats
            if (!maSachQ.isEmpty()) {
                 std::string isbn = TachISBNTuMaSach(maSachQ.toStdString());
                 PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDS, isbn);
                 if (dauSach) dauSach->soLuotMuon++;
            }
            
            themMuonTra(p, mt);
        }
    } else {
        std::cerr << "[DB ERROR] SQL muon_tra: " << q2.lastError().text().toStdString() << "\n";
    }
    
    // 4. Recompute counters
    std::function<void(PTRDG)> recomputeCounts;
    recomputeCounts = [&](PTRDG node) {
        if (!node) return;
        recomputeCounts(node->left);
        
        int cnt = 0;
        MUONTRA p = node->data.dsmt;
        int safety = 0;
        while (p && safety < 10000) {
            if (p->data.TrangThai == 0) cnt++;
            p = p->next; safety++;
        }
        node->data.soSachDangMuon = cnt;
        
        recomputeCounts(node->right);
    };
    recomputeCounts(root);
}

// Duyệt cây InOrder ra vector (Thay thế mảng tĩnh, an toàn bộ nhớ)
void DuyetCayRaVector(PTRDG root, std::vector<PTRDG>& result) {
    if (root == nullptr) return;

    std::stack<PTRDG> s;
    PTRDG curr = root;

    while (curr != nullptr || !s.empty()) {
        // Reach the left most Node of the curr Node
        while (curr != nullptr) {
            s.push(curr);
            curr = curr->left;
        }

        // Current must be NULL at this point
        curr = s.top();
        s.pop();

        // Add to vector
        result.push_back(curr);

        // We have visited the node and its left subtree. Now, it's right subtree's turn
        curr = curr->right;
    }
}