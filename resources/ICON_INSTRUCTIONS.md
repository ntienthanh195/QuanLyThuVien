ICON conversion and placement

Steps to use the uploaded image as application icon for Windows and in Qt resources:

1) Save the uploaded image to the project
   - Put the original image file into `resources/icons/` and name it `app_icon_source.png` (or `.jpg` / `.svg`).

2) Convert to `.ico` for Windows (example using ImageMagick `magick` on Windows PowerShell):

```powershell
magick resources\icons\app_icon_source.png -resize 256x256 resources\app_icon.ico
magick resources\icons\app_icon_source.png -resize 48x48 resources\icons\app_icon_48.png
magick resources\icons\app_icon_source.png -define icon:auto-resize=256,128,96,64,48,32,16 resources\app_icon.ico
```

If `magick` is not available, you can use any online converter or tools like `icotool` / GIMP to export `.ico` containing multiple sizes.

3) Place the `.ico` in the project root `resources/` as `app_icon.ico` (the project already references `resources/app_icon.ico`).

4) Qt resources
   - `resources/resources.qrc` and `resources/icons.qrc` now reference `icons/app_icon_source.png`.
   - If you want the `.ico` also available as a Qt resource, copy `app_icon.ico` into `resources/` (it's already referenced by `resources.qrc`).

5) Windows RC & .pro
   - `QuanLyThuVien_V2.pro` now contains `win32: RC_ICONS = resources/app_icon.ico` so the `.ico` will be embedded on Windows builds.

6) Rebuild
   - Run qmake + make (or use the VS Code tasks provided). For PowerShell, from project root:

```powershell
C:/Qt/6.10.1/mingw_64/bin/qmake.exe QuanLyThuVien_V2.pro
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe
```

Or use the workspace task `Build & Compile Qt (Full Build)`.

Notes
- If you prefer the icon inside `resources/icons/` instead, update `resources/app_icon.rc` to point to `"icons/app_icon.ico"`.
- I could convert and add the `.ico` file directly if you give me permission to write binary image files here; otherwise, follow the commands above and I'll finish the last steps (add the `.ico` file and update `.rc` if needed).