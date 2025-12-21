#include <iostream>
#include <string>
#include <vector>
#include "utils/XuLyChuoi.h"

struct TestCase {
    std::string input;
    std::string expected;
    std::string description;
};

int main() {
    std::vector<TestCase> tests = {
        // Basic cases
        {"   NGUYEN    van   A  ", "Nguyen Van A", "Basic whitespace + mixed case"},
        {"chăm RốCH     THI", "Chăm Rốch Thi", "Vietnamese diacritics mixed case"},
        {"TRẦN văn MINH", "Trần Văn Minh", "Common Vietnamese name"},
        
        // Edge cases - diacritics
        {"nguyễn thị hương", "Nguyễn Thị Hương", "Lowercase with diacritics"},
        {"LÊ VĂN THÀNH", "Lê Văn Thành", "Uppercase with diacritics"},
        {"ĐỖ Minh Đức", "Đỗ Minh Đức", "Đ/đ character"},
        {"võ thị ÁNH", "Võ Thị Ánh", "Mixed case with ô and Á"},
        
        // Edge cases - whitespace
        {"  ", "", "Only spaces"},
        {"", "", "Empty string"},
        {"     Lê     ", "Lê", "Excessive leading/trailing spaces"},
        
        // Edge cases - special patterns
        {"a", "A", "Single character"},
        {"đ", "Đ", "Single Vietnamese character"},
        {"PHẠM     hoàng     LONG", "Phạm Hoàng Long", "Multiple consecutive spaces"},
        
        // Complex Vietnamese names
        {"trương ngọc ánh", "Trương Ngọc Ánh", "All lowercase Vietnamese"},
        {"BÙI XUÂN PHƯƠNG", "Bùi Xuân Phương", "All uppercase Vietnamese"},
        {"hồ chí minh", "Hồ Chí Minh", "Famous name"},
        {"đặng thị ngọc trâm", "Đặng Thị Ngọc Trâm", "Long name with diacritics"}
    };
    
    int passed = 0;
    int failed = 0;
    
    std::cout << "\\n========================================\\n";
    std::cout << "   NORMALIZATION TEST SUITE\\n";
    std::cout << "========================================\\n\\n";
    
    for (size_t i = 0; i < tests.size(); ++i) {
        const TestCase& tc = tests[i];
        try {
            std::string result = ChuanHoaHoTenHienDai(tc.input);
            
            bool pass = (result == tc.expected);
            
            std::cout << "Test #" << i << ": " << tc.description << "\\n";
            std::cout << "  IN : '" << tc.input << "'\\n";
            std::cout << "  OUT: '" << result << "'\\n";
            std::cout << "  EXP: '" << tc.expected << "'\\n";
            std::cout << "  " << (pass ? "PASS" : "FAIL") << "\\n\\n";
            
            if (pass) {
                passed++;
            } else {
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << "Test #" << i << ": " << tc.description << "\\n";
            std::cout << "  EXCEPTION: " << e.what() << "\\n\\n";
            failed++;
        }
    }
    
    std::cout << "========================================\\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\\n";
    std::cout << "Success rate: " << (tests.size() > 0 ? (passed * 100 / tests.size()) : 0) << "%\\n";
    std::cout << "========================================\\n\\n";
    
    // Test validation function
    std::cout << "\\n========================================\\n";
    std::cout << "   VALIDATION TEST SUITE\\n";
    std::cout << "========================================\\n\\n";
    
    std::vector<std::pair<std::string, bool>> validationTests = {
        {"Nguyễn Văn A", true},
        {"Trần Thị Hương", true},
        {"123 Invalid", false},
        {"", false},
        {"   ", false},
        {"A", false},  // Too short
        {"Lê-Văn-Minh", true},
        {"Võ Thị Ánh", true},
        {"Name@Invalid", false}
    };
    
    int validPassed = 0;
    for (size_t i = 0; i < validationTests.size(); ++i) {
        try {
            std::string error;
            bool result = KiemTraHoTenHopLe(validationTests[i].first, error);
            bool expected = validationTests[i].second;
            bool pass = (result == expected);
            
            std::cout << "Validation #" << i << ": '" << validationTests[i].first << "'\\n";
            std::cout << "  Expected: " << (expected ? "Valid" : "Invalid") << "\\n";
            std::cout << "  Result: " << (result ? "Valid" : "Invalid") << "\\n";
            if (!error.empty()) {
                std::cout << "  Error: " << error << "\\n";
            }
            std::cout << "  " << (pass ? "PASS" : "FAIL") << "\\n\\n";
            
            if (pass) validPassed++;
        } catch (const std::exception& e) {
            std::cout << "Validation #" << i << ": EXCEPTION - " << e.what() << "\\n\\n";
        }
    }
    
    std::cout << "========================================\\n";
    std::cout << "Validation: " << validPassed << "/" << validationTests.size() << " passed\\n";
    std::cout << "========================================\\n\\n";
    
    return (failed == 0 && validPassed == validationTests.size()) ? 0 : 1;
}
