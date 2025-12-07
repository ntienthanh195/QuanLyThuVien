#include <fstream>
#include <iostream>
#include <string>
#include "models/ThaoTacFile.h"
#include "models/KiemTraDuLieu.h"
#include "utils/XuLyChuoi.h"
#include "models/QuanLySach.h"
#include "utils/ThongBao.h"
#include "models/DauSach.h"
#include "utils/BackupManager.h"
#include "Constants.h"
#include "utils/FilePathManager.h"
#include "utils/DatabaseManager.h"
#include "utils/EventBus.h"
#include <QSqlQuery>
#include <QVariant>
#include "utils/Perf.h"

using namespace std;

static bool KiemTraSoTrangFile(int soTrang, std::ostream &out) {
    if (soTrang <= 0 || soTrang > MAX_SO_TRANG) {
        thongBao(out, "So trang phai tu 1-" + to_string(MAX_SO_TRANG) + "!", LOI);
        return false;
    }
    return true;
}

void NapDanhSachDauSach(const char *path, PTRDS dsDauSach[], int &soLuongDauSach, std::ostream &out) {
    PerfTimer perf("NapDanhSachDauSach");
    try {
        // If a SQLite DB is available, prefer loading from DB
        QString dbPath = FilePathManager::getLibraryDbPath();
        if (DatabaseManager::instance().connect(dbPath)) {
            QSqlQuery q(DatabaseManager::instance().database());
            if (q.exec("SELECT isbn, ten_sach, so_trang, tac_gia, nam_xuat_ban, the_loai FROM dau_sach ORDER BY id")) {
                int soDong = 0, thanhCong = 0, boQua = 0;
                while (q.next()) {
                    ++soDong;
                    QString isbnQ = q.value(0).toString();
                    QString tenQ = q.value(1).toString();
                    int soTrangQ = q.value(2).toInt();
                    QString tacGiaQ = q.value(3).toString();
                    int namQ = q.value(4).toInt();
                    QString theLoaiQ = q.value(5).toString();

                    std::string isbn = isbnQ.toStdString();
                    std::string tenSach = tenQ.toStdString();
                    std::string tacGia = tacGiaQ.toStdString();
                    std::string theLoai = theLoaiQ.toStdString();
                    int soTrang = soTrangQ;
                    int namXuatBan = namQ;

                    // reuse existing validation by attempting to add via themDauSach
                    if (themDauSach(dsDauSach, soLuongDauSach, isbn, tenSach, soTrang, tacGia, namXuatBan, theLoai, true)) {
                        dsDauSach[soLuongDauSach - 1]->soLuotMuon = 0;
                        ++thanhCong;
                    } else {
                        ++boQua;
                    }
                }
                InTongKet("Doc dau sach (DB)", thanhCong, soDong, boQua, out);
                DatabaseManager::instance().close();
                return;
            }
            // if DB query failed, fall back to file reading below
            DatabaseManager::instance().close();
        }

        std::ifstream inp(path);
        if (!inp.is_open()) {                                                       // khong mo duoc file
            thongBao(out, "Loi: Khong the mo file dau sach: " + std::string(path), LOI);
            
            // Thu tao file moi neu khong ton tai
            std::ofstream createFile(path);
            if (createFile.is_open()) {
                createFile.close();
                thongBao(out, "Da tao file moi: " + std::string(path), THONG_TIN);
            }
            return;
        }

    int soDong = 0, thanhCong = 0, boQua = 0;                                    // dem so dong, thanh cong, bo qua
    std::string dong;                                                           // luu noi dung dong hien tai
    char buffer[DUNG_LUONG_CACHE_MAC_DINH];                                     // bo dem doc nhanh

    while (inp.getline(buffer, DUNG_LUONG_CACHE_MAC_DINH)) {                    // doc tung dong den het file
        dong = buffer;                                                          // chuyen buffer sang string
        ++soDong;                                                               // tang so thu tu dong
        
        // Kiem tra dong rong hoac chi chua khoang trang
        if (dong.empty() || dong.find_first_not_of(" \t\r\n") == std::string::npos) {
            ++boQua;
            continue;
        }

        std::string truong[SO_TRUONG_DAU_SACH];                                  // mang luu cac truong sau khi tach
        if (!TachTruong(dong, '|', truong, SO_TRUONG_DAU_SACH)) {               // tach bang dau '|'
            thongBao(out, thongBaoLoi(soDong, "Sai dinh dang dong: " + dong, true), CANH_BAO);
            ++boQua;                                                            // sai so truong -> bo qua
            continue;
        }
        
        // Kiem tra NULL bytes trong cac truong (phong chong du lieu bi hong)
        bool coDuLieuLoi = false;
        for (int i = 0; i < SO_TRUONG_DAU_SACH; ++i) {
            if (truong[i].find('\0') != std::string::npos) {
                thongBao(out, thongBaoLoi(soDong, "Du lieu chua ky tu NULL khong hop le", true), CANH_BAO);
                coDuLieuLoi = true;
                break;
            }
        }
        if (coDuLieuLoi) {
            ++boQua;
            continue;
        }

        int soTrang, namXuatBan;                                                // bien tam luu so trang, nam xb
        std::string isbn, tenSach, tacGia, theLoai;                             // bien tam luu du lieu chuan hoa
        std::string loi;                                                        // luu thong tin loi

        // isbn: kiem tra rong, dinh dang, trung
        loi = KiemTraChuoiRong(truong[0], "ISBN");                              // kiem rong
        std::string loiISBN = ChuanHoaISBNFile(truong[0], isbn);                // chuan hoa 10/13 so
        std::string loiTrung = KiemTraTrungISBN(isbn);                          // kiem trung trong danh sach

        if (!loi.empty() || !loiISBN.empty() || !loiTrung.empty()) {            // co loi nao do
            std::string loiHienThi = loi.empty() ? (loiISBN.empty() ? loiTrung : loiISBN) : loi;
            thongBao(out, thongBaoLoi(soDong, loiHienThi + " ('" + truong[0] + "')", true), CANH_BAO);
            ++boQua;                                                            // bo qua dong
            continue;
        }

        // ten sach: kiem do dai, ky tu
        try {
            loi = ChuanHoaTenUnicode(truong[1], MIN_TEN_SACH, MAX_TEN_SACH, tenSach);
            if (!loi.empty()) {                                                     // ten khong hop le
                thongBao(out, thongBaoLoi(soDong, loi + " ('" + truong[1] + "')", true), CANH_BAO);
                ++boQua;
                continue;
            }
        } catch (const std::exception& e) {
            thongBao(out, thongBaoLoi(soDong, "Loi xu ly ten sach: " + std::string(e.what()), true), CANH_BAO);
            ++boQua;
            continue;
        }

        // so trang: chuyen chuoi -> so, kiem gioi han
        if (!ChuyenChuoiThanhSoNguyen(CatKhoangTrang(truong[2]), soTrang, false) || !KiemTraSoTrangFile(soTrang, out)) {
            thongBao(out, thongBaoLoi(soDong, "So trang khong hop le: " + truong[2], true), CANH_BAO);
            ++boQua;
            continue;
        }

        // tac gia: kiem do dai, ky tu
        try {
            loi = ChuanHoaTenUnicode(truong[3], MIN_TAC_GIA, MAX_TAC_GIA, tacGia);
            if (!loi.empty()) {
                thongBao(out, thongBaoLoi(soDong, loi + " ('" + truong[3] + "')", true), CANH_BAO);
                ++boQua;
                continue;
            }
        } catch (const std::exception& e) {
            thongBao(out, thongBaoLoi(soDong, "Loi xu ly tac gia: " + std::string(e.what()), true), CANH_BAO);
            ++boQua;
            continue;
        }

        // nam xuat ban: phai la so, trong khoang 1800 - hien tai
        if (!ChuyenChuoiThanhSoNguyen(CatKhoangTrang(truong[4]), namXuatBan, false)) {
            thongBao(out, thongBaoLoi(soDong, "Nam xuat ban phai la so: '" + truong[4] + "'", true), CANH_BAO);
            ++boQua;
            continue;
        }
        loi = KiemTraNamXuatBan(namXuatBan);
        if (!loi.empty()) {
            thongBao(out, thongBaoLoi(soDong, loi + " ('" + truong[4] + "')", true), CANH_BAO);
            ++boQua;
            continue;
        }

        // the loai: kiem do dai, ky tu
        try {
            loi = ChuanHoaTenUnicode(truong[5], MIN_THE_LOAI, MAX_THE_LOAI, theLoai);
            if (!loi.empty()) {
                thongBao(out, thongBaoLoi(soDong, loi + " ('" + truong[5] + "')", true), CANH_BAO);
                ++boQua;
                continue;
            }
        } catch (const std::exception& e) {
            thongBao(out, thongBaoLoi(soDong, "Loi xu ly the loai: " + std::string(e.what()), true), CANH_BAO);
            ++boQua;
            continue;
        }

        // them dau sach vao mang
        if (themDauSach(dsDauSach, soLuongDauSach, isbn, tenSach, soTrang, tacGia, namXuatBan, theLoai, true)) {
            dsDauSach[soLuongDauSach - 1]->soLuotMuon = 0;
            ++thanhCong;                                                        // them thanh cong
        } else {
            thongBao(out, thongBaoLoi(soDong, "Khong the them dau sach (da day?): " + truong[1], true), CANH_BAO);
            ++boQua;                                                            // mang day hoac loi
        }
    }

    if (inp.eof()) {                                                            // doc het file binh thuong
        InTongKet("Doc dau sach", thanhCong, soDong, boQua, out);
    } else if (inp.fail()) {                                                    // loi giua chung
        thongBao(out, "Loi: Gap su co khi doc file dau sach!", LOI);
    }
    inp.close();                                                                // dong file
    } catch (const std::exception &e) {
        thongBao(out, "Loi ngoai le khi nap dau sach: " + std::string(e.what()), LOI);
    } catch (...) {
        thongBao(out, "Loi khong xac dinh khi nap dau sach", LOI);
    }
}


