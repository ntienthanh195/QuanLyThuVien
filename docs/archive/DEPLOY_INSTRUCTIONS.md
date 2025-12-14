# HƯỚNG DẪN ĐÓNG GÓI (DEPLOYMENT)

## 1. Đối với Lập Trình Viên (Dev)
- Chạy script `setup_runtime.ps1` một lần duy nhất.
- Code sẽ tự động tìm Python ở thư mục gốc dự án. Bác cứ code thoải mái.

## 2. Đối với Người Dùng Cuối (Khách Hàng)
Khi bác gửi phần mềm cho khách, cấu trúc thư mục phải như sau:
```
ThuMuc_PhanMem_Khach/
├── QuanLyThuVien.exe      (File chạy chính)
├── python_runtime/        <-- COPY NGUYÊN FOLDER NÀY TỪ MÁY BÁC SANG
│   ├── python.exe
│   └── ...
├── tools/                 (Script python import)
└── ...
```
**Khách hàng không cần cài đặt gì cả**, chỉ cần click vào `QuanLyThuVien.exe` là chạy!
