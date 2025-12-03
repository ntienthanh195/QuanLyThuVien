#include "KiemTraDuLieu.h"
#include "ThongBao.h"
#include "XuLyChuoi.h"
#include "QuanLySach.h"
#include "NgayThang.h"
#include "DauSach.h"
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