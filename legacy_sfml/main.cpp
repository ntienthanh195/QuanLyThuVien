
#include <iostream>
#include <filesystem>
#include <SFML/Graphics.hpp>
#include "QuanLySach.h"
#include "ThaoTacFile.h"
#include "ThongBao.h"
#include "GiaoDienSFML.h"
#include "DocGia.h"
#include "NgayThang.h"

// Bien toan cuc cho du lieu sach
extern PTRDS dsDauSach[MAX_DAUSACH];
extern int soLuongDauSach;
extern bool duLieuDaThayDoi;

// Bien toan cuc cho doc gia
PTRDG rootDocGia = nullptr;

int main() {
    std::cout << "[main] Bat dau...\n";

    // Nap du lieu sach tu file
    std::cout << "Dang nap du lieu dau sach...\n";
    NapDanhSachDauSach(FILE_DAUSACH, dsDauSach, soLuongDauSach, std::cout);
    NapDanhMucSach(FILE_DANHMUCSACH, dsDauSach, soLuongDauSach, std::cout);

    if (soLuongDauSach > 0) {
        std::cout << "Nap thanh cong: " << soLuongDauSach << " dau sach.\n";
        sapXepDauSachTheoTen(dsDauSach, 0, soLuongDauSach - 1);
    } else {
        std::cout << "Khong nap duoc dau sach nao.\n";
    }

    // Nap du lieu doc gia (phai nap sau sach)
    std::cout << "Dang nap du lieu doc gia...\n";
    rootDocGia = loadDocGia(dsDauSach, soLuongDauSach);
    if (rootDocGia != nullptr) {
        std::cout << "Nap thanh cong du lieu doc gia.\n";
    } else {
        std::cout << "Khong nap duoc du lieu doc gia (file co the rong).\n";
    }

    // Khoi tao cua so giao dien
    sf::RenderWindow window(sf::VideoMode(CHIEU_RONG, CHIEU_CAO), "Quan Ly Thu Vien");
    window.setFramerateLimit(60);
    sf::Font font;

    // Tu dong tim font
    const char* fonts[] = {"bin/DejaVuSans.ttf", "files/DejaVuSans.ttf", "arial.ttf", "C:/Windows/Fonts/arial.ttf"};
    std::string fontPath;
    for (auto f : fonts) {
        if (std::filesystem::exists(f)) {
            fontPath = f;
            break;
        }
    }

    if (fontPath.empty() || !font.loadFromFile(fontPath)) {
        std::cerr << "Loi: Khong tim/load duoc font!\n";
        GiaiPhongToanBoDauSach(dsDauSach, soLuongDauSach);
        GiaiPhongCay(rootDocGia);
        return 1;
    }

    if (!KhoiTaoGiaoDienSFML(window, font, fontPath)) {
        std::cerr << "Loi khoi tao giao dien!\n";
        GiaiPhongToanBoDauSach(dsDauSach, soLuongDauSach);
        GiaiPhongCay(rootDocGia);
        return 2;
    }

    // Vong lap chinh cua chuong trinh
    ChayChuongTrinhSFML(window, font);

    // Luu du lieu truoc khi thoat
    if (duLieuDaThayDoi) {
        thongBao(std::cout, "Dang luu thay doi...", THONG_TIN);
        GhiDanhSachDauSach(FILE_DAUSACH, dsDauSach, soLuongDauSach, false, std::cout);
        GhiDanhMucSach(FILE_DANHMUCSACH, dsDauSach, soLuongDauSach, false, std::cout);
        saveDocGia(rootDocGia);
        std::cout << "Luu du lieu doc gia thanh cong.\n";
    } else {
        std::cout << "Khong co thay doi.\n";
    }

    // Giai phong bo nho
    std::cout << "Dang giai phong bo nho...\n";
    GiaiPhongToanBoDauSach(dsDauSach, soLuongDauSach);
    GiaiPhongCay(rootDocGia);
    std::cout << "[main] Ket thuc.\n";
    return 0;
}