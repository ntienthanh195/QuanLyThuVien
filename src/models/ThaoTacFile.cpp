#include <fstream>
#include <iostream>
#include <string>
/**
 * @file ThaoTacFile.cpp
 * @path src/models/ThaoTacFile.cpp
 * @description Triển khai các hàm đọc và ghi dữ liệu ra file văn bản (.txt).
 * Quản lý lưu trữ/khôi phục trạng thái cây học viên và mảng đầu sách.
 * @project Library Management System (QuanLyThuVien)
 */
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
// #include "utils/Perf.h"

using namespace std;

// static bool KiemTraSoTrangFile(int soTrang, std::ostream &out) {
/*
    if (soTrang <= 0 || soTrang > MAX_SO_TRANG) {
        thongBao(out, "So trang phai tu 1-" + to_string(MAX_SO_TRANG) + "!", LOI);
        return false;
    }
    return true;
}
*/

void NapDanhSachDauSach(const char *path, std::vector<PTRDS>& dsDauSach, int &soLuongDauSach, std::ostream &out) {
    // LEGACY FUNCTION DEPRECATED
    // We now load exclusively from library.db via QuanLySach::NapDanhSachTuDB
    Q_UNUSED(dsDauSach);
    Q_UNUSED(soLuongDauSach);
    Q_UNUSED(out);
    std::cout << "[LEGACY] Skipping NapDanhSachDauSach from text file: " << path << "\n";
}


void NapDanhMucSach(const char *path, std::vector<PTRDS>& dsDauSach, int soLuongDauSach, std::ostream &out) {
   // LEGACY FUNCTION DEPRECATED
   // We now load exclusively from library.db via QuanLySach::NapDanhSachTuDB
   Q_UNUSED(dsDauSach);
   Q_UNUSED(soLuongDauSach);
   Q_UNUSED(out);
   std::cout << "[LEGACY] Skipping NapDanhMucSach from text file: " << path << "\n";
}



bool GhiDanhSachDauSach(const char *path, std::vector<PTRDS>& dsDauSach, int soLuongDauSach, bool silent, std::ostream &out) {
    // LEGACY FUNCTION DEPRECATED - DB handles persistence
    Q_UNUSED(path);
    Q_UNUSED(dsDauSach);
    Q_UNUSED(soLuongDauSach);
    Q_UNUSED(out);
    if (!silent) std::cout << "[LEGACY] Skipping GhiDanhSachDauSach to text file.\n";
    return true;
}



bool GhiDanhMucSach(const char *path, std::vector<PTRDS>& dsDauSach, int soLuongDauSach, bool silent, std::ostream &out) {
   // LEGACY FUNCTION DEPRECATED - DB handles persistence
   Q_UNUSED(path);
   Q_UNUSED(dsDauSach);
   Q_UNUSED(soLuongDauSach);
   Q_UNUSED(out);
   if (!silent) std::cout << "[LEGACY] Skipping GhiDanhMucSach to text file.\n";
   return true;
}



void InTongKet(const std::string &hanhDong, int thanhCong, int tongSo, int boQua, std::ostream &out) {
    thongBao(out, hanhDong + ": " + std::to_string(thanhCong) + "/" + std::to_string(tongSo) + " thanh cong, " + std::to_string(boQua) + " bi bo qua.", THONG_TIN);
}

// ----- DocGia legacy text I/O helpers moved here -----
void ThaoTacFile_LuuGiaoDich(const MUONTRA& dsmt, std::ofstream& file){
    for (const auto& mt : dsmt) {
        std::string maSach = PTRDMS_to_String(mt.banSaoSach);
        file << "MT|"
             << maSach << "|"
             << mt.NgayMuon << "|"
             << mt.NgayTra << "|"
             << mt.TrangThai << "\n";
    }
}

void ThaoTacFile_NapGiaoDich(PTRDG docGia, std::ifstream& file, std::vector<PTRDS>& dsDauSach, int n){
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
    // LEGACY FUNCTION DEPRECATED - DB handles persistence
    Q_UNUSED(path);
    Q_UNUSED(root);
    Q_UNUSED(out);
    if (!silent) std::cout << "[LEGACY] Skipping GhiDocGiaFile to text file.\n";
    return true;
}

PTRDG NapDocGiaFile(const char* path, std::vector<PTRDS>& dsDauSach, int soLuongDS, std::ostream &out){
    // LEGACY FUNCTION DEPRECATED - We load from DB explicitly
    Q_UNUSED(dsDauSach);
    Q_UNUSED(soLuongDS);
    Q_UNUSED(out);
    std::cout << "[LEGACY] Skipping NapDocGiaFile from text file: " << path << "\n";
    return nullptr;
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