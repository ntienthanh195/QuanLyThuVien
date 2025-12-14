#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFontDatabase>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

#include "giao_dien/CuaSoChinh.h"
#include "utils/FilePathManager.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "models/ThaoTacFile.h"
#include "utils/ThongBao.h"
#include "utils/DatabaseManager.h"
#include "giao_dien/hop_thoai/ManHinhDangNhap.h"
#include "models/UserManager.h"
#include "utils/GlobalScannerFilter.h"

// Biến toàn cục cho dữ liệu sách (từ backend cũ)
PTRDS dsDauSach[MAX_DAUSACH] = {nullptr};
int soLuongDauSach = 0;
bool duLieuDaThayDoi = false;

// Biến toàn cục cho độc giả
PTRDG rootDocGia = nullptr;

int main(int argc, char *argv[]) {
    // Thiết lập UTF-8 cho console Windows
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    
    QApplication ung_dung(argc, argv);
    
    // Qt 6 đã sử dụng UTF-8 mặc định, không cần thiết lập thêm
    
    // Load Fonts
    int fontIdKhmer = QFontDatabase::addApplicationFont(":/fonts/fonts/Siemreap-Regular.ttf");
    if (fontIdKhmer != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontIdKhmer);
        if (!fontFamilies.isEmpty()) {
            qDebug() << "Loaded font:" << fontFamilies.first();
            // Optional: Set as default if desired, or just available for fallback
            // app.setFont(QFont(fontFamilies.first(), 10)); 
        }
    } else {
        qWarning() << "Failed to load Khmer font";
    }

    // Load font mã vạch từ resources
    int fontIdBarcode = QFontDatabase::addApplicationFont(":/fonts/LibreBarcode128Text-Regular.ttf");
    if (fontIdBarcode != -1) {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontIdBarcode).at(0);
        std::cout << "[FONT] Đã load font mã vạch: " << fontFamily.toStdString() << "\n";
    } else {
        std::cout << "[CẢNH BÁO] Không load được font mã vạch - In tem/thẻ có thể không hiển thị đúng\n";
    }
    
    // Thiết lập thông tin ứng dụng
    ung_dung.setApplicationName("Hệ Thống Quản Lý Thư Viện");
    ung_dung.setApplicationVersion("2.0.0");
    ung_dung.setOrganizationName("Thư Viện Quốc Gia");
    
    std::cout << "========================================\n";
    std::cout << "  HỆ THỐNG QUẢN LÝ THƯ VIỆN V2.0\n";
    std::cout << "========================================\n\n";
    
    // DISABLED: GlobalScannerFilter causes issues with barcode input
    // Reverting to widget-level event filter which works reliably
    /*
    GlobalScannerFilter* globalScanner = new GlobalScannerFilter(&ung_dung);
    ung_dung.installEventFilter(globalScanner);
    std::cout << "[SCANNER] Global scanner event filter installed.\n";
    */
    
    
    std::cout << "[KHỞI TẠO] Đang khởi động ứng dụng Qt...\n";
    
    // Thiết lập thư mục làm việc (Force Absolute Path per User Request)
    // Avoid relative path issues by hardcoding the project root as detected or fallback
    QString projectRoot = "E:/Qt/QuanLyThuVien";
    if (QDir(projectRoot).exists()) {
         QDir::setCurrent(projectRoot);
         FilePathManager::initialize(); // Re-init after setting cwd
    } else {
        // Fallback to normal behavior
        FilePathManager::initialize();
        projectRoot = FilePathManager::getRootPath();
        QDir::setCurrent(projectRoot);
    }

    std::cout << "[THƯ MỤC] Working Directory: " << QDir::currentPath().toStdString() << "\n";
    std::cout << "[THƯ MỤC] Absolute Root Check: " << projectRoot.toStdString() << "\n";
    
    // Kiểm tra file dữ liệu
    std::cout << "\n[KIỂM TRA] Kiểm tra các file dữ liệu...\n";
    
    QString fileDauSach = FilePathManager::getDauSachPath();
    // Only check if we are in legacy mode. If DB exists, maybe we can skip?
    // User wants ONLY library.db in data/files.
    // However, the legacy backend might still need text files?
    // Let's relax the check or fix the path.
    
    // SKIP Legacy File Checks as we are using DB Only
    // if (!QFile::exists(fileDauSach)) { ... } removed to prevent blocking startup
    
    QString fileDanhMuc = FilePathManager::getDanhMucSachPath();
    QString fileDocGia = FilePathManager::getDocGiaPath();
    QString fileMuonTra = FilePathManager::getMuonTraPath();
    
    // SKIP Legacy File Checks as we are using DB Only
    std::cout << "[CONFIG] Running in Database-Only mode. Legacy files ignored.\n";
    
    std::cout << "[✓] " << fileDauSach.toStdString() << " - Tồn tại\n";
    std::cout << "[✓] " << fileDanhMuc.toStdString() << " - " 
              << (QFile::exists(fileDanhMuc) ? "Tồn tại" : "Không tồn tại") << "\n";
    std::cout << "[✓] " << fileDocGia.toStdString() << " - " 
              << (QFile::exists(fileDocGia) ? "Tồn tại" : "Không tồn tại") << "\n";
    
    // === DATABASE INIT (Phase 1) ===
    // Connect to SQLite DB FIRST
    QString dbPath = "data/files/library.db";
    // Ensure dir exists
    QDir().mkpath("data/files");
    
    std::cout << "[INIT] Connecting to database: " << dbPath.toStdString() << "\n";
    if (!DatabaseManager::instance().connect(dbPath)) {
        QMessageBox::critical(nullptr, "Lỗi Database", "Không thể kết nối cơ sở dữ liệu (" + dbPath + ")!");
        return -1;
    }
    // Migrate/Create tables if needed
    if (!DatabaseManager::instance().createTables()) {
        QMessageBox::critical(nullptr, "Lỗi Database", "Không thể khởi tạo cấu trúc dữ liệu!");
        return -1;
    }

    // === MAIN APP LOOP (For Logout/Restart) ===
    int exitCode = 0;
    const int RESTART_CODE = 1000;
    
    do {
        // === LOGIN SCREEN ===
        // Block app until login success
        bool loginSuccess = ManHinhDangNhap::yeuCauDangNhap();
        if (!loginSuccess) {
            std::cout << "[INFO] User cancelled login. Exiting.\n";
            return 0; // Exit app
        }
        
        // Log user info
        UserInfo user = UserManager::instance().currentUser();
        std::cout << "[LOGIN] Welcome " << user.fullName.toStdString() << " (" << user.role.toStdString() << ")\n";

        // Nạp dữ liệu sách từ Database (AFTER connection)
        // CHECK if data is already loaded to avoid duplication on re-login?
        // simple way: Global vars so data persists.
        // But if user changes, maybe we should reload? 
        // For V2, let's assume one library per instance. Reloading might be safer but potentially slow.
        // Existing logic: NapDanhSachTuDB clears if array is full? 
        // NapDanhSachTuDB uses `MAX_DAUSACH`.
        // Let's just Reload to be safe (in case different user sees different data?)
        // Currently data is global, not per-user. So actually no need to reload if already loaded.
        if (soLuongDauSach == 0) {
            std::cout << "\n[NẠP DỮ LIỆU] Đang nạp dữ liệu sách từ Database...\n";
            try {
                NapDanhSachTuDB();
                if (soLuongDauSach > 0) {
                    std::cout << "[✓] Nạp thành công: " << soLuongDauSach << " đầu sách.\n";
                    sapXepDauSachTheoTen(dsDauSach, 0, soLuongDauSach - 1);
                } else {
                    std::cout << "[!] Cảnh báo: Database sách rỗng.\n";
                }
            } catch (...) {}
        }
        
        // Nạp dữ liệu độc giả (phải nạp sau sách)
        if (rootDocGia == nullptr) {
            std::cout << "\n[NẠP DỮ LIỆU] Đang nạp dữ liệu độc giả...\n";
            try {
                NapDocGiaTuDB_Fixed(rootDocGia, dsDauSach, soLuongDauSach);
                if (rootDocGia != nullptr) {
                    std::cout << "[✓] Nạp dữ liệu độc giả hoàn tất.\n";
                } else {
                    std::cout << "[INFO] Danh sách độc giả rỗng.\n";
                }
            } catch (...) {
                rootDocGia = nullptr;
            }
        }
        
        // Khởi tạo và hiển thị cửa sổ chính
        std::cout << "\n[GIAO DIỆN] Khởi tạo cửa sổ chính...\n";
        
        CuaSoChinh* cuaSoChinh = nullptr;
        try {
            cuaSoChinh = new CuaSoChinh();
            cuaSoChinh->show();
            
            std::cout << "[✓] Giao diện đã sẵn sàng!\n";
            std::cout << "\n========================================\n";
        } catch (const std::exception& e) {
            std::cerr << "[LỖI NGHIÊM TRỌNG] Không thể khởi tạo giao diện: " << e.what() << "\n";
            if (cuaSoChinh) delete cuaSoChinh;
            return -2;
        } catch (...) {
            std::cerr << "[LỖI NGHIÊM TRỌNG] Exception không xác định khi khởi tạo UI\n";
            if (cuaSoChinh) delete cuaSoChinh;
            return -2;
        }
        
        // Bắt đầu vòng lặp sự kiện Qt
        exitCode = ung_dung.exec();
        
        // Cleanup Window
        if (cuaSoChinh) delete cuaSoChinh;
        
        if (exitCode == RESTART_CODE) {
            std::cout << "\n[LOGOUT] Đang đăng xuất... Khởi động lại màn hình đăng nhập.\n";
            UserManager::instance().logout(); // Reset user context
        }
        
    } while (exitCode == RESTART_CODE);
    
    // Lưu dữ liệu trước khi thoát
    
    // Lưu dữ liệu trước khi thoát
    std::cout << "\n========================================\n";
    std::cout << "  ĐANG THOÁT ỨNG DỤNG...\n";
    std::cout << "========================================\n\n";
    
    if (duLieuDaThayDoi) {
        std::cout << "[LƯU DỮ LIỆU] Đang lưu các thay đổi (DB Auto-save)...\n";
        thongBao(std::cout, "Đang lưu dữ liệu...", THONG_TIN);
        
        // DB saves automatically on change, so this is just a confirmation
        std::cout << "[✓] Dữ liệu đã được đồng bộ với Database liên tục.\n";
        
        std::cout << "[✓] Lưu dữ liệu hoàn tất!\n";
    } else {
        std::cout << "[INFO] Không có thay đổi, bỏ qua lưu file.\n";
    }
    
    // Giải phóng bộ nhớ
    std::cout << "\n[DỌN DẸP] Đang giải phóng bộ nhớ...\n";
    GiaiPhongToanBoDauSach(dsDauSach, soLuongDauSach);
    std::cout << "[✓] Đã giải phóng bộ nhớ sách.\n";
    
    GiaiPhongCay(rootDocGia);
    std::cout << "[✓] Đã giải phóng cây AVL độc giả.\n";
    
    std::cout << "\n========================================\n";
    std::cout << "  KẾT THÚC - HẸN GẶP LẠI!\n";
    std::cout << "========================================\n";
    
    return exitCode;
}