void NapDanhMucSach(const char *path, PTRDS dsDauSach[], int soLuongDauSach, std::ostream &out) {
    PerfTimer perf("NapDanhMucSach");
    try {
        // Prefer DB if available
        QString dbPath = FilePathManager::getLibraryDbPath();
        if (DatabaseManager::instance().connect(dbPath)) {
            QSqlQuery q(DatabaseManager::instance().database());
            if (q.exec("SELECT b.ma_sach, b.trang_thai, b.vi_tri, d.isbn FROM ban_sao b JOIN dau_sach d ON b.dau_sach_id = d.id ORDER BY b.id")) {
                int soDong = 0, thanhCong = 0, boQua = 0;
                while (q.next()) {
                    ++soDong;
                    QString maSachQ = q.value(0).toString();
                    int trangThaiQ = q.value(1).toInt();
                    QString viTriQ = q.value(2).toString();
                    QString isbnQ = q.value(3).toString();

                    std::string maSach = maSachQ.toStdString();
                    std::string viTri = viTriQ.toStdString();
                    std::string isbn = isbnQ.toStdString();
                    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
                    if (!dauSach) {
                        ++boQua;
                        continue;
                    }
                    if (KiemTraTrungmaSach(maSach, out)) {
                        ++boQua;
                        continue;
                    }
                    std::string loiThem = themDanhMucSach(dauSach->dms, maSach, static_cast<TrangThaiSach>(trangThaiQ), viTri);
                    if (!loiThem.empty()) {
                        ++boQua;
                        continue;
                    }
                    ++thanhCong;
                }
                InTongKet("Doc danh muc sach (DB)", thanhCong, soDong, boQua, out);
                DatabaseManager::instance().close();
                CapNhatTongBanSao(dsDauSach, soLuongDauSach);
                return;
            }
            DatabaseManager::instance().close();
        }

        std::ifstream inp(path);
        if (!inp.is_open()) {
            thongBao(out, "Loi: Khong the mo file danh muc sach: " + std::string(path), LOI);
            return;
        }

    int soDong = 0, thanhCong = 0, boQua = 0;                                    // dem dong, thanh cong, bo qua
    std::string dong;                                                           // luu noi dung dong
    char buffer[DUNG_LUONG_CACHE_MAC_DINH];                                     // bo dem doc nhanh

    while (inp.getline(buffer, DUNG_LUONG_CACHE_MAC_DINH)) {                    // doc tung dong
        dong = buffer;
        ++soDong;
        
        // Kiem tra dong rong hoac chi chua khoang trang
        if (dong.empty() || dong.find_first_not_of(" \t\r\n") == std::string::npos) {
            ++boQua;
            continue;
        }

        std::string truong[3];                                                  // mang luu: masach | trangthai | vitri
        if (!TachTruong(dong, '|', truong, 3)) {                                // sai so truong
            thongBao(out, thongBaoLoiDMS(soDong, "Sai dinh dang dong: " + dong, true), CANH_BAO);
            ++boQua;
            continue;
        }

        std::string maSach = CatKhoangTrang(truong[0]);                         // cat khoang trang thua
        int trangThai = PhanTichTrangThaiSach(truong[1], out);                  // 0,1,2 -> enum
        std::string viTri = ChuanHoaViTri(truong[2]);                           // chuan hoa vi tri ke

        if (trangThai == -1 || viTri.empty() || viTri.length() > MAX_VI_TRI_KE) { // trang thai hoac vi tri loi
            thongBao(out, thongBaoLoiDMS(soDong, "Trang thai hoac vi tri khong hop le: " + dong, true), CANH_BAO);
            ++boQua;
            continue;
        }

        std::string isbn = TachISBNTuMaSach(maSach);                            // lay 13 ky tu dau cua ma sach
        PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);    // tim dau sach cha
        if (!dauSach) {                                                         // khong tim thay
            thongBao(out, thongBaoLoiDMS(soDong, "Khong tim thay dau sach cho ma sach: " + maSach, true), CANH_BAO);
            ++boQua;
            continue;
        }

        if (KiemTraTrungmaSach(maSach, out)) {                                  // ma sach da ton tai
            ++boQua;
            continue;
        }

        std::string loiThem = themDanhMucSach(dauSach->dms, maSach, static_cast<TrangThaiSach>(trangThai), viTri);
        if (!loiThem.empty()) {                                                 // loi khi them vao dms
            thongBao(out, thongBaoLoiDMS(soDong, "Khong the them ma sach: " + maSach + ". (" + loiThem + ")", true), CANH_BAO);
            ++boQua;
            continue;
        }

        ++thanhCong;                                                            // them thanh cong
    }

    if (inp.eof()) {                                                            // doc het file
        InTongKet("Doc danh muc sach", thanhCong, soDong, boQua, out);
    } else if (inp.fail()) {                                                    // loi doc giua chung
        thongBao(out, "Loi: Gap su co khi doc file danh muc sach!", LOI);
    }
    inp.close();                                                                // dong file
    CapNhatTongBanSao(dsDauSach, soLuongDauSach);                               // cap nhat lai tong ban sao
    } catch (const std::exception &e) {
        thongBao(out, "Loi ngoai le khi nap danh muc sach: " + std::string(e.what()), LOI);
    } catch (...) {
        thongBao(out, "Loi khong xac dinh khi nap danh muc sach", LOI);
    }
}



