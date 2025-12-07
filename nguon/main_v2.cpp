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
    
    // Load font mã vạch từ resources
    int fontId = QFontDatabase::addApplicationFont(":/fonts/LibreBarcode128Text-Regular.ttf");
    if (fontId != -1) {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
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
    
    std::cout << "[KHỞI TẠO] Đang khởi động ứng dụng Qt...\n";
    
    // Thiết lập thư mục làm việc
    // Khởi tạo FilePathManager
    FilePathManager::initialize();
    
    QString thuMucGoc = FilePathManager::getRootPath();
    QDir::setCurrent(thuMucGoc);
    std::cout << "[THƯ MỤC] Working Directory: " << thuMucGoc.toStdString() << "\n";
    std::cout << "[THƯ MỤC] Files Directory: " << FilePathManager::getFilesDirectory().toStdString() << "\n";
    
    // Kiểm tra file dữ liệu
    std::cout << "\n[KIỂM TRA] Kiểm tra các file dữ liệu...\n";
    
    QString fileDauSach = FilePathManager::getDauSachPath();
    QString fileDanhMuc = FilePathManager::getDanhMucSachPath();
    QString fileDocGia = FilePathManager::getDocGiaPath();
    QString fileMuonTra = FilePathManager::getMuonTraPath();
    
    if (!QFile::exists(fileDauSach)) {
        std::cerr << "[LỖI] Không tìm thấy " << fileDauSach.toStdString() << "\n";
        std::cerr << "       Thư mục hiện tại: " << QDir::currentPath().toStdString() << "\n";
        std::cerr << "       Thư mục gốc: " << thuMucGoc.toStdString() << "\n";
        std::cerr << "\n[HƯỚNG DẪN SỬA LỖI]\n";
        std::cerr << "1. Trong Qt Creator, bấm vào Projects (biểu tượng cặp táp)\n";
        std::cerr << "2. Chọn Run (bên dưới bộ Kit)\n";
        std::cerr << "3. Tìm mục 'Working Directory'\n";
        std::cerr << "4. Bấm Browse và chọn: E:/Qt/QuanLyThuVien/\n";
        std::cerr << "5. Chạy lại chương trình\n\n";
        QMessageBox::critical(nullptr, "Lỗi Dữ Liệu", 
            QString("⚠️ KHÔNG TÌM THẤY FILE DỮ LIỆU\n\n"
            "Chương trình đang chạy từ: %1\n"
            "Nhưng file dữ liệu nằm ở: %2\n\n"
            "CÁCH SỬA:\n"
            "1. Mở Projects (biểu tượng cặp táp)\n"
            "2. Chọn Run → Working Directory\n"
            "3. Đặt là: E:/Qt/QuanLyThuVien/\n\n"
            "Hoặc copy thư mục 'data/' vào: %1"
            ).arg(QDir::currentPath()).arg(thuMucGoc));
        return -1;
    }
    
    std::cout << "[✓] " << fileDauSach.toStdString() << " - Tồn tại\n";
    std::cout << "[✓] " << fileDanhMuc.toStdString() << " - " 
              << (QFile::exists(fileDanhMuc) ? "Tồn tại" : "Không tồn tại") << "\n";
    std::cout << "[✓] " << fileDocGia.toStdString() << " - " 
              << (QFile::exists(fileDocGia) ? "Tồn tại" : "Không tồn tại") << "\n";
    
    // Nạp dữ liệu sách từ file
    std::cout << "\n[NẠP DỮ LIỆU] Đang nạp dữ liệu đầu sách...\n";
    
    try {
        NapDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, std::cout);
        NapDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, std::cout);
        
        if (soLuongDauSach > 0) {
            std::cout << "[✓] Nạp thành công: " << soLuongDauSach << " đầu sách.\n";
            sapXepDauSachTheoTen(dsDauSach, 0, soLuongDauSach - 1);
            std::cout << "[✓] Sắp xếp đầu sách theo tên - Hoàn tất.\n";
        } else {
            std::cout << "[!] Cảnh báo: Không nạp được đầu sách nào - Khởi tạo danh sách rỗng.\n";
        }
    } catch (...) {
        std::cerr << "[LỖI] Exception khi nạp dữ liệu sách - Tiếp tục với dữ liệu rỗng.\n";
    }
    
    // Nạp dữ liệu độc giả (phải nạp sau sách)
    std::cout << "\n[NẠP DỮ LIỆU] Đang nạp dữ liệu độc giả...\n";
    
    try {
        rootDocGia = loadDocGia(dsDauSach, soLuongDauSach);
        
        if (rootDocGia != nullptr) {
            // Đếm số độc giả
            int soDocGia = DemDocGiaTheoTrangThai(rootDocGia, -1); // -1 = tất cả
            std::cout << "[✓] Nạp thành công: " << soDocGia << " độc giả.\n";
        } else {
            std::cout << "[!] Cảnh báo: Không nạp được độc giả - Khởi tạo danh sách rỗng.\n";
        }
    } catch (...) {
        std::cerr << "[LỖI] Exception khi nạp dữ liệu độc giả - Tiếp tục với dữ liệu rỗng.\n";
        rootDocGia = nullptr;
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
    int ketQua = ung_dung.exec();
    
    // Lưu dữ liệu trước khi thoát
    std::cout << "\n========================================\n";
    std::cout << "  ĐANG THOÁT ỨNG DỤNG...\n";
    std::cout << "========================================\n\n";
    
    if (duLieuDaThayDoi) {
        std::cout << "[LƯU DỮ LIỆU] Đang lưu các thay đổi...\n";
        thongBao(std::cout, "Đang lưu dữ liệu...", THONG_TIN);
        
        GhiDanhSachDauSach(FilePathManager::getDauSachPathStd().c_str(), dsDauSach, soLuongDauSach, false, std::cout);
        std::cout << "[✓] Đã lưu danh sách đầu sách.\n";
        
        GhiDanhMucSach(FilePathManager::getDanhMucSachPathStd().c_str(), dsDauSach, soLuongDauSach, false, std::cout);
        std::cout << "[✓] Đã lưu danh mục sách.\n";
        
        saveDocGia(rootDocGia);
        std::cout << "[✓] Đã lưu dữ liệu độc giả.\n";
        
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
    
    return ketQua;
}
