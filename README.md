# Hệ Thống Quản Lý Thư Viện (Phiên bản 2.5)

Phần mềm quản lý thư viện chuyên dụng cho môi trường offline, được tối ưu cho tốc độ xử lý dữ liệu lớn và hỗ trợ đầy đủ ngôn ngữ Khmer. Dự án được phát triển trên nền tảng C++ và Qt Framework, đảm bảo tính ổn định lâu dài mà không phụ thuộc vào internet hay cấu hình máy tính cao.

---

## Các tính năng thực tế

### 1. Nghiệp vụ Mượn - Trả (Circulation)
*   **Hỗ trợ máy quét:** Tự động nhận diện mã vạch để làm thủ tục mượn/trả nhanh. Hệ thống tự phân biệt mã sách và mã thẻ độc giả.
*   **Ràng buộc dữ liệu:** Tự động khóa quyền mượn nếu độc giả đang có sách quá hạn hoặc chưa hoàn thành phí phạt.
*   **Xử lý sự cố:** Có quy trình riêng cho các trường hợp báo mất sách, đền bù tiền hoặc tìm lại được sách sau khi đã đền.
*   **Âm thanh thông báo:** Tích hợp tiếng bíp khi quét thành công hoặc cảnh báo lỗi, giúp thủ thư thao tác liên tục không cần nhìn màn hình.

### 2. Kiểm kê kho (Inventory)
*   **Chế độ quét nhanh:** Thủ thư chỉ cần cầm máy quét đi dọc các kệ sách, hệ thống sẽ tự đối soát với cơ sở dữ liệu.
*   **Báo cáo chênh lệch:** Liệt kê chính xác danh sách sách bị mất, sách nằm sai vị trí kệ hoặc sách đang báo mượn nhưng thực tế vẫn nằm trên kệ.

### 3. In ấn & Mã vạch
*   **Thẻ độc giả:** Tự động tạo và in thẻ có mã QR định danh.
*   **Tem nhãn sách:** In tem dán gáy sách (spine labels) với đầy đủ thông tin mã vạch, vị trí kệ và tên sách.
*   **Báo cáo PDF:** Xuất các mẫu báo cáo thống kê, danh sách quá hạn, biên bản kiểm kê ra định dạng PDF để lưu trữ hoặc ký duyệt.

### 4. Quản lý dữ liệu
*   **Xử lý Excel:** Nhập (Import) hàng nghìn đầu sách từ file Excel vào hệ thống chỉ mất vài giây.
*   **Đa ngôn ngữ:** Hỗ trợ tốt tiếng Khmer (font Siemreap), phù hợp cho các học viện và chùa Nam tông.
*   **An toàn dữ liệu:** Cơ chế tự động sao lưu (Auto Backup) trước khi thực hiện các thay đổi lớn và nhật ký thao tác (Audit Log) để kiểm tra lại lịch sử chỉnh sửa.

---

## Thông số kỹ thuật

*   **Ngôn ngữ:** C++17.
*   **Framework:** Qt 6.10 (Widgets).
*   **Cơ sở dữ liệu:** SQLite (chế độ WAL giúp tăng tốc độ đọc/ghi đồng thời).
*   **Cấu trúc dữ liệu:** Sử dụng cây AVL để quản lý danh sách độc giả, đảm bảo tìm kiếm cực nhanh ngay cả khi dữ liệu lên đến hàng chục nghìn bản ghi.

---

## Hướng dẫn biên dịch (Build)

### Yêu cầu
*   Windows 10/11.
*   Bộ cài Qt 6.x và trình biên dịch MinGW.

### Các bước thực hiện
1. Mở thư mục dự án bằng Qt Creator hoặc Command Prompt.
2. Chạy lệnh cấu hình:
   ```bash
   qmake QuanLyThuVien_V2.pro "CONFIG+=release"
   ```
3. Biên dịch:
   ```bash
   mingw32-make
   ```
4. Đóng gói (thu thập DLL):
   Chạy file `deploy.ps1` bằng PowerShell để tạo thư mục chạy độc lập (Portable).

---

## Thông tin phát triển

Dự án được thực hiện bởi **Chăm Rốch Thi**. 
Mọi thông tin góp ý hoặc hỗ trợ kỹ thuật, vui lòng liên hệ qua email hoặc trực tiếp tại Cần Thơ.

*Bản quyền © 2025. Giữ toàn quyền.*