bool GhiDanhSachDauSach(const char *path, PTRDS dsDauSach[], int soLuongDauSach, bool silent, std::ostream &out) {
    try {
        // Tao backup truoc khi ghi
        if (!silent) {
            std::string loiBackup = AutoBackup();
            if (!loiBackup.empty()) {
                thongBao(out, "Canh bao: " + loiBackup, CANH_BAO);
            }
        }
        
        // Try writing to DB if available
        QString dbPath = FilePathManager::getLibraryDbPath();
        if (DatabaseManager::instance().connect(dbPath)) {
            QSqlQuery q(DatabaseManager::instance().database());
            DatabaseManager::instance().beginTransaction();
            bool ok = true;
            for (int i = 0; i < soLuongDauSach; ++i) {
                if (!dsDauSach[i]) continue;
                q.prepare("INSERT INTO dau_sach (isbn, ten_sach, so_trang, tac_gia, nam_xuat_ban, the_loai, so_luot_muon) "
                          "VALUES (:isbn, :ten, :so_trang, :tac_gia, :nam, :the_loai, :luot) "
                          "ON CONFLICT(isbn) DO UPDATE SET ten_sach=excluded.ten_sach, so_trang=excluded.so_trang, tac_gia=excluded.tac_gia, nam_xuat_ban=excluded.nam_xuat_ban, the_loai=excluded.the_loai, so_luot_muon=excluded.so_luot_muon;");
                q.bindValue(":isbn", QString::fromStdString(dsDauSach[i]->ISBN));
                q.bindValue(":ten", QString::fromStdString(dsDauSach[i]->tenSach));
                q.bindValue(":so_trang", dsDauSach[i]->soTrang);
                q.bindValue(":tac_gia", QString::fromStdString(dsDauSach[i]->tacGia));
                q.bindValue(":nam", dsDauSach[i]->namXuatBan);
                q.bindValue(":the_loai", QString::fromStdString(dsDauSach[i]->theLoai));
                q.bindValue(":luot", dsDauSach[i]->soLuotMuon);
                if (!q.exec()) { ok = false; }
            }
            // Rebuild ban_sao table from in-memory data afterwards (delete all and reinsert)
            if (ok) {
                if (!DatabaseManager::instance().execute("DELETE FROM ban_sao;")) ok = false;
                else {
                    for (int i = 0; i < soLuongDauSach && ok; ++i) {
                        if (!dsDauSach[i]) continue;
                        // find dau_sach id
                        QSqlQuery q2(DatabaseManager::instance().database());
                        q2.prepare("SELECT id FROM dau_sach WHERE isbn = :isbn LIMIT 1");
                        q2.bindValue(":isbn", QString::fromStdString(dsDauSach[i]->ISBN));
                        if (!q2.exec() || !q2.next()) { ok = false; break; }
                        int dauId = q2.value(0).toInt();
                        PTRDMS node = dsDauSach[i]->dms;
                        while (node) {
                            QSqlQuery q3(DatabaseManager::instance().database());
                            q3.prepare("INSERT OR IGNORE INTO ban_sao (ma_sach, dau_sach_id, trang_thai, vi_tri) VALUES (:ma, :did, :tt, :vt);");
                            q3.bindValue(":ma", QString::fromStdString(node->maSach));
                            q3.bindValue(":did", dauId);
                            q3.bindValue(":tt", static_cast<int>(node->trangThai));
                            q3.bindValue(":vt", QString::fromStdString(node->viTri));
                            if (!q3.exec()) { ok = false; break; }
                            node = node->next;
                        }
                    }
                }
            }
            if (ok) {
                DatabaseManager::instance().commit();
                DatabaseManager::instance().close();
                if (!silent) InTongKet("Ghi dau sach (DB)", soLuongDauSach, soLuongDauSach, 0, out);
                // DB save succeeded — do not write legacy text files when using DB-only mode
                try { EventBus::instance().dataChanged(); } catch(...) {}
                return true;
            } else {
                DatabaseManager::instance().rollback();
                DatabaseManager::instance().close();
            }
        }

        // Legacy: write to text file as fallback/compatibility
        std::ofstream outf(path);
        if (!outf.is_open()) {
            if (!silent)
                thongBao(out, "Loi: Khong the mo file de ghi dau sach: " + std::string(path), LOI);
            return false;
        }

        int thanhCong = 0;                                                          // dem so dau sach da ghi
        for (int i = 0; i < soLuongDauSach; ++i) {                                  // duyet mang dau sach
            if (dsDauSach[i]) {                                                     // chi ghi neu ton tai
                outf << dsDauSach[i]->ISBN << "|"                                   // isbn
                     << dsDauSach[i]->tenSach << "|"                                // ten sach
                     << dsDauSach[i]->soTrang << "|"                                // so trang
                     << dsDauSach[i]->tacGia << "|"                                 // tac gia
                     << dsDauSach[i]->namXuatBan << "|"                             // nam xuat ban
                     << dsDauSach[i]->theLoai << "\n";                              // the loai
                ++thanhCong;
            }
        }
        outf.close();                                                               // dong file

        if (!silent)
            InTongKet("Ghi dau sach", thanhCong, soLuongDauSach, soLuongDauSach - thanhCong, out);

        // Notify UI about data change so widgets refresh immediately
        try { EventBus::instance().dataChanged(); } catch(...) {}
        return true;
    } catch (const std::exception &e) {
        if (!silent)
            thongBao(out, "Loi ngoai le khi ghi dau sach: " + std::string(e.what()), LOI);
        return false;
    } catch (...) {
        if (!silent)
            thongBao(out, "Loi khong xac dinh khi ghi dau sach", LOI);
        return false;
    }
}



