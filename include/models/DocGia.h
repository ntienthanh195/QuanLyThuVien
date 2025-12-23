/**
 * @file DocGia.h
 * @path include/models/DocGia.h
 * @description Định nghĩa cấu trúc dữ liệu và các hàm API cho Quản lý Học viên (Độc giả).
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef DOCGIA_H

#define DOCGIA_H
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <ctime>
#include "DauSach.h"
// #include "models/MuonTra.h" // Removed to prevent circular dependency
using std::string;
using std::vector;

enum TrangThaiGiaoDich {
    GD_DANG_MUON = 0,           // Đang mượn (Chưa trả)
    GD_DA_TRA = 1,              // Đã trả (Hoàn tất)
    GD_MAT_CHO_THANH_TOAN = 2,  // Báo mất - Chờ đền bù (Nợ)
    GD_MAT_DA_THANH_TOAN = 3,   // Đã đền bù (Xong trách nhiệm)
    GD_MAT_CHO_HOAN_TIEN = 4,   // Tìm thấy sách sau khi đền - Chờ hoàn tiền
    GD_MAT_DA_HOAN_TIEN = 5     // Đã hoàn tiền cho học viên (Kết thúc luồng tiền)
};

enum TrangThaiTheDocGia {
    TT_KHOA = 0, // Thẻ bị khóa (vô hiệu hóa)
    TT_HOAT_DONG = 1 // Thẻ đang hoạt động
};

//danh sach muon tra
struct MuonTra{
    PTRDMS banSaoSach;// thay the string maSach thanh con tro chi toi DanhMucSach* va khai bao trang thai 
    string NgayMuon;
    string NgayTra;
    int TrangThai;// 0,1,2,3 theo enum TrangThaiGiaoDich
    // Feature 4: Financial & Extension
    int soLanGiaHan = 0;
    bool daThanhToan = false;
    long long tienPhat = 0;
    long long tienHoanTra = 0; // Số tiền cần hoàn trả cho học viên (nếu tìm thấy sách sau khi đã đền)
    // Cache the ma_sach value at the time of transaction to avoid dangling pointer derefs
    string maSachCached;
};
// STRUCT MuonTra remains same, but NODE is gone
typedef std::vector<MuonTra> MUONTRA;

//PTRDGdoc gia
//PTRDGdoc gia
//PTRDGdoc gia

struct NodeDG; // Forward declaration for shared_ptr
// Sử dụng shared_ptr để quản lý bộ nhớ tự động, tránh leak
typedef std::shared_ptr<NodeDG> PTRDG;

struct TheDocGia {
    int MaThe;
    std::string Ho;
    std::string Ten;
    std::string Phai; // Changed from bool to string (e.g., "Nam", "Nu")
    TrangThaiTheDocGia TrangThai; // Using new enum
    MUONTRA dsmt; // Danh sach muon tra (vector)
    
    // Cached stats for performance
    int soSachDangMuon;
    long long tongNo; // New field for total outstanding debt
    std::string NgayLapThe; // YYYY-MM-DD
    std::string NgayHetHan; // YYYY-MM-DD
    time_t ngayHetHanSomNhat; // Cache for optimized queries (epoch time)
    int tongSachDaMuonHistory; // Historical count
    bool coTheMuon; // Realtime boolean flag

    // NEW FIELDS (PHASE 2)
    string SDT;
    string Lop;      // Don vi / Lop
    string Email;
    string NgaySinh;
    int LoaiDocGia = 0; // 0: HocSinh (Student), 1: GiaoVien (Teacher)
};

struct NodeDG {
    TheDocGia data;
    int bf;
    int height;
    PTRDG left;
    PTRDG right;
    
    // Constructor for easy make_shared
    NodeDG() : bf(0), height(1), left(nullptr), right(nullptr) {
        data = {};
        data.dsmt = {};
        // Initialize other data members of TheDocGia to default values
        data.MaThe = 0;
        data.Ho = "";
        data.Ten = "";
        data.Phai = "";
        data.TrangThai = TT_HOAT_DONG;
        data.soSachDangMuon = 0;
        data.tongNo = 0;
        data.NgayLapThe = "";
        data.NgayHetHan = "";
        data.ngayHetHanSomNhat = 0;
        data.tongSachDaMuonHistory = 0;
        data.coTheMuon = true;
        data.SDT = "";
        data.Lop = "";
        data.Email = "";
        data.NgaySinh = "";
        data.LoaiDocGia = 0;
    }
    
    // Destructor to clean up raw pointers if any (dsmt is raw linked list currently)
    ~NodeDG() {
       // Vector cleans itself automatically. No special cleanup needed.
    }
};

extern PTRDG rootDocGia;
extern bool duLieuDocGiaThayDoi; // Flag to track changes

struct ThongTinQuaHan{
    PTRDG docGia;
    int soNgayQuaHanMax;
};
//struct tu them
struct ThongTinSachDangMuon_DTO {
    string maSach;
    string tenSach;     // <== Thêm trường này
    string ngayMuon;
    int soNgayGiu;    // <== Thêm trường này
    int trangThai;    // 0: Borrowing, 1: Returned (Unpaid if listed)
    
    // Feature 4 Extras
    long long tienPhat = 0;
    long long tienHoanTra = 0; // [NEW] Added for refund scenarios
    bool daThanhToan = false;
    int soLanGiaHan = 0; // Feature Renewal
};

struct TopSachDTO {
    PTRDS dauSach;//dung con tro thay vi tao mot field da co san
};

struct DocGiaQuaHanDTO {
    PTRDG docGia;
    int soNgayQuaHanMax;
};
// Prototype AVL
PTRDG RotateLeft(PTRDG root);
PTRDG RotateRight(PTRDG root);
PTRDG RotateRight(PTRDG root);
void InsertDocGia(PTRDG& root, PTRDG node, bool syncDB = true);

// DATABASE LOADING
void NapDocGiaTuDB_Fixed(PTRDG &root, std::vector<PTRDS>& dsDauSach, int soLuongDS);

// Prototype Chức năng chính (a)
PTRDG taoDocGia(std::string ho, std::string ten, bool phai, int trangthai, PTRDG root);
PTRDG taoDocGia(std::string ho, std::string ten, bool phai, int trangthai, int mathe);
void xoaDocGia(PTRDG& root, int mathe);
PTRDG timDocGia(PTRDG root, int mathe);
void inDocGiaInOrder(PTRDG root); // In theo Mã thẻ tăng dần
void display(PTRDG root); // Hàm in 1 độc giả

// CẢI TIẾN: Thay đổi prototype để tránh rò rỉ bộ nhớ (a)
void hieuChinhDocGia(PTRDG root, const TheDocGia& new_data);

// Prototype cho chức năng sắp xếp (b)
void QuickSortDocGia(PTRDG arr[], int low, int high);
void DuyetCayRaMang(PTRDG root, PTRDG arr[], int &count);
void DuyetCayRaVector(PTRDG root, vector<PTRDG>& result);
void InDocGiaTheoTenHo(PTRDG root); // In ra danh sách sắp xếp theo Tên+Họ

// Prototype Giải phóng bộ nhớ
void GiaiPhongCay(PTRDG& root);
// void giaiPhongDsmt(MUONTRA& dsmt); // REMOVED: vector auto-cleans

// Prototype File I/O - Legacy helpers can stay in ThaoTacFile
// MuonTra functions are now in models/MuonTra.h
#include "models/MuonTra.h"

// Feature 4: Financial & Extension functions

long long TinhTongNo(PTRDG docGia);
void CapNhatQuyenMuon(PTRDG docGia); // [NEW] Realtime Permission Check

// Prototype (h)
// LayDSSachDangMuon moved to MuonTra.h

// Prototype Danh sách độc giả mượn quá hạn (i)
// TinhSoNgayQuaHanLonNhat moved to MuonTra.h

int LayDSDocGiaQuaHan(PTRDG root, DocGiaQuaHanDTO arr[], int maxKetQua);
int LayTopSach(std::vector<PTRDS>& dsDauSach, int soLuongDS, TopSachDTO arr[]);

// Prototype File I/O
string PTRDMS_to_String(PTRDMS p);
PTRDMS MaSach_to_PTRDMS(const string& maSach, std::vector<PTRDS>& dsDauSach, int soLuongDauSach);
// Legacy low-level file helpers for docgia/muontra moved to ThaoTacFile

void saveDocGia(PTRDG root);
PTRDG loadDocGia(std::vector<PTRDS>& dsDauSach, int soLuongDS);
// One-off migration: normalize saved names by loading and re-saving file (creates backup)
void MigrateNormalizeDocGiaFile(PTRDG root);

// Prototype helper ham loc
bool KiemTraBatDauBang(const std::string& fullString, const std::string& prefix);

// TIM KIEM NANG CAO DOC GIA
int TimDocGiaTheoTen(PTRDG root, const std::string& ten, PTRDG ketQua[], int maxKetQua);       // tim theo ten (khong phan biet hoa/thuong)
int TimDocGiaTheoHo(PTRDG root, const std::string& ho, PTRDG ketQua[], int maxKetQua);         // tim theo ho (khong phan biet hoa/thuong)
int TimDocGiaTheoTrangThai(PTRDG root, int trangThai, PTRDG ketQua[], int maxKetQua);          // lay danh sach doc gia theo trang thai
int TimDocGiaTheoHoTen(PTRDG root, const std::string& hoTen, PTRDG ketQua[], int maxKetQua);   // tim theo ho + ten

// THONG KE - Statistics for DocGia
struct ThongKeDocGia {
    int tongSoDocGia;            // tong so doc gia
    int docGiaHoatDong;          // doc gia dang hoat dong (TrangThai = 1)
    int docGiaBiKhoa;            // doc gia bi khoa (TrangThai = 0)
    int docGiaDangMuonSach;      // doc gia dang muon sach
    int tongSoSachDangMuon;      // tong so sach dang muon
    int docGiaCoSachQuaHan;      // doc gia co sach qua han
    
    // Feature 5 Stats
    int docGiaHetHan;
    int docGiaSapHetHan;
};

ThongKeDocGia LayThongKeDocGia(PTRDG root, std::vector<PTRDS>& dsDauSach, int soLuongDauSach);
int DemDocGiaTheoTrangThai(PTRDG root, int trangThai);
int DemDocGiaDangMuonSach(PTRDG root);


#endif