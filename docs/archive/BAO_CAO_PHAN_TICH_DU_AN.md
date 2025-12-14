# BÁO CÁO PHÂN TÍCH & ĐÁNH GIÁ CHI TIẾT DỰ ÁN
**APP QUẢN LÝ THƯ VIỆN OFFLINE TRÊN DESKTOP**
---
**Ngày đánh giá:** 14/12/2025
**Phiên bản:** Development
**Người đánh giá:** Antigravity AI

Dưới đây là đánh giá chi tiết theo 27 tiêu chí yêu cầu, trình bày dưới dạng liệt kê.

---

## I. MODULE HỆ THỐNG (System Admin)

**1. Đăng nhập / Đăng xuất**
- **Trạng thái:** 🟢 HOÀN THÀNH (Tốt)
- **Đánh giá:** Hệ thống bảo mật mật khẩu bằng thuật toán SHA256 kèm Salt (Muối) riêng cho từng user, đảm bảo an toàn cao. Logic phiên làm việc hoạt động ổn định.
- **Mã nguồn:** `UserManager.cpp`, `ManHinhDangNhap.cpp`.

**2. Quản lý người dùng & Phân quyền**
- **Trạng thái:** 🟡 CẦN CẢI THIỆN
- **Đánh giá:** Đã có cơ sở dữ liệu (`nguoi_dung`, `vai_tro`) và logic kiểm tra quyền (`hasPermission`) ở backend. Tuy nhiên, giao diện quản lý người dùng (`WidgetQuanLyNguoiDung`) còn khá đơn giản, chưa trực quan trong việc tick chọn quyền hạn chi tiết cho từng vai trò.

**3. Cấu hình tham số (Settings)**
- **Trạng thái:** 🟢 HOÀN THÀNH (Tốt)
- **Đánh giá:** Cho phép thay đổi linh hoạt các tham số nghiệp vụ như: Số sách tối đa được mượn, Số ngày mượn, Tiền phạt/ngày. Cấu hình được lưu trữ bền vững.
- **Mã nguồn:** `SettingsManager.cpp`, `ManHinhCauHinh.cpp`.

**4. Sao lưu & Phục hồi dữ liệu (Backup/Restore)**
- **Trạng thái:** 🟢 HOÀN THÀNH (Tốt)
- **Đánh giá:** Đã tích hợp đầy đủ quy trình sao lưu file database (`library.db`) ra file `.bak` an toàn và quy trình phục hồi ngược lại. Hỗ trợ sao lưu thủ công theo yêu cầu.
- **Mã nguồn:** `BackupManager.cpp`.

**5. Nhật ký hệ thống (System Log)**
- **Trạng thái:** 🟢 HOÀN THÀNH (Tốt)
- **Đánh giá:** Module `AuditLogger` hoạt động nền, tự động ghi lại mọi thao tác quan trọng (Ai đăng nhập, ai xóa sách, ai sửa tiền phạt...) giúp truy vết khi có sự cố.

---

## II. MODULE QUẢN LÝ TÀI LIỆU (Sách)

**6. Nhập mới sách**
- **Trạng thái:** 🟢 HOÀN THÀNH (Đầy đủ)
- **Đánh giá:** Form nhập liệu chi tiết với đầy đủ các trường (Mã, Tên, Giá, Năm XB...). Có kiểm tra dữ liệu đầu vào logic (ví dụ: Năm XB không được lớn hơn năm hiện tại).
- **Mã nguồn:** `ManHinhThemSach.cpp`.

**7. Quản lý Mã cá biệt**
- **Trạng thái:** 🟢 HOÀN THÀNH (Xuất sắc)
- **Đánh giá:** Backend sử dụng Danh sách liên kết (`dms`) để quản lý từng cuốn sách riêng biệt (Bản sao). Hệ thống tự sinh mã `ISBN-1`, `ISBN-2`... và quản lý trạng thái từng cuốn (Rách, Mất, Đang mượn) rất chặt chẽ.

**8. Import từ Excel**
- **Trạng thái:** 🟢 HOÀN THÀNH
- **Đánh giá:** Sử dụng Python script (`ImportManager.cpp`) để đọc file Excel, hỗ trợ cả định dạng `.xlsx`. Có giao diện hiển thị tiến trình (Progress Bar) và xem trước (Preview) giúp tránh lỗi data rác.

**9. In Mã vạch / Tem sách**
- **Trạng thái:** 🟢 HOÀN THÀNH
- **Đánh giá:** Tính năng in hoạt động tốt. Hệ thống tự vẽ mã vạch (Barcode 1D) và QR Code trực tiếp, không phụ thuộc thư viện ngoài, giúp phần mềm nhẹ và dễ đóng gói (Portable).
- **Mã nguồn:** `PrintHelper.cpp`.

**10. Kiểm kê kho**
- **Trạng thái:** 🔴 CHƯA CÓ (Thiếu)
- **Đánh giá:** Chưa có module "Kiểm kê". Phần mềm cần bổ sung chức năng cho phép quét mã vạch liên tục để đối soát giữa tệp sách thực tế trên kệ và dữ liệu trong phần mềm.

