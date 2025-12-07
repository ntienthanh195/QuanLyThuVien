# 📚 HỆ THỐNG QUẢN LÝ THƯ VIỆN V2.0

## 🎯 Giới Thiệu

Phiên bản 2.0 của hệ thống Quản lý Thư viện được xây dựng lại hoàn toàn với:
- ✨ **Giao diện mới**: Modern Dark Theme, tên file/hàm/biến tiếng Việt đồng bộ
- 🔄 **Backend cũ**: Giữ nguyên logic xử lý dữ liệu đã được kiểm chứng
- 🎨 **UI/UX**: Material Design, responsive, animations

## 📁 Cấu Trúc Dự Án

```
QuanLyThuVien/
├── bao_gom/                    # Header files mới (tiếng Việt)
│   ├── giao_dien/             # UI headers
│   │   ├── CuaSoChinh.h       # MainWindow
│   │   ├── BangDieuKhien.h    # Dashboard
│   │   ├── WidgetQuanLySach.h
│   │   ├── WidgetQuanLyDocGia.h
│   │   └── WidgetMuonTraSach.h
│   └── tien_ich/              # Utilities
│
├── nguon/                      # Source files mới (tiếng Việt)
│   ├── main_v2.cpp            # Entry point
│   └── giao_dien/             # UI implementations
│       ├── CuaSoChinh.cpp
│       ├── BangDieuKhien.cpp
│       ├── WidgetQuanLySach.cpp
│       ├── WidgetQuanLyDocGia.cpp
│       └── WidgetMuonTraSach.cpp
│
├── include/                    # Backend headers (cũ - KHÔNG THAY ĐỔI)
│   ├── models/                # DauSach.h, DocGia.h, QuanLySach.h
│   └── utils/                 # NgayThang.h, ThongBao.h, XuLyChuoi.h
│
├── src/                        # Backend source (cũ - KHÔNG THAY ĐỔI)
│   ├── models/                # Logic xử lý dữ liệu
│   └── utils/                 # Tiện ích
│
├── resources/                  # Tài nguyên
│   ├── modern-dashboard.qss   # Stylesheet
│   ├── resources.qrc          # Qt resources
│   ├── fonts/                 # Phông chữ
│   └── icons/                 # Biểu tượng
│
├── data/                       # Dữ liệu (KHÔNG THAY ĐỔI)
│   ├── dausach.txt
│   ├── danhmucsach.txt
│   ├── docgia.txt
│   └── muontra.txt
│
├── QuanLyThuVien_V2.pro       # Qt project file MỚI
└── README_V2.md               # File này
```

## 🚀 Hướng Dẫn Build & Run

### Yêu Cầu

- **Qt 6.10.1** (hoặc cao hơn)
- **MinGW 13.1.0** (64-bit)
- **qmake**, **mingw32-make**
- **Windows 10/11**

### Các Bước Build

#### 1. Sử dụng Qt Creator (Khuyến nghị)

```
1. Mở Qt Creator
2. File -> Open File or Project...
3. Chọn: QuanLyThuVien_V2.pro
4. Cấu hình Kit: Desktop Qt 6.10.1 MinGW 64-bit
5. Build -> Build Project (Ctrl+B)
6. Run -> Run (Ctrl+R)
```

#### 2. Sử dụng Command Line

```powershell
# B1: Generate Makefile
C:/Qt/6.10.1/mingw_64/bin/qmake.exe QuanLyThuVien_V2.pro CONFIG+=debug

# B2: Compile
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe

# B3: Run
Start-Process "debug_v2\QuanLyThuVien_V2.exe"
```

#### 3. Build Release

```powershell
C:/Qt/6.10.1/mingw_64/bin/qmake.exe QuanLyThuVien_V2.pro CONFIG+=release
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe
```

File exe sẽ ở: `release_v2/QuanLyThuVien_V2.exe`

## 🎨 Giao Diện

### Màn Hình Chính
- **Sidebar Menu**: Navigation với icons
- **Dashboard**: Thống kê tổng quan
- **Thanh trạng thái**: Hiển thị thông báo real-time

### Các Module

#### 1. 📖 Quản Lý Sách
- Hiển thị danh sách đầu sách
- Thêm/sửa/xóa sách
- Thêm/xóa bản sao
- Tìm kiếm theo ISBN, tên, tác giả
- Lọc theo thể loại, trạng thái

#### 2. 👥 Quản Lý Độc Giả
- Hiển thị danh sách độc giả (AVL Tree)
- Thêm/sửa/xóa độc giả
- Khóa/mở khóa thẻ
- Tìm kiếm theo mã thẻ, họ, tên
- Xem lịch sử mượn trả

#### 3. 🔄 Mượn Trả Sách
- **Tab Mượn**: Nhập mã thẻ, chọn sách, xác nhận
- **Tab Trả**: Chọn sách đang mượn, tính phạt, xác nhận
- **Tab Lịch sử**: Xem tất cả giao dịch

