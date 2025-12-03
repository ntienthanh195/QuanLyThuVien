

# Hệ thống Quản lý Thư viện 📚

Một dự án C++ đồ họa (GUI) để quản lý thư viện, được xây dựng bằng C++17, các cấu trúc dữ liệu cơ bản (Mảng con trỏ, DSLK), và thư viện đồ họa **SFML**.

## 1\. Đề bài: Tổ chức các danh sách

Đây là mục tiêu của đồ án:

  * **Đầu sách:** Mảng con trỏ (danh sách tuyến tính) - Luôn sắp xếp tăng dần theo tên sách.
  * **Danh mục sách:** Danh sách liên kết đơn (theo `MaSach`).
  * **Thẻ độc giả:** Cây nhị phân tìm kiếm (theo `MATHE`).
  * **Mượn trả:** Danh sách liên kết đơn (theo `MaSach`).

## 2\. Tiến Độ Chức Năng Hiện Tại

Dự án đã được tách thành backend (logic) và frontend (UI SFML).

### ✅ Đã Hoàn Thành (Backend & UI)

  * **c. Nhập đầu sách và đánh mã sách tự động:**
      * Giao diện "Thêm Đầu Sách" cho phép nhập thông tin.
      * Tự động sinh `MaSach` (ví dụ: `ISBN-1`, `ISBN-2`) khi thêm bản sao.
  * **d. In danh sách đầu sách theo thể loại:**
      * Tích hợp vào màn hình Quản lý Sách với chế độ **"Xem theo Thể Loại"**.
      * Danh sách được gom nhóm theo thể loại (sắp xếp A-Z) và hỗ trợ **cuộn chuột** (`sf::View` hoặc `sf::RenderTexture`).
  * **e. Tìm thông tin sách dựa vào tên sách:**
      * Ô "Tìm Kiếm" cho phép lọc sách theo Tên, Tác giả, ISBN, và Thể loại.
      * Nút **"Chi Tiết"** mở cửa sổ modal hiển thị đầy đủ "các mã sách" và "trạng thái" của chúng.
  * **(Bổ sung) Quản lý Bản Sao:**
      * Hỗ trợ **"Thêm Bản Sao"** cho một đầu sách đã có (có modal riêng).
      * Hỗ trợ **"Thanh Lý"** các bản sao (trạng thái 0 -\> 2) với hộp thoại xác nhận.

### ❌ Chưa Triển Khai

  * **a-b. Quản lý độc giả (Thêm, xóa, sửa, in danh sách).**
  * **f-g. Xử lý mượn/trả sách.**
  * **h. Liệt kê sách độc giả đang mượn.**
  * **i. In danh sách độc giả quá hạn.**
  * **j. Top 10 sách mượn nhiều nhất.**

## 3\. Cấu Trúc Thư Mục (Đã Cập Nhật cho SFML)

Cấu trúc thư mục dự án được tổ chức theo mô hình tách biệt Giao diện (UI) và Logic (Backend):

```
📦 LibraryManagement/
├── 📂 data/            # Ma nguon .cpp cho logic backend (QuanLySach.cpp, ...)
├── 📂 files/           # Chua cac file du lieu .txt
├── 📂 include/         # Chua TAT CA cac file header .h
├── 📂 utils/           # Ma nguon .cpp cho cac ham tien ich (XuLyChuoi.cpp, ...)
├── 📂 ui/              # Ma nguon .cpp cho Giao dien SFML (ManHinh...)
├── 📂 lib/             # Thu muc chua thu vien SFML
├── 📂 bin/             # Thu muc chua file .exe da bien dich va font
├── 📂 .vscode/         # Cau hinh cho VS Code (tasks.json)
├── 📝 main.cpp         # Diem vao chuong trinh (Khoi tao SFML)
├── .gitignore
└── 📖 README.md
```

## 4\. Yêu Cầu & Biên Dịch (SFML)

Phiên bản này yêu cầu thư viện đồ họa **SFML**.

  * **Trình biên dịch:** MinGW-w64 g++ (Hỗ trợ C++17)
  * **Thư viện:** SFML (ví dụ: 2.5.1) cho MinGW
  * **Hệ điều hành:** Windows

### 1\. Cài đặt SFML

  * Tải SFML cho MinGW (32-bit hoặc 64-bit tùy trình biên dịch của bạn).
  * Giải nén và đặt các thư mục `include`, `lib` của SFML vào trong thư mục `/lib/SFML` của dự án (hoặc một vị trí khác và cập nhật đường dẫn biên dịch).

