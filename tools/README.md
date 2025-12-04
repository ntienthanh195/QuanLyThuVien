normalize_test: small CLI tool to exercise ChuanHoaHoTen normalization

Build (PowerShell):
C:/Qt/6.10.1/mingw_64/bin/qmake.exe QuanLyThuVien_V2.pro ; C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe

Or compile only the test (uses mingw g++):

```powershell
g++ -Iinclude -Iinclude/utils tools/normalize_test.cpp src/utils/XuLyChuoi.cpp -o tools/normalize_test.exe
./tools/normalize_test.exe
```
