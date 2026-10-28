# Hệ Thống Quản Lý Thư Viện (Library Management System)

Ứng dụng desktop hoạt động theo mô hình offline-first, được thiết kế chuyên dụng cho công tác quản lý thư viện tại các cơ sở giáo dục và học viện, tối ưu hóa cho môi trường phần cứng có tài nguyên giới hạn. Phát triển trên nền tảng C++17 và Qt 6.10 (Qt Widgets), hệ thống kết hợp các cấu trúc dữ liệu tùy chỉnh trong bộ nhớ (in-memory) cùng cơ sở dữ liệu quan hệ được tối ưu nhằm đạt hiệu năng truy vấn dưới mili-giây đối với tập dữ liệu lớn.

![Bảng điều khiển hệ thống](Ảnh%20chụp%20màn%20hình%202026-06-30%20114701.png)

---

## Thông số Kỹ thuật

| Thành phần | Công nghệ lựa chọn | Mô tả chi tiết |
|---|---|---|
| **Ngôn ngữ lập trình** | C++17 | Sử dụng các tiêu chuẩn C++ hiện đại, con trỏ thông minh (smart pointers) để quản lý bộ nhớ tự động và các cấu trúc dữ liệu STL. |
| **Giao diện người dùng** | Qt 6.10 (Qt Widgets) | Các thành phần giao diện native nhẹ, được định hình phong cách qua công cụ QSS tùy chỉnh. |
| **Cơ sở dữ liệu** | SQLite 3 (Chế độ WAL) | Kích hoạt Write-Ahead Logging để hỗ trợ các thao tác đọc/ghi đồng thời mà không gây khóa tài nguyên cơ sở dữ liệu. |
| **Cấu trúc dữ liệu Độc giả** | Cây AVL (AVL Tree) | Cây tìm kiếm nhị phân tự cân bằng, đảm bảo độ phức tạp thời gian cho các thao tác tìm kiếm, thêm và xóa đạt mức tối ưu $O(\log N)$. |
| **Tổ chức Đầu sách & Bản sao** | Lai ghép Vector + Linked List | Lưu trữ đầu sách trong Vector để áp dụng tìm kiếm nhị phân $O(\log N)$, kết hợp danh sách liên kết đơn cho các bản sao nhằm tiết kiệm tài nguyên. |

---

## Kiến trúc Hệ thống Cốt lõi

### 1. Cấu trúc Dữ liệu & Tối ưu hóa Bộ nhớ
*   **Cây AVL quản lý Độc giả:** Để tránh hiện tượng thắt nút cổ chai bộ nhớ khi số lượng độc giả đạt hàng chục nghìn, danh sách độc giả được duy trì dưới dạng cây nhị phân cân bằng. Các phép xoay cây (xoay trái/xoay phải) được kích hoạt động trong quá trình chèn hoặc xóa nút để bảo toàn chiều cao tối ưu của cây.

    ![Quản lý danh sách học viên qua cấu trúc cây AVL](Ảnh%20chụp%20màn%20hình%202026-06-30%20114710.png)

*   **Lưu trữ Lai ghép cho Kho sách:** Các đầu sách được sắp xếp tuần tự theo mã ISBN trong Vector để thực hiện tìm kiếm nhị phân nhanh. Mỗi đầu sách liên kết trực tiếp với một danh sách liên kết đơn chứa các bản sao vật lý hiện có, giúp giảm thiểu đáng kể dung lượng bộ nhớ runtime.

    ![Quản lý đầu sách và bản sao với tìm kiếm nhị phân](Ảnh%20chụp%20màn%20hình%202026-06-30%20114706.png)

*   **Đồng bộ sự kiện thời gian thực:** Áp dụng mô hình Observer thông qua một `EventBus` an toàn đa luồng (thread-safe). Các widget giao diện đăng ký nhận sự kiện và tự động cập nhật trạng thái khi có thay đổi từ tầng dữ liệu, loại bỏ hoàn toàn sự phụ thuộc chéo (tight coupling) giữa các thành phần MVC.

### 2. Quản lý Giao dịch & Máy trạng thái Nghiệp vụ
*   **Máy trạng thái hữu hạn (FSM) cho Mượn - Trả:** Quy trình lưu chuyển sách được quản lý thông qua mô hình FSM gồm 6 trạng thái nghiệp vụ (Đang mượn, Đã trả, Mất - Chờ thanh toán, Mất - Đã thanh toán, Mất - Chờ hoàn tiền, Mất - Đã hoàn tiền).

    ![Màn hình nghiệp vụ mượn trả và giao dịch](Ảnh%20chụp%20màn%20hình%202026-06-30%20114725.png)

*   **Ràng buộc điều kiện nghiêm ngặt:** Hệ thống thực hiện kiểm tra thời gian thực đối với tính hợp lệ của thẻ (ngày hết hạn, giới hạn số lượng sách đang mượn và số dư phí phạt quá hạn tối đa cho phép) trước khi ghi nhận bất kỳ giao dịch mượn mới nào.
*   **Tính toán phí phạt tự động:** Dựa trên các tham số cấu hình lưu trữ trong bảng cài đặt hệ thống, cơ chế tính toán phí phạt quá hạn được kích hoạt tự động theo số ngày trễ hạn thực tế.

