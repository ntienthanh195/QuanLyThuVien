/**
 * @file KiemTraDuLieu.cpp
 * @path src/models/KiemTraDuLieu.cpp
 * @description Tập hợp các hàm tiện ích để kiểm tra tính hợp lệ của dữ liệu đầu vào.
 * Ví dụ: kiểm tra chuỗi rỗng, định dạng ISBN, kiểm tra ký tự đặc biệt.
 * @project Library Management System (QuanLyThuVien)
 */
#include "models/KiemTraDuLieu.h"

#include "utils/ThongBao.h"
#include "utils/XuLyChuoi.h"
#include "models/QuanLySach.h"
#include "utils/NgayThang.h"
#include "models/DauSach.h"
#include "Constants.h"

extern PTRDS dsDauSach[];
extern int soLuongDauSach;

std::string KiemTraChuoiRong(const std::string& chuoi, const std::string& tenTruong){
    if(chuoi.empty()) { 
        return "Loi: " + tenTruong + " khong duoc de trong!\nVui long nhap " + tenTruong + "."; 
    }
    if(KiemTraChuoiRongHoacKhoangTrang(chuoi)) { 
        return "Loi: " + tenTruong + " chi chua khoang trang!\nVui long nhap " + tenTruong + " hop le."; 
    }
    return "";
}

std::string KiemTraTrungISBN(const std::string& ISBN) {
    bool coDauSach = (TimDauSachTheoISBN(dsDauSach, soLuongDauSach, ISBN) != nullptr);
    if(coDauSach) {
        return "Loi: ISBN '" + ISBN + "' da ton tai trong he thong!\nVui long nhap ISBN khac hoac cap nhat dau sach da co.";
    }
    return "";
}

bool KiemTraTrungmaSach(const std::string& maSach, std::ostream& out) {
    bool coMaSach = (timDanhMucTheoMaSach(maSach, dsDauSach, soLuongDauSach, out, true) != nullptr);
    if(coMaSach) thongBao(out, "Trung ma sach!", LOI);
    return coMaSach;
}

std::string KiemTraChuoiVaDodai(const std::string& str, const std::string& fieldName, int maxLen) {
    std::string loiRong = KiemTraChuoiRong(str, fieldName);
    if (!loiRong.empty()) {
        return loiRong;
    }
    if (str.length() > static_cast<size_t>(maxLen)) {
        return "Loi: " + fieldName + " qua dai (" + std::to_string(str.length()) + "/" + std::to_string(maxLen) + " ky tu)!\nVui long rut ngan " + fieldName + " xuong toi da " + std::to_string(maxLen) + " ky tu.";
    }
    return "";
}

std::string KiemTraNamXuatBan(int nam) {
    int namHienTai = LayNamHienTai();
    if (nam < NAM_XUAT_BAN_MIN ||
        nam > namHienTai + NAM_XUAT_BAN_OFFSET_MAX) {
        return "Loi: Nam xuat ban '" + std::to_string(nam) + "' khong hop le!\nNam xuat ban phai tu " + std::to_string(NAM_XUAT_BAN_MIN) + " den " + std::to_string(namHienTai) + ".\nVi du: " + std::to_string(namHienTai - 5);
    }
    return "";
}

std::string KiemTraISBN(const std::string& ISBN) {
    std::string isbnChuan;
    std::string loi = ChuanHoaISBNCore(ISBN, isbnChuan, false, false, false);
    return loi; 
}

bool KiemTraDanhSachHopLe(void* danhSach, int soLuong, int maxSize, const std::string& tenDS, std::ostream& out) {
    if (!danhSach) {
        thongBao(out, tenDS + " khong hop le (null pointer)!", LOI);
        return false;
    }
    if (soLuong < 0) {
        thongBao(out, "So luong " + tenDS + " khong the am (" + std::to_string(soLuong) + ")!", LOI);
        return false;
    }
    if (soLuong > maxSize) {
        thongBao(out, tenDS + " vuot qua gioi han (" + std::to_string(soLuong) + "/" + std::to_string(maxSize) + ")!", LOI);
        return false;
    }
    return true;
}


