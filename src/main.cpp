#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <iostream>
#include "views/MainWindow.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "models/ThaoTacFile.h"
#include "models/DBLoader.h"
#include "utils/ThongBao.h"
#include "utils/FilePathManager.h"
#include "utils/BackupManager.h"
#include "utils/EventBus.h"
#include "utils/Perf.h"
#include <QMetaObject>
#include <thread>

// Bien toan cuc cho du lieu sach
PTRDS dsDauSach[MAX_DAUSACH] = {nullptr};
int soLuongDauSach = 0;
bool duLieuDaThayDoi = false;

// Bien toan cuc cho doc gia
PTRDG rootDocGia = nullptr;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PerfTimer perfMainTotal("main_total_startup");
    
    // Thiết lập thông tin ứng dụng
    app.setApplicationName("Quản Lý Thư Viện");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("Library Management System");
    
    // Set application icon
    app.setWindowIcon(QIcon(":/icons/app_icon.ico"));
    
    std::cout << "[main] Khoi tao ung dung Qt...\n";
    
    // Initialize FilePathManager (will prefer data/ and copy legacy files/ -> data/ if needed)
    FilePathManager::initialize();
    QString projectRoot = FilePathManager::getRootPath();
    QString filesDir = FilePathManager::getFilesDirectory();
    QDir::setCurrent(projectRoot);
    std::cout << "Working Directory: " << projectRoot.toStdString() << "\n";
    std::cout << "Using data directory: " << filesDir.toStdString() << "\n";
    // Run a light archive check: pack old backup files into monthly/yearly archives
    AutoArchiveBackups();
    
    // Khoi tao va hien thi cua so chinh
    MainWindow mainWindow;
    mainWindow.show();

    // Nạp dữ liệu ở background để UI khởi động nhanh hơn. Khi hoàn tất,
    // phát tín hiệu `EventBus::dataChanged()` để các widget refresh.
    std::thread loaderThread([&]() {
        PerfTimer perfBackgroundLoad("background_data_load");
        std::cout << "[background] Dang nap du lieu dau sach..." << std::endl;
        QString dbPath = FilePathManager::getLibraryDbPath();

        // Load into temporary storage to avoid races with UI readers
        PTRDS localDs[MAX_DAUSACH] = {nullptr};
        int localCount = 0;

        if (QFile::exists(dbPath)) {
            std::cout << "[background] Found DB, loading from: " << dbPath.toStdString() << "\n";
            if (!NapDauSachTuDB(dbPath, localDs, localCount, std::cout)) {
                std::cout << "[background] DB loader failed; falling back to text files." << std::endl;
                NapDanhMucSach(FILE_DANHMUCSACH, localDs, localCount, std::cout);
            }
        } else {
            NapDanhMucSach(FILE_DANHMUCSACH, localDs, localCount, std::cout);
        }

        if (localCount > 0) {
            std::cout << "[background] Nap thanh cong (local): " << localCount << " dau sach.\n";
            sapXepDauSachTheoTen(localDs, 0, localCount - 1);
        } else {
            std::cout << "[background] Khong nap duoc dau sach nao.\n";
        }

        std::cout << "[background] Dang nap du lieu doc gia (local)..." << std::endl;
        PTRDG localRoot = loadDocGia(localDs, localCount);
        if (localRoot != nullptr) {
            std::cout << "[background] Nap thanh cong du lieu doc gia (local).\n";
            MigrateNormalizeDocGiaFile(localRoot);
        } else {
            std::cout << "[background] Khong nap duoc du lieu doc gia (file co the rong).\n";
        }

        // Swap into global under mutex to avoid races
        try {
            extern std::mutex g_ds_mutex; // declared in QuanLySach.h, defined in QuanLySach.cpp
            std::lock_guard<std::mutex> lk(g_ds_mutex);

            // Free existing global data if any to avoid leaks (only on startup this is likely empty)
            GiaiPhongToanBoDauSach(dsDauSach, soLuongDauSach);

            // Move local pointers into global array
            for (int i = 0; i < localCount && i < MAX_DAUSACH; ++i) {
                dsDauSach[i] = localDs[i];
                localDs[i] = nullptr; // avoid double ownership
            }
            // Null remaining slots
            for (int i = localCount; i < MAX_DAUSACH; ++i) dsDauSach[i] = nullptr;
            soLuongDauSach = localCount;
            rootDocGia = localRoot;
        } catch (...) {
            std::cout << "[background] Exception while swapping data into global storage." << std::endl;
        }

        // Notify UI in main thread
        QMetaObject::invokeMethod(&EventBus::instance(), "dataChanged", Qt::QueuedConnection);
    });
    loaderThread.detach();
    
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
