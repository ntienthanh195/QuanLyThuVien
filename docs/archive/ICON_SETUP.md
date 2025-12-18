# Hướng dẫn tạo Icon cho ứng dụng

## Bước 1: Lưu hình ảnh
1. Lưu hình ảnh bạn đã gửi vào thư mục `resources/` với tên `library_icon_source.png`
2. Đảm bảo hình ảnh có kích thước tối thiểu 256x256 pixels

## Bước 2: Cài đặt Pillow (nếu chưa có)
```bash
pip install Pillow
```

## Bước 3: Chạy script tạo icon
```bash
python create_icon.py
```

Script sẽ tạo file `resources/app_icon.ico` với nhiều kích thước khác nhau (16x16, 32x32, 48x48, 64x64, 128x128, 256x256).

## Bước 4: Build lại ứng dụng
```bash
qmake LibraryManagement.pro
mingw32-make
```

## Cấu trúc files:
- `resources/library_icon_source.png` - Hình ảnh gốc
- `resources/app_icon.ico` - File icon đã được tạo
- `resources/app_icon.rc` - File resource cho Windows
- `LibraryManagement.pro` - Đã được cập nhật để sử dụng icon

## Lưu ý:
- File .rc được tham chiếu trong .pro file với dòng: `win32:RC_FILE = resources/app_icon.rc`
- Icon sẽ xuất hiện trên file .exe sau khi build
- Icon cũng sẽ hiện trên thanh taskbar khi chạy ứng dụng
