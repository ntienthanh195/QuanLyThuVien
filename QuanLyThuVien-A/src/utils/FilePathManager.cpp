/**
 * @file FilePathManager.cpp
 * @path src/utils/FilePathManager.cpp
 * @description Quản lý đường dẫn tập tin và thư mục trong dự án.
 * Đảm bảo các file dữ liệu (.txt, .db) và tài nguyên được truy xuất đúng vị trí cho dù chạy ở chế độ IDE hay Portable.
 * @project Library Management System (QuanLyThuVien)
 */
#include "utils/FilePathManager.h"


// Định nghĩa biến static
QString FilePathManager::rootPath = "";
QString FilePathManager::filesDir = "";
bool FilePathManager::isInitialized = false;