bool GhiDanhMucSach(const char *path, PTRDS dsDauSach[], int soLuongDauSach, bool silent, std::ostream &out) {
    try {
        // Tao backup truoc khi ghi
        if (!silent) {
            std::string loiBackup = AutoBackup();
            if (!loiBackup.empty()) {
                thongBao(out, "Canh bao: " + loiBackup, CANH_BAO);
            }
        }
        
        // Try writing to DB if available
        QString dbPath = FilePathManager::getLibraryDbPath();
        if (DatabaseManager::instance().connect(dbPath)) {
            bool ok = true;
            DatabaseManager::instance().beginTransaction();
            // delete all ban_sao, then re-insert from in-memory
            if (!DatabaseManager::instance().execute("DELETE FROM ban_sao;")) ok = false;
            for (int i = 0; i < soLuongDauSach && ok; ++i) {
                if (!dsDauSach[i]) continue;
                QSqlQuery q(DatabaseManager::instance().database());
                q.prepare("SELECT id FROM dau_sach WHERE isbn = :isbn LIMIT 1");
                q.bindValue(":isbn", QString::fromStdString(dsDauSach[i]->ISBN));
                if (!q.exec() || !q.next()) { ok = false; break; }
                int dauId = q.value(0).toInt();
                PTRDMS dms = dsDauSach[i]->dms;
                while (dms && ok) {
                    QSqlQuery q2(DatabaseManager::instance().database());
                    q2.prepare("INSERT OR IGNORE INTO ban_sao (ma_sach, dau_sach_id, trang_thai, vi_tri) VALUES (:ma, :did, :tt, :vt);");
                    q2.bindValue(":ma", QString::fromStdString(dms->maSach));
                    q2.bindValue(":did", dauId);
                    q2.bindValue(":tt", static_cast<int>(dms->trangThai));
                    q2.bindValue(":vt", QString::fromStdString(dms->viTri));
                    if (!q2.exec()) { ok = false; break; }
                    dms = dms->next;
                }
            }
            if (ok) DatabaseManager::instance().commit(); else DatabaseManager::instance().rollback();
            DatabaseManager::instance().close();
            // DB save succeeded — do not write legacy text files when using DB-only mode
            try { EventBus::instance().dataChanged(); } catch(...) {}
            return true;
        }

        std::ofstream outf(path);
        if (!outf.is_open()) {
            if (!silent)
                thongBao(out, "Loi: Khong the mo file de ghi danh muc sach: " + std::string(path), LOI);
            return false;
        }

        int thanhCong = 0;                                                          // dem so ban sao da ghi
        for (int i = 0; i < soLuongDauSach; ++i) {                                  // duyet tung dau sach
            if (dsDauSach[i]) {
                PTRDMS dms = dsDauSach[i]->dms;                                     // lay dslk ban sao

                while (dms) {                                                       // duyet DSLK don
                    outf << dms->maSach << "|"                                      // ma sach
                         << dms->trangThai << "|"                                   // trang thai (0/1/2)
                         << dms->viTri << "\n";                                     // vi tri ke
                    ++thanhCong;
                    dms = dms->next;                                                // node tiep theo
                }
            }
        }
        outf.close();                                                               // dong file

        if (!silent)
            InTongKet("Ghi danh muc sach", thanhCong, thanhCong, 0, out);

        // Notify UI about data change so widgets refresh immediately
        try { EventBus::instance().dataChanged(); } catch(...) {}
        return true;
    } catch (const std::exception &e) {
        if (!silent)
            thongBao(out, "Loi ngoai le khi ghi danh muc sach: " + std::string(e.what()), LOI);
        return false;
    } catch (...) {
        if (!silent)
            thongBao(out, "Loi khong xac dinh khi ghi danh muc sach", LOI);
        return false;
    }
}



