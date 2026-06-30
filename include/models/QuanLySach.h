/**
 * @file QuanLySach.h
 * @path include/models/QuanLySach.h
 * @description Header định nghĩa cấu trúc Đầu sách (DauSach) và Bản sao sách (DanhMucSach).
 * Quản lý mảng con trỏ đầu sách và danh sách liên kết bản sao.
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef QUANLYSACH_H

#define QUANLYSACH_H

#include <string>
#include <iostream>
#include <mutex>
#include "DauSach.h"

// bien toan cuc
extern std::vector<PTRDS> dsDauSach;   // mang dau sach
extern int soLuongDauSach;             // so dau sach hien co
extern bool duLieuDaThayDoi;           // co thay doi du lieu
// Mutex to protect access to global dsDauSach/soLuongDauSach when swapping from background loader
extern std::mutex g_ds_mutex;

//  NHOM 1: SINH MA & CHUYEN DOI TRANG THAI 
std::string sinhMaSach(const std::string &chiSo, int &soThuTu);                 // sinh ma: ISBN-1, ISBN-2,...
const char* TenTrangThai(TrangThaiSach tt);                                     // enum -> chuoi
int PhanTichTrangThaiSach(const std::string &s, std::ostream &out);             // chuoi -> enum

//  NHOM 2: QUAN LY DANH MUC SACH (DMS - DSLK DON) 
std::string themDanhMucSach(PTRDS dauSach, const std::string &ma, TrangThaiSach tt, const std::string &vt);  // chen vao dau (updated Phase 5)
bool ChenNodeDMSVaoDauSach(PTRDS d, const std::string &ma, int tt, const std::string &vt);               // chen vao dau dms (dung khi doc file)
bool capNhatTrangThaiSach(PTRDMS dms, const std::string &ma, TrangThaiSach tt);                         // cap nhat trang thai ban sao
PTRDMS timDanhMucTheoMaSach(const std::string &ma, std::vector<PTRDS>& ds, int n, std::ostream &out, bool silent = false); // tim node theo ma sach
int TimSoThuTuLonNhat(PTRDS dauSach);                                                                     // tim so thu tu lon nhat trong DSLK ban sao
std::string ThemNhieuBanSao(PTRDS dauSach, int soLuong, const std::string &viTri = "");                  // them nhieu ban sao 1 luc

//  NHOM 3: QUAN LY DAU SACH (MANG CON TRO) 
bool themDauSach(std::vector<PTRDS>& ds, int &n, const std::string &isbn, const std::string &ten, int trang,
                 const std::string &tg, int nam, const std::string &tl, bool anLang = false, long long giaTien = 0, const std::string& nhaXuatBan = "");  // them + validation + sort
PTRDS TimDauSachTheoISBN(std::vector<PTRDS>& ds, int n, const std::string &isbn);                           // linear search O(N)

//  NHOM 4: TIM KIEM & LIET KE 
struct KetQuaTimKiem {
    PTRDS sach;
    int loaiKhop;  // 1: ten sach, 2: tac gia, 3: the loai, 4: isbn
};

int timKiemLogic(std::vector<PTRDS>& ds, int n, const std::string &tk, KetQuaTimKiem kq[]);                 // tim + sap xep theo loai khop
int TimTheLoaiDuyNhat(std::vector<PTRDS>& ds, int n, std::string tl[]);                                     // lay danh sach the loai duy nhat
int TimViTriDuyNhat(std::vector<PTRDS>& ds, int n, std::string vt[]);                                       // lay danh sach vi tri duy nhat tu cac ban sao
int TimSachTheoTheLoai(std::vector<PTRDS>& ds, int n, const std::string &tl, PTRDS kq[]);                   // loc sach theo 1 the loai
int DemSoLuongBanSaoTheoTrangThai(PTRDS dauSach, TrangThaiSach trangThai);                     // dem so luong ban sao theo trang thai

// TIM KIEM NANG CAO
int TimSachTheoNamXuatBan(std::vector<PTRDS>& ds, int n, int nam, PTRDS kq[]);                             // tim sach theo nam xuat ban
int TimSachTheoKhoangNam(std::vector<PTRDS>& ds, int n, int namBatDau, int namKetThuc, PTRDS kq[]);       // tim sach trong khoang nam
int LocSachTheoTrangThai(std::vector<PTRDS>& ds, int n, TrangThaiSach trangThai, PTRDS kq[]);             // loc sach co ban sao voi trang thai cu the
int TimSachTheoTacGia(std::vector<PTRDS>& ds, int n, const std::string &tacGia, PTRDS kq[]);              // tim sach theo tac gia (khong phan biet chu hoa/thuong)

// struct thong tin tong quan dau sach
struct ThongTinDauSach {
    int tongBanSao;
    int choMuonDuoc;
    int dangMuon;
    int thanhLy;
};

ThongTinDauSach LayThongTinDauSach(PTRDS dauSach);                                             // lay thong tin chi tiet dau sach (duyet 1 lan)

//  NHOM 5: SAP XEP 
void hoanDoiDauSach(PTRDS &a, PTRDS &b);                                                        // swap 2 con tro
void hoanDoiKetQua(KetQuaTimKiem &a, KetQuaTimKiem &b);                                         // swap 2 ket qua tim kiem
void insertionSort(std::vector<PTRDS>& arr, int left, int right);                                           // insertion sort cho mang nho
int partition(std::vector<PTRDS>& arr, int left, int right);                                                // partition cho quicksort
void sapXepDauSachTheoTen(std::vector<PTRDS>& arr, int left, int right);                                    // hybrid: quicksort + insertion
void SapXepBanSaoTheoMa(PTRDMS arr[], int n);                                                   // bubble sort ban sao theo so hau to
void SapXepTheLoaiTheoTen(std::string arr[], int n);                                            // insertion sort the loai
void sapXepKetQuaTimKiem(KetQuaTimKiem arr[], int n);                                           // quicksort ket qua theo loai khop + ten

//  NHOM 6: QUAN LY BO NHO 
void GiaiPhongDanhMucSach(PTRDMS &dms);                                                         // xoa DSLK ban sao
void GiaiPhongDauSach(PTRDS &d);                                                                // xoa 1 dau sach + DSLK
void GiaiPhongToanBoDauSach(std::vector<PTRDS>& ds, int &n);                                                // xoa toan bo mang
int DemTongSoBanSao(std::vector<PTRDS>& ds, int n);                                                         // dem tong ban sao
void CapNhatTongBanSao(std::vector<PTRDS>& ds, int n);                                                      // cap nhat field tongBanSao

//  NHOM 7: XOA & CAP NHAT 
std::string XoaDauSach(std::vector<PTRDS>& ds, int &n, const std::string &isbn);                            // xoa dau sach (neu khong con ban sao) - HAM NOI BO
bool XoaSachTheoMaSach(std::vector<PTRDS>& ds, int n, const std::string &ma, std::ostream &out);            // xoa 1 ban sao
std::string CapNhatDauSach(std::vector<PTRDS>& ds, int n, const std::string &isbn,
                           const std::string &ten, int trang, const std::string &tg,
                           int nam, const std::string &tl, const std::string &viTri = "");     // cap nhat + sort neu ten doi
std::string CapNhatViTriTatCaBanSao(PTRDS dauSach, const std::string &viTri);                  // cap nhat vi tri cho tat ca ban sao (chi update khi khac)
bool KiemTraCoTheXoaDauSach(PTRDS dauSach);                                                     // kiem tra co the xoa dau sach khong

//  NHOM 8: SERVICE LAYER CHO UI (ENCAPSULATION - SEPARATION OF CONCERNS) 
// Cac ham nay la diem truy cap duy nhat tu UI, giup tach biet hoan toan UI va Backend
// UI KHONG DUOC truy cap truc tiep vao dsDauSach, soLuongDauSach

// Getter - Lay thong tin (Read-only)
PTRDS* LayDanhSachDauSach();

// THONG KE - Statistics Functions
struct ThongKeSach {
    int tongSoDauSach;           // tong so dau sach
    int tongSoBanSao;            // tong so ban sao
    int soBanSaoCoTheMuon;       // so ban sao cho muon duoc
    int soBanSaoDangMuon;        // so ban sao dang muon
    int soBanSaoThanhLy;         // so ban sao da thanh ly
    double tyLeChoMuon;          // ty le sach co the muon (%)
    int soTheLoaiKhacNhau;       // so luong the loai khac nhau
};

ThongKeSach LayThongKeSach();                                                  // lay thong ke toan he thong
int DemSachTheoTrangThai(PTRDS dauSach, TrangThaiSach trangThai);             // dem ban sao theo trang thai cua 1 dau sach
int DemTongSoBanSaoTheoTrangThai(TrangThaiSach trangThai);                    // dem tong so ban sao theo trang thai trong he thong                                                                   // lay con tro mang (read-only)
int LaySoLuongDauSach();                                                                       // lay so luong dau sach hien tai
bool KiemTraDuLieuThayDoi();                                                                   // kiem tra du lieu co thay doi

// Truy van - Tim kiem & Loc du lieu
void LayDanhSachTimKiem(const std::string& tuKhoa, KetQuaTimKiem* ketQua, int& soLuongKetQua, int maxKetQua); // tim kiem & sap xep
void LayDanhSachTheoTheLoai(std::string* cacTheLoai, int& soTheLoai);                          // lay danh sach the loai duy nhat
void LayDanhSachViTri(std::string* cacViTri, int& soViTri);                                    // lay danh sach vi tri duy nhat
int LayDanhSachSachTheoTheLoai(const std::string& theLoai, PTRDS* ketQua, int maxKetQua);     // lay danh sach sach cung the loai (da sap xep theo ten)
PTRDS TimDauSach(const std::string& isbn);                                                     // tim 1 dau sach theo ISBN
ThongTinDauSach LayThongTinDauSach(const std::string& isbn);                                   // lay thong tin tong quan

// Command - Thao tac du lieu (Create, Update, Delete)
std::string ThemDauSachMoi(const std::string& isbn, const std::string& ten, int trang,
                           const std::string& tg, int nam, const std::string& tl, 
                           int soLuongBanSao, const std::string& viTri = "", long long giaTien = 0, const std::string& nhaXuatBan = "", bool emitSignal = true);                  // them dau sach + ban sao (all-in-one)
std::string ThemBanSaoMoi(const std::string& isbn, int soLuong, const std::string& viTri = ""); // them ban sao vao dau sach co san
std::string CapNhatThongTinDauSach(const std::string &isbn, const std::string &ten, int trang,
                                   const std::string &tg, int nam, const std::string &tl, 
                                   const std::string &viTri = "", long long giaTien = -1, const std::string& nhaXuatBan = "");            // cap nhat thong tin              // cap nhat thong tin dau sach
std::string XoaDauSachTheoISBN(const std::string& isbn);                                       // xoa dau sach (validation + auto sort)
std::string ThanhLyBanSaoTheoMa(const std::string& maSach);                                    // thanh ly 1 ban sao
std::string XoaBanSaoTheoMa(const std::string& maSach);                                        // xoa 1 ban sao (validation + cap nhat tong)

// Utility - Ho tro UI
void DanhDauDuLieuThayDoi();                                                                   // danh dau du lieu da thay doi (de luu file)
int LayDanhSachBanSaoSapXep(const std::string& isbn, PTRDMS* mangKetQua, int maxKetQua);      // lay danh sach ban sao da sap xep theo ma
std::string LayViTriDauTienCuaDauSach(const std::string& isbn);                               // lay vi tri cua ban sao dau tien
bool CoTheBanSao(const std::string& isbn);                                                     // kiem tra dau sach co ban sao hay khong

// DATABASE LOADING
void NapDanhSachTuDB();                                                                       // Load initial data from SQLite

#endif // QUANLYSACH_H