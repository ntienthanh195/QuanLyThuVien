# Hệ Thống Quản Lý Thư Viện
**Phiên bản:** 2.1 (Release Candidate 2)  
**Ngày cập nhật:** 14/12/2025

Phần mềm quản lý thư viện chuyên nghiệp dành cho trường học và học viện, hỗ trợ đa ngôn ngữ (Tiếng Việt, Tiếng Khmer, English).

---

## Tính Năng Chính

### Quản Lý
- **Sách:** Quản lý đầu sách và bản sao, tìm kiếm nâng cao, import từ Excel/CSV
- **Học Viên:** Quản lý thông tin, trạng thái thẻ, lịch sử mượn trả
- **Nhân Sự:** Phân quyền Admin/Thủ thư với audit logging
- **Mượn/Trả:** Quy trình 1 chạm với barcode scanner, tự động tính phạt quá hạn

### In Ấn
- In thẻ thư viện với mã QR
- In mã vạch Code128 cho sách
- In phiếu mượn/trả tự động
- Hỗ trợ máy in nhiệt K80 và A4

### Báo Cáo & Thống Kê
- Dashboard thời gian thực
- Top 10 sách/học viên
- Báo cáo quá hạn với tính năng filter
- Export Excel/PDF

### Hệ Thống
- **Đa ngôn ngữ:** Chuyển đổi tức thì giữa Việt-Khmer-English
- **Sao lưu:** Backup tự động và thủ công
- **Database:** SQLite với EventBus realtime sync

---

## Yêu Cầu Hệ Thống

- **OS:** Windows 10/11 (64-bit)
- **Qt:** 6.10.1 với MinGW 64-bit
- **RAM:** Tối thiểu 2GB (khuyến nghị 4GB)
- **Màn hình:** Độ phân giải tối thiểu 1366x768

---

## Hướng Dẫn Build

### 1. Chuẩn Bị Môi Trường

```powershell
# Clone repository
git clone <repository-url>
cd QuanLyThuVien
```

### 2. Build Release

```powershell
# Chạy qmake
C:/Qt/6.10.1/mingw_64/bin/qmake.exe QuanLyThuVien_V2.pro -spec win32-g++ "CONFIG+=release"

# Compile
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe release
```

### 3. Đóng Gói (Deploy)

```powershell
# Tự động đóng gói với script
./deploy.ps1

# Kết quả: QuanLyThuVien_Portable/
```

**Chi tiết:** Xem `HUONG_DAN_DONG_GOI.md`

---

## Cấu Trúc Dự Án

```
QuanLyThuVien/
├── nguon/                  # UI Code (Vietnamese naming - legacy)
│   ├── giao_dien/         # Main widgets
│   └── main_v2.cpp        # Entry point
├── src/                    # Modern code (English naming)
│   ├── models/            # Business logic
│   ├── utils/             # Utilities (LanguageManager, PrintHelper...)
│   └── views/             # Dialogs and widgets
├── include/               # Header files
├── resources/             # Icons, fonts, QSS themes
├── data/                  # Database and sample data
├── tools/                 # Helper scripts
└── docs/archive/          # Legacy documentation

Artifacts (Reports):
└── .gemini/antigravity/brain/.../
    ├── BAO_CAO_UX_TONG_HOP.md        # Comprehensive UX audit
    ├── BAO_CAO_TONG_KET_HIEU_CHINH.md # Implementation summary
    └── HUONG_DAN_SU_DUNG_NHANH.md     # Quick user guide
```

---

## Công Nghệ

- **Framework:** Qt 6.10.1 (C++ Widgets)
- **Database:** SQLite + Custom binary files
- **Architecture:** MVC with EventBus pattern
- **Barcode:** Code128 generator (custom)
- **PDF:** QPrinter with custom templates
- **Build:** qmake + MinG
---

## Khắc Phục Sự Cố

### Lỗi: "Qt6Core.dll is missing"
```powershell
cd QuanLyThuVien_Portable
C:/Qt/6.10.1/mingw_64/bin/windeployqt.exe QuanLyThuVien.exe --release
```

### Lỗi: Thiếu MinGW DLLs
```powershell
copy C:\Qt\Tools\mingw1310_64\bin\libgcc_s_seh-1.dll QuanLyThuVien_Portable\
copy C:\Qt\Tools\mingw1310_64\bin\libstdc++-6.dll QuanLyThuVien_Portable\
copy C:\Qt\Tools\mingw1310_64\bin\libwinpthread-1.dll QuanLyThuVien_Portable\
```

### Lỗi: Font chữ Khmer bị ô vuông
- Cài đặt font **Khmer OS Siemreap** (có sẵn trong `resources/fonts/`)
- Copy `Siemreap-Regular.ttf` vào `C:\Windows\Fonts\`

---

## Roadmap (Phase 3)

### Đang Thực Hiện
- [ ] Cleanup emoji trong UI (70+ vị trí)
- [ ] Chuẩn hóa icon set (SVG only)
- [ ] Fix accessibility (contrast ratio, keyboard navigation)
- [ ] Thay thế QMessageBox bằng Toast notifications

### Kế Hoạch
- [ ] Bulk operations (xóa nhiều, export nhiều)
- [ ] Dashboard animations
- [ ] Module kiểm kê kho
- [ ] Web portal cho học viên

**Chi tiết:** Xem `BAO_CAO_UX_TONG_HOP.md`

---

## Đóng Góp

Dự án hiện đang trong giai đoạn **Release Candidate**. Nếu phát hiện lỗi hoặc có đề xuất, vui lòng:

1. Kiểm tra `BAO_CAO_UX_TONG_HOP.md` xem vấn đề đã được ghi nhận chưa
2. Tạo issue mô tả chi tiết (steps to reproduce, expected vs actual)
3. Attach screenshots nếu là UI bug

---

## License

Proprietary - For Internal Use Only

**Phát triển cho:** Học viện Phật giáo Nam Tông Khemr 
**Team:** Chăm Rốch Thi  
**Năm:** 2025
