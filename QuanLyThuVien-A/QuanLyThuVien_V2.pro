QT += core gui widgets printsupport charts sql concurrent svg

CONFIG += c++17

greaterThan(QT_MAJOR_VERSION, 4): QT += core

# Tên ứng dụng
TARGET = QuanLyThuVien
TEMPLATE = app

# Define source directory for IDE development (Phase 11)
DEFINES += APP_SOURCE_DIR=\\\"$$PWD\\\"

# Resources
RESOURCES += resources/icons.qrc

# Đường dẫn include
INCLUDEPATH += \
    include \
    include/ui \
    include/ui/dialogs \
    include/utils \
    include/models \
    include/qrcode

# ============================================
# BACKEND - Giữ nguyên từ dự án cũ
# ============================================

# Models Backend (GIỮ NGUYÊN)
SOURCES += \
    src/models/DocGia.cpp \
    src/models/MuonTra.cpp \
    src/models/QuanLySach.cpp \
    src/models/KiemTraDuLieu.cpp \
    src/models/ThaoTacFile.cpp \
    src/models/NhapLieu.cpp \
    src/models/UserManager.cpp

HEADERS += \
    include/Constants.h \
    include/models/DauSach.h \
    include/models/DocGia.h \
    include/models/MuonTra.h \
    include/models/QuanLySach.h \
    include/models/KiemTraDuLieu.h \
    include/models/ThaoTacFile.h \
    include/models/NhapLieu.h \
    include/models/UserManager.h

# Utils Backend (GIỮ NGUYÊN MỘT SỐ)
SOURCES += \
    src/utils/NgayThang.cpp \
    src/utils/EventBus.cpp \
    src/utils/ThongBao.cpp \
    src/utils/XuLyChuoi.cpp \
    src/utils/FileManager.cpp \
    src/utils/PrintHelper.cpp \
    src/utils/BackupManager.cpp \
    src/utils/LanguageManager.cpp \
    src/utils/AuditLogger.cpp \
    src/utils/FilePathManager.cpp \
    src/utils/Utf8Helper.cpp \
    src/utils/ToastNotification.cpp \
    src/utils/DatabaseManager.cpp \
    src/utils/AutoSaveManager.cpp \
    src/utils/CustomPrintDialog.cpp \
    src/utils/SearchableListWidget.cpp \
    src/utils/QrCodeGenerator.cpp \
    src/utils/TransactionLogger.cpp \
    src/utils/PrintQueueManager.cpp \
    src/utils/DBBatcher.cpp \
    src/qrcode/qrcodegen.cpp \
    src/utils/SettingsManager.cpp \
    src/utils/GlobalScannerFilter.cpp

HEADERS += \
    include/utils/DatabaseManager.h \
    include/utils/PrintHelper.h \
    include/utils/BackupManager.h \
    include/utils/LanguageManager.h \
    include/utils/AuditLogger.h \
    include/utils/Utf8Helper.h \
    include/utils/ToastNotification.h \
    include/utils/FilePathManager.h \
    include/utils/NgayThang.h \
    include/utils/EventBus.h \
    include/utils/ThongBao.h \
    include/utils/XuLyChuoi.h \
    include/utils/FileManager.h \
    include/utils/AutoSaveManager.h \
    include/utils/CustomPrintDialog.h \
    include/utils/SearchableListWidget.h \
    include/utils/QrCodeGenerator.h \
    include/utils/TransactionLogger.h \
    include/utils/PrintQueueManager.h \
    include/qrcode/qrcodegen.hpp \
    include/utils/SettingsManager.h \
    include/utils/GlobalScannerFilter.h

# ============================================
# GIAO DIỆN MỚI - UI với tên tiếng Việt
# ============================================

# Main entry point
SOURCES += \
    src/main.cpp

# Giao diện chính (Main UI)
SOURCES += \
    src/ui/CuaSoChinh.cpp \
    src/ui/BangDieuKhien.cpp \
    src/ui/WidgetQuanLySach.cpp \
    src/ui/WidgetQuanLyDocGia.cpp \
    src/ui/WidgetMuonTraSach.cpp \
    src/ui/WidgetLichSuMuonTra.cpp

HEADERS += \
    include/ui/CuaSoChinh.h \
    include/ui/BangDieuKhien.h \
    include/ui/WidgetQuanLySach.h \
    include/ui/WidgetQuanLyDocGia.h \
    include/ui/WidgetMuonTraSach.h \
    include/ui/WidgetLichSuMuonTra.h

