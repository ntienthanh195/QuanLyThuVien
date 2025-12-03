#include <fstream>
#include <iostream>
#include <string>
#include "ThaoTacFile.h"
#include "KiemTraDuLieu.h"
#include "XuLyChuoi.h"
#include "QuanLySach.h"
#include "ThongBao.h"
#include "DauSach.h"
#include "Constants.h"

using namespace std;

static bool KiemTraSoTrangFile(int soTrang, std::ostream &out) {
    if (soTrang <= 0 || soTrang > MAX_SO_TRANG) {
        thongBao(out, "So trang phai tu 1-" + to_string(MAX_SO_TRANG) + "!", LOI);
        return false;
    }
    return true;
}

void NapDanhSachDauSach(const char *path, PTRDS dsDauSach[], int &soLuongDauSach, std::ostream &out) {
    std::ifstream inp(path);                                                    // mo file dau sach
    if (!inp.is_open()) {                                                       // khong mo duoc file
        thongBao(out, "Loi: Khong the mo file dau sach: " + std::string(path), LOI);
        return;
    }

    int soDong = 0, thanhCong = 0, boQua = 0;                                    // dem so dong, thanh cong, bo qua
    std::string dong;                                                           // luu noi dung dong hien tai
    char buffer[DUNG_LUONG_CACHE_MAC_DINH];                                     // bo dem doc nhanh

    while (inp.getline(buffer, DUNG_LUONG_CACHE_MAC_DINH)) {                    // doc tung dong den het file
        dong = buffer;                                                          // chuyen buffer sang string
        ++soDong;                                                               // tang so thu tu dong

        std::string truong[SO_TRUONG_DAU_SACH];                                  // mang luu cac truong sau khi tach
        if (!TachTruong(dong, '|', truong, SO_TRUONG_DAU_SACH)) {               // tach bang dau '|'
            thongBao(out, thongBaoLoi(soDong, "Sai dinh dang dong: " + dong, true), CANH_BAO);
            ++boQua;                                                            // sai so truong -> bo qua
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
        loi = ChuanHoaTenUnicode(truong[1], MIN_TEN_SACH, MAX_TEN_SACH, tenSach);
        if (!loi.empty()) {                                                     // ten khong hop le
            thongBao(out, thongBaoLoi(soDong, loi + " ('" + truong[1] + "')", true), CANH_BAO);
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
        loi = ChuanHoaTenUnicode(truong[3], MIN_TAC_GIA, MAX_TAC_GIA, tacGia);
        if (!loi.empty()) {
            thongBao(out, thongBaoLoi(soDong, loi + " ('" + truong[3] + "')", true), CANH_BAO);
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
        loi = ChuanHoaTenUnicode(truong[5], MIN_THE_LOAI, MAX_THE_LOAI, theLoai);
        if (!loi.empty()) {
            thongBao(out, thongBaoLoi(soDong, loi + " ('" + truong[5] + "')", true), CANH_BAO);
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
}


void NapDanhMucSach(const char *path, PTRDS dsDauSach[], int soLuongDauSach, std::ostream &out) {
    std::ifstream inp(path);                                                    // mo file danh muc sach
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
}



bool GhiDanhSachDauSach(const char *path, PTRDS dsDauSach[], int soLuongDauSach, bool silent, std::ostream &out) {
    std::ofstream outf(path);                                                   // mo file ghi dau sach
    if (!outf.is_open()) {                                                      // khong mo duoc
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
    return true;
}



bool GhiDanhMucSach(const char *path, PTRDS dsDauSach[], int soLuongDauSach, bool silent, std::ostream &out) {
    std::ofstream outf(path);                                                   // mo file ghi dms
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
    return true;
}



void InTongKet(const std::string &hanhDong, int thanhCong, int tongSo, int boQua, std::ostream &out) {
    thongBao(out, hanhDong + ": " + std::to_string(thanhCong) + "/" + std::to_string(tongSo) + " thanh cong, " + std::to_string(boQua) + " bi bo qua.", THONG_TIN);
}