### 2\. Biên dịch (Lệnh khuyên dùng)

Mở terminal tại thư mục gốc `LibraryManagement` và chạy lệnh sau:

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude -Ilib/SFML/include -g main.cpp ui/*.cpp data/*.cpp utils/*.cpp -Llib/SFML/lib -lsfml-graphics -lsfml-window -lsfml-system -o bin/main.exe
```

**Giải thích lệnh:**

  * `g++ ... -g`: Gọi trình biên dịch với cờ debug.
  * `main.cpp ui/*.cpp data/*.cpp utils/*.cpp`: Gom tất cả các file mã nguồn `.cpp` cần thiết (UI, data, utils).
  * `-Iinclude -Ilib/SFML/include`: Chỉ dẫn trình biên dịch tìm file header (`.h`) trong thư mục `include/` và `lib/SFML/include/`.
  * `-Llib/SFML/lib`: Chỉ dẫn linker tìm file thư viện (`.a`) trong thư mục `lib/SFML/lib/`.
  * `-lsfml-graphics -lsfml-window -lsfml-system`: **(Quan trọng)** Liên kết với các module SFML cần thiết.
  * `-o bin/main.exe`: Tạo file thực thi tên là `main.exe` trong thư mục `bin/`.

### 3\. Chạy chương trình

Sau khi biên dịch thành công:

```bash
.\bin\main.exe
```

*Lưu ý: Cần sao chép file font (ví dụ: `DejaVuSans.ttf`) vào thư mục `bin/` để chương trình có thể tải font và chạy.*

## 5\. Hướng Dẫn Sử Dụng Giao Diện

### Màn Hình Chính (Menu)

  * **1. QUAN LY DAU SACH:** Đi đến màn hình quản lý sách.
  * **(2, 3):** (Chưa triển khai) Sẽ dùng cho Độc Giả và Mượn Trả.
  * **4. THONG TIN:** Xem thông tin đồ án.
  * **5. THOAT:** Thoát chương trình (sẽ tự động lưu nếu có thay đổi).

### Màn Hình Quản Lý Sách

Đây là màn hình chính với 2 khu vực:

#### Khu vực bên trái (Bảng)

  * **Tìm kiếm:** Gõ Tên/Tác giả/ISBN/Thể loại vào ô "Tìm Kiếm" và nhấn "TIM".
  * **Xem Tất Cả:** Nhấn "XÓA TIM" để xem tất cả sách (có phân trang).
  * **Xem theo Thể Loại:**
      * Nhấn **"Xem theo The Loai"** để chuyển sang chế độ xem gom nhóm (sắp xếp A-Z).
      * Dùng **bánh xe chuột** để cuộn xem hết danh sách.
      * Nhấn "Xem Tim Kiem/Tat Ca" để quay lại chế độ bảng.
  * **Chọn Sách:** **Double-click** (nhấn đúp chuột) vào một hàng sách trong bảng (chỉ ở chế độ "Tìm Kiếm") để chọn.
  * **Chức năng (sau khi chọn):**
      * `THEM BAN SAO`: Mở modal để thêm bản sao mới cho sách đã chọn.
      * `CHI TIET`: Mở modal xem danh sách các bản sao (đã sắp xếp tăng dần 1, 2, 3,..), cho phép "Thanh Lý".
      * `SUA`: Chuyển form bên phải sang chế độ "HIỆU CHỈNH ĐẦU SÁCH".
      * `XOA`: Yêu cầu xác nhận xóa (chỉ khi sách không còn bản sao nào).
      * `HUY CHON`: Bỏ chọn sách hiện tại.

#### Khu vực bên phải (Form & Thông Báo)

  * **Form Thêm/Sửa:**
      * Có gợi ý chữ mờ cho các ô nhập liệu (ví dụ: "10 hoac 13 chu so", "Toi da 100 ky tu",...).
      * **Undo Nhập Liệu:** Khi đang gõ, nhấn **Ctrl+Z** để hoàn tác. Hướng dẫn được ghi trên form.
  * **Khung Thông Báo:**
      * Hiển thị lỗi (màu đỏ), thành công (màu xanh lá), hoặc các hộp thoại xác nhận ("Xác nhận Xóa", "Xác nhận Thanh Lý").


```
```