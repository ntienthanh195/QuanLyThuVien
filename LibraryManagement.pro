QT += core gui widgets printsupport

CONFIG += c++17

# Enable QRegularExpression
greaterThan(QT_MAJOR_VERSION, 4): QT += core

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Include paths
INCLUDEPATH += \
    include \
    include/models \
    include/views \
    include/views/dialogs \
    include/utils

# Source files
SOURCES += \
    src/main.cpp \
    src/models/DocGia.cpp \
    src/models/MuonTra.cpp \
    src/models/QuanLySach.cpp \
    src/models/KiemTraDuLieu.cpp \
    src/models/ThaoTacFile.cpp \
    src/models/NhapLieu.cpp \
    src/views/MainWindow.cpp \
    src/views/WidgetBangDieuKhien.cpp \
    src/views/WidgetQuanLySach.cpp \
    src/views/WidgetQuanLyDocGia.cpp \
    src/views/WidgetMuonTraSach.cpp \
    src/views/BadgeStyledItemDelegate.cpp \
    src/views/dialogs/ManHinhGioiThieu.cpp \
    src/views/dialogs/ManHinhThemSach.cpp \
    src/views/dialogs/ManHinhChiTietSach.cpp \
    src/views/dialogs/ManHinhThemBanSao.cpp \
    src/views/dialogs/ManHinhSuaSach.cpp \
    src/views/dialogs/ManHinhThemDocGia.cpp \
    src/views/dialogs/ManHinhSuaDocGia.cpp \
    src/views/dialogs/ManHinhTop10Sach.cpp \
    src/views/dialogs/ManHinhBaoCaoQuaHan.cpp \
    src/views/dialogs/ManHinhThongKe.cpp \
    src/views/dialogs/ManHinhBackup.cpp \
    src/views/dialogs/ManHinhTop10.cpp \
    src/views/dialogs/ManHinhTop10DocGia.cpp \
    src/views/dialogs/ManHinhQuaHan.cpp \
    src/views/dialogs/ManHinhDemoPhase1.cpp \
    src/utils/NgayThang.cpp \
    src/utils/ThongBao.cpp \
    src/utils/XuLyChuoi.cpp \
    src/utils/FileManager.cpp \
    src/utils/BackupManager.cpp \
    src/utils/ResponsiveHelper.cpp \
    src/utils/AnimationHelper.cpp \
    src/utils/ConfirmationDialog.cpp \
    src/utils/KieuNutBam.cpp \
    src/utils/TruongNhapLieu.cpp \
    src/utils/KieuBangDuLieu.cpp \
    src/views/dialogs/ManHinhDemoBang.cpp

HEADERS += \
    include/Constants.h \
    include/models/DauSach.h \
    include/models/DocGia.h \
    include/models/QuanLySach.h \
    include/models/KiemTraDuLieu.h \
    include/models/ThaoTacFile.h \
    include/models/NhapLieu.h \
    include/views/MainWindow.h \
    include/views/WidgetBangDieuKhien.h \
    include/views/WidgetQuanLySach.h \
    include/views/WidgetQuanLyDocGia.h \
    include/views/WidgetMuonTraSach.h \
    include/views/BadgeStyledItemDelegate.h \
    include/views/dialogs/ManHinhGioiThieu.h \
    include/views/dialogs/ManHinhThemSach.h \
    include/views/dialogs/ManHinhChiTietSach.h \
    include/views/dialogs/ManHinhThemBanSao.h \
    include/views/dialogs/ManHinhSuaSach.h \
    include/views/dialogs/ManHinhThemDocGia.h \
    include/views/dialogs/ManHinhSuaDocGia.h \
    include/views/dialogs/ManHinhTop10Sach.h \
    include/views/dialogs/ManHinhBaoCaoQuaHan.h \
    include/views/dialogs/ManHinhThongKe.h \
    include/views/dialogs/ManHinhBackup.h \
    include/views/dialogs/ManHinhTop10.h \
    include/views/dialogs/ManHinhTop10DocGia.h \
    include/views/dialogs/ManHinhQuaHan.h \
    include/views/dialogs/ManHinhDemoPhase1.h \
    include/utils/NgayThang.h \
    include/utils/ThongBao.h \
    include/utils/XuLyChuoi.h \
    include/utils/FileManager.h \
    include/utils/BackupManager.h \
    include/utils/ResponsiveHelper.h \
    include/utils/AnimationHelper.h \
    include/utils/ConfirmationDialog.h \
    include/utils/KieuNutBam.h \
    include/utils/TruongNhapLieu.h \
    include/utils/KieuBangDuLieu.h \
    include/views/dialogs/ManHinhDemoBang.h

# UI files (tạm thời comment out vì chưa có)
# FORMS += \
#     ui/MainWindow.ui \
#     ui/BookManagementWidget.ui \
#     ui/ReaderManagementWidget.ui \
#     ui/LoanManagementWidget.ui \
#     ui/AboutDialog.ui

# Resources
RESOURCES += \
    resources/resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
