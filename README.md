
# 📚 HỆ THỐNG QUẢN LÝ THƯ VIỆN V2.0

## 1. Giới thiệu

Hệ thống Quản lý Thư viện V2.0 là phần mềm quản lý thư viện hiện đại, giao diện đẹp, dễ sử dụng, tích hợp nhiều tính năng nghiệp vụ chuyên sâu, hỗ trợ đóng gói portable và import dữ liệu Excel không cần cài Python.

### Điểm nổi bật
- Giao diện Dark Theme hiện đại, thân thiện.
- Import Excel thông minh, kiểm tra lỗi trước khi nhập.
- In ấn nâng cao: phiếu mượn, thẻ độc giả, mã vạch Code128, mã QR.
- Đóng gói portable, chạy trực tiếp không cần cài đặt.
- Tích hợp Python portable cho xử lý dữ liệu.

## 2. Cấu trúc dự án

```
QuanLyThuVien/
├── bao_gom/            # Header UI & logic V2
├── nguon/              # Source code UI & logic V2
├── include/            # Header backend (core cũ)
├── src/                # Source backend (core cũ)
├── resources/          # Font, icon, QSS, resource
├── tools/              # Script phụ trợ, import, xử lý dữ liệu
├── python_runtime/     # Python portable (tự sinh khi chạy script)
├── setup_runtime.ps1   # Script cài Python portable + openpyxl
├── QuanLyThuVien_V2.pro # File cấu hình Qt project
├── deploy.ps1          # Script đóng gói tự động
├── HUONG_DAN_DONG_GOI.md # Hướng dẫn đóng gói chi tiết
└── ...
```

## 3. Hướng dẫn cài đặt, build và chạy

### Yêu cầu hệ thống
- Windows 10/11 (64-bit)
- Qt 6.10.1 (MinGW 64-bit)
- RAM tối thiểu 4GB (khuyến nghị 8GB)

### Các bước build & chạy
1. **Cài đặt Qt và MinGW** (nếu chưa có).
2. **Mở dự án**: Mở file `QuanLyThuVien_V2.pro` bằng Qt Creator.
3. **Cài Python portable (lần đầu)**:
   - Chuột phải vào `setup_runtime.ps1` → Run with PowerShell.
   - Script sẽ tự động tải Python portable và cài openpyxl.
4. **Build & chạy**:
   - Nhấn `Ctrl+R` trong Qt Creator để chạy.
   - Hoặc dùng task trong VS Code: `Build & Compile Qt (Full Build)`.

## 4. Hướng dẫn đóng gói & phân phối

### Đóng gói tự động (Khuyên dùng)
1. Build release:
   ```powershell
   C:/Qt/6.10.1/mingw_64/bin/qmake.exe QuanLyThuVien_V2.pro -spec win32-g++ "CONFIG+=release"
   C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe release
   ```
2. Chạy script đóng gói:
   ```powershell
   ./deploy.ps1
   ```
3. Kết quả: Thư mục `QuanLyThuVien_Portable` chứa toàn bộ app, có thể copy đi máy khác, không cần cài đặt.

### Đóng gói thủ công (tùy chọn)
Xem chi tiết trong file `HUONG_DAN_DONG_GOI.md`.

### Phân phối cho người dùng cuối
- Copy toàn bộ thư mục `QuanLyThuVien_Portable` hoặc nén thành file ZIP.
- Người dùng chỉ cần giải nén, chạy `LibraryManagement.exe`.
- Không cần cài Qt, Python, hay bất kỳ phần mềm nào khác.

## 5. Tính năng chính (hiện có) và đề xuất cải tiến

### 5.1 Quản lý sách & nhập liệu (hiện có)
- Quản lý đầu sách và bản sao: thêm / sửa / xóa, cập nhật số lượng, trạng thái mượn.
- Tìm kiếm, lọc nâng cao theo tựa, tác giả, ISBN, chuyên mục, năm xuất bản.
- Import Excel (.xlsx) với giao diện Preview:
   - Mapping cột linh hoạt, xem trước dữ liệu trước khi ghi.
   - Kiểm tra hợp lệ (ISBN, ngày, kiểu dữ liệu), báo lỗi chi tiết cho từng dòng.
   - Hỗ trợ rollback (khôi phục) nếu quá trình import gặp lỗi.

