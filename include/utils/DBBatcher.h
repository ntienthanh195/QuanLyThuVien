#pragma once

#include <vector>
#include <string>

#include "models/QuanLySach.h" // PTRDS, PTRDMS, KetQuaTimKiem
#include "models/DocGia.h"    // PTRDG

namespace DBBatcher {

// Books
std::vector<PTRDS> snapshotDauSach();
std::vector<PTRDMS> layBanSaoTheoISBN(const std::string &isbn);
std::vector<KetQuaTimKiem> timKiemDauSach(const std::string &tuKhoa, int maxResults = 100);

// Readers
std::vector<PTRDG> snapshotDocGia();
std::vector<PTRDG> timDocGiaTheoHoTen(const std::string &hoTen, int maxResults = 1000);
std::vector<PTRDG> timDocGiaTheoTen(const std::string &ten, int maxResults = 1000);
std::vector<PTRDG> timDocGiaTheoHo(const std::string &ho, int maxResults = 1000);
std::vector<PTRDG> timDocGiaTheoTrangThai(int status, int maxResults = 1000);

} // namespace DBBatcher
