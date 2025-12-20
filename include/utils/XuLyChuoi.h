/**
 * @file XuLyChuoi.h
 * @path include/utils/XuLyChuoi.h
 * @description Header các hàm tiện ích xử lý chuỗi và Tiếng Việt.
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef XULYCHUOI_H

#define XULYCHUOI_H

#include <string>
#include <iostream>
#include <QString>
#include "ThongBao.h"
#include "DauSach.h"

// ===== HAM XU LY CHUOI CO BAN =====

// Chuyen doi chu hoa/thuong
std::string ChuyenInThuong(const std::string& chuoi);
std::string ChuyenInHoa(const std::string& chuoi); 
std::string ChuyenThanhTitleCase(const std::string& chuoi);
std::string ChuyenThanhTitleCaseUnicode(const std::string& chuoi);  // Ho tro UTF-8 tieng Viet

// Cat va chuan hoa khoang trang
std::string CatKhoangTrang(const std::string& chuoi);
std::string ChuanHoaKhoangTrang(const std::string& chuoi);

// Kiem tra va lay ky tu
std::string LayChiCacKyTuSo(const std::string& chuoi);
bool KiemTraChuoiRongHoacKhoangTrang(const std::string& chuoi);

// Chuyen doi chuoi thanh so
bool ChuyenChuoiThanhSoNguyen(const std::string& chuoi, int& ketQua, bool chiChoPhepSoDuong = false);

// Chuan hoa chuoi theo dinh dang rieng
std::string ChuanHoaChuoiTimKiem(const std::string& chuoi);  // Chuyen chu thuong + chuan hoa khoang trang (cho tim kiem)
std::string ChuanHoaGioiTinh(const std::string& chuoiGioiTinh);  // Chuan hoa gioi tinh: "Nam" hoac "Nu"
std::string ChuanHoaViTri(const std::string& chuoiViTri);  // Chuan hoa vi tri ke sach

// ===== HAM CHUAN HOA DU LIEU =====

// Chuan hoa ten (cho ten sach, tac gia, the loai, v.v.)
std::string ChuanHoaTenUnicode(const std::string& chuoiDauVao, size_t doDaiToiThieu, 
                                size_t doDaiToiDa, std::string& chuoiKetQua);

// Chuan hoa ho ten nguoi: trim/collapse spaces and apply Unicode title-case
std::string ChuanHoaHoTen(const std::string& chuoiHoTen);

// ===== HAM XU LY UNICODE HIEN DAI (SU DUNG QT) =====

// Chuyen std::string UTF-8 <-> QString
QString ChuyenSangQString(const std::string& chuoi);
std::string ChuyenSangStdString(const QString& qstr);

// Chuan hoa ho ten hien dai voi Qt (tot nhat cho tieng Viet)
std::string ChuanHoaHoTenHienDai(const std::string& chuoiHoTen);

// Kiem tra tinh hop le cua ho ten
bool KiemTraHoTenHopLe(const std::string& hoTen, std::string& thongBaoLoi);

// Chuan hoa ten Unicode hien dai (dung cho ten sach, tac gia, the loai)
std::string ChuanHoaTenHienDai(const std::string& ten, size_t doDaiToiThieu, 
                                 size_t doDaiToiDa, std::string& thongBaoLoi);
// Chuan hoa ISBN
std::string ChuanHoaISBNCore(const std::string& chuoiISBN, std::string& ketQua, 
                              bool tuDong, bool nhapThuCong, bool laDocFile);
std::string ChuanHoaISBNFile(const std::string& chuoiISBN, std::string& ketQua);

// Chuan hoa ngay thang
bool ChuanHoaNgay(std::string& chuoiNgay, std::ostream& out);

// Bo dau va chuyen thanh chu thuong (dung cho so sanh)
std::string BoDauVaThuong(const std::string& chuoi);

// ===== STRUCT VA HAM HO TRO TIM KIEM =====

struct ThongTinSachChuanHoa {
    std::string tenSach;
    std::string tacGia;
    std::string theLoai;
    std::string isbn;
};

ThongTinSachChuanHoa ChuanHoaDuLieuSach(const std::string& tenSach, const std::string& tacGia,
                                        const std::string& theLoai, const std::string& isbn);

// ===== HAM TACH CHUOI =====

// Tach cac thanh phan tu chuoi
bool TachTruong(const std::string& dong, char kyTuPhanCach, std::string mangKetQua[], int soTruongMongDoi);
int TachTokenKhoangTrang(const std::string& chuoi, std::string mangToken[], int soTokenToiDa);

// Tach thong tin tu ma sach
std::string ChuanHoaMaBanSao(const std::string& maBanSao); // Chuan hoa: 123-001 -> 123-1
std::string TachISBNTuMaSach(const std::string& maSach);  // Lay ISBN tu "ISBN-123" -> "ISBN"
int LaySoHauToMaSach(PTRDMS nodeSach);  // Lay so tu "ISBN-123" -> 123

// ===== HAM TIM KIEM VA KHOP CHUOI =====

bool ChuaTatCaTuKhoa(const std::string& chuoiNguon, const std::string mangTuKhoa[], int soTuKhoa);
bool KhopTieuDeKhongDau(const std::string& tieuDeChuan, const std::string& tuKhoaChuan,
                        const std::string mangToken[], int soToken);

#endif // XULYCHUOI_H