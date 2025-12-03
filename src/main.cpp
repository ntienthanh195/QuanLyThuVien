#include <QApplication>
#include <QMessageBox>
#include <iostream>
#include "views/MainWindow.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "models/ThaoTacFile.h"
#include "utils/ThongBao.h"

// Bien toan cuc cho du lieu sach
PTRDS dsDauSach[MAX_DAUSACH] = {nullptr};
int soLuongDauSach = 0;
bool duLieuDaThayDoi = false;

// Bien toan cuc cho doc gia
PTRDG rootDocGia = nullptr;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Thiết lập thông tin ứng dụng
    app.setApplicationName("Quản Lý Thư Viện");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("Library Management System");
    
    std::cout << "[main] Khoi tao ung dung Qt...\n";
    
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
    
    // Khoi tao va hien thi cua so chinh
    MainWindow mainWindow;
    mainWindow.show();
    
    std::cout << "[main] Bat dau vong lap su kien Qt...\n";
    int result = app.exec();
    
    // Luu du lieu truoc khi thoat
    std::cout << "[main] Dang thoat...\n";
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
    
    return result;
}