void InTongKet(const std::string &hanhDong, int thanhCong, int tongSo, int boQua, std::ostream &out) {
    thongBao(out, hanhDong + ": " + std::to_string(thanhCong) + "/" + std::to_string(tongSo) + " thanh cong, " + std::to_string(boQua) + " bi bo qua.", THONG_TIN);
}

// ----- DocGia legacy text I/O helpers moved here -----
void ThaoTacFile_LuuGiaoDich(MUONTRA dsmt, std::ofstream& file){
    MUONTRA p = dsmt;
    while(p != nullptr){
        std::string maSach = PTRDMS_to_String(p->data.banSaoSach);
        file << "MT|"
             << maSach << "|"
             << p->data.NgayMuon << "|"
             << p->data.NgayTra << "|"
             << p->data.TrangThai << "\n";
        p = p->next;
    }
}

void ThaoTacFile_NapGiaoDich(PTRDG docGia, std::ifstream& file, PTRDS dsDauSach[], int n){
    if (docGia == nullptr) return;
    std::string line;
    while (true) {
        std::streampos pos = file.tellg();
        if (!std::getline(file, line)) break;
        if (line.empty()) continue;

        std::string prefix;
        std::string data;
        size_t p = line.find("|");
        if (p != std::string::npos) {
            prefix = line.substr(0, p);
            data = line.substr(p + 1);
        } else {
            prefix = line;
            data = "";
        }

        if (prefix == "END_DG") {
            break;
        }

        if (prefix != "MT") {
            file.clear();
            file.seekg(pos);
            break;
        }

        std::string fields[4];
        if (!TachTruong(data, '|', fields, 4)) {
            std::cerr << "NapGiaoDich: Khong tach duoc truong MT: " << line << std::endl;
            continue;
        }

        MuonTra mt;
        mt.NgayMuon = fields[1];
        mt.NgayTra = fields[2];
        try { mt.TrangThai = std::stoi(fields[3]); } catch (...) { std::cerr << "NapGiaoDich: Loi chuyen doi TrangThai MT: '" << fields[3] << "'" << std::endl; continue; }

        std::string maSachFile = fields[0];
        mt.banSaoSach = MaSach_to_PTRDMS(maSachFile, dsDauSach, n);

        if (!maSachFile.empty()) {
            std::string isbn = TachISBNTuMaSach(maSachFile);
            PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, n, isbn);
            if (dauSach != nullptr) {
                dauSach->soLuotMuon++;
            } else {
                std::cerr << "NapGiaoDich: Khong tim thay dau sach cho maSach='" << maSachFile << "'" << std::endl;
            }
        }

        themMuonTra(docGia, mt);
    }
}

