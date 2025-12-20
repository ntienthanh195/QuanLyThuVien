# 📚 Hệ Thống Quản Lý Thư Viện (V2.2)

**Phiên bản:** 2.2 
**Cập nhật cuối:** 20/12/2025  
**Trạng thái:** Hoàn thiện - Sẵn sàng triển khai (Zero-Configuration)

---

## 🚀 Tính Năng Nổi Bật: "Cắm là Chạy" (Zero-Configuration)

## 🚀 Giải Pháp "Cắm là Chạy" (Zero-Configuration Portability)

Dự án được thiết kế với triết lý **"Phần mềm phục vụ con người"**, đặc biệt là những người dùng không am hiểu sâu về kỹ thuật. Chúng tôi đã triển khai một hệ sinh thái tự động hóa toàn diện:

### 1. Hệ Thống Tự Khởi Tạo & Di Trú (Auto-Migration & Seeding)
- **Tự động hóa sơ đồ (Schema):** Khi khởi chạy lần đầu trên máy mới, ứng dụng tự quét và tạo toàn bộ cấu trúc bảng SQLite. Nếu database cũ thiếu cột (do cập nhật phiên bản), hệ thống tự động `ALTER TABLE` để bổ sung mà không làm mất dữ liệu.
- **Dữ liệu mồi (Data Seeding):** Tự động thiết lập tài khoản Admin mặc định, phân quyền Thủ thư và các cấu hình hệ thống (giới hạn mượn, tiền phạt) ngay khi database được tạo mới.
- **Chuẩn hóa dữ liệu cũ:** Cơ chế thông minh tự động tìm kiếm và "sửa lỗi" các mã sách có hậu tố không chuẩn (như `-001` thành `-1`), đảm bảo tính nhất quán của dữ liệu trên mọi môi trường.

### 2. Môi Trường Python Tự Phục Hồi (Self-Healing Runtime)
- **Vượt rào quản lý hệ thống:** Đối với các máy tính bị khóa quyền cài đặt thư viện (lỗi *externally-managed-environment* thường gặp trên MSYS2/MinGW), ứng dụng sẽ tự động cô lập một môi trường ảo riêng biệt (`.venv`) ngay trong thư mục dự án.
- **Quản lý Dependencies thông minh:** Ứng dụng tự động kiểm tra và cài đặt thư viện `openpyxl`. Nếu môi trường hệ thống không khả dụng, nó sẽ tự "chữa lành" bằng cách xây dựng lại runtime cục bộ, hỗ trợ cả cấu trúc thư mục Windows (`Scripts`) và Unix-style (`bin`).

### 3. Trải Nghiệm Đa Ngôn Ngữ Nhất Quán
- **Chuyển vùng dữ liệu:** Không chỉ là dịch văn bản, hệ thống hỗ trợ font chữ đặc thù (Khmer OS Siemreap) được nhúng sẵn vào gói cài đặt, đảm bảo hiển thị hoàn hảo ngay cả trên máy tính chưa cài font.
- **Real-time Sync:** Mọi thay đổi về ngôn ngữ được áp dụng tức thì trên toàn bộ giao diện mà không cần khởi động lại app.

---

## 🛠 Tính Năng Chuyên Sâu (Functional Deep-Dive)

### 📖 Quản Lý Sách & Kho Số Hóa
- **Quản lý phân cấp:** Phân tách rõ ràng giữa "Đầu sách" (Metadata) và "Bản sao" (Physical copies).
- **ISBN & Metadata:** Tự động chuẩn hóa mã ISBN, hỗ trợ lưu trữ thông tin nhà xuất bản, tác giả, năm xuất bản và giá tiền.
- **Số hóa quy trình nhập liệu:** Nhập hàng nghìn bản ghi từ Excel chỉ trong vài giây với hệ thống kiểm lỗi logic (validation) cực kỳ chặt chẽ.
- **In ấn mã vạch:** Tích hợp bộ tạo mã vạch Code128, cho phép in tem dán cho từng cuốn sách trực tiếp từ phần mềm.

### 🎓 Hệ Thống Học Viên & Quản Lý Mượn Trả
- **Thẻ thư viện thông minh:** Tự động tạo mã QR định danh cho từng học viên, hỗ trợ in thẻ nhựa hoặc thẻ giấy chuyên nghiệp.
- **Quy trình mượn trả "Một chạm":** Sử dụng máy quét mã vạch để tối giản hóa thao tác, giảm thiểu sai sót con người.
- **Quản lý nghiệp vụ chặt chẽ:** Tự động tính toán hạn trả, tiền phạt quá hạn dựa trên cấu hình linh hoạt (số sách tối đa, số ngày mượn, đơn giá phạt).
- **Lịch sử toàn diện:** Truy xuất chi tiết lịch sử mượn trả của từng học viên và từng đầu sách.

### 📊 Phân Tích Dữ Liệu & Quản Trị Hệ Thống
- **Dashboard thời gian thực:** Cung cấp biểu đồ thống kê trực quan về tình hình mượn trả, top sách được yêu thích và tình trạng kho.
- **Audit Logging (Nhật ký hệ thống):** Ghi lại mọi thao tác quan trọng (thêm/sửa/xóa/đăng nhập) để đảm bảo tính minh bạch và bảo mật.
- **Quản lý phân quyền:** Hệ thống phân quyền chi tiết giữa Quản trị viên (Toàn quyền) và Thủ thư (Chỉ quản lý nghiệp vụ).
- **Cấu hình động:** Cho phép thay đổi các quy định về mượn trả và giao diện mà không cần can thiệp vào mã nguồn.
- **Backup dữ liệu tự động định kỳ.**

---

## 💻 Yêu Cầu Hệ Thống & Build

- **OS:** Windows 10/11 (64-bit).
- **Runtime:** Ứng dụng đi kèm mọi thư viện cần thiết (DLLs).
- **Python (Tùy chọn):** Nếu cần Import Excel, chỉ cần có Python cài sẵn, ứng dụng sẽ tự lo phần còn lại.

### Hướng Dẫn Build (Dành cho Lập trình viên)
```powershell
# 1. Build Release
qmake.exe QuanLyThuVien_V2.pro -spec win32-g++ "CONFIG+=release"
mingw32-make.exe release

# 2. Đóng gói Portable
./deploy.ps1
```
*Kết quả nằm trong thư mục `QuanLyThuVien_Portable/`.*

---

## 📁 Cấu Trúc Dự Án
- `src/`: Mã nguồn C++ hiện đại (MVC Pattern).
- `include/`: Các file header định nghĩa hệ thống.
- `tools/`: Các script Python hỗ trợ (Quan trọng cho tính năng Excel).
- `resources/`: Icons, Fonts (Khmer OS Siemreap) và QSS Styles.
- `data/`: Cơ sở dữ liệu SQLite và file cấu hình.

---

## 📋 Thông Tin Bản Quyền
- **Dự án:** Library Management System
- **Phát triển cho:** Học viện Phật giáo Nam Tông Khmer
- **Tác giả:** Chăm Rốch Thi & Team
- **Giấy phép:** Nội bộ (Proprietary)