# Hộp thoại & Màn hình phụ (Dialogs)
SOURCES += \
    src/ui/dialogs/HopThoaiThemSach.cpp \
    src/ui/dialogs/HopThoaiThemDocGia.cpp \
    src/ui/dialogs/HopThoaiSuaSach.cpp \
    src/ui/dialogs/HopThoaiSuaDocGia.cpp \
    src/ui/dialogs/HopThoaiThemBanSao.cpp \
    src/ui/dialogs/HopThoaiThongKe.cpp \
    src/ui/dialogs/HopThoaiTop10.cpp \
    src/ui/dialogs/HopThoaiQuetMaVach.cpp \
    src/ui/dialogs/HopThoaiHangDoiIn.cpp \
    src/ui/dialogs/HopThoaiCauHinhIn.cpp \
    src/ui/dialogs/ManHinhChiTietSach.cpp \
    src/ui/dialogs/ManHinhChiTietDocGia.cpp \
    src/ui/dialogs/HopThoaiThanhToan.cpp \
    src/ui/dialogs/ManHinhKiemKe.cpp

HEADERS += \
    include/ui/dialogs/HopThoaiThemSach.h \
    include/ui/dialogs/HopThoaiThemDocGia.h \
    include/ui/dialogs/HopThoaiSuaSach.h \
    include/ui/dialogs/HopThoaiSuaDocGia.h \
    include/ui/dialogs/HopThoaiThemBanSao.h \
    include/ui/dialogs/HopThoaiThongKe.h \
    include/ui/dialogs/HopThoaiTop10.h \
    include/ui/dialogs/HopThoaiQuetMaVach.h \
    include/ui/dialogs/HopThoaiHangDoiIn.h \
    include/ui/dialogs/HopThoaiCauHinhIn.h \
    include/ui/dialogs/ManHinhChiTietSach.h \
    include/ui/dialogs/ManHinhChiTietDocGia.h \
    include/ui/dialogs/HopThoaiThanhToan.h \
    include/ui/dialogs/ManHinhKiemKe.h \
    include/ui/WidgetQuanLyNguoiDung.h \
    include/ui/dialogs/HopThoaiCaiDat.h

# Tiện ích UI mới (animation, responsive helpers)
SOURCES += \
    src/utils/ResponsiveHelper.cpp \
    src/utils/AnimationHelper.cpp \
    src/utils/ConfirmationDialog.cpp \
    src/ui/dialogs/ManHinhDangNhap.cpp \
    src/ui/WidgetQuanLyNguoiDung.cpp \
    src/ui/dialogs/HopThoaiCaiDat.cpp

HEADERS += \
    include/utils/ResponsiveHelper.h \
    include/utils/AnimationHelper.h \
    include/utils/ConfirmationDialog.h \
    include/ui/dialogs/ManHinhDangNhap.h

# ============================================
# TÀI NGUYÊN
# ============================================

RESOURCES += \
    resources/resources.qrc

# ============================================
# CẤU HÌNH BUILD
# ============================================

# Cấu hình debug/release
CONFIG(debug, debug|release) {
    DESTDIR = debug
    OBJECTS_DIR = debug/obj
    MOC_DIR = debug/moc
    RCC_DIR = debug/rcc
} else {
    DESTDIR = release
    OBJECTS_DIR = release/obj
    MOC_DIR = release/moc
    RCC_DIR = release/rcc
}

# Cài đặt deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Disable deprecated APIs
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

# Windows icon (nếu có)
# win32: RC_ICONS = tai_nguyen/bieu_tuong/app_icon.ico
win32: RC_ICONS = resources/app_icon.ico

# ============================================
# CHỈ DÙNG 1 THƯ MỤC data/ DUY NHẤT
# ============================================
# Không copy data/ vào build output
# App sẽ đọc/ghi trực tiếp từ thư mục data/ ở root project (FilePathManager xử lý khi cần)

# ManHinhQuanLyIn
SOURCES += src/ui/dialogs/ManHinhQuanLyIn.cpp
SOURCES += src/utils/ClipboardHelper.cpp
SOURCES += src/ui/dialogs/ManHinhCauHinh.cpp

HEADERS += include/ui/dialogs/ManHinhQuanLyIn.h
HEADERS += include/utils/ClipboardHelper.h
HEADERS += include/ui/dialogs/ManHinhCauHinh.h
HEADERS += include/ui/dialogs/ManHinhGioiThieu.h

SOURCES += src/ui/dialogs/ManHinhGioiThieu.cpp

SOURCES += src/ui/dialogs/ManHinhPreviewImport.cpp
SOURCES += src/utils/ImportManager.cpp
HEADERS += include/ui/dialogs/ManHinhPreviewImport.h
HEADERS += include/utils/ImportManager.h
SOURCES += src/qrcode/Code128.cpp
HEADERS += include/qrcode/Code128.h

# ============================================
# QXlsx Library (Native Excel Support) - Added for Task #ImportExcel
# ============================================
# include(3rdparty/QXlsx/QXlsx/QXlsx.pri)  # Tạm thời comment - thiếu file