static void ThaoTacFile_LuuNodeVaoFile_recursive(PTRDG node, std::ofstream& file){
    if(node == nullptr) return;

    file << "DG|"
         << node->data.MaThe << "|"
         << node->data.Ho << "|"
         << node->data.Ten << "|"
         << node->data.Phai << "|"
         << node->data.TrangThai << "\n";

    ThaoTacFile_LuuGiaoDich(node->data.dsmt, file);

    file << "END_DG\n";

    ThaoTacFile_LuuNodeVaoFile_recursive(node->left, file);
    ThaoTacFile_LuuNodeVaoFile_recursive(node->right, file);
}

void ThaoTacFile_LuuNodeVaoFile(PTRDG node, std::ofstream& file){
    ThaoTacFile_LuuNodeVaoFile_recursive(node, file);
}

bool GhiDocGiaFile(const char* path, PTRDG root, bool silent, std::ostream &out){
    try {
        std::ofstream file(path);
        if(!file.is_open()){
            if (!silent) thongBao(out, std::string("Loi: khong the mo file '") + path + "' de ghi!", LOI);
            return false;
        }

        ThaoTacFile_LuuNodeVaoFile(root, file);
        file.close();

        if (!silent) thongBao(out, "Thong tin: Da Luu file docgia.txt thanh cong.", THONG_TIN);
        return true;
    } catch (...) {
        if (!silent) thongBao(out, "Loi khong xac dinh khi luu doc gia", LOI);
        return false;
    }
}