std::string KiemTraTongSoBanSao(const std::string& isbn, int soLuongThem){
    PTRDS dauSach = TimDauSachTheoISBN(dsDauSach, soLuongDauSach, isbn);
    if(!dauSach) return ""; // hong tim thay dau sach
    
    int soBanSaoHienTai = dauSach->tongBanSao;
    int tongSoSauKhiThem = soBanSaoHienTai + soLuongThem;
    
    if(tongSoSauKhiThem > MAX_BAN_SAO) {
        return "Loi: Tong so ban sao se vuot qua gioi han!\nHien tai: " + std::to_string(soBanSaoHienTai) + " ban sao.\nThem " + std::to_string(soLuongThem) + " ban sao se thanh " + std::to_string(tongSoSauKhiThem) + " (gioi han: " + std::to_string(MAX_BAN_SAO) + ").\nVui long giam so luong can them.";
    }
    return ""; 
}

// ==================== VALIDATION FUNCTIONS MOI ====================

// Kiem tra so dien thoai Viet Nam (10-11 chu so, bat dau bang 0)
std::string KiemTraSoDienThoai(const std::string& sdt) {
    if (sdt.empty()) {
        return "Loi: So dien thoai khong duoc de trong!";
    }
    
    // Kiem tra chi chua chu so
    for (char c : sdt) {
        if (c < '0' || c > '9') {
            return "Loi: So dien thoai chi duoc chua chu so!\nVi du: 0328378331";
        }
    }
    
    // Kiem tra do dai 10-11 chu so
    if (sdt.length() < 10 || sdt.length() > 11) {
        return "Loi: So dien thoai phai co 10-11 chu so!\nVi du: 0328378331";
    }
    
    // Kiem tra bat dau bang 0
    if (sdt[0] != '0') {
        return "Loi: So dien thoai phai bat dau bang so 0!\nVi du: 0328378331";
    }
    
    return "";
}

// Kiem tra CMND/CCCD (9 hoac 12 chu so)
std::string KiemTraCMND(const std::string& cmnd) {
    if (cmnd.empty()) {
        return "Loi: So CMND/CCCD khong duoc de trong!";
    }
    
    // Kiem tra chi chua chu so
    for (char c : cmnd) {
        if (c < '0' || c > '9') {
            return "Loi: So CMND/CCCD chi duoc chua chu so!";
        }
    }
    
    // Kiem tra do dai: CMND (9 so) hoac CCCD (12 so)
    if (cmnd.length() != 9 && cmnd.length() != 12) {
        return "Loi: CMND phai co 9 chu so, CCCD phai co 12 chu so!\nVi du: 123456789 hoac 012345678901";
    }
    
    return "";
}

// Kiem tra email co ban
std::string KiemTraEmail(const std::string& email) {
    if (email.empty()) {
        return "";
    }
    
    // Kiem tra co @ va .
    size_t posAt = email.find('@');
    if (posAt == std::string::npos) {
        return "Loi: Email phai chua ky tu @!\nVi du: user@example.com";
    }
    
    size_t posDot = email.find('.', posAt);
    if (posDot == std::string::npos) {
        return "Loi: Email phai chua ten mien (vd: .com, .vn)!\nVi du: user@example.com";
    }
    
    // Kiem tra @ khong o dau/cuoi
    if (posAt == 0 || posAt == email.length() - 1) {
        return "Loi: Email khong hop le!\nVi du: user@example.com";
    }
    
    // Kiem tra phan sau @ co ky tu
    if (posDot == posAt + 1 || posDot == email.length() - 1) {
        return "Loi: Email khong hop le!\nVi du: user@example.com";
    }
    
    // Kiem tra do dai email
    if (email.length() > 100) {
        return "Loi: Email qua dai (toi da 100 ky tu)!";
    }
    
    return "";
}

