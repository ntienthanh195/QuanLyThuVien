# 📁 Thư mục dữ liệu / Data Directory

Thư mục **DUY NHẤT** chứa toàn bộ file dữ liệu của hệ thống.

## 📄 Các file dữ liệu

| File | Mô tả | Encoding |
|------|-------|----------|
| `dausach.txt` | Danh sách đầu sách (ISBN, tên, tác giả, thể loại, năm, số trang) | UTF-8 |
| `danhmucsach.txt` | Danh mục các bản sao sách (mã sách, trạng thái, vị trí) | UTF-8 |
| `docgia.txt` | Thông tin độc giả (mã thẻ, họ tên, giới tính, trạng thái) | UTF-8 |
| `muontra.txt` | Lịch sử mượn/trả sách | UTF-8 |

## 🎯 Đường dẫn tuyệt đối

Ứng dụng sẽ **đọc/ghi trực tiếp** từ thư mục này, không copy sang build output.

```
E:\Qt\QuanLyThuVien\files\
```

## 📝 Format dữ liệu

### dausach.txt
```
ISBN|TenSach|SoTrang|TacGia|NamXuatBan|TheLoai
```

### danhmucsach.txt
```
MaSach|TrangThai|ViTri|ISBN
```

### docgia.txt
```
MaThe|Ho|Ten|GioiTinh|TrangThai
```

### muontra.txt
```
MaSach|MaThe|NgayMuon|NgayTra|TrangThai
```

## ⚠️ Lưu ý

- **KHÔNG XÓA** các file dữ liệu khi đang chạy ứng dụng
- Hệ thống tự động backup vào `files/backup/` khi có thay đổi
- Tất cả file dùng encoding **UTF-8** để hỗ trợ tiếng Việt
