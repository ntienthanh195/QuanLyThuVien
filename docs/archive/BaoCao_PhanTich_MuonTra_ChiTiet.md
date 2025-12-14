# BÁO CÁO PHÂN TÍCH VÀ GIẢI PHÁP CẢI THIỆN TRẢI NGHIỆM MƯỢN TRẢ SÁCH

## 1. TỔNG QUAN VẤN ĐỀ
Dựa trên phản hồi và hình ảnh cung cấp, hệ thống Mượn Trả Sách hiện đang gặp 3 vấn đề chính ảnh hưởng đến trải nghiệm người dùng (UX) và tính chính xác của nghiệp vụ.

## 2. CHI TIẾT VÀ GIẢI PHÁP

### VẤN ĐỀ 1: CỘT "HỌ" QUÁ NGẮN, MẤT TÊN ĐỘC GIẢ
**Hiện tượng:**
- Trong bảng danh sách độc giả bên trái, cột "Họ" bị thu hẹp quá mức.
- Tên độc giả (ví dụ: "Chăm Rốch...") bị cắt bớt hoặc hiển thị "...", người dùng khó nhận diện.

**Nguyên nhân Kỹ thuật:**
- Cấu hình `QHeaderView::ResizeMode` chưa tối ưu.
- Có thể đang dùng `Fixed` width hoặc chia tỷ lệ không đều cho cột Họ.
- Các cột ít quan trọng hơn (Giới tính, Trạng thái) lại chiếm quá nhiều không gian.

**Giải pháp UX:**
- **Tối ưu độ rộng cột:**
    - Cột STT: Fixed (40px).
    - Cột Mã Thẻ: Fixed (hoặc ResizeToContents, min 80px).
    - **Cột Họ & Tên đệm:** Stretch (Ưu tiên giãn tối đa).
    - Cột Tên: ResizeToContents (hoặc gộp vào Họ Tên).
    - Cột Giới Tính: Fixed (60px) hoặc ẩn nếu không gian hẹp.
    - Cột Trạng Thái: Fixed (90px).
- **Gộp cột (Optional):** Gộp "Họ" và "Tên" thành một cột "Họ và Tên" duy nhất để tận dụng không gian tốt hơn.

---

### VẤN ĐỀ 2: DANH SÁCH SÁCH ĐANG MƯỢN KHÔNG CUỘN ĐƯỢC
**Hiện tượng:**
- Tại khung bên phải (Thông tin độc giả & Sách đang mượn).
- Khi danh sách sách dài (ví dụ: 3 cuốn trở lên), bảng danh sách bị cố định chiều cao, không hiện thanh cuộn hoặc thanh cuộn bị vô hiệu hóa.

**Nguyên nhân Kỹ thuật:**
- `QTableView` (hoặc widget chứa danh sách) có thể bị set `FixedHeight` hoặc không nằm trong Layout co giãn (`QVBoxLayout`).
- `QSizePolicy` của bảng sách có thể đang là `Fixed` thay vì `Expanding`.

**Giải pháp UX:**
- Đặt bảng danh sách sách vào `QScrollArea` hoặc đảm bảo `QTableView` có `VerticalScrollBarPolicy` là `AsNeeded`.
- Điều chỉnh Layout để bảng danh sách chiếm phần "Còn lại" (Stretch factor) của khung bên phải.
- Đặt `MinHeight` cho bảng khoảng 150-200px để luôn thấy được ít nhất 3-4 cuốn.

---

### VẤN ĐỀ 3: TRẢ ĐÚNG HẠN VẪN BÁO NỢ & THIẾU QUY TRÌNH THANH TOÁN
**Hiện tượng:**
- Khi thực hiện trả sách (kể cả đúng hạn), hệ thống vẫn ghi nhận là "Nợ" hoặc "Chưa thanh toán".
- Không có nút chức năng để "Thanh toán", "Xóa nợ" hay xử lý tiền phạt ngay tại màn hình này.

**Nguyên nhân Kỹ thuật:**
- **Logic Tính Phạt:** Có thể thuật toán tính ngày quá hạn (`NgayTra - NgayMuon`) đang sai lệch, hoặc mặc định luôn có phí phạt.
- **Trạng thái Mặc định:** Khi tạo phiếu trả, trạng thái có thể đang mặc định set là "Chưa Thanh Toán" (0) thay vì kiểm tra tiền phạt = 0 thì set là "Đã Thanh Toán" (1).
- **Thiếu UI:** Chưa implement dialog hoặc nút bấm xử lý nghiệp vụ thu tiền.

**Giải pháp UX & Logic:**
- **Sửa Logic Tính Phạt:**
    - Nếu `NgayTra <= HanTra`: Tiền phạt = 0 ➔ Tự động set trạng thái = Đã Thanh Toán (Hoàn tất).
    - Nếu `NgayTra > HanTra`: Tính tiền phạt (ví dụ: 5.000đ/ngày).
- **Quy Trình Trả Sách Mới:**
    1. Nhấn nút "Trả Sách".
    2. Hệ thống kiểm tra quá hạn.
    3. **Nếu KHÔNG quá hạn:** Thông báo "Trả sách thành công" ➔ Cập nhật danh sách.
    4. **Nếu CÓ quá hạn:**
       - Hiển thị Dialog: "Sách quá hạn X ngày. Tiền phạt: Y vnđ".
       - Các lựa chọn: 
         - [Thanh Toán Ngay]: Ghi nhận đã trả + đã thu tiền.
         - [Ghi Nợ]: Ghi nhận đã trả sách + lưu nợ vào hồ sơ độc giả.
         - [Hủy]: Không trả sách nữa.

## 3. KẾ HOẠCH TRIỂN KHAI
1. **Refactor UI Bảng Độc Giả:** Cân chỉnh lại `HeaderView`.
2. **Fix Layout Bảng Sách:** Đảm bảo khả năng Scroll.
3. **Cập nhật Logic Trả Sách:** Thêm popup xác nhận thanh toán/ghi nợ khi trả sách.

Bắt đầu thực hiện chỉnh sửa ngay sau đây.
