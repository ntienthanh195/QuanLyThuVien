# 📚 Hệ Thống Quản Lý Thư Viện (Qt Desktop Application)

## 🌟 Giới Thiệu

Ứng dụng desktop hoàn chỉnh để quản lý thư viện, được xây dựng bằng **Qt 6.10.1** framework. Chạy như một ứng dụng thực sự trên Windows với giao diện đẹp mắt và nhiều tính năng mạnh mẽ.

## ✨ Tính Năng Chính

### 📖 Quản Lý Sách
- ✅ Tìm kiếm sách real-time với debounce 300ms
- ✅ Sắp xếp theo cột (click header)
- ✅ Thêm/Sửa/Xóa đầu sách và bản sao
- ✅ Import/Export CSV
- ✅ In danh sách sách với HTML formatting
- ✅ Xem chi tiết bản sao, thanh lý sách

### 👥 Quản Lý Độc Giả
- ✅ Tìm kiếm và sắp xếp độc giả
- ✅ Khóa/Mở khóa tài khoản
- ✅ Xem lịch sử mượn trả chi tiết
- ✅ In thẻ độc giả (khổ A7)
- ✅ Cảnh báo quá hạn và trạng thái
- ✅ Export danh sách CSV
- ✅ Toolbar với các thao tác nhanh

### 📚 Mượn - Trả Sách
- ✅ Model bảng 7 cột với màu sắc rõ ràng
- ✅ Tự động tính phí phạt (5,000đ/ngày)
- ✅ Gia hạn nhanh
- ✅ In biên lai (Ctrl+P)
- ✅ Trả nhiều sách cùng lúc
- ✅ Tự động cập nhật khi đổi độc giả

### 📊 Báo Cáo & Thống Kê

#### 🏆 Top 10 Sách (Nâng Cao)
- ✅ Bộ lọc thời gian: Tất cả, Tháng này, Tháng trước, Tùy chỉnh
- ✅ So sánh xu hướng với kỳ trước (🔼/🔽)
- ✅ Bảng 7 cột với chỉ số thay đổi
- ✅ Panel phân tích xu hướng màu sắc
- ✅ Xuất báo cáo PDF (Ctrl+P)

#### ⚠️ Báo Cáo Quá Hạn (Nâng Cao)
- ✅ Thống kê doanh thu từ phí phạt
- ✅ Đếm độc giả hoạt động (7/30 ngày)
- ✅ Bảng 8 cột với cột phí phạt
- ✅ Xem chi tiết từng độc giả
- ✅ Breakdown sách quá hạn và phí
- ✅ Xuất báo cáo PDF với tổng kết

#### 📈 Thống Kê Hệ Thống
- ✅ Tổng quan sách: Đầu sách, bản sao, tỷ lệ cho mượn
- ✅ Thống kê độc giả: Tổng số, hoạt động, bị khóa
- ✅ Số sách đang mượn và quá hạn
- ✅ Giao diện 2 cột với màu sắc trực quan

### 💾 Backup & Restore
- ✅ Backup tự động với timestamp
- ✅ Khôi phục từ các bản backup
- ✅ Quản lý danh sách backup

## 🚀 Cài Đặt & Chạy

### Yêu Cầu Hệ Thống
- **Windows 10/11** (64-bit)
- **Qt 6.10.1** (MinGW 13.1.0)
- **C++17** compiler

### Cách 1: Chạy File EXE (Đơn Giản)
```bash
# Chỉ cần double-click file:
release\LibraryManagement.exe
```

### Cách 2: Sử Dụng Script
```bash
# Build và chạy lần đầu
build-and-run.bat

# Chạy nhanh (không build lại)
run-app.bat
```

### Cách 3: Build Thủ Công
```bash
# 1. Generate Makefile
C:\Qt\6.10.1\mingw_64\bin\qmake.exe LibraryManagement.pro

# 2. Build application
C:\Qt\Tools\mingw1310_64\bin\mingw32-make.exe

# 3. Run
release\LibraryManagement.exe
```

## 📂 Cấu Trúc Project

