# 📚 Library Management System - Qt6 Version

Hệ thống quản lý thư viện được viết lại hoàn toàn với Qt6 Framework từ phiên bản SFML cũ.

## 🎯 Tính năng chính

- ✅ **Quản lý Sách**: Thêm, sửa, xóa, tìm kiếm đầu sách và bản sao
- ✅ **Quản lý Độc Giả**: Quản lý thông tin độc giả (AVL Tree)
- ✅ **Mượn/Trả Sách**: Xử lý giao dịch mượn, trả, báo mất
- ✅ **Báo cáo**: Top 10 sách, danh sách quá hạn
- ✅ **Giao diện**: Qt Widgets với Model/View pattern

## 🏗️ Kiến trúc

```
├── src/
│   ├── main.cpp              # Entry point
│   ├── models/               # Business logic (AVL, Linked List, Array)
│   └── views/                # Qt Widgets UI
├── include/
│   ├── models/               # Model headers
│   ├── views/                # View headers
│   └── utils/                # Utility headers
├── ui/                       # Qt Designer files (optional)
├── resources/                # Icons, fonts
└── files/                    # Data files (.txt)
```

## 📋 Yêu cầu hệ thống

- **Qt 6.x** (Qt6::Core, Qt6::Widgets)
- **C++17** compiler
- **CMake 3.16+** hoặc **qmake**
- **Windows 10/11** (có thể build trên Linux/macOS)

### Cài đặt Qt6

**Windows:**
```powershell
# Download Qt Online Installer từ https://www.qt.io/download
# Chọn Qt 6.x for Desktop (MinGW hoặc MSVC)
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt install qt6-base-dev qt6-tools-dev
```

**macOS:**
```bash
brew install qt@6
```

## 🔨 Build Instructions

### Phương pháp 1: Qt Creator (Khuyên dùng)

1. Mở **Qt Creator**
2. File → Open Project → Chọn `LibraryManagement.pro`
3. Configure project với Qt 6.x kit
4. Build (Ctrl+B) và Run (Ctrl+R)

### Phương pháp 2: Command line với qmake

```powershell
# Từ thư mục gốc dự án
qmake LibraryManagement.pro
mingw32-make        # Hoặc nmake nếu dùng MSVC
.\bin\LibraryManagement.exe
```

### Phương pháp 3: CMake

```powershell
# Tạo thư mục build
mkdir build
cd build

# Configure với CMake
cmake .. -G "MinGW Makefiles"    # Hoặc "NMake Makefiles" cho MSVC

# Build
cmake --build .

# Run
.\LibraryManagement.exe
```

### Phương pháp 4: CMake với Qt Creator

1. Mở Qt Creator
2. File → Open File or Project → Chọn `CMakeLists.txt`
3. Configure và Build

## 📦 Cấu trúc dữ liệu

| Thành phần | Cấu trúc | Sắp xếp |
|------------|----------|---------|
| Đầu sách | Mảng con trỏ | Theo tên (Quick Sort) |
| Danh mục sách | Linked List đơn | Theo mã sách |
| Độc giả | AVL Tree | Theo mã thẻ |
| Mượn trả | Linked List đơn | Theo thứ tự mượn |

## 🎨 Giao diện

### Màn hình chính
- Menu với 5 nút: Quản lý Sách, Độc Giả, Mượn/Trả, Thông tin, Thoát

### Màn hình Quản lý Sách
- **QTableView**: Hiển thị danh sách sách (7 cột)
- **Tìm kiếm**: Theo ISBN, tên, tác giả, thể loại
- **CRUD**: Thêm, sửa, xóa, thêm bản sao
- **Chi tiết**: Xem thông tin bản sao

### Màn hình Quản lý Độc Giả
- **QTableView**: Hiển thị độc giả (5 cột)
- **Tìm kiếm**: Theo mã thẻ
- **CRUD**: Thêm, sửa, xóa độc giả
- **Xem sách đang mượn**

### Màn hình Mượn/Trả
- Form nhập: Mã thẻ, ISBN/Mã sách
- Chức năng: Mượn, Trả, Báo mất
- Báo cáo: Quá hạn, Top 10

## 📁 File dữ liệu

Dữ liệu được lưu trong thư mục `files/`:

- `dausach.txt`: Thông tin đầu sách
- `danhmucsach.txt`: Danh sách bản sao
- `docgia.txt`: Thông tin độc giả
- `muontra.txt`: Giao dịch mượn trả (embedded)

**Format lưu trữ**: Text file với delimiter `|`

## 🐛 Troubleshooting

### Lỗi: "Qt6 not found"
```bash
# Set Qt6 path
export CMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64
# Hoặc trên Windows
set CMAKE_PREFIX_PATH=C:\Qt\6.x\mingw_64
```

### Lỗi: "Cannot open include file"
- Kiểm tra `INCLUDEPATH` trong .pro file
- Kiểm tra `include_directories` trong CMakeLists.txt
- Đảm bảo các header files có trong `include/models/` và `include/utils/`

### Lỗi: "Undefined reference to..."
- Kiểm tra tất cả .cpp files đã được thêm vào `SOURCES` (.pro) hoặc `set(xxx_SOURCES ...)` (CMake)
- Clean và rebuild project

## 🔄 Migration từ SFML

| SFML | Qt6 |
|------|-----|
| `sf::RenderWindow` | `QMainWindow` |
| `sf::Text` | `QLabel` |
| Custom rendering | `QWidget`, `QTableView` |
| `sf::Event` | `signals/slots` |
| `std::fstream` | `QFile`, `QTextStream` |
| Manual UI | Qt Layouts |

## 📝 TODO

- [ ] Implement Add/Edit Book dialog
- [ ] Implement Add/Edit Reader dialog
- [ ] Implement Top 10 Books report
- [ ] Implement Overdue report
- [ ] Add real icons
- [ ] Add Qt stylesheets
- [ ] Implement .ui files with Qt Designer
- [ ] Add unit tests
- [ ] Add documentation

## 👥 Contributors

- Original SFML version: Dự án cũ
- Qt6 migration: Mới

## 📄 License

Educational project - Free to use and modify

---

**Version**: 2.0.0 (Qt6)  
**Last Updated**: December 2025