// Kiem tra ngay thang hop le
std::string KiemTraNgayHopLe(int ngay, int thang, int nam) {
    if (thang < 1 || thang > 12) {
        return "Loi: Thang phai tu 1 den 12!";
    }
    
    if (nam < 1900 || nam > 2100) {
        return "Loi: Nam phai tu 1900 den 2100!";
    }
    
    // So ngay trong thang
    int soNgayTrongThang[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Nam nhuan
    if ((nam % 4 == 0 && nam % 100 != 0) || (nam % 400 == 0)) {
        soNgayTrongThang[2] = 29;
    }
    
    if (ngay < 1 || ngay > soNgayTrongThang[thang]) {
        return "Loi: Ngay khong hop le cho thang " + std::to_string(thang) + 
               "!\nThang " + std::to_string(thang) + " co toi da " + 
               std::to_string(soNgayTrongThang[thang]) + " ngay.";
    }
    
    return "";
}

// Kiem tra so nguyen
std::string KiemTraSoNguyen(const std::string& str, const std::string& tenTruong, int min, int max) {
    if (str.empty()) {
        return "Loi: " + tenTruong + " khong duoc de trong!";
    }
    
    // Kiem tra ky tu dau tien co the la dau -
    size_t start = 0;
    if (str[0] == '-') {
        start = 1;
        if (str.length() == 1) {
            return "Loi: " + tenTruong + " khong hop le!";
        }
    }
    
    // Kiem tra cac ky tu con lai phai la chu so
    for (size_t i = start; i < str.length(); i++) {
        if (str[i] < '0' || str[i] > '9') {
            return "Loi: " + tenTruong + " chi duoc chua chu so!\nVi du: 123";
        }
    }
    
    // Chuyen doi thanh so
    try {
        long long num = std::stoll(str);
        if (num < min || num > max) {
            return "Loi: " + tenTruong + " phai nam trong khoang " + 
                   std::to_string(min) + " den " + std::to_string(max) + "!";
        }
    } catch (...) {
        return "Loi: " + tenTruong + " qua lon hoac khong hop le!";
    }
    
    return "";
}

// Kiem tra so thuc
std::string KiemTraSoThuc(const std::string& str, const std::string& tenTruong, double min, double max) {
    if (str.empty()) {
        return "Loi: " + tenTruong + " khong duoc de trong!";
    }
    
    // Kiem tra format so thuc
    // bool coDau = false;
    bool coCham = false;
    size_t start = 0;
    
    if (str[0] == '-' || str[0] == '+') {
        // coDau = true;
        start = 1;
    }
    
    for (size_t i = start; i < str.length(); i++) {
        if (str[i] == '.') {
            if (coCham) {
                return "Loi: " + tenTruong + " chi duoc chua 1 dau cham!\nVi du: 123.45";
            }
            coCham = true;
        } else if (str[i] < '0' || str[i] > '9') {
            return "Loi: " + tenTruong + " khong hop le!\nVi du: 123.45";
        }
    }
    
    // Chuyen doi thanh so
    try {
        double num = std::stod(str);
        if (num < min || num > max) {
            return "Loi: " + tenTruong + " phai nam trong khoang " + 
                   std::to_string(min) + " den " + std::to_string(max) + "!";
        }
    } catch (...) {
        return "Loi: " + tenTruong + " khong hop le!";
    }
    
    return "";
}

// Kiem tra ten nguoi (chi chua chu cai va khoang trang)
std::string KiemTraTenNguoi(const std::string& ten, const std::string& tenTruong) {
    if (ten.empty()) {
        return "Loi: " + tenTruong + " khong duoc de trong!";
    }
    
    if (KiemTraChuoiRongHoacKhoangTrang(ten)) {
        return "Loi: " + tenTruong + " chi chua khoang trang!";
    }
    
    // Kiem tra do dai
    if (ten.length() > 50) {
        return "Loi: " + tenTruong + " qua dai (toi da 50 ky tu)!";
    }
    
    // Kiem tra chi chua chu cai va khoang trang
    for (size_t i = 0; i < ten.length(); i++) {
        unsigned char c = ten[i];
        // Cho phep chu cai (a-z, A-Z), chu tieng Viet (128-255), va khoang trang
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ' || c >= 128)) {
            return "Loi: " + tenTruong + " chi duoc chua chu cai va khoang trang!\nVi du: Nguyen Van A";
        }
    }
    
    // Kiem tra khong bat dau/ket thuc bang khoang trang
    if (ten[0] == ' ' || ten[ten.length() - 1] == ' ') {
        return "Loi: " + tenTruong + " khong duoc bat dau hoac ket thuc bang khoang trang!";
    }
    
    return "";
}