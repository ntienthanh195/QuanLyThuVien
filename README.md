# 📚 HỆ THỐNG QUẢN LÝ THƯ VIỆN V2.0

## 🎯 Giới Thiệu

Phiên bản 2.0 của hệ thống Quản lý Thư viện là một bước lột xác toàn diện với giao diện hiện đại và các tính năng nghiệp vụ chuyên sâu.

✨ **Điểm Nổi Bật:**
- **Giao diện Modern Dark Theme**: Thiết kế phẳng, đẹp mắt, thân thiện người dùng.
- **Import Excel Thông Minh**: Xem trước dữ liệu (Preview), kiểm tra lỗi logic trước khi nhập.
- **Hệ thống In Ấn Nâng Cao**: Hỗ trợ in phiếu mượn, thẻ độc giả, và **Mã vạch Code128** sắc nét (High-DPI).
- **Deployment Portable**: "Đóng gói mang đi" dễ dàng, tích hợp sẵn môi trường Python nhẹ.

## 📁 Cấu Trúc Dự Án

```
QuanLyThuVien/
├── bao_gom/                    # Header files (UI & Logic V2)
├── nguon/                      # Source files (Implementations)
├── include/                    # Backend headers (Legacy Core)
├── src/                        # Backend source (Legacy Core)
├── resources/                  # Assets (Fonts, Icons, QSS)
│
├── tools/                      # Scripts phụ trợ
│   └── import_from_excel.py    # Logic đọc Excel bằng Python
├── python_runtime/             # Môi trường Python Portable (Tự sinh)
├── setup_runtime.ps1           # Script cài đặt môi trường
├── QuanLyThuVien_V2.pro        # Project configuration
└── DEPLOY_INSTRUCTIONS.md      # Hướng dẫn đóng gói chi tiết
```

## 🚀 Hướng Dẫn Build & Run

### Yêu Cầu Hệ Thống
- **Qt 6.10.1** (MinGW 64-bit)
- **Windows 10/11**
- **RAM**: 4GB+ (Khuyên dùng 8GB)

### Các Bước Build
1. **Mở dự án**: Open `QuanLyThuVien_V2.pro` bằng Qt Creator.
2. **Setup Môi trường (Lần đầu)**:
   - Chạy script `setup_runtime.ps1` (Chuột phải -> Run with PowerShell) tại thư mục gốc.
   - Script sẽ tự động tải Python Portable và thư viện `openpyxl`.
3. **Build & Run**: Nhấn `Ctrl+R` để chạy.

## ✨ Tính Năng Chi Tiết

### 1. 📖 Quản Lý Sách & Nhập Liệu
- **CRUD Sách**: Thêm, xoá, sửa thông tin sách và quản lý bản sao.
- **Import Excel**:
  - Hỗ trợ file `.xlsx`.
  - **Màn hình Preview**: Kiểm tra dữ liệu dạng bảng trước khi lưu vào DB.
  - Tự động phát hiện lỗi sai định dạng.

### 2. �️ In Ấn & Mã Vạch
- **In Tem Sách**: Tạo mã vạch **Code128** chuẩn công nghiệp, tự động điều chỉnh độ phân giải cho máy in nhiệt.
- **In Thẻ Độc Giả**: Mẫu thẻ hiện đại kèm mã QR.
- **In Phiếu & Báo Cáo**:
  - Phiếu mượn/trả (K80/A6).
  - Báo cáo thống kê Top 10, Sách quá hạn.
- **Xem Trước (Print Preview)**: Tích hợp ngay trong phần mềm.

### 3. 👥 Quản Lý Độc Giả
- Sử dụng cấu trúc cây **AVL Tree** giúp tìm kiếm cực nhanh theo Mã thẻ hoặc Tên.
- Quản lý trạng thái thẻ (Hoạt động/Khóa).
- Tra cứu lịch sử mượn trả chi tiết.

### 4. 🔄 Mượn Trả Tác Vụ
- Quy trình mượn sách 1 chạm.
- Tính toán tiền phạt tự động khi trả quá hạn hoặc mất sách.
- Kiểm tra điều kiện mượn chặt chẽ (Số lượng, quá hạn...).

## 📦 Hướng Dẫn Đóng Gói (Deploy)

Hệ thống hỗ trợ tính năng **Portable Import** (Import Excel không cần cài đặt Python).

1. **Với Lập trình viên**:
   - Chạy script `setup_runtime.ps1` một lần duy nhất tại thư mục gốc.
   
2. **Với Người dùng cuối**:
   - Khi bàn giao, hãy copy folder `python_runtime` đi kèm với file `.exe`.
   - Xem chi tiết tại: [DEPLOY_INSTRUCTIONS.md](DEPLOY_INSTRUCTIONS.md)

## �️ Công Nghệ Sử Dụng
- **Frontend**: Qt Widgets (C++), QSS Styling.
- **Backend**: C++ (STL, AVL Tree), Python (Data Processing).
- **Database**: File-based storage (Custom binary/text format).
- **Barcode**: Custom Code128 Rendering Engine.

---

**Developed by Team QuanLyThuVien | 2025**