### 3. Kiểm soát Kho sách & Đối soát Vật lý
*   **Đối soát luồng mã vạch tuyến tính:** Module kiểm kê hỗ trợ kết nối trực tiếp với máy quét mã vạch. Luồng dữ liệu quét thô được so khớp tuyến tính với trạng thái hiện thời trong cơ sở dữ liệu để xuất biên bản chênh lệch về các bản sách bị mất, đặt sai vị trí kệ hoặc sai lệch trạng thái.
*   **Hàng đợi in ấn bất đồng bộ:** Lớp `PrintQueueManager` đóng vai trò quản lý hàng đợi in ấn (thẻ độc giả QR, tem nhãn gáy sách), tách biệt hoàn toàn tác vụ IO của thiết bị in ra khỏi luồng xử lý giao diện chính (GUI thread), ngăn chặn tình trạng treo ứng dụng (UI freezing).

### 4. Tích hợp, Bảo mật & Khả năng Chịu lỗi
*   **Xử lý dữ liệu hàng loạt:** Tích hợp bộ thư viện `QXlsx` để thực hiện việc nhập/xuất tệp Excel dung lượng lớn, có đi kèm kiểm tra tính hợp lệ của schema dữ liệu đầu vào.
*   **Tính toàn vẹn của Giao dịch dữ liệu:** Các thao tác thay đổi trạng thái lớn được bọc trong các SQL transaction (`BEGIN TRANSACTION`, `COMMIT`, `ROLLBACK`), đảm bảo dữ liệu không bị ghi nhận nửa chừng nếu xảy ra sự cố phần cứng hoặc mất điện đột ngột.
*   **Mã hóa và Giám sát bảo mật:** Mật khẩu người dùng được băm bằng thuật toán SHA-256 kết hợp mã muối (Salt) ngẫu nhiên. Mọi hành động thao tác dữ liệu nhạy cảm đều được ghi nhận vào nhật ký hệ thống (Audit Log) bất biến.
*   **Cơ chế Sao lưu tự động:** Hệ thống tự động tạo snapshot dự phòng của cơ sở dữ liệu trước khi thực hiện các tác vụ nhập liệu hàng loạt hoặc cập nhật cấu trúc bảng.

---

## Cấu trúc Thư mục Dự án

```text
QuanLyThuVien/
├── include/              # Định nghĩa các tệp tiêu đề (.h)
│   ├── models/           # Cấu trúc dữ liệu và logic nghiệp vụ (DauSach, DocGia, MuonTra)
│   ├── ui/               # Khai báo các Widget và các hộp thoại giao diện
│   └── utils/            # Lớp tiện ích (DatabaseManager, EventBus, BackupManager, PrintHelper)
├── src/                  # Triển khai mã nguồn chính (.cpp)
│   ├── models/           # Chi tiết mã nguồn logic nghiệp vụ
│   ├── ui/               # Xử lý sự kiện giao diện và liên kết tín hiệu (signals-slots)
│   └── utils/            # Chi tiết mã nguồn các lớp tiện ích
├── resources/            # Stylesheets (QSS), Icons và Fonts hệ thống hỗ trợ đa ngữ
├── data/                 # Thư mục lưu trữ dữ liệu runtime của ứng dụng
│   ├── files/            # Cơ sở dữ liệu SQLite sản xuất (library.db)
│   ├── backup/           # File snapshot cơ sở dữ liệu tự động
│   └── logs/             # Nhật ký hoạt động hệ thống và nhật ký lỗi
├── 3rdparty/             # Thư viện liên kết bên thứ ba (QXlsx)
├── tests/                # Bộ kiểm thử đơn vị (Unit Tests)
└── tools/                # Các tập lệnh Python hỗ trợ chuyển đổi dữ liệu và bảo trì DB
```

---

## Hướng dẫn Biên dịch & Đóng gói

### Yêu cầu môi trường
*   **Qt SDK:** Phiên bản 6.10 hoặc cao hơn (yêu cầu gói cài đặt Qt Widgets).
*   **Bộ công cụ biên dịch:** MinGW 13.1 (64-bit) hoặc trình biên dịch MSVC tương đương hỗ trợ C++17.
*   **Hệ điều hành:** Windows 10 hoặc Windows 11 (64-bit).

### Các bước biên dịch từ mã nguồn (Dòng lệnh)
1. Cấu hình môi trường biên dịch thông qua qmake:
   ```bash
   qmake QuanLyThuVien_V2.pro -spec win32-g++ "CONFIG+=release"
   ```
2. Thực hiện quá trình build tệp thực thi:
   ```bash
   mingw32-make release
   ```
3. Đóng gói các tệp thư viện liên kết động (`.dll`) và thư mục tài nguyên đi kèm:
   ```powershell
   powershell.exe -ExecutionPolicy Bypass -File .\deploy.ps1
   ```
   Thư mục đóng gói hoàn chỉnh sẵn sàng chạy độc lập sẽ được tạo tại `QuanLyThuVien_Portable`.

---

Thông tin nguồn và bản quyền: [NOTICE.md](NOTICE.md).
