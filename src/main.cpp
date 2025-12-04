#include <QApplication>
#include <QMessageBox>
#include <QDir>
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
    
    // Set application icon
    app.setWindowIcon(QIcon(":/icons/app_icon.ico"));
    
    std::cout << "[main] Khoi tao ung dung Qt...\n";
    
    // Tìm thư mục project root chứa files/
    QString exePath = QCoreApplication::applicationDirPath();
    QString projectRoot = exePath;
    
    // Thử các đường dẫn có thể
    QStringList possiblePaths = {
        exePath,                                      // Chạy trực tiếp từ thư mục chứa exe
        QDir(exePath + "/..").absolutePath(),         // debug/ hoặc release/
        QDir(exePath + "/../..").absolutePath(),      // build/Desktop_Qt_*/
        QDir(exePath + "/../../..").absolutePath(),   // build/Desktop_Qt_*/debug hoặc release
        "E:/Qt/QuanLyThuVien"                         // Đường dẫn tuyệt đối backup
    };
    
    // Tìm thư mục chứa files/dausach.txt
    bool found = false;
    for (const QString& path : possiblePaths) {
        QString testPath = path + "/files/dausach.txt";
        if (QFile::exists(testPath)) {
            projectRoot = path;
            found = true;
            std::cout << "Found project root: " << projectRoot.toStdString() << "\n";
            break;
        }
    }
    
    if (!found) {
        std::cerr << "ERROR: Cannot find files/dausach.txt!\n";
        std::cerr << "Searched paths:\n";
        for (const QString& path : possiblePaths) {
            std::cerr << "  - " << path.toStdString() << "/files/\n";
        }
    }
    
    QDir::setCurrent(projectRoot);
    std::cout << "Working Directory: " << projectRoot.toStdString() << "\n";
    
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
        // One-off migration: rewrite file to normalize names permanently
        std::cout << "Chay migration chuan hoa ten doc gia (backup -> rewrite)...\n";
        MigrateNormalizeDocGiaFile(rootDocGia);
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