**11. Thanh lý sách**
- **Trạng thái:** 🟡 CẦN CẢI THIỆN (Một phần)
- **Đánh giá:** Trong code đã có trạng thái `THANH_LY` để đánh dấu sách hư hỏng. Tuy nhiên chưa có giao diện quản lý danh sách thanh lý riêng biệt để duyệt hủy hàng loạt.

---

## III. MODULE QUẢN LÝ ĐỘC GIẢ

**12. Quản lý hồ sơ độc giả**
- **Trạng thái:** 🟢 HOÀN THÀNH (Tốt)
- **Đánh giá:** Quản lý đầy đủ thông tin (Họ tên, Ngày sinh, Lớp). Dữ liệu được tổ chức tốt (Cây nhị phân tìm kiếm BST hoặc Hash Map) giúp truy xuất nhanh.
- **Mã nguồn:** `WidgetQuanLyDocGia.cpp`, `DocGia.cpp`.

**13. Import Độc giả từ Excel**
- **Trạng thái:** 🟢 HOÀN THÀNH
- **Đánh giá:** Dùng chung cơ chế Import mạnh mẽ của module Sách. Hỗ trợ nhập danh sách lớp/nhân viên nhanh chóng.

**14. Cấp thẻ / In thẻ**
- **Trạng thái:** 🟢 HOÀN THÀNH
- **Đánh giá:** Logic tự sinh mã thẻ hoạt động tốt. Module in ấn hỗ trợ in thẻ thư viện có ảnh thẻ và mã QR Code để điểm danh.

**15. Gia hạn thẻ / Khóa thẻ**
- **Trạng thái:** 🟡 CẦN CẢI THIỆN
- **Đánh giá:** Backend có hỗ trợ trạng thái khóa/mở thẻ. Tuy nhiên giao diện chưa có nút thao tác nhanh (ví dụ nút "Gia hạn thêm 1 năm") mà phải vào sửa thông tin chi tiết.

---

## IV. MODULE MƯỢN - TRẢ (Nghiệp vụ cốt lõi)

**16. Mượn sách**
- **Trạng thái:** 🟢 HOÀN THÀNH (Xuất sắc)
- **Đánh giá:** Logic "Smart Borrow" rất thông minh:
    - Nhập ISBN: Hệ thống tự tìm bản sao rảnh ngẫu nhiên.
    - Nhập Mã cá biệt: Hệ thống chọn chính xác cuốn sách đó.
    - Tự động chặn mượn nếu: Thẻ hết hạn, Đang nợ quá hạn, hoặc đã mượn quá số lượng quy định.
- **Mã nguồn:** `MuonTra.cpp`.

**17. Trả sách**
- **Trạng thái:** 🟢 HOÀN THÀNH
- **Đánh giá:** Quy trình nhanh gọn: Chỉ cần quét mã sách -> Phần mềm tự tìm ra ai đang mượn -> Thực hiện trả. Tính toán số ngày quá hạn chính xác dựa trên cấu hình.

**18. Gia hạn mượn**
- **Trạng thái:** 🟡 CẦN CẢI THIỆN
- **Đánh giá:** Trong code backend có lưu biến `soLanGiaHan`. Cần rà soát lại giao diện để đảm bảo nút "Gia hạn" dễ nhìn và dễ thao tác cho thủ thư.

**19. Xử lý vi phạm & Phạt**
- **Trạng thái:** 🟡 CẦN CẢI THIỆN (Thiếu in ấn)
- **Đánh giá:** Logic tính tiền phạt (quá hạn hoặc làm mất sách) đã hoạt động đúng. Tuy nhiên, phần mềm **thiếu chức năng in biên lai phạt** ngay tại thời điểm thu tiền để giao cho độc giả.

**20. Lịch sử mượn trả**
- **Trạng thái:** 🟢 HOÀN THÀNH
- **Đánh giá:** Mỗi độc giả đều có danh sách lịch sử giao dịch chi tiết. Thủ thư có thể xem lại dễ dàng toàn bộ quá trình mượn trả của từng người.

---

## V. MODULE TRA CỨU

**21. Tìm kiếm nâng cao**
- **Trạng thái:** 🟡 CẦN CẢI THIỆN
- **Đánh giá:** Tìm kiếm cơ bản (Theo tên, mã, tác giả) hoạt động nhanh. Tuy nhiên phần mềm chỉ có 1 ô tìm kiếm chung (Unified Search), **thiếu bộ lọc nâng cao** (Ví dụ lọc đồng thời: Thể loại = Văn học VÀ Năm XB > 2020).

**22. Tra cứu trạng thái**
- **Trạng thái:** 🟢 HOÀN THÀNH (Tốt)
- **Đánh giá:** Màn hình chi tiết sách hiển thị rất rõ ràng: Cuốn nào đang rảnh, cuốn nào đang mượn (Ai mượn, bao giờ phải trả).

**23. Lọc dữ liệu**
- **Trạng thái:** 🟡 CẦN CẢI THIỆN (Sơ sài)
- **Đánh giá:** Cần bổ sung các bộ lọc nhanh (Combobox) ngay trên đầu danh sách sách để lọc nhanh theo Ngôn ngữ, Danh mục...