#### 4. 📊 Thống Kê & Báo Cáo
- Top 10 sách được mượn
- Top 10 độc giả
- Sách quá hạn
- Biểu đồ trực quan

## 🔧 Cấu Trúc Code

### Tên File/Hàm/Biến Tiếng Việt

```cpp
// Header file
#include "giao_dien/CuaSoChinh.h"

class CuaSoChinh : public QMainWindow {
    Q_OBJECT
    
private:
    void thieLapGiaoDien();           // Setup UI
    void thieLapMenuBen();            // Setup sidebar
    void capNhatThanhTrangThai();     // Update status
    
    QStackedWidget *boChuyenManHinh;  // Stacked widget
    WidgetQuanLySach *widgetSach;     // Book widget
    QPushButton *nutTimKiem;          // Search button
    QLabel *nhanTieuDe;               // Title label
};
```

### Backend Integration

```cpp
// Sử dụng backend cũ
#include "models/DauSach.h"      // Từ include/models/
#include "models/DocGia.h"
#include "models/QuanLySach.h"

// Biến toàn cục từ backend
extern PTRDS dsDauSach[];
extern int soLuongDauSach;
extern PTRDG rootDocGia;

// Gọi hàm backend
NapDanhSachDauSach(FILE_DAUSACH, dsDauSach, soLuongDauSach);
timDocGia(rootDocGia, maThe);
MuonSach(docGia, isbn, dsDauSach, soLuongDauSach);
```

## 📚 Cấu Trúc Dữ Liệu

### Backend (Không thay đổi)

```cpp
// Đầu sách (Array)
struct DauSach {
    std::string ISBN;
    std::string tenSach;
    int soTrang;
    std::string tacGia;
    int namXuatBan;
    std::string theLoai;
    PTRDMS dms;         // Linked list bản sao
    int tongBanSao;
    int soLuotMuon;
};

// Độc giả (AVL Tree)
struct TheDocGia {
    int MaThe;
    std::string Ho;
    std::string Ten;
    bool Phai;
    int TrangThai;
    int soSachDangMuon;
    MUONTRA dsmt;       // Linked list mượn trả
};
```

## 🎯 Chức Năng Đã Implement

✅ **Core Features**:
- [x] Nạp/lưu dữ liệu từ file
- [x] Hiển thị danh sách sách
- [x] Hiển thị danh sách độc giả
- [x] Tìm kiếm cơ bản
- [x] Navigation giữa các màn hình
- [x] Status bar thông báo

🚧 **Đang Phát Triển**:
- [ ] Dialog thêm/sửa sách
- [ ] Dialog thêm/sửa độc giả
- [ ] Xử lý mượn/trả sách đầy đủ
- [ ] Thống kê với biểu đồ
- [ ] Top 10 reports
- [ ] Backup/Restore

## 🛠️ Troubleshooting

### Lỗi "Cannot find data/dausach.txt"

**Nguyên nhân**: Working directory không đúng

**Giải pháp**:
```powershell
# Đảm bảo chạy từ thư mục gốc
cd E:\Qt\QuanLyThuVien
.\debug_v2\QuanLyThuVien_V2.exe
```

### Build failed

**Kiểm tra**:
1. Qt version >= 6.10
2. MinGW 64-bit đã cài đặt
3. PATH environment variables
4. File `.pro` không bị lỗi syntax

### UI không hiển thị đúng

**Kiểm tra**:
1. File `resources/modern-dashboard.qss` tồn tại
2. Qt Charts module đã cài đặt
3. Font files trong `resources/fonts/`

## 📝 Lưu Ý

### Phát Triển Thêm

1. **Thêm Dialog mới**:
   - Tạo file `.h` trong `bao_gom/giao_dien/hop_thoai/`
   - Tạo file `.cpp` trong `nguon/giao_dien/hop_thoai/`
   - Thêm vào `QuanLyThuVien_V2.pro`

2. **Sử dụng Backend**:
   - Include file từ `include/models/`
   - Gọi extern variables: `dsDauSach`, `rootDocGia`
   - Sử dụng các hàm có sẵn

3. **Styling**:
   - Chỉnh sửa `resources/modern-dashboard.qss`
   - Sử dụng màu theo palette đã định nghĩa
   - Follow naming convention tiếng Việt

## 🤝 Đóng Góp

Để đóng góp vào dự án:

1. Giữ nguyên backend cũ
2. Tên file/hàm/biến mới phải bằng tiếng Việt
3. Follow style guide hiện có
4. Test kỹ trước khi commit
5. Update README nếu thêm tính năng mới

## 📞 Liên Hệ

- **Dự án**: Hệ Thống Quản Lý Thư Viện
- **Version**: 2.0.0
- **Qt Version**: 6.10.1
- **Build Date**: December 2025

---

**Powered by Qt Framework | Modern C++17 | AVL Tree | Material Design**

🎉 **Enjoy coding!**