### 5.2 In ấn & mã vạch (hiện có)
- In tem sách chuẩn Code128, tối ưu cho máy in nhiệt và in độ phân giải cao.
- In thẻ độc giả kèm QR/Barcode, mẫu thẻ có thể tùy chỉnh.
- Print Preview, hỗ trợ khổ giấy K80/A6 và cấu hình máy in.
- Hỗ trợ in hàng loạt (batch printing) cho tem và phiếu.

### 5.3 Quản lý độc giả (hiện có)
- Lưu trữ thông tin độc giả, phân loại, trạng thái thẻ (hoạt động/khóa).
- Tìm kiếm nhanh theo mã thẻ, tên, số điện thoại; xem lịch sử mượn trả.

### 5.4 Mượn — Trả (hiện có)
- Mượn/trả 1 chạm (scan/nhập mã), kiểm tra tự động điều kiện mượn (hạn mức, nợ, quá hạn).
- Tính tiền phạt tự động khi trả quá hạn và ghi log giao dịch.

---

## 5.x Đề xuất cải tiến (tính năng nên thêm / ưu tiên)
Những mục dưới đây là gợi ý để nâng cấp sản phẩm, giúp dễ dùng hơn và phù hợp với môi trường triển khai thực tế.

1) Sao lưu & phục hồi (ưu tiên cao)
   - Tự động sao lưu định kỳ (daily/weekly) và chức năng phục hồi.
   - Hỗ trợ xuất/import DB dạng SQLite/CSV để di chuyển dữ liệu.

2) Quản lý người dùng & phân quyền (ưu tiên cao)
   - Vai trò: `Admin`, `Thủ thư`, `Người dùng`.
   - Audit log (ghi lại các hành động quan trọng: xóa, thay đổi thông tin, import).

3) Cải tiến quá trình Import (ưu tiên trung bình)
   - Mapping thông minh (gợi ý cột), preview clean (chuẩn hóa tên, loại bỏ khoảng trắng, chuẩn hóa dấu).
   - Kiểm tra ràng buộc toàn bộ batch trước khi ghi.

4) Báo cáo & phân tích (ưu tiên trung bình)
   - Biểu đồ tương tác (Top 10 sách, biểu đồ mượn theo thời gian).
   - Export báo cáo sang CSV/PDF; lịch xuất báo cáo định kỳ.

5) API / Remote access (ưu tiên thấp)
   - REST API read-only cho báo cáo và tra cứu (bảo mật bằng token).
   - Sử dụng để tích hợp với website hoặc hệ thống bên ngoài.

6) Tùy chỉnh in nâng cao
   - Profile in cho từng máy để chỉnh offset/scale.
   - Template in (JSON) để người dùng tùy chỉnh layout phiếu và thẻ.

7) Trải nghiệm người dùng (UX)
   - Auto-complete thông minh, gợi ý hành động, phím tắt cho thao tác thường dùng.

### Ví dụ kịch bản (use cases)
- Thủ thư import 500 cuốn: mở Preview -> Clean -> Import batch, kiểm tra lỗi dòng.
- In tem 200 bản sao: tạo batch in, chọn profile máy in, xuất tem.
- Lấy báo cáo sách quá hạn: chạy báo cáo, export CSV, gửi email cho quản lý.

---


## 6. Công nghệ sử dụng
- **Frontend:** Qt Widgets (C++), QSS
- **Backend:** C++ (STL, AVL Tree), Python (import Excel)
- **Database:** File nhị phân/text custom
- **Barcode:** Engine sinh mã Code128, QR

## 7. Xử lý lỗi thường gặp
- Thiếu DLL Qt: Chạy lại script đóng gói hoặc windeployqt
- Thiếu DLL MinGW: Copy thủ công từ thư mục MinGW/bin
- Không có dữ liệu: Copy thư mục `data/` vào cùng thư mục app

## 8. Liên hệ & hỗ trợ
Nếu gặp lỗi build, đóng gói, hoặc sử dụng, hãy kiểm tra lại các bước trên hoặc liên hệ nhóm phát triển.

---
**Phát triển bởi Team QuanLyThuVien | 2025**