---

## VI. MODULE BÁO CÁO & THỐNG KÊ (Xuất Excel)

**24. Thống kê Kho sách**
- **Trạng thái:** 🟢 HOÀN THÀNH
- **Đánh giá:** Màn hình Thống kê (`ManHinhThongKe.cpp`) hiển thị đầy đủ các chỉ số quan trọng: Tổng đầu sách, Tổng bản sao, Số sách hư hỏng, Số sách đang mượn...

**25. Thống kê Mượn - Trả**
- **Trạng thái:** 🟢 HOÀN THÀNH
- **Đánh giá:** Đã có các báo cáo Top độc giả mượn nhiều nhất, Top sách "Hot" nhất.

**26. Báo cáo Quá hạn**
- **Trạng thái:** 🟢 HOÀN THÀNH
- **Đánh giá:** Có màn hình riêng liệt kê danh sách các độc giả đang giữ sách quá hạn để thủ thư liên hệ nhắc nhở.

**27. Báo cáo Tài chính**
- **Trạng thái:** 🔴 CHƯA CÓ (Thiếu)
- **Đánh giá:** Đây là thiếu sót lớn nhất. Phần mềm có tính tiền phạt nhưng chưa có màn hình tổng hợp doanh thu (Tiền phạt + Tiền đền bù) theo Tháng/Quý để báo cáo lên cấp trên.

---

## VII. ĐÁNH GIÁ ĐẶC THÙ CHO HỌC VIỆN PHẬT GIÁO NAM TÔNG KHMER (QUAN TRỌNG)

Dựa trên bối cảnh triển khai tại **Học viện Phật giáo Nam Tông Khmer (Cần Thơ)** mới thành lập, hệ thống cần bổ sung các tiêu chuẩn đặc thù sau để phù hợp văn hóa và nghiệp vụ nhà trường:

**1. Hỗ trợ hiển thị Tiếng Khmer & Pali**
- **Vấn đề:** Các đầu sách Kinh tạng, Luật tạng thường có tên gốc tiếng Pali hoặc Khmer. Tên Tăng/Ni sinh cũng có thể dùng tiếng Khmer.
- **Yêu cầu:** 
    - Hệ thống phải hỗ trợ chuẩn **Unicode (UTF-8)** tuyệt đối.
    - Module In ấn (`PrintHelper`) phải tích hợp font chữ Khmer đặc thù (như *Khmer OS Siemreap* hoặc *DaunPenh*) để tránh lỗi in ra ký tự ô vuông khi in Thẻ/Tem sách.
    - Tìm kiếm phải hỗ trợ gõ được tiếng Khmer.

**2. Tích hợp Phật Lịch (Buddhist Calendar)**
- **Vấn đề:** Trong môi trường Phật giáo, ngày tháng thường tính theo Phật Lịch (VD: Năm 2025 là PL. 2569).
- **Đề xuất:**
    - Trên giao diện chính và các Phiếu Mượn/Trả, Thẻ Thư Viện: Cần hiển thị song song **Dương Lịch / Phật Lịch**.
    - Ví dụ: *Ngày 14/12/2025 (PL. 2569)*.

**3. Phân loại Sách kết hợp (Đa dạng)**
- **Vấn đề:** Thư viện phục vụ cả sách Phật học (Tam Tạng) và sách giáo khoa/đại cương (Văn học, Lịch sử, Ngoại ngữ...) phục vụ đào tạo phổ thông.
- **Đề xuất:** Module "Thể loại" cần hỗ trợ cấu trúc **Cây thư mục đa nhánh**:
    - Nhánh 1: *Phật Giáo (Kinh/Luật/Luận...)*
    - Nhánh 2: *Kiến thức phổ thông (Văn học/Lịch sử/KHXH...)*
    - Nhánh 3: *Ngoại ngữ (Pali/Khmer/Anh...)*
    - Không ép buộc toàn bộ theo cấu trúc Tam Tạng.

**4. Từ ngữ & Xưng hô giao diện**
- **Vấn đề:** Đối tượng người học đa dạng (Tăng sinh, Tu nữ, Sinh viên thế tục...).
- **Đề xuất:** Dùng từ **"Học viên"** là chuẩn xác và bao quát nhất. (Tránh dùng "Độc giả" nghe xa lạ, hoặc "Tăng sinh" thì không đủ).
- **Tiền phạt:** Giữ nguyên cơ chế phạt nhưng có thể tùy biến tên gọi trong Cấu hình (để phù hợp từng trường hợp).

---

## TỔNG KẾT

**Điểm mạnh:**
*   Core Backend C++ tự xây dựng rất vững chắc, tối ưu hiệu năng.
*   Logic nghiệp vụ Mượn/Trả thông minh, chặt chẽ.
*   Giao diện đẹp, hiện đại, không bị lỗi thời.

**Điểm yếu cần khắc phục ngay:**
1.  Thiếu module **Kiểm kê kho**.
2.  Thiếu module **Báo cáo tài chính**.
3.  Thiếu chức năng **In biên lai thu tiền**.
