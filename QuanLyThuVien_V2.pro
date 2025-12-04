QT += core gui widgets printsupport charts

CONFIG += c++17

greaterThan(QT_MAJOR_VERSION, 4): QT += core

# Tên ứng dụng
TARGET = QuanLyThuVien
TEMPLATE = app

# Resources
RESOURCES += resources/icons.qrc

# Đường dẫn include
INCLUDEPATH += \
    bao_gom \
    bao_gom/giao_dien \
    bao_gom/giao_dien/hop_thoai \
    bao_gom/tien_ich \
    include \
    include/models \
    include/views \
    include/utils \
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
    src/models/NhapLieu.cpp

HEADERS += \
    include/Constants.h \
    include/models/DauSach.h \
    include/models/DocGia.h \
    include/models/QuanLySach.h \
    include/models/KiemTraDuLieu.h \
    include/models/ThaoTacFile.h \
    include/models/NhapLieu.h

# Utils Backend (GIỮ NGUYÊN MỘT SỐ)
SOURCES += \
    src/utils/NgayThang.cpp \
    src/utils/ThongBao.cpp \
    src/utils/XuLyChuoi.cpp \
    src/utils/FileManager.cpp \
    src/utils/BackupManager.cpp \
    src/utils/FilePathManager.cpp \
    src/utils/Utf8Helper.cpp \
    src/utils/AutoSaveManager.cpp \
    src/utils/PrintHelper.cpp \
    src/utils/QrCodeGenerator.cpp \
    src/utils/TransactionLogger.cpp \
    src/utils/PrintQueueManager.cpp \
    src/qrcode/qrcodegen.cpp

HEADERS += \
    include/utils/NgayThang.h \
    include/utils/ThongBao.h \
    include/utils/XuLyChuoi.h \
    include/utils/FileManager.h \
    include/utils/BackupManager.h \
    include/utils/FilePathManager.h \
    include/utils/Utf8Helper.h \
    include/utils/AutoSaveManager.h \
    include/utils/PrintHelper.h \
    include/utils/QrCodeGenerator.h \
    include/utils/TransactionLogger.h \
    include/utils/PrintQueueManager.h \
    include/qrcode/qrcodegen.hpp

# ============================================
# GIAO DIỆN MỚI - UI với tên tiếng Việt
# ============================================

# Main entry point
SOURCES += \
    nguon/main_v2.cpp

# Giao diện chính
SOURCES += \
    nguon/giao_dien/CuaSoChinh.cpp \
    nguon/giao_dien/BangDieuKhien.cpp \
    nguon/giao_dien/WidgetQuanLySach.cpp \
    nguon/giao_dien/WidgetQuanLyDocGia.cpp \
    nguon/giao_dien/WidgetMuonTraSach.cpp

HEADERS += \
    bao_gom/giao_dien/CuaSoChinh.h \
    bao_gom/giao_dien/BangDieuKhien.h \
    bao_gom/giao_dien/WidgetQuanLySach.h \
    bao_gom/giao_dien/WidgetQuanLyDocGia.h \
    bao_gom/giao_dien/WidgetMuonTraSach.h

# Dialog phụ
SOURCES += \
    nguon/giao_dien/hop_thoai/HopThoaiThemSach.cpp \
    nguon/giao_dien/hop_thoai/HopThoaiThemDocGia.cpp \
    nguon/giao_dien/hop_thoai/HopThoaiSuaSach.cpp \
    nguon/giao_dien/hop_thoai/HopThoaiSuaDocGia.cpp \
    nguon/giao_dien/hop_thoai/HopThoaiThemBanSao.cpp \
    nguon/giao_dien/hop_thoai/HopThoaiThongKe.cpp \
    nguon/giao_dien/hop_thoai/HopThoaiTop10.cpp \
    nguon/giao_dien/hop_thoai/HopThoaiIn.cpp \
    nguon/giao_dien/hop_thoai/HopThoaiInAn.cpp \
    nguon/giao_dien/hop_thoai/HopThoaiQuetMaVach.cpp \
    nguon/giao_dien/hop_thoai/HopThoaiHangDoiIn.cpp \
    src/views/dialogs/ManHinhChiTietSach.cpp

HEADERS += \
    bao_gom/giao_dien/hop_thoai/HopThoaiThemSach.h \
    bao_gom/giao_dien/hop_thoai/HopThoaiThemDocGia.h \
    bao_gom/giao_dien/hop_thoai/HopThoaiSuaSach.h \
    bao_gom/giao_dien/hop_thoai/HopThoaiSuaDocGia.h \
    bao_gom/giao_dien/hop_thoai/HopThoaiThemBanSao.h \
    bao_gom/giao_dien/hop_thoai/HopThoaiThongKe.h \
    bao_gom/giao_dien/hop_thoai/HopThoaiTop10.h \
    bao_gom/giao_dien/hop_thoai/HopThoaiIn.h \
    bao_gom/giao_dien/hop_thoai/HopThoaiInAn.h \
    bao_gom/giao_dien/hop_thoai/HopThoaiQuetMaVach.h \
    bao_gom/giao_dien/hop_thoai/HopThoaiHangDoiIn.h \
    include/views/dialogs/ManHinhChiTietSach.h

# Tiện ích UI mới (animation, responsive helpers)
SOURCES += \
    src/utils/ResponsiveHelper.cpp \
    src/utils/AnimationHelper.cpp \
    src/utils/ConfirmationDialog.cpp

HEADERS += \
    include/utils/ResponsiveHelper.h \
    include/utils/AnimationHelper.h \
    include/utils/ConfirmationDialog.h

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
    DESTDIR = debug_v2
    OBJECTS_DIR = debug_v2/obj
    MOC_DIR = debug_v2/moc
    RCC_DIR = debug_v2/rcc
} else {
    DESTDIR = release_v2
    OBJECTS_DIR = release_v2/obj
    MOC_DIR = release_v2/moc
    RCC_DIR = release_v2/rcc
}

# Cài đặt deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Disable deprecated APIs
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

# Windows icon (nếu có)
# win32: RC_ICONS = tai_nguyen/bieu_tuong/app_icon.ico

# ============================================
# CHỈ DÙNG 1 THƯ MỤC files/ DUY NHẤT
# ============================================
# Không copy files/ vào build output
# App sẽ đọc/ghi trực tiếp từ thư mục files/ ở root project
