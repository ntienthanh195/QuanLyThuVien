# Cấu trúc dự án Qt Library Management

## 📁 Cấu trúc thư mục mới

```
LibraryManagement/
├── src/                      # Source files
│   ├── main.cpp             # Entry point
│   ├── models/              # Business logic (từ data/)
│   │   ├── DauSach.cpp
│   │   ├── DocGia.cpp
│   │   ├── QuanLySach.cpp
│   │   └── ...
│   ├── views/               # Qt UI classes
│   │   ├── MainWindow.cpp
│   │   ├── BookManagementWidget.cpp
│   │   ├── ReaderManagementWidget.cpp
│   │   └── LoanManagementWidget.cpp
│   └── utils/               # Utilities
│       ├── NgayThang.cpp
│       ├── ThongBao.cpp
│       └── FileManager.cpp
│
├── include/                 # Header files
│   ├── Constants.h
│   ├── models/
│   ├── views/
│   └── utils/
│
├── ui/                      # Qt Designer files (để trống - sẽ tạo sau)
├── resources/               # Icons, fonts
│   └── resources.qrc
│
├── data/                    # Thư mục cũ (backup)
├── files/                   # Data files (.txt)
├── CMakeLists.txt           # CMake build
└── LibraryManagement.pro    # Qt project file

```

## 🔧 Build Instructions

### Sử dụng Qt Creator:
1. Mở file `LibraryManagement.pro`
2. Configure project với Qt6
3. Build & Run

### Sử dụng CMake:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## 📝 Tiến độ chuyển đổi

### ✅ Đã hoàn thành:
- [x] Tạo cấu trúc thư mục Qt
- [x] Tạo CMakeLists.txt và .pro file
- [x] Tạo main.cpp với QApplication
- [x] Tạo MainWindow với menu chính
- [x] Tạo BookManagementWidget (quản lý sách)
- [x] Tạo ReaderManagementWidget (quản lý độc giả)
- [x] Tạo LoanManagementWidget (mượn/trả)
- [x] Copy các model files từ data/
- [x] Tạo AboutDialog

### 🔨 Đang làm:
- [ ] Update include paths trong model files
- [ ] Fix compilation errors
- [ ] Test build với Qt

### ⏳ Sẽ làm tiếp:
- [ ] Tạo dialogs cho Add/Edit Book
- [ ] Tạo dialogs cho Add/Edit Reader
- [ ] Implement TopBooks report
- [ ] Implement Overdue report
- [ ] Add icons và resources
- [ ] Polish UI với stylesheets
- [ ] Tạo .ui files với Qt Designer (optional)

## 🎯 Điểm khác biệt chính

| Aspect | SFML (Cũ) | Qt6 (Mới) |
|--------|-----------|-----------|
| Window | sf::RenderWindow | QMainWindow |
| UI | Custom rendering | Qt Widgets |
| Events | SFML events | Signals/Slots |
| Tables | Manual rendering | QTableView + Model |
| Layout | Fixed positions | Qt Layouts |
| File I/O | std::fstream | QFile + QTextStream |
| Fonts | sf::Font | QFont |

## 🚀 Next Steps

1. Fix include paths trong các file .cpp
2. Test compilation
3. Implement các dialog còn thiếu
4. Add icons và resources
5. Polish UI
