Hướng dẫn chuyển đổi và đặt biểu tượng

Các bước sử dụng ảnh đã tải lên làm biểu tượng (icon) cho ứng dụng trên Windows và trong tài nguyên Qt:

1) Lưu ảnh gốc vào dự án
   - Đặt tệp ảnh gốc vào `resources/icons/` và đặt tên là `app_icon_source.png` (hoặc `.jpg` / `.svg`).

2) Chuyển sang định dạng `.ico` cho Windows (ví dụ dùng ImageMagick `magick` trên PowerShell):

```powershell
magick resources\icons\app_icon_source.png -define icon:auto-resize=256,128,96,64,48,32,16 resources\app_icon.ico
```

Nếu máy bạn không có `magick`, có thể dùng công cụ trực tuyến hoặc GIMP/icotool để xuất `.ico` chứa nhiều kích thước.

3) Đặt file `.ico` vào thư mục dự án `resources/` với tên `app_icon.ico` (dự án hiện đã tham chiếu `resources/app_icon.ico`).

4) Tài nguyên Qt
   - `resources/resources.qrc` và `resources/icons.qrc` hiện đã tham chiếu `icons/app_icon_source.png`, nên ảnh gốc sẽ có sẵn qua Qt resources.
   - Nếu bạn muốn `.ico` cũng có trong qresource, copy `app_icon.ico` vào `resources/` (nó đã được tham chiếu trong `resources.qrc`).

5) Windows RC & `.pro`
   - `QuanLyThuVien_V2.pro` đã được cập nhật để nhúng icon trên Windows: `win32: RC_ICONS = resources/app_icon.ico`.

6) Build lại
   - Chạy qmake rồi build (hoặc dùng các task trong workspace). Ví dụ PowerShell từ thư mục dự án:

```powershell
C:/Qt/6.10.1/mingw_64/bin/qmake.exe QuanLyThuVien_V2.pro
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe
```

Ghi chú
- Nếu bạn muốn mình tự động chuyển và thêm `resources/app_icon.ico`, mình có thể thực hiện và commit file `.ico` nhị phân vào repo nếu bạn đồng ý.
- Nếu muốn để icon trong `resources/icons/` thay vì `resources/`, cần chỉnh `resources/app_icon.rc` (nếu dùng) sang đường dẫn mới.

Muốn mình làm tiếp bước chuyển `.ico` và thêm vào repo không? Trả lời `Có` hoặc `Không`.
