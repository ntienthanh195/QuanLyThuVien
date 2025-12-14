#ifndef DOCGIA_H
#define DOCGIA_H
#include <string>
#include <fstream>
#include "DauSach.h"
using std::string;

//danh sach muon tra
struct MuonTra{
    PTRDMS banSaoSach;// thay the string maSach thanh con tro chi toi DanhMucSach* va khai bao trang thai 
    string NgayMuon;
    string NgayTra;
    int TrangThai;//0: dang muon, 1: da tra, 2: lam mat sach
    // Feature 4: Financial & Extension
    int soLanGiaHan = 0;
    bool daThanhToan = false;
    long long tienPhat = 0;
    // Cache the ma_sach value at the time of transaction to avoid dangling pointer derefs
    string maSachCached;
};
struct NodeMT{
    MuonTra data;
    NodeMT* next;
};
typedef NodeMT* MUONTRA;

//PTRDGdoc gia
//PTRDGdoc gia
struct TheDocGia{
    int MaThe;
    string Ho;
    string Ten;
    bool Phai;//0 nam 1 nu
    int TrangThai = 1;//0 = khoa, 1=hoat dong
    
    // Feature 5 Details
    string NgayLapThe; 
    string NgayHetHan;

    // CẢI TIẾN: Thêm trường đếm sách để kiểm tra giới hạn O(1)
    int soSachDangMuon = 0; 
    
    MUONTRA dsmt;//danh sach muon tra linked list
};

//binary search tree (AVL)
struct NodeDG{
    TheDocGia data;
    int bf;
    int height;
    NodeDG *left, *right;
};
typedef NodeDG* PTRDG;

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
void NapDocGiaTuDB_Fixed(PTRDG &root, PTRDS dsDauSach[], int soLuongDS);

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
void InDocGiaTheoTenHo(PTRDG root); // In ra danh sách sắp xếp theo Tên+Họ

// Prototype Giải phóng bộ nhớ
void GiaiPhongCay(PTRDG& root);
void giaiPhongDsmt(MUONTRA& dsmt);

// Prototype File I/O - Legacy helpers can stay in ThaoTacFile
// MuonTra functions are now in models/MuonTra.h
#include "models/MuonTra.h"

// Feature 4: Financial & Extension functions

long long TinhTongNo(PTRDG docGia);

// Prototype (h)
// LayDSSachDangMuon moved to MuonTra.h

// Prototype Danh sách độc giả mượn quá hạn (i)
// TinhSoNgayQuaHanLonNhat moved to MuonTra.h

int LayDSDocGiaQuaHan(PTRDG root, DocGiaQuaHanDTO arr[], int maxKetQua);
int LayTopSach(PTRDS dsDauSach[], int soLuongDS, TopSachDTO arr[]);

// Prototype File I/O
string PTRDMS_to_String(PTRDMS p);
PTRDMS MaSach_to_PTRDMS(const string& maSach, PTRDS dsDauSach[], int soLuongDauSach);
// Legacy low-level file helpers for docgia/muontra moved to ThaoTacFile

void saveDocGia(PTRDG root);
PTRDG loadDocGia(PTRDS dsDauSach[], int soLuongDS);
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

ThongKeDocGia LayThongKeDocGia(PTRDG root, PTRDS dsDauSach[], int soLuongDauSach);
int DemDocGiaTheoTrangThai(PTRDG root, int trangThai);
int DemDocGiaDangMuonSach(PTRDG root);


#endif