#include "models/NhapLieu.h"

// Mock NhapLieu 
std::string NhapTen(const char* nhan, size_t minLength, size_t maxLength, std::istream& in, std::ostream& out) {
    return "TestName";
}
int NhapSoNguyen(const char* nhan, int minVal, int maxVal, std::istream& in, std::ostream& out) {
    return minVal;
}
std::string NhapPhai(std::istream& in, std::ostream& out) {
    return "Nam";
}
std::string NhapISBNThuCong(std::istream& in, std::ostream& out) {
    return "ISBN-TEST";
}