PTRDG NapDocGiaFile(const char* path, PTRDS dsDauSach[], int soLuongDS, std::ostream &out){
    try {
        PTRDG root = nullptr;
        std::ifstream file(path);
        if(!file.is_open()){
            thongBao(out, std::string("Khong mo duoc file ") + path + " de doc!", LOI);
            std::ofstream createFile(path);
            if (createFile.is_open()) { createFile.close(); thongBao(out, std::string("Da tao file moi: ") + path, THONG_TIN); }
            return nullptr;
        }

        PTRDG docGiaHienTai = nullptr;
        std::string line;
        std::string fields[6];

        while(std::getline(file, line)){
            if(line.empty()) continue;
            std::string prefix;
            std::string data;
            size_t pos = line.find("|");
            if(pos != std::string::npos){ prefix = line.substr(0, pos); data = line.substr(pos + 1); } else { prefix = line; }

            if (prefix == "DG") {
                if (!TachTruong(data, '|', fields, 5)) continue;
                int mathe, phai, trangthai;
                std::string ho, ten;
                try { mathe = std::stoi(fields[0]); ho = fields[1]; ten = fields[2]; phai = std::stoi(fields[3]); trangthai = std::stoi(fields[4]); } catch (...) { continue; }

                docGiaHienTai = taoDocGia(ho, ten, phai != 0, trangthai, mathe);
                InsertDocGia(root, docGiaHienTai);
                ThaoTacFile_NapGiaoDich(docGiaHienTai, file, dsDauSach, soLuongDS);
            }
        }
        file.close();
        return root;
    } catch (...) {
        thongBao(out, "Loi khong xac dinh khi nap doc gia", LOI);
        return nullptr;
    }
}

bool ThaoTacFile_BackupFile(const std::string& srcPath, const std::string& backupPath) {
    try {
        std::ifstream src(srcPath, std::ios::binary);
        if (!src.is_open()) return false;
        std::ofstream dst(backupPath, std::ios::binary);
        if (!dst.is_open()) { src.close(); return false; }
        dst << src.rdbuf();
        dst.close(); src.close();
        return true;
    } catch (...) { return false; }
}