```
LibraryManagement/
├── src/
│   ├── main.cpp                    # Qt Application entry point
│   ├── models/                     # Business logic
│   │   ├── DocGia.cpp             # Reader management (AVL tree)
│   │   ├── QuanLySach.cpp         # Book management
│   │   ├── MuonTra.cpp            # Loan/return operations
│   │   └── ThaoTacFile.cpp        # File I/O
│   ├── views/
│   │   ├── MainWindow.cpp         # Main window with tabs
│   │   ├── WidgetQuanLySach.cpp   # Book management widget
│   │   ├── WidgetQuanLyDocGia.cpp # Reader management widget
│   │   ├── WidgetMuonTraSach.cpp  # Loan/return widget
│   │   └── dialogs/               # Dialog windows
│   │       ├── ManHinhTop10.cpp   # Advanced Top 10 report
│   │       ├── ManHinhQuaHan.cpp  # Overdue report with revenue
│   │       ├── ManHinhThongKe.cpp # System statistics
│   │       └── ...
│   └── utils/                      # Utilities
│       ├── NgayThang.cpp          # Date calculations
│       ├── XuLyChuoi.cpp          # String processing
│       └── BackupManager.cpp      # Backup system
├── include/                        # Header files
├── files/                          # Data files
│   ├── dausach.txt                # Book catalog
│   ├── danhmucsach.txt            # Book copies
│   ├── docgia.txt                 # Readers
│   └── muontra.txt                # Loan records
├── release/
│   └── LibraryManagement.exe      # Final executable
├── LibraryManagement.pro          # Qt project file
├── build-and-run.bat              # Build & run script
└── run-app.bat                    # Quick run script
```

## 🎨 Giao Diện

### Màn Hình Chính
- 🏠 **Home Screen**: Menu với 6 nút lớn
- 📚 **Tab Quản Lý Sách**: Bảng sách + Form thêm/sửa
- 👥 **Tab Quản Lý Độc Giả**: Bảng độc giả + Lịch sử
- 📖 **Tab Mượn Trả**: Tìm độc giả + Bảng sách mượn

### Phím Tắt
- **Ctrl+B**: Quản lý sách
- **Ctrl+R**: Quản lý độc giả
- **Ctrl+L**: Mượn trả sách
- **Ctrl+T**: Top 10 sách
- **Ctrl+S**: Thống kê hệ thống
- **Ctrl+H**: Báo cáo quá hạn
- **Ctrl+P**: In báo cáo (trong dialogs)
- **Ctrl+Q**: Thoát ứng dụng

## 📊 Dữ Liệu

### Cấu Trúc Dữ Liệu
- **Sách**: Mảng con trỏ, sắp xếp theo tên
- **Độc Giả**: Cây AVL cân bằng, key = MaThe
- **Mượn Trả**: Danh sách liên kết đơn cho mỗi độc giả

### File Dữ Liệu
- `dausach.txt`: ISBN|Tên|Trang|Tác giả|Năm|Thể loại|Số lượng
- `danhmucsach.txt`: MaSach|ISBN|TrangThai|ViTri
- `docgia.txt`: MaThe|Ho|Ten|Phai|TrangThai
- `muontra.txt`: MaThe|MaSach|NgayMuon|NgayTra|TrangThai

### Backup Tự Động
- Backup tạo timestamp: `backup_YYYYMMDD_HHMMSS_*.txt`
- Lưu trong thư mục `files/`
- Khôi phục 1-click từ dialog Backup

## 🔧 Phát Triển

### Build Modes
```bash
# Release mode (optimized, small size)
qmake CONFIG+=release
mingw32-make

# Debug mode (with symbols)
qmake CONFIG+=debug
mingw32-make
```

### Thêm Module Mới
1. Tạo `.cpp` và `.h` trong `src/` và `include/`
2. Thêm vào `LibraryManagement.pro`:
```qmake
SOURCES += src/views/MyNewWidget.cpp
HEADERS += include/views/MyNewWidget.h
```
3. Rebuild: `qmake && mingw32-make`

### Coding Style
- C++17 standard
- Qt naming conventions
- Tiếng Việt cho UI text
- English for code/comments

## 📝 Changelog

### Version 2.0.0 (December 2025)
- ✅ Chuyển đổi hoàn toàn từ SFML sang Qt 6.10.1
- ✅ Xây dựng ứng dụng desktop đầy đủ
- ✅ 3 widget chính: Sách, Độc giả, Mượn trả
- ✅ Báo cáo nâng cao với PDF export
- ✅ Revenue tracking cho phí phạt
- ✅ Trend analysis cho Top 10
- ✅ System statistics dashboard
- ✅ Backup & restore system

### Version 1.0.0 (2024)
- SFML-based console application
- Basic book/reader management
- Simple loan/return operations

## 🐛 Known Issues

- ⚠️ IntelliSense có thể báo lỗi Qt headers (không ảnh hưởng build)
- ⚠️ Resource file (resources.qrc) empty warning (có thể bỏ qua)

## 📞 Support

Nếu gặp lỗi khi build:
1. Check Qt installation path trong `.pro` file
2. Verify MinGW 13.1.0 installed
3. Run `qmake` trước khi `mingw32-make`
4. Clean build: `mingw32-make clean`

## 📜 License

Educational project - Free to use for learning purposes.

---

**🎉 Chúc bạn sử dụng app vui vẻ!**

*Built with ❤️ using Qt 6.10.1*
#   Q u a n L y T h u V i e n  
 