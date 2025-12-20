/**
 * @file LanguageManager.cpp
 * @path src/utils/LanguageManager.cpp
 * @description Quản lý đa ngôn ngữ (Localization).
 * Xử lý: tải file ngôn ngữ (JSON/INI), chuyển đổi ngôn ngữ thời gian thực (Vietnamese, English, Khmer) cho toàn bộ UI.
 * @project Library Management System (QuanLyThuVien)
 */
#include "utils/LanguageManager.h"

#include <QDebug>

LanguageManager& LanguageManager::instance()
{
    static LanguageManager _instance;
    return _instance;
}

LanguageManager::LanguageManager(QObject *parent) : QObject(parent), m_currentLang(Language::Vietnamese)
{
    loadDictionary();
}

void LanguageManager::setLanguage(Language lang)
{
    if (m_currentLang != lang) {
        m_currentLang = lang;
        emit languageChanged(m_currentLang);
    }
}

Language LanguageManager::currentLanguage() const
{
    return m_currentLang;
}

void LanguageManager::cycleLanguage()
{
    switch (m_currentLang) {
    case Language::Vietnamese:
        setLanguage(Language::Khmer);
        break;
    case Language::Khmer:
        setLanguage(Language::English);
        break;
    case Language::English:
        setLanguage(Language::Vietnamese);
        break;
    }
}

QString LanguageManager::getText(const QString& key)
{
    if (key.isEmpty()) {
        qWarning() << "LanguageManager::getText: empty key!";
        return QString();
    }
    
    if (m_dictionary.contains(key)) {
        if (m_dictionary[key].contains(m_currentLang)) {
            QString text = m_dictionary[key][m_currentLang];
            if (!text.isEmpty()) {
                return text;
            }
            qWarning() << "LanguageManager: Empty translation for key" << key;
        }
        // Fallback to Vietnamese
        if (m_dictionary[key].contains(Language::Vietnamese)) {
            return m_dictionary[key][Language::Vietnamese];
        }
    }
    
    qWarning() << "LanguageManager: Key not found:" << key;
    return key; // Return key as fallback
}

void LanguageManager::loadDictionary()
{
    // === MAIN MENU ===
    m_dictionary["MENU_DASHBOARD"] = {
        {Language::Vietnamese, "Bảng Điều Khiển"},
        {Language::Khmer, "ផ្ទាំងគ្រប់គ្រង (Dashboard)"},
        {Language::English, "Dashboard"}
    };
    
    // NOTE: MENU_* keys are defined later (around line 881+) with sidebar icons
    
    
    // === DASHBOARD ===
    m_dictionary["DASH_TOTAL_BOOKS"] = {
        {Language::Vietnamese, "Tổng số đầu sách"},
        {Language::Khmer, "ចំនួនសៀវភៅសរុប"},
        {Language::English, "Total Books"}
    };
    m_dictionary["DASH_TOTAL_READERS"] = {
        {Language::Vietnamese, "Tổng số học viên"},
        {Language::Khmer, "ចំនួនសិស្សសរុប"},
        {Language::English, "Total Students"}
    };
    m_dictionary["DASH_BORROWING"] = {
        {Language::Vietnamese, "Đang mượn sách"},
        {Language::Khmer, "កំពុងខ្ចី"},
        {Language::English, "Borrowing"}
    };
    m_dictionary["DASH_OVERDUE"] = {
        {Language::Vietnamese, "Sách quá hạn"},
        {Language::Khmer, "សៀវភៅហួសកំណត់"},
        {Language::English, "Overdue Books"}
    };
    
    // === COMMON ===
    m_dictionary["BTN_ADD"] = {
        {Language::Vietnamese, "Thêm Mới"},
        {Language::Khmer, "បន្ថែមថ្មី (Add)"},
        {Language::English, "Add New"}
    };
    m_dictionary["BTN_EDIT"] = {
        {Language::Vietnamese, "Chỉnh Sửa"},
        {Language::Khmer, "កែប្រែ (Edit)"},
        {Language::English, "Edit"}
    };
    m_dictionary["BTN_DELETE"] = {
        {Language::Vietnamese, "Xóa"},
        {Language::Khmer, "លុប (Delete)"},
        {Language::English, "Delete"}
    };
    m_dictionary["BTN_SAVE"] = {
        {Language::Vietnamese, "Lưu Lại"},
        {Language::Khmer, "រក្សាទុក (Save)"},
        {Language::English, "Save"}
    };
    m_dictionary["BTN_CANCEL"] = {
        {Language::Vietnamese, "Hủy Bỏ"},
        {Language::Khmer, "បោះបង់ (Cancel)"},
        {Language::English, "Cancel"}
    };
    m_dictionary["BTN_REPORTS"] = {
        {Language::Vietnamese, " Xem Báo Cáo"},
        {Language::Khmer, " របាយការណ៍ (Reports)"},
        {Language::English, " Reports"}
    };
    m_dictionary["BTN_TOP10"] = {
        {Language::Vietnamese, "Top 10"},
        {Language::Khmer, "Top 10"},
        {Language::English, "Top 10"}
    };
    
    m_dictionary["TITLE_SELECT_EXCEL_IMPORT"] = {
        {Language::Vietnamese, "Chọn file Excel để nhập dữ liệu"},
        {Language::Khmer, "ជ្រើសរើសឯកសារ Excel"},
        {Language::English, "Select Excel file to import"}
    };
    m_dictionary["DASH_TASK_IMPORT"] = {
        {Language::Vietnamese, "Nhập Dữ Liệu"},
        {Language::Khmer, "នាំចូលទិន្នន័យ (Import)"},
        {Language::English, "Import Data"}
    };
    m_dictionary["DASH_DESC_IMPORT"] = {
        {Language::Vietnamese, "Nhập sách, học viên từ file Excel cực nhanh."},
        {Language::Khmer, "នាំចូលសៀវភៅ និងសិស្សពីឯកសារ Excel យ៉ាងរហ័ស।"},
        {Language::English, "Import books & students from Excel file instantly."}
    };
    m_dictionary["MSG_IMPORT_SUCCESS"] = {
        {Language::Vietnamese, "Nhập dữ liệu thành công!"},
        {Language::Khmer, "ការនាំចូលទិន្នន័យបានជោគជ័យ!"},
        {Language::English, "Data imported successfully!"}
    };
    m_dictionary["MSG_IMPORT_FAILED"] = {
        {Language::Vietnamese, "Nhập dữ liệu thất bại. Vui lòng kiểm tra lại file."},
        {Language::Khmer, "ការនាំចូលទិន្នន័យបានបរាជ័យ។"},
        {Language::English, "Data import failed. Please check the file."}
    };
    


    m_dictionary["BTN_REFRESH"] = {
        {Language::Vietnamese, "Làm mới"},
        {Language::Khmer, "ផ្ទុកឡើងវិញ (Refresh)"},
        {Language::English, "Refresh"}
    };
    m_dictionary["BTN_DELETE"] = {
        {Language::Vietnamese, "Xóa"},
        {Language::Khmer, "លុប (Delete)"},
        {Language::English, "Delete"}
    };
    m_dictionary["BTN_EDIT"] = {
        {Language::Vietnamese, "Sửa"},
        {Language::Khmer, "កែសម្រួល (Edit)"},
        {Language::English, "Edit"}
    };
    m_dictionary["BTN_PRINT"] = {
        {Language::Vietnamese, "In"},
        {Language::Khmer, "បោះពុម្ព (Print)"},
        {Language::English, "Print"}
    };

    // === BOOK MANAGEMENT ===
    m_dictionary["PAGE_TITLE_BOOKS"] = {
        {Language::Vietnamese, "Quản Lý Sách"},
        {Language::Khmer, "គ្រប់គ្រងសៀវភៅ (Books)"},
        {Language::English, "Book Management"}
    };
    m_dictionary["DIALOG_ADD_BOOK_TITLE"] = {
        {Language::Vietnamese, "THÊM ĐẦU SÁCH MỚI"},
        {Language::Khmer, "បន្ថែមសៀវភៅថ្មី"},
        {Language::English, "ADD NEW BOOK"}
    };
    m_dictionary["GROUP_BOOK_INFO"] = {
        {Language::Vietnamese, "Thông Tin Sách"},
        {Language::Khmer, "ព័ត៌មានសៀវភៅ"},
        {Language::English, "Book Information"}
    };
    m_dictionary["LBL_PRICE"] = {
        {Language::Vietnamese, "Giá Sách"},
        {Language::Khmer, "តម្លៃ"},
        {Language::English, "Price"}
    };
    m_dictionary["LBL_LOCATION"] = {
        {Language::Vietnamese, "Vị Trí"},
        {Language::Khmer, "ទីតាំង"},
        {Language::English, "Location"}
    };
    m_dictionary["BTN_ADD_BOOK_CONFIRM"] = {
        {Language::Vietnamese, "Thêm Sách"},
        {Language::Khmer, "បន្ថែម"},
        {Language::English, "Add Book"}
    };
    m_dictionary["BTN_REFRESH_FORM"] = {
        {Language::Vietnamese, "Làm Mới"},
        {Language::Khmer, "សម្អាត (Clear)"},
        {Language::English, "Clear Form"}
    };
    m_dictionary["BTN_ADD_BOOK"] = {
        {Language::Vietnamese, "Thêm Sách"},
        {Language::Khmer, "បន្ថែមសៀវភៅ (Add Book)"},
        {Language::English, "Add Book"}
    };
    m_dictionary["BTN_IMPORT"] = {
        {Language::Vietnamese, "Nhập Excel"},
        {Language::Khmer, "នាំចូល Excel (Import)"},
        {Language::English, "Import Excel"}
    };
    m_dictionary["BTN_IMPORT_DATA"] = {
        {Language::Vietnamese, "Nhập Dữ Liệu"},
        {Language::Khmer, "នាំចូលទិន្នន័យ (Import)"},
        {Language::English, "Import Data"}
    };
    m_dictionary["BTN_EXPORT_PRINT"] = {
        {Language::Vietnamese, "In Ấn ▾"},
        {Language::Khmer, "បោះពុម្ព (Print) ▾"},
        {Language::English, "Print ▾"}
    };
    m_dictionary["BTN_DETAILS"] = {
        {Language::Vietnamese, "Chi Tiết"},
        {Language::Khmer, "លម្អិត (Details)"},
        {Language::English, "Details"}
    };
    m_dictionary["BTN_LIQUIDATE"] = {
        {Language::Vietnamese, "Thanh Lý"},
        {Language::Khmer, "លុបចោល (Liquidate)"},
        {Language::English, "Liquidate"}
    };
    m_dictionary["BTN_DELETE_COPY"] = {
        {Language::Vietnamese, "Xóa (Vĩnh viễn)"},
        {Language::Khmer, "លុប (Delete)"},
        {Language::English, "Delete (Permanent)"}
    };
    m_dictionary["BTN_ADD_COPY"] = {
        {Language::Vietnamese, "Thêm Bản Sao"},
        {Language::Khmer, "បន្ថែមច្បាប់ចម្លង"},
        {Language::English, "Add Copy"}
    };
    m_dictionary["BTN_PRINT_BARCODE"] = {
        {Language::Vietnamese, "In Mã Vạch"},
        {Language::Khmer, "បោះពុម្ពបាកូដ"},
        {Language::English, "Print Barcode"}
    };
    m_dictionary["PLACEHOLDER_SEARCH_BOOK"] = {
        {Language::Vietnamese, "Nhập ISBN, tên sách, tác giả..."},
        {Language::Khmer, "បញ្ចូល ISBN, ឈ្មោះសៀវភៅ, អ្នកនិពន្ធ..."},
        {Language::English, "Enter ISBN, Title, Author..."}
    };
    m_dictionary["TABLE_HEADER_ISBN"] = {
        {Language::Vietnamese, "ISBN"},
        {Language::Khmer, "ISBN"},
        {Language::English, "ISBN"}
    };
    m_dictionary["TABLE_HEADER_TITLE"] = {
        {Language::Vietnamese, "Tên Sách"},
        {Language::Khmer, "ឈ្មោះសៀវភៅ (Title)"},
        {Language::English, "Title"}
    };
    m_dictionary["TABLE_HEADER_AUTHOR"] = {
        {Language::Vietnamese, "Tác Giả"},
        {Language::Khmer, "អ្នកនិពន្ធ (Author)"},
        {Language::English, "Author"}
    };
    m_dictionary["TABLE_HEADER_CATEGORY"] = {
        {Language::Vietnamese, "Thể Loại"},
        {Language::Khmer, "ប្រភេទ (Category)"},
        {Language::English, "Category"}
    };
    m_dictionary["TABLE_HEADER_YEAR"] = {
        {Language::Vietnamese, "Năm XB"},
        {Language::Khmer, "ឆ្នាំ (Year)"},
        {Language::English, "Year"}
    };
    m_dictionary["TABLE_HEADER_PAGES"] = {
        {Language::Vietnamese, "Số Trang"},
        {Language::Khmer, "ទំព័រ (Pages)"},
        {Language::English, "Pages"}
    };
    m_dictionary["TABLE_HEADER_COPIES"] = {
        {Language::Vietnamese, "Số Lượng"},
        {Language::Khmer, "ចំនួន (Copies)"},
        {Language::English, "Copies"}
    };
    m_dictionary["TABLE_HEADER_ACTIONS"] = {
        {Language::Vietnamese, "Thao Tác"},
        {Language::Khmer, "សកម្មភាព (Actions)"},
        {Language::English, "Actions"}
    };

    m_dictionary["LBL_STATUS_AVAILABLE"] = {
        {Language::Vietnamese, "Có sẵn"},
        {Language::Khmer, "អាចខ្ចីបាន"},
        {Language::English, "Available"}
    };
    m_dictionary["LBL_STATUS_BORROWED"] = {
        {Language::Vietnamese, "Đang mượn"},
        {Language::Khmer, "កំពុងខ្ចី"},
        {Language::English, "Borrowed"}
    };
    m_dictionary["LBL_STATUS_LIQUIDATED"] = {
        {Language::Vietnamese, "Đã thanh lý"},
        {Language::Khmer, "បានលុបចោល"},
        {Language::English, "Liquidated"}
    };
    m_dictionary["LBL_STATUS_UNKNOWN"] = {
        {Language::Vietnamese, "Không rõ"},
        {Language::Khmer, "មិនស្គាល់ (Unknown)"},
        {Language::English, "Unknown"}
    };

    m_dictionary["MSG_SELECT_COPY_LIQUIDATE"] = {
        {Language::Vietnamese, "Vui lòng chọn bản sao để thanh lý!"},
        {Language::Khmer, "សូមជ្រើសរើសច្បាប់ចម្លងដើម្បីលុបចោល!"},
        {Language::English, "Please select a copy to liquidate!"}
    };
    m_dictionary["MSG_CONFIRM_LIQUIDATE_TITLE"] = {
        {Language::Vietnamese, "Xác nhận thanh lý"},
        {Language::Khmer, "បញ្ជាក់ការលុបចោល"},
        {Language::English, "Confirm Liquidation"}
    };
    m_dictionary["MSG_CONFIRM_LIQUIDATE_BODY"] = {
        {Language::Vietnamese, "Bạn có chắc muốn thanh lý bản sao %1?"},
        {Language::Khmer, "តើអ្នកច្បាស់ទេថាចង់លុបចោលច្បាប់ចម្លង %1?"},
        {Language::English, "Are you sure you want to liquidate copy %1?"}
    };
    m_dictionary["MSG_LIQUIDATE_SUCCESS"] = {
        {Language::Vietnamese, "Thanh lý thành công!"},
        {Language::Khmer, "បានលុបចោលដោយជោគជ័យ!"},
        {Language::English, "Liquidated successfully!"}
    };
    m_dictionary["DIALOG_ADD_COPY_TITLE"] = {
        {Language::Vietnamese, "Thêm Bản Sao"},
        {Language::Khmer, "បន្ថែមច្បាប់ចម្លង"},
        {Language::English, "Add Copy"}
    };
    m_dictionary["DIALOG_ADD_COPY_MSG"] = {
        {Language::Vietnamese, "Số lượng bản sao cần thêm:"},
        {Language::Khmer, "ចំនួនច្បាប់ចម្លង:"},
        {Language::English, "Number of copies:"}
    };
    m_dictionary["DIALOG_LOCATION_TITLE"] = {
        {Language::Vietnamese, "Vị Trí"},
        {Language::Khmer, "ទីតាំង"},
        {Language::English, "Location"}
    };
    m_dictionary["DIALOG_LOCATION_MSG"] = {
        {Language::Vietnamese, "Nhập vị trí lưu trữ (Tùy chọn):"},
        {Language::Khmer, "បញ្ចូលទីតាំង (Optional):"},
        {Language::English, "Enter location (Optional):"}
    };
    m_dictionary["MSG_ADD_COPY_SUCCESS"] = {
        {Language::Vietnamese, "Thêm bản sao thành công!"},
        {Language::Khmer, "បន្ថែមច្បាប់ចម្លងដោយជោគជ័យ!"},
        {Language::English, "Copy added successfully!"}
    };
    m_dictionary["MSG_ENTER_PRINT_QUANTITY"] = {
        {Language::Vietnamese, "Nhập số lượng tem cần in:"},
        {Language::Khmer, "បញ្ចូលចំនួន:"},
        {Language::English, "Enter quantity:"}
    };

    m_dictionary["LBL_TITLE"] = {
        {Language::Vietnamese, "Tên sách"},
        {Language::Khmer, "ចំណងជើង (Title)"},
        {Language::English, "Title"}
    };
    m_dictionary["LBL_AUTHOR"] = {
        {Language::Vietnamese, "Tác giả"},
        {Language::Khmer, "អ្នកនិពន្ធ (Author)"},
        {Language::English, "Author"}
    };
    m_dictionary["LBL_ISBN"] = {
        {Language::Vietnamese, "Mã ISBN"},
        {Language::Khmer, "លេខ ISBN"},
        {Language::English, "ISBN"}
    };
    m_dictionary["LBL_CATEGORY"] = {
        {Language::Vietnamese, "Thể loại"},
        {Language::Khmer, "ប្រភេទ (Category)"},
        {Language::English, "Category"}
    };
    m_dictionary["LBL_PUBLISH_YEAR"] = {
        {Language::Vietnamese, "Năm xuất bản"},
        {Language::Khmer, "ឆ្នាំបោះពុម្ព"},
        {Language::English, "Publish Year"}
    };
    m_dictionary["LBL_PAGE_COUNT"] = {
        {Language::Vietnamese, "Số trang"},
        {Language::Khmer, "ចំនួនទំព័រ"},
        {Language::English, "Page Count"}
    };


    // === LOAN / RETURN ===
    m_dictionary["PAGE_TITLE_LOAN_RETURN"] = {
        {Language::Vietnamese, "MƯỢN TRẢ SÁCH"},
        {Language::Khmer, "ការខ្ចី-សងសៀវភៅ (Loan/Return)"},
        {Language::English, "LOAN & RETURN"}
    };
    m_dictionary["TAB_BORROW"] = {
        {Language::Vietnamese, "Mượn Sách"},
        {Language::Khmer, "ខ្ចីសៀវភៅ (Borrow)"},
        {Language::English, "Borrow"}
    };
    m_dictionary["TAB_BORROW_RETURN"] = {
        {Language::Vietnamese, "Mượn/Trả Sách"},
        {Language::Khmer, "ខ្ចី/សងសៀវភៅ (Borrow/Return)"},
        {Language::English, "Borrow/Return"}
    };
    m_dictionary["TAB_RETURN"] = {
        {Language::Vietnamese, "Trả Sách"},
        {Language::Khmer, "សងសៀវភៅ (Return)"},
        {Language::English, "Return"}
    };
    // TAB HISTORY
    m_dictionary["TAB_HISTORY"] = {
        {Language::Vietnamese, "Lịch Sử Mượn Trả"},
        {Language::Khmer, "ប្រវត្តិខ្ចីសង"}, // History Loan/Return
        {Language::English, "Loan History"}
    };

    // DIALOG ADD READER
    m_dictionary["DIALOG_ADD_READER_TITLE"] = {
        {Language::Vietnamese, "Thêm Học Viên Mới"},
        {Language::Khmer, "បន្ថែមសិស្សថ្មី"},
        {Language::English, "Add New Student"}
    };
    m_dictionary["DIALOG_EDIT_READER_TITLE"] = {
        {Language::Vietnamese, "Chỉnh Sửa Học Viên"},
        {Language::Khmer, "កែប្រែសិស្ស"},
        {Language::English, "Edit Student"}
    };
    m_dictionary["LBL_READER_CODE"] = {
        {Language::Vietnamese, "Mã Thẻ Học Viên"},
        {Language::Khmer, "លេខកូដប័ណ្ណសិស្ស"},
        {Language::English, "Student Code"}
    };
    m_dictionary["LBL_AUTO_GENERATED"] = {
        {Language::Vietnamese, "Mã thẻ được tạo tự động"},
        {Language::Khmer, "លេខកូដត្រូវបានបង្កើតដោយស្វ័យប្រវត្តិ"},
        {Language::English, "Code generated automatically"}
    };
    m_dictionary["MSG_CODE_READONLY"] = {
        {Language::Vietnamese, "Mã thẻ không thể chỉnh sửa"},
        {Language::Khmer, "លេខកូដមិនអាចកែប្រែបានទេ"},
        {Language::English, "Code cannot be edited"}
    };
    m_dictionary["LBL_LASTNAME"] = {
        {Language::Vietnamese, "Họ Đệm"},
        {Language::Khmer, "ត្រកូល"},
        {Language::English, "Last Name"}
    };
    m_dictionary["LBL_FIRSTNAME"] = {
        {Language::Vietnamese, "Tên"},
        {Language::Khmer, "ឈ្មោះ"},
        {Language::English, "First Name"}
    };
    m_dictionary["LBL_GENDER"] = {
        {Language::Vietnamese, "Giới Tính"},
        {Language::Khmer, "ភេទ"},
        {Language::English, "Gender"}
    };
    m_dictionary["BTN_CLEAR_FORM"] = {
        {Language::Vietnamese, "Xóa Form"},
        {Language::Khmer, "លុប"},
        {Language::English, "Clear Form"}
    };
    m_dictionary["BTN_SAVE_READER"] = {
        {Language::Vietnamese, "Thêm Học Viên"},
        {Language::Khmer, "បន្ថែម"},
        {Language::English, "Add Student"}
    };
    m_dictionary["BTN_SAVE_CHANGES"] = {
        {Language::Vietnamese, "Lưu Thay Đổi"},
        {Language::Khmer, "រក្សាទុកការផ្លាស់ប្តូរ"},
        {Language::English, "Save Changes"}
    };
    m_dictionary["BTN_CANCEL_DIALOG"] = { // Renamed to avoid conflict with existing BTN_CANCEL
        {Language::Vietnamese, "Hủy"},
        {Language::Khmer, "បោះបង់"},
        {Language::English, "Cancel"}
    };
    // ERRORS / MESSAGES
    m_dictionary["MSG_MISSING_INFO"] = {
        {Language::Vietnamese, "Vui lòng nhập đầy đủ thông tin!"},
        {Language::Khmer, "សូមបញ្ចូលព័ត៌មានឱ្យបានពេញលេញ!"},
        {Language::English, "Please enter full information!"}
    };
    m_dictionary["MSG_SUCCESS_ADD"] = {
        {Language::Vietnamese, "Đã thêm học viên thành công!"},
        {Language::Khmer, "បានបន្ថែមសិស្សដោយជោគជ័យ!"},
        {Language::English, "Student added successfully!"}
    };
    m_dictionary["MSG_ERROR_ADD"] = {
        {Language::Vietnamese, "Không thể thêm học viên. Mã thẻ có thể đã tồn tại!"},
        {Language::Khmer, "មិនអាចបន្ថែមសិស្សបានទេ។ លេខកូដអាចមានរួចហើយ!"},
        {Language::English, "Cannot add student. Code may already exist!"}
    };
    m_dictionary["TITLE_WARNING"] = {
        {Language::Vietnamese, "Thiếu Thông Tin"},
        {Language::Khmer, "ព័ត៌មានមិនគ្រប់គ្រាន់"},
        {Language::English, "Missing Information"}
    };
    m_dictionary["TITLE_SUCCESS"] = {
        {Language::Vietnamese, "Thành Công"},
        {Language::Khmer, "ជោគជ័យ"},
        {Language::English, "Success"}
    };
    m_dictionary["TITLE_ERROR"] = {
        {Language::Vietnamese, "Lỗi"},
        {Language::Khmer, "កំហុស"},
        {Language::English, "Error"}
    };
    m_dictionary["MSG_MISSING_LASTNAME"] = {
        {Language::Vietnamese, "Vui lòng nhập họ đệm!"},
        {Language::Khmer, "សូមបញ្ចូលត្រកូល!"},
        {Language::English, "Please enter last name!"}
    };
    m_dictionary["MSG_MISSING_FIRSTNAME"] = {
        {Language::Vietnamese, "Vui lòng nhập tên!"},
        {Language::Khmer, "សូមបញ្ចូលឈ្មោះ!"},
        {Language::English, "Please enter first name!"}
    };
    m_dictionary["BTN_SCAN_CARD"] = {
        {Language::Vietnamese, "Quét Thẻ"},
        {Language::Khmer, "ស្កេនកាត (Scan Card)"},
        {Language::English, "Scan Card"}
    };
    m_dictionary["BTN_SCAN_BOOK"] = {
        {Language::Vietnamese, "Quét Sách"},
        {Language::Khmer, "ស្កេនសៀវភៅ (Scan Book)"},
        {Language::English, "Scan Book"}
    };
    m_dictionary["GROUP_READER_INFO"] = {
        {Language::Vietnamese, "THÔNG TIN HỌC VIÊN"},
        {Language::Khmer, "ព័ត៌មានសិស្ស (Student Info)"},
        {Language::English, "STUDENT INFO"}
    };
    m_dictionary["GROUP_PERSONAL_INFO"] = {
        {Language::Vietnamese, "THÔNG TIN CÁ NHÂN"},
        {Language::Khmer, "ព័ត៌មានផ្ទាល់ខ្លួន (Personal Info)"},
        {Language::English, "PERSONAL INFORMATION"}
    };
    m_dictionary["GROUP_BOOKS_BORROWED"] = {
        {Language::Vietnamese, "SÁCH ĐANG MƯỢN"},
        {Language::Khmer, "សៀវភៅដែលកំពុងខ្ចី (Borrowed Books)"},
        {Language::English, "BORROWED BOOKS"}
    };
    m_dictionary["MSG_CONFIRM_CLEAR"] = {
        {Language::Vietnamese, "Bạn có chắc muốn xóa tất cả dữ liệu đã nhập?"},
        {Language::Khmer, "តើអ្នកពិតជារចង់លុបទិន្នន័យដែលបានបញ្ចូលទាំងអស់ឬ?"},
        {Language::English, "Are you sure you want to clear all data?"}
    };
    m_dictionary["MSG_TRY_AGAIN"] = {
        {Language::Vietnamese, "Vui lòng thử lại."},
        {Language::Khmer, "សូមព្យាយាមម្ដងទៀត។"},
        {Language::English, "Please try again."}
    };
    m_dictionary["MSG_READER_NOT_FOUND"] = {
        {Language::Vietnamese, "Không tìm thấy thông tin học viên!"},
        {Language::Khmer, "រកមិនឃើញព័ត៌មានសិស្ស!"},
        {Language::English, "Student information not found!"}
    };
    m_dictionary["MSG_SUCCESS_UPDATE"] = {
        {Language::Vietnamese, "Đã cập nhật thông tin học viên thành công!"},
        {Language::Khmer, "ព័ត៌មានសិស្សត្រូវបានធ្វើបច្ចុប្បន្នភាពដោយជោគជ័យ!"},
        {Language::English, "Student updated successfully!"}
    };
    m_dictionary["MSG_CANNOT_LOCK_BORROWING"] = {
        {Language::Vietnamese, "Không thể khóa học viên đang mượn %1 cuốn sách!"},
        {Language::Khmer, "មិនអាចចាក់សោសិស្សដែលកំពុងខ្ចីសៀវភៅ %1 ក្បាលបានទេ!"},
        {Language::English, "Cannot lock student borrowing %1 books!"}
    };
    m_dictionary["MSG_WARNING_LOCK"] = {
        {Language::Vietnamese, "Không Thể Khóa"},
        {Language::Khmer, "មិនអាចចាក់សោបានទេ"},
        {Language::English, "Cannot Lock"}
    };
    m_dictionary["LBL_STATUS"] = {
        {Language::Vietnamese, "Trạng Thái"},
        {Language::Khmer, "ស្ថានភាព"},
        {Language::English, "Status"}
    };
    m_dictionary["GROUP_STATS"] = {
        {Language::Vietnamese, "Thống Kê"},
        {Language::Khmer, "ស្ថិតិ"},
        {Language::English, "Statistics"}
    };
    m_dictionary["LBL_BORROWED_COUNT"] = {
        {Language::Vietnamese, "Số sách đang mượn: <b>%1</b>"},
        {Language::Khmer, "ចំនួនសៀវភៅដែលខ្ចី: <b>%1</b>"},
        {Language::English, "Borrowed books: <b>%1</b>"}
    };
    m_dictionary["BTN_CONFIRM_BORROW"] = {
        {Language::Vietnamese, "MƯỢN"},
        {Language::Khmer, "ខ្ចី (BORROW)"},
        {Language::English, "BORROW"}
    };
    m_dictionary["BTN_CONFIRM_RETURN"] = {
        {Language::Vietnamese, "TRẢ"},
        {Language::Khmer, "សង (RETURN)"},
        {Language::English, "RETURN"}
    };
    m_dictionary["BTN_LOST"] = {
        {Language::Vietnamese, "BÁO MẤT"},
        {Language::Khmer, "រាយការណ៍ថាបាត់ (LOST)"},
        {Language::English, "REPORT LOST"}
    };
    m_dictionary["BTN_EXTEND"] = {
        {Language::Vietnamese, "GIA HẠN"},
        {Language::Khmer, "ពន្យារពេល (EXTEND)"},
        {Language::English, "EXTEND"}
    };
    m_dictionary["BTN_PAY"] = {
        {Language::Vietnamese, "THANH TOÁN"},
        {Language::Khmer, "បង់ប្រាក់ (PAY)"},
        {Language::English, "PAY"}
    };
    m_dictionary["BTN_REPRINT"] = {
        {Language::Vietnamese, "In Phiếu"},
        {Language::Khmer, "បោះពុម្ពបង្កាន់ដៃ (Print Receipt)"},
        {Language::English, "Print Receipt"}
    };
    m_dictionary["LBL_TOTAL_READERS"] = {
        {Language::Vietnamese, "Tổng: %1 học viên"},
        {Language::Khmer, "សរុប: %1 សិស្ស"},
        {Language::English, "Total: %1 students"}
    };

    // === READER MANAGEMENT ===
    m_dictionary["PAGE_TITLE_READERS"] = {
        {Language::Vietnamese, "Quản Lý Học Viên"},
        {Language::Khmer, "គ្រប់គ្រងសិស្ស (Students)"},
        {Language::English, "Student Management"}
    };
    m_dictionary["BTN_ADD_READER"] = {
        {Language::Vietnamese, "Thêm Học Viên"},
        {Language::Khmer, "បន្ថែមសិស្ស (Add Student)"},
        {Language::English, "Add Student"}
    };
    m_dictionary["PLACEHOLDER_SEARCH_READER"] = {
        {Language::Vietnamese, "Tìm kiếm theo Mã thẻ, Họ tên..."},
        {Language::Khmer, "ស្វែងរកតាមលេខកូដ, ឈ្មោះ..."},
        {Language::English, "Search by ID, Name..."}
    };
    m_dictionary["TABLE_HEADER_CODE"] = {
        {Language::Vietnamese, "Mã Thẻ"},
        {Language::Khmer, "លេខកូដ (ID)"},
        {Language::English, "Student ID"}
    };
    m_dictionary["TABLE_HEADER_LASTNAME"] = {
        {Language::Vietnamese, "Họ"},
        {Language::Khmer, "ត្រកូល (Last Name)"},
        {Language::English, "Last Name"}
    };
    m_dictionary["TABLE_HEADER_FIRSTNAME"] = {
        {Language::Vietnamese, "Tên"},
        {Language::Khmer, "ឈ្មោះ (First Name)"},
        {Language::English, "First Name"}
    };
    m_dictionary["TABLE_HEADER_GENDER"] = {
        {Language::Vietnamese, "Phái"},
        {Language::Khmer, "ភេទ (Gender)"},
        {Language::English, "Gender"}
    };
    m_dictionary["TABLE_HEADER_STATUS"] = {
        {Language::Vietnamese, "Trạng Thái"},
        {Language::Khmer, "ស្ថានភាព (Status)"},
        {Language::English, "Status"}
    };
    m_dictionary["BTN_CLOSE"] = {
        {Language::Vietnamese, "Đóng"},
        {Language::Khmer, "បិទ"},
        {Language::English, "Close"}
    };
    m_dictionary["DIALOG_OVERDUE_TITLE"] = {
        {Language::Vietnamese, "DANH SÁCH HỌC VIÊN QUÁ HẠN"},
        {Language::Khmer, "បញ្ជីសិស្សដែលហួសកំណត់ (Overdue List)"},
        {Language::English, "OVERDUE STUDENT LIST"}
    };
    m_dictionary["MSG_OVERDUE_WARNING_HTML"] = {
        {Language::Vietnamese, "<b>Cảnh báo:</b> Những học viên dưới đây đang mượn sách quá hạn. Vui lòng liên hệ để nhắc nhở trả sách!"},
        {Language::Khmer, "<b>ការព្រមាន:</b> សិស្សខាងក្រោមនេះកំពុងខ្ចីសៀវភៅហួសកំណត់។ សូមទាក់ទងដើម្បីរំលឹក!"},
        {Language::English, "<b>Warning:</b> The following students have overdue books. Please contact them!"}
    };
    m_dictionary["DIALOG_STATS_TITLE"] = {
        {Language::Vietnamese, "Thống Kê Hệ Thống"},
        {Language::Khmer, "ស្ថិតិប្រព័ន្ធ (System Stats)"},
        {Language::English, "System Statistics"}
    };
    m_dictionary["TITLE_STATS_HEADER"] = {
        {Language::Vietnamese, "THỐNG KÊ HỆ THỐNG THƯ VIỆN"},
        {Language::Khmer, "ស្ថិតិប្រព័ន្ធបណ្ណាល័យ"},
        {Language::English, "LIBRARY SYSTEM STATISTICS"}
    };
    m_dictionary["MSG_STATS_NOTE_HTML"] = {
        {Language::Vietnamese, "<b>Lưu ý:</b> Thống kê được cập nhật tự động từ dữ liệu hiện tại trong hệ thống"},
        {Language::Khmer, "<b>ចំណាំ:</b> ស្ថិតិត្រូវបានធ្វើបច្ចុប្បន្នភាពដោយស្វ័យប្រវត្តិពីទិន្នន័យបច្ចុប្បន្ន"},
        {Language::English, "<b>Note:</b> Statistics are automatically updated from current system data"}
    };
    m_dictionary["LBL_TOTAL_TITLES"] = {
        {Language::Vietnamese, "Tổng số đầu sách:"}, // Keep icons? Yes.
        {Language::Khmer, "ចំនួនចំណងជើងសៀវភៅសរុប:"},
        {Language::English, "Total Book Titles:"}
    };
    m_dictionary["LBL_TOTAL_COPIES"] = {
        {Language::Vietnamese, "Tổng số bản sao:"},
        {Language::Khmer, "ចំនួនច្បាប់ចម្លងសរុប:"},
        {Language::English, "Total Copies:"}
    };
    m_dictionary["LBL_AVAILABLE_COPIES"] = {
        {Language::Vietnamese, "Có thể mượn:"},
        {Language::Khmer, "អាចខ្ចីបាន:"},
        {Language::English, "Available to Borrow:"}
    };
    m_dictionary["LBL_BORROWED_COPIES"] = {
        {Language::Vietnamese, "Đang được mượn:"},
        {Language::Khmer, "កំពុងត្រូវបានខ្ចី:"},
        {Language::English, "Currently Borrowed:"}
    };
    m_dictionary["LBL_LIQUIDATED_COPIES"] = {
        {Language::Vietnamese, "Đã thanh lý:"},
        {Language::Khmer, "បានលុបចោល (Liquidated):"},
        {Language::English, "Liquidated:"}
    };
    m_dictionary["LBL_BORROW_RATE"] = {
        {Language::Vietnamese, "Tỷ lệ cho mượn:"},
        {Language::Khmer, "អត្រាខ្ចី:"},
        {Language::English, "Borrow Rate:"}
    };
    m_dictionary["LBL_TOTAL_CATEGORIES"] = {
        {Language::Vietnamese, "Số thể loại khác nhau:"},
        {Language::Khmer, "ចំនួនប្រភេទផ្សេងគ្នា:"},
        {Language::English, "Total Categories:"}
    };
    // Reader stats reuse existing? Or specific?
    // "Tổng số học viên" -> LBL_TOTAL_READERS.
    m_dictionary["LBL_TOTAL_READERS"] = {
        {Language::Vietnamese, "Tổng số học viên:"},
        {Language::Khmer, "ចំនួនសិស្សសរុប:"},
        {Language::English, "Total Students:"}
    };
    m_dictionary["LBL_ACTIVE_READERS"] = {
        {Language::Vietnamese, "Đang hoạt động:"},
        {Language::Khmer, "សកម្ម (Active):"},
        {Language::English, "Active:"}
    };
    m_dictionary["LBL_LOCKED_READERS"] = {
        {Language::Vietnamese, "Bị khóa:"},
        {Language::Khmer, "ត្រូវបានបិទ (Locked):"},
        {Language::English, "Locked:"}
    };
    m_dictionary["LBL_READERS_BORROWING"] = {
        {Language::Vietnamese, "Đang mượn sách:"},
        {Language::Khmer, "កំពុងខ្ចីសៀវភៅ:"},
        {Language::English, "Borrowing Books:"}
    };
    m_dictionary["LBL_TOTAL_BORROWED_BOOKS"] = {
        {Language::Vietnamese, "Tổng sách đang mượn:"},
        {Language::Khmer, "សៀវភៅខ្ចីសរុប:"},
        {Language::English, "Total Borrowed Books:"}
    };
    m_dictionary["LBL_READERS_OVERDUE"] = {
        {Language::Vietnamese, "Có sách quá hạn:"},
        {Language::Khmer, "មានសៀវភៅហួសកំណត់:"},
        {Language::English, "Has Overdue Books:"}
    };
    m_dictionary["GROUP_STATS_BOOKS"] = {
        {Language::Vietnamese, "Thống Kê Sách"},
        {Language::Khmer, "ស្ថិតិសៀវភៅ"},
        {Language::English, "Book Statistics"}
    };
    m_dictionary["GROUP_STATS_READERS"] = {
        {Language::Vietnamese, "Thống Kê Học Viên"},
        {Language::Khmer, "ស្ថិតិសិស្ស"},
        {Language::English, "Reader Statistics"}
    };
    m_dictionary["TITLE_INFO"] = {
        {Language::Vietnamese, "Thông Báo"},
        {Language::Khmer, "ជូនដំណឹង"},
        {Language::English, "Info"}
    };
    m_dictionary["TITLE_ERROR_NAME"] = {
        {Language::Vietnamese, "Lỗi Họ Đệm"},
        {Language::Khmer, "កំហុសនាមត្រកូល"},
        {Language::English, "Last Name Error"}
    };
    m_dictionary["TITLE_ERROR_FIRSTNAME"] = {
        {Language::Vietnamese, "Lỗi Tên"},
        {Language::Khmer, "កំហុសឈ្មោះ"},
        {Language::English, "First Name Error"}
    };
    m_dictionary["TITLE_ERROR_SYSTEM"] = {
        {Language::Vietnamese, "Lỗi Hệ Thống"},
        {Language::Khmer, "កំហុសប្រព័ន្ធ"},
        {Language::English, "System Error"}
    };
    m_dictionary["TITLE_ERROR_UNKNOWN"] = {
        {Language::Vietnamese, "Lỗi Không Xác Định"},
        {Language::Khmer, "កំហុសមិនស្គាល់"},
        {Language::English, "Unknown Error"}
    };
    m_dictionary["MSG_ERROR_NAME_FORMAT_HINT"] = {
        {Language::Vietnamese, "\n\nHọ đệm chỉ được chứa chữ cái và khoảng trắng, tối đa 50 ký tự."},
        {Language::Khmer, "\n\nនាមត្រកូលអាចមានតែអក្សរ និងដកឃ្លាប៉ុណ្ណោះ (អតិបរមា ៥០ តួ)។"},
        {Language::English, "\n\nLast name only letters/spaces allowed (max 50 chars)."}
    };
    m_dictionary["MSG_ERROR_FIRSTNAME_FORMAT_HINT"] = {
        {Language::Vietnamese, "\n\nTên chỉ được chứa chữ cái và khoảng trắng, tối đa 50 ký tự."},
        {Language::Khmer, "\n\nឈ្មោះអាចមានតែអក្សរ និងដកឃ្លាប៉ុណ្ណោះ (អតិបរមា ៥០ តួ)។"},
        {Language::English, "\n\nFirst name only letters/spaces allowed (max 50 chars)."}
    };
    m_dictionary["MSG_ERROR_UPDATE_FAILED"] = {
        {Language::Vietnamese, "Đã xảy ra lỗi khi cập nhật học viên:\n%1"},
        {Language::Khmer, "មានកំហុសក្នុងការកែប្រែទិន្នន័យសិស្ស:\n%1"},
        {Language::English, "Error updating reader:\n%1"}
    };
    m_dictionary["MSG_ERROR_ADD_FAILED"] = {
        {Language::Vietnamese, "Đã xảy ra lỗi khi thêm học viên:\n%1"},
        {Language::Khmer, "មានកំហុសក្នុងការបន្ថែមសិស្ស:\n%1"},
        {Language::English, "Error adding reader:\n%1"}
    };
    m_dictionary["MSG_ERROR_UNKNOWN_ACTION"] = {
        {Language::Vietnamese, "Đã xảy ra lỗi không xác định!"},
        {Language::Khmer, "មានកំហុសមិនស្គាល់មួយបានកើតឡើង!"},
        {Language::English, "An unknown error occurred!"}
    };
    m_dictionary["MSG_SELECT_COPY_DELETE"] = {
        {Language::Vietnamese, "Vui lòng chọn bản sao để xóa!"},
        {Language::Khmer, "សូមជ្រើសរើសច្បាប់ចម្លងដើម្បីលុប!"},
        {Language::English, "Please select a copy to delete!"}
    };
    m_dictionary["MSG_CONFIRM_DELETE_COPY_TITLE"] = {
        {Language::Vietnamese, "Xác nhận xóa bản sao"},
        {Language::Khmer, "បញ្ជាក់ការលុបច្បាប់ចម្លង"},
        {Language::English, "Confirm Delete Copy"}
    };
    m_dictionary["MSG_CONFIRM_DELETE_COPY_BODY"] = {
        {Language::Vietnamese, "Bạn có chắc chắn muốn xóa bản sao %1 không? Hành động này không thể hoàn tác!"},
        {Language::Khmer, "តើអ្នកច្បាស់ទេថាចង់លុបច្បាប់ចម្លង %1? សកម្មភាពនេះមិនអាចត្រឡប់វិញបានទេ!"},
        {Language::English, "Are you sure you want to delete copy %1? This action cannot be undone!"}
    };
    m_dictionary["MSG_DELETE_COPY_SUCCESS"] = {
        {Language::Vietnamese, "Xóa bản sao thành công!"},
        {Language::Khmer, "បានលុបចោលច្បាប់ចម្លងដោយជោគជ័យ!"},
        {Language::English, "Copy deleted successfully!"}
    };
    m_dictionary["MSG_REFRESH_SUCCESS"] = {
        {Language::Vietnamese, "Đã làm mới dữ liệu!"},
        {Language::Khmer, "ទិន្នន័យត្រូវបានធ្វើបច្ចុប្បន្នភាព!"},
        {Language::English, "Data refreshed!"}
    };
     m_dictionary["MSG_ERROR_SEARCH"] = {
        {Language::Vietnamese, "Lỗi khi tìm kiếm!"},
        {Language::Khmer, "កំហុសក្នុងការស្វែងរក! (Error)"},
        {Language::English, "Search Error!"}
    };
    m_dictionary["LBL_DAYS"] = {
        {Language::Vietnamese, "ngày"},
        {Language::Khmer, "ថ្ងៃ"},
        {Language::English, "days"}
    };
    m_dictionary["MSG_ERROR_SEARCH_READER"] = {
        {Language::Vietnamese, "Lỗi khi tìm kiếm học viên!"},
        {Language::Khmer, "កំហុសក្នុងការស្វែងរកសិស្ស!"},
        {Language::English, "Error searching student!"}
    };
    m_dictionary["MSG_READER_NOT_FOUND_SIMPLE"] = {
        {Language::Vietnamese, "Không tìm thấy học viên!"},
        {Language::Khmer, "រកមិនឃើញសិស្សទេ!"},
        {Language::English, "Student not found!"}
    };
    m_dictionary["MSG_ENTER_NAME"] = {
        {Language::Vietnamese, "Vui lòng nhập tên!"},
        {Language::Khmer, "សូមបញ្ចូលឈ្មោះ!"},
        {Language::English, "Please enter first name!"}
    };
    m_dictionary["MSG_ENTER_LASTNAME"] = {
        {Language::Vietnamese, "Vui lòng nhập họ!"},
        {Language::Khmer, "សូមបញ្ចូលនាមត្រកូល!"},
        {Language::English, "Please enter last name!"}
    };
    m_dictionary["MSG_ENTER_FULLNAME"] = {
        {Language::Vietnamese, "Vui lòng nhập họ tên!"},
        {Language::Khmer, "សូមបញ្ចូលឈ្មោះពេញ!"},
        {Language::English, "Please enter full name!"}
    };
    m_dictionary["MSG_SEARCHING_PROGRESS"] = {
        {Language::Vietnamese, "Đang tìm kiếm..."},
        {Language::Khmer, "កំពុងស្វែងរក..."},
        {Language::English, "Searching..."}
    };
    m_dictionary["TITLE_FILTER_STATUS"] = {
        {Language::Vietnamese, "Lọc theo trạng thái"},
        {Language::Khmer, "ត្រងតាមស្ថានភាព"},
        {Language::English, "Filter by Status"}
    };
    m_dictionary["LBL_CHOOSE_STATUS"] = {
        {Language::Vietnamese, "Chọn trạng thái:"},
        {Language::Khmer, "ជ្រើសរើសស្ថានភាព:"},
        {Language::English, "Choose Status:"}
    };
    m_dictionary["OPTION_STATUS_ACTIVE"] = {
        {Language::Vietnamese, "1 - Hoạt động"},
        {Language::Khmer, "1 - សកម្ម"},
        {Language::English, "1 - Active"}
    };
    m_dictionary["OPTION_STATUS_LOCKED"] = {
        {Language::Vietnamese, "0 - Bị khóa"},
        {Language::Khmer, "0 - ត្រូវបានបិទ"},
        {Language::English, "0 - Locked"}
    };
    m_dictionary["MSG_FILTERING_PROGRESS"] = {
        {Language::Vietnamese, "Đang lọc..."},
        {Language::Khmer, "កំពុងត្រង..."},
        {Language::English, "Filtering..."}
    };
    m_dictionary["MSG_ERROR_SEARCH_INVALID_RESULT"] = {
        {Language::Vietnamese, "Kết quả tìm kiếm không hợp lệ!"},
        {Language::Khmer, "លទ្ធផលស្វែងរកមិនត្រឹមត្រូវ!"},
        {Language::English, "Invalid search result!"}
    };
    m_dictionary["MSG_FOUND_RESULTS"] = {
        {Language::Vietnamese, "Tìm thấy %1 kết quả"},
        {Language::Khmer, "រកឃើញ %1 លទ្ធផល"},
        {Language::English, "Found %1 results"}
    };
    m_dictionary["MSG_ENTER_VALID_YEAR"] = {
        {Language::Vietnamese, "Vui lòng nhập năm hợp lệ (VD: 2020)!"},
        {Language::Khmer, "សូមបញ្ចូលឆ្នាំត្រឹមត្រូវ (ឧទាហរណ៍: 2020)!"},
        {Language::English, "Please enter a valid year (e.g. 2020)!"}
    };
    m_dictionary["MSG_ERROR_SEARCH_YEAR"] = {
        {Language::Vietnamese, "Lỗi khi tìm kiếm theo năm!"},
        {Language::Khmer, "កំហុសស្វែងរកតាមឆ្នាំ!"},
        {Language::English, "Error searching by year!"}
    };
    m_dictionary["MSG_FOUND_BOOKS_YEAR"] = {
        {Language::Vietnamese, "Tìm thấy %1 sách xuất bản năm %2"},
        {Language::Khmer, "រកឃើញសៀវភៅ %1 ដែលបោះពុម្ពនៅឆ្នាំ %2"},
        {Language::English, "Found %1 books published in %2"}
    };
    m_dictionary["MSG_ENTER_YEAR_RANGE"] = {
        {Language::Vietnamese, "Vui lòng nhập khoảng năm (VD: 2015-2020)!"},
        {Language::Khmer, "សូមបញ្ចូលចន្លោះឆ្នាំ (ឧទាហរណ៍: 2015-2020)!"},
        {Language::English, "Please enter year range (e.g. 2015-2020)!"}
    };
    m_dictionary["MSG_INVALID_YEAR_RANGE"] = {
        {Language::Vietnamese, "Khoảng năm không hợp lệ!"},
        {Language::Khmer, "ចន្លោះឆ្នាំមិនត្រឹមត្រូវ!"},
        {Language::English, "Invalid year range!"}
    };

    // HopThoaiCauHinhIn
    m_dictionary["DIALOG_CONFIG_PRINT_TITLE"] = { {Language::Vietnamese, "Cấu Hình In Ấn"}, {Language::Khmer, "ការកំណត់ការបោះពុម្ព"}, {Language::English, "Print Configuration"} };
    m_dictionary["LBL_CONFIG_LIB_NAME"] = { {Language::Vietnamese, "Tên thư viện:"}, {Language::Khmer, "ឈ្មោះបណ្ណាល័យ:"}, {Language::English, "Library Name:"} };
    m_dictionary["LBL_CONFIG_LIB_ABBR"] = { {Language::Vietnamese, "Tên viết tắt:"}, {Language::Khmer, "ឈ្មោះកាត់:"}, {Language::English, "Abbreviation:"} };
    m_dictionary["CB_PRINT_COLOR"] = { {Language::Vietnamese, "In màu sắc"}, {Language::Khmer, "បោះពុម្ពពណ៌"}, {Language::English, "Color Print"} };
    m_dictionary["CB_SHOW_LOGO"] = { {Language::Vietnamese, "Hiển thị logo"}, {Language::Khmer, "បង្ហាញរូបសញ្ញា"}, {Language::English, "Show Logo"} };
    m_dictionary["CB_SILENT_PRINT"] = { {Language::Vietnamese, "Tự động in (Không hỏi)"}, {Language::Khmer, "បោះពុម្ពដោយស្វ័យប្រវត្តិ (មិនសួរ)"}, {Language::English, "Silent Print (No Prompt)"} };
    m_dictionary["LBL_LOGO_PATH"] = { {Language::Vietnamese, "Đường dẫn logo:"}, {Language::Khmer, "ផ្លូវរូបសញ្ញា:"}, {Language::English, "Logo Path:"} };
    m_dictionary["LBL_QR_OUTPUT_DIR"] = { {Language::Vietnamese, "Thư mục QR (tương đối/absolute):"}, {Language::Khmer, "ថត QR (ទំនាក់ទំនង/ដាច់ខាត):"}, {Language::English, "QR Directory (Relative/Absolute):"} };
    m_dictionary["LBL_PDF_OUTPUT_DIR"] = { {Language::Vietnamese, "Thư mục PDF (tương đối/absolute):"}, {Language::Khmer, "ថត PDF (ទំនាក់ទំនង/ដាច់ខាត):"}, {Language::English, "PDF Directory (Relative/Absolute):"} };
    m_dictionary["BTN_CHOOSE"] = { {Language::Vietnamese, "Chọn..."}, {Language::Khmer, "ជ្រើសរើស..."}, {Language::English, "Browse..."} };
    // === MISSING KEYS ADDED ===
    m_dictionary["TABLE_HEADER_BOOK_CODE"] = { {Language::Vietnamese, "Mã Sách"}, {Language::Khmer, "លេខកូដសៀវភៅ"}, {Language::English, "Book Code"} };
    m_dictionary["TABLE_HEADER_OVERDUE"] = { {Language::Vietnamese, "Quá Hạn"}, {Language::Khmer, "ហួសកំណត់"}, {Language::English, "Overdue"} };
    // === MISSING KEYS ADDED FOR READER DETAILS ===
    m_dictionary["TABLE_HEADER_BORROW_DATE"] = { {Language::Vietnamese, "Ngày Mượn"}, {Language::Khmer, "ថ្ងៃខ្ចី"}, {Language::English, "Borrow Date"} };
    m_dictionary["TABLE_HEADER_DUE_DATE"] = { {Language::Vietnamese, "Hạn Trả"}, {Language::Khmer, "ថ្ងៃសង"}, {Language::English, "Due Date"} };
    m_dictionary["TABLE_HEADER_STATUS"] = { {Language::Vietnamese, "Trạng Thái"}, {Language::Khmer, "ស្ថានភាព"}, {Language::English, "Status"} };
    m_dictionary["LBL_BORROWING"] = { {Language::Vietnamese, "Đang Mượn"}, {Language::Khmer, "កំពុងខ្ចី"}, {Language::English, "Borrowing"} };
    m_dictionary["STATUS_RETURNED"] = { {Language::Vietnamese, "Đã Trả"}, {Language::Khmer, "បានសង"}, {Language::English, "Returned"} };
    m_dictionary["MSG_WARN_RETURN_NOT_BORROWED"] = { {Language::Vietnamese, "Sách này không ở trạng thái đang mượn!"}, {Language::Khmer, "សៀវភៅនេះមិនស្ថិតក្នុងស្ថានភាពកំពុងខ្ចីទេ!"}, {Language::English, "This book is not currently borrowed!"} };
    m_dictionary["MSG_CONFIRM_RETURN_TITLE"] = { {Language::Vietnamese, "Xác Nhận Trả Sách"}, {Language::Khmer, "បញ្ជាក់ការសងសៀវភៅ"}, {Language::English, "Confirm Return"} };
     m_dictionary["MSG_CONFIRM_RETURN_BOOK"] = { {Language::Vietnamese, "Bạn có chắc muốn trả sách %1?"}, {Language::Khmer, "តើអ្នកពិតជារចង់សងសៀវភៅ %1 ដែរឬទេ?"}, {Language::English, "Are you sure you want to return book %1?"} };
    m_dictionary["MSG_RETURN_SUCCESS"] = { {Language::Vietnamese, "Trả sách thành công!"}, {Language::Khmer, "សងសៀវភៅដោយជោគជ័យ!"}, {Language::English, "Return successful!"} };
    m_dictionary["MSG_WARN_LOST_NOT_BORROWED"] = { {Language::Vietnamese, "Sách này không ở trạng thái đang mượn để báo mất!"}, {Language::Khmer, "សៀវភៅនេះមិនស្ថិតក្នុងស្ថានភាពកំពុងខ្ចីដើម្បីរាយការណ៍ថាបាត់ទេ!"}, {Language::English, "Book not borrowed, cannot report lost!"} };
    m_dictionary["MSG_CONFIRM_LOST_TITLE"] = { {Language::Vietnamese, "Xác Nhận Báo Mất"}, {Language::Khmer, "បញ្ជាក់ការបាត់បង់"}, {Language::English, "Confirm Lost"} };
    m_dictionary["MSG_CONFIRM_LOST_HTML"] = { {Language::Vietnamese, "Sách: <b>%1</b><br>Giá sách: %2 VNĐ<br>Phí phạt: %3 VNĐ<br><b>Tổng cộng: %4 VNĐ</b><br><br>Bạn có chắc chắn muốn báo mất sách này?"}, {Language::Khmer, "សៀវភៅ: <b>%1</b><br>តម្លៃ: %2 VNĐ<br>ពិន័យ: %3 VNĐ<br><b>សរុប: %4 VNĐ</b><br><br>តើអ្នកពិតជារចង់រាយការណ៍ថាបាត់សៀវភៅនេះមែនទេ?"}, {Language::English, "Book: <b>%1</b><br>Price: %2<br>Fine: %3<br><b>Total: %4</b><br><br>Report lost?"} };
    m_dictionary["MSG_LOST_RECORDED_TITLE"] = { {Language::Vietnamese, "Đã Ghi Nhận"}, {Language::Khmer, "បានកត់ត្រា"}, {Language::English, "Recorded"} };
    m_dictionary["MSG_LOST_RECORDED_SUCCESS"] = { {Language::Vietnamese, "Đã báo mất sách %1.<br>Thu phí: %4 VNĐ"}, {Language::Khmer, "បានរាយការណ៍ថាបាត់សៀវភៅ %1.<br>កម្រៃសេវា: %4 VNĐ"}, {Language::English, "Reported lost %1.<br>Total: %4"} };
    m_dictionary["TITLE_READER_DETAILS"] = { {Language::Vietnamese, "Chi Tiết Học Viên"}, {Language::Khmer, "ព័ត៌មានលម្អិតសិស្ស"}, {Language::English, "Reader Details"} };
    m_dictionary["HEADER_READER_DETAILS"] = { {Language::Vietnamese, "CHI TIẾT HỌC VIÊN"}, {Language::Khmer, "ព័ត៌មានលម្អិតសិស្ស"}, {Language::English, "READER DETAILS"} };
    m_dictionary["GROUP_HISTORY"] = { {Language::Vietnamese, "LỊCH SỬ MƯỢN TRẢ"}, {Language::Khmer, "ប្រវត្តិខ្ចីសង"}, {Language::English, "LOAN HISTORY"} };
    m_dictionary["BTN_RETURN_BOOK"] = { {Language::Vietnamese, "Trả Sách"}, {Language::Khmer, "សងសៀវភៅ"}, {Language::English, "Return Book"} };
    m_dictionary["BTN_REPORT_LOST"] = { {Language::Vietnamese, "Báo Mất"}, {Language::Khmer, "រាយការណ៍ថាបាត់"}, {Language::English, "Report Lost"} };
    m_dictionary["BTN_PRINT_CARD_TV"] = { {Language::Vietnamese, "In Thẻ TV"}, {Language::Khmer, "បោះពុម្ពកាត"}, {Language::English, "Print Card"} };
    m_dictionary["MSG_READER_INFO_HTML"] = { {Language::Vietnamese, "<b>Họ Tên:</b> %1 %2<br><b>Mã Thẻ:</b> %3<br><b>Giới Tính:</b> %4<br><b>Trạng Thái:</b> %5<br><b>Sách Đang Mượn:</b> %6 cuốn"}, {Language::Khmer, "<b>ឈ្មោះ:</b> %1 %2<br><b>លេខកូដ:</b> %3<br><b>ភេទ:</b> %4<br><b>ស្ថានភាព:</b> %5<br><b>សៀវភៅកំពុងខ្ចី:</b> %6 ក្បាល"}, {Language::English, "<b>Name:</b> %1 %2<br><b>ID:</b> %3<br><b>Gender:</b> %4<br><b>Status:</b> %5<br><b>Borrowing:</b> %6 books"} };
    m_dictionary["GENDER_FEMALE"] = { {Language::Vietnamese, "Nữ"}, {Language::Khmer, "ស្រី"}, {Language::English, "Female"} };
    m_dictionary["GENDER_MALE"] = { {Language::Vietnamese, "Nam"}, {Language::Khmer, "ប្រុស"}, {Language::English, "Male"} };
    m_dictionary["STATUS_ACTIVE"] = { {Language::Vietnamese, "Hoạt động"}, {Language::Khmer, "សកម្ម"}, {Language::English, "Active"} };

    // === BOOK DETAILS DIALOG ===
    m_dictionary["TITLE_BOOK_DETAILS"] = { {Language::Vietnamese, "Chi Tiết Sách"}, {Language::Khmer, "ព័ត៌មានលម្អិតសៀវភៅ"}, {Language::English, "Book Details"} };
    m_dictionary["HEADER_BOOK_DETAILS"] = { {Language::Vietnamese, "CHI TIẾT ĐẦU SÁCH"}, {Language::Khmer, "ព័ត៌មានលម្អិតសៀវភៅ"}, {Language::English, "BOOK DETAILS"} };
    m_dictionary["GROUP_GENERAL_INFO"] = { {Language::Vietnamese, "Thông Tin Chung"}, {Language::Khmer, "ព័ត៌មនទូទៅ"}, {Language::English, "General Info"} };
    m_dictionary["GROUP_COPIES_LIST"] = { {Language::Vietnamese, "Danh Sách Bản Sao"}, {Language::Khmer, "បញ្ជីច្បាប់ចម្លង"}, {Language::English, "Copies List"} };
    
    m_dictionary["LBL_ISBN"] = { {Language::Vietnamese, "ISBN"}, {Language::Khmer, "ISBN"}, {Language::English, "ISBN"} };
    m_dictionary["LBL_AUTHOR"] = { {Language::Vietnamese, "Tác giả"}, {Language::Khmer, "អ្នកនិពន្ធ"}, {Language::English, "Author"} };
    m_dictionary["LBL_CATEGORY"] = { {Language::Vietnamese, "Thể loại"}, {Language::Khmer, "ប្រភេទ"}, {Language::English, "Category"} };
    m_dictionary["LBL_PUBLISH_YEAR"] = { {Language::Vietnamese, "Năm xuất bản"}, {Language::Khmer, "ឆ្នាំបោះពុម្ព"}, {Language::English, "Year"} };
    m_dictionary["LBL_PAGE_COUNT"] = { {Language::Vietnamese, "Số trang"}, {Language::Khmer, "ចំនួនទំព័រ"}, {Language::English, "Pages"} };
    m_dictionary["LBL_PRICE"] = { {Language::Vietnamese, "Giá tiền"}, {Language::Khmer, "តម្លៃ"}, {Language::English, "Price"} };
    m_dictionary["LBL_TOTAL_COPIES"] = { {Language::Vietnamese, "Tổng bản sao"}, {Language::Khmer, "ច្បាប់ចម្លងសរុប"}, {Language::English, "Total Copies"} };
    m_dictionary["LBL_AVAILABLE_COPIES"] = { {Language::Vietnamese, "Có thể mượn"}, {Language::Khmer, "អាចខ្ចីបាន"}, {Language::English, "Available"} };
    m_dictionary["LBL_BORROWED_COPIES"] = { {Language::Vietnamese, "Đang mượn"}, {Language::Khmer, "កំពុងខ្ចី"}, {Language::English, "Borrowed"} };
    m_dictionary["LBL_LIQUIDATED_COPIES"] = { {Language::Vietnamese, "Đã thanh lý"}, {Language::Khmer, "បានលុបចោល"}, {Language::English, "Liquidated"} };

    m_dictionary["BTN_ADD_COPY"] = { {Language::Vietnamese, "Thêm Bản Sao"}, {Language::Khmer, "បន្ថែមច្បាប់ចម្លង"}, {Language::English, "Add Copy"} };
    m_dictionary["BTN_DELETE_COPY"] = { {Language::Vietnamese, "Xóa Bản Sao"}, {Language::Khmer, "លុបច្បាប់ចម្លង"}, {Language::English, "Delete Copy"} };
    m_dictionary["BTN_LIQUIDATE"] = { {Language::Vietnamese, "Thanh Lý"}, {Language::Khmer, "លុបចោល (Liquidate)"}, {Language::English, "Liquidate"} };
    m_dictionary["BTN_PRINT_BARCODE"] = { {Language::Vietnamese, "In Tem Mã Vạch"}, {Language::Khmer, "បោះពុម្ពបាកូដ"}, {Language::English, "Print Barcode"} };
    
    m_dictionary["MSG_CONFIRM_DELETE_COPY_TITLE"] = { {Language::Vietnamese, "Xác nhận xóa bản sao"}, {Language::Khmer, "បញ្ជាក់ការលុប"}, {Language::English, "Confirm Delete"} };
    m_dictionary["MSG_CONFIRM_DELETE_COPY_BODY"] = { {Language::Vietnamese, "Bạn có chắc muốn xóa bản sao:\n%1?\n\nHành động này sẽ xóa vĩnh viễn bản sao khỏi hệ thống!"}, {Language::Khmer, "តើអ្នកពិតជារចង់លុបច្បាប់ចម្លង៖\n%1?\n\nសកម្មភាពនេះនឹងលុបជារៀងរហូត!"}, {Language::English, "Delete copy:\n%1?\n\nThis cannot be undone!"} };
    m_dictionary["MSG_DELETE_COPY_SUCCESS"] = { {Language::Vietnamese, "Đã xóa bản sao thành công!"}, {Language::Khmer, "បានលុបច្បាប់ចម្លងដោយជោគជ័យ!"}, {Language::English, "Copy deleted successfully!"} };
    
    m_dictionary["MSG_CONFIRM_LIQUIDATE_TITLE"] = { {Language::Vietnamese, "Xác nhận thanh lý"}, {Language::Khmer, "បញ្ជាក់ការលុបចោល"}, {Language::English, "Confirm Liquidation"} };
    m_dictionary["MSG_CONFIRM_LIQUIDATE_BODY"] = { {Language::Vietnamese, "Bạn có chắc muốn thanh lý bản sao:\n%1?\n\nHành động này không thể hoàn tác!"}, {Language::Khmer, "តើអ្នកពិតជារចង់លុបចោលច្បាប់ចម្លង៖\n%1?\n\nសកម្មភាពនេះមិនអាចត្រឡប់វិញបានទេ!"}, {Language::English, "Liquidate copy:\n%1?\n\nThis cannot be undone!"} };
    m_dictionary["MSG_LIQUIDATE_SUCCESS"] = { {Language::Vietnamese, "Đã thanh lý bản sao thành công!"}, {Language::Khmer, "បានលុបចោលច្បាប់ចម្លងដោយជោគជ័យ!"}, {Language::English, "Copy liquidated successfully!"} };
    
    m_dictionary["MSG_REFRESH_SUCCESS"] = { {Language::Vietnamese, "Đã làm mới dữ liệu!"}, {Language::Khmer, "ទិន្នន័យត្រូវបានធ្វើបច្ចុប្បន្នភាព!"}, {Language::English, "Data refreshed!"} };
    m_dictionary["MSG_SELECT_COPY_DELETE"] = { {Language::Vietnamese, "Vui lòng chọn bản sao cần xóa!"}, {Language::Khmer, "សូមជ្រើសរើសច្បាប់ចម្លងដើម្បីលុប!"}, {Language::English, "Please select a copy to delete!"} };
    m_dictionary["MSG_SELECT_COPY_LIQUIDATE"] = { {Language::Vietnamese, "Vui lòng chọn bản sao cần thanh lý!"}, {Language::Khmer, "សូមជ្រើសរើសច្បាប់ចម្លងដើម្បីលុបចោល!"}, {Language::English, "Please select a copy to liquidate!"} };

    m_dictionary["LBL_STATUS_AVAILABLE"] = { {Language::Vietnamese, "Có thể mượn"}, {Language::Khmer, "អាចខ្ចីបាន"}, {Language::English, "Available"} };
    m_dictionary["LBL_STATUS_Borrowed"] = { {Language::Vietnamese, "Đang được mượn"}, {Language::Khmer, "កំពុងត្រូវបានខ្ចី"}, {Language::English, "Borrowed"} };
    m_dictionary["LBL_STATUS_LIQUIDATED"] = { {Language::Vietnamese, "Đã thanh lý"}, {Language::Khmer, "បានលុបចោល"}, {Language::English, "Liquidated"} };
    m_dictionary["LBL_STATUS_UNKNOWN"] = { {Language::Vietnamese, "Không rõ"}, {Language::Khmer, "មិនស្គាល់"}, {Language::English, "Unknown"} };
    
    m_dictionary["DIALOG_ADD_COPY_TITLE"] = { {Language::Vietnamese, "Thêm Bản Sao"}, {Language::Khmer, "បន្ថែមច្បាប់ចម្លង"}, {Language::English, "Add Copy"} };
    m_dictionary["DIALOG_ADD_COPY_MSG"] = { {Language::Vietnamese, "Số lượng bản sao muốn thêm:"}, {Language::Khmer, "ចំនួនច្បាប់ចម្លង:"}, {Language::English, "Number of copies:"} };
    m_dictionary["DIALOG_LOCATION_TITLE"] = { {Language::Vietnamese, "Vị trí"}, {Language::Khmer, "ទីតាំង"}, {Language::English, "Location"} };
    m_dictionary["DIALOG_LOCATION_MSG"] = { {Language::Vietnamese, "Vị trí lưu trữ (tùy chọn):"}, {Language::Khmer, "ទីតាំងផ្ទុក (ស្រេចចិត្ត):"}, {Language::English, "Storage location (optional):"} };
    m_dictionary["MSG_ADD_COPY_SUCCESS"] = { {Language::Vietnamese, "Thêm bản sao thành công!"}, {Language::Khmer, "បន្ថែមច្បាប់ចម្លងដោយជោគជ័យ!"}, {Language::English, "Copy added successfully!"} };

    m_dictionary["DIALOG_SELECT_QR_DIR"] = { {Language::Vietnamese, "Chọn thư mục lưu QR"}, {Language::Khmer, "ជ្រើសរើសថតរក្សាទុក QR"}, {Language::English, "Select QR Directory"} };
    m_dictionary["DIALOG_SELECT_PDF_DIR"] = { {Language::Vietnamese, "Chọn thư mục lưu PDF"}, {Language::Khmer, "ជ្រើសរើសថតរក្សាទុក PDF"}, {Language::English, "Select PDF Directory"} };
    m_dictionary["MSG_ERROR_CREATE_DIR"] = { {Language::Vietnamese, "Không thể tạo thư mục PDF: "}, {Language::Khmer, "មិនអាចបង្កើតថត PDF បានទេ: "}, {Language::English, "Cannot create PDF directory: "} };

    // HopThoaiHangDoiIn
    m_dictionary["DIALOG_PRINT_QUEUE_TITLE"] = { {Language::Vietnamese, "Hàng Đợi In Thẻ Hàng Loạt"}, {Language::Khmer, "ជួររង់ចាំបោះពុម្ពប័ណ្ណជាក្រុម"}, {Language::English, "Batch Card Print Queue"} };
    m_dictionary["HEADER_PRINT_QUEUE"] = { {Language::Vietnamese, "HÀNG ĐỢI IN THẺ HỌC VIÊN"}, {Language::Khmer, "ជួររង់ចាំបោះពុម្ពប័ណ្ណសិស្ស"}, {Language::English, "STUDENT CARD PRINT QUEUE"} };
    m_dictionary["LBL_QUEUE_COUNT"] = { {Language::Vietnamese, "Số thẻ: %1"}, {Language::Khmer, "ចំនួនប័ណ្ណ: %1"}, {Language::English, "Card Count: %1"} };
    m_dictionary["LBL_QUEUE_PAGES"] = { {Language::Vietnamese, "Số trang A4: %1 (%2 cột)"}, {Language::Khmer, "ចំនួនទំព័រ A4: %1 (%2 ជួរឈរ)"}, {Language::English, "A4 Pages: %1 (%2 cols)"} };
    m_dictionary["LBL_QUEUE_COLUMNS"] = { {Language::Vietnamese, "Số cột:"}, {Language::Khmer, "ជួរឈរ:"}, {Language::English, "Columns:"} };
    m_dictionary["TOOLTIP_QUEUE_COLUMNS"] = { {Language::Vietnamese, "Số cột trên mỗi trang A4 (1-3)"}, {Language::Khmer, "ចំនួនជួរឈរលើទំព័រ A4 (1-3)"}, {Language::English, "Columns per A4 page (1-3)"} };
    m_dictionary["BTN_ADD_READER_QUEUE"] = { {Language::Vietnamese, "Thêm học viên"}, {Language::Khmer, "បន្ថែមសិស្ស"}, {Language::English, "Add Student"} };
    m_dictionary["BTN_ADD_BATCH_QUEUE"] = { {Language::Vietnamese, "Thêm nhiều"}, {Language::Khmer, "បន្ថែមច្រើន"}, {Language::English, "Add Batch"} };
    m_dictionary["BTN_DELETE_QUEUE"] = { {Language::Vietnamese, "Xóa"}, {Language::Khmer, "លុប"}, {Language::English, "Delete"} };
    m_dictionary["BTN_DELETE_ALL_QUEUE"] = { {Language::Vietnamese, "Xóa tất cả"}, {Language::Khmer, "លុបទាំងអស់"}, {Language::English, "Delete All"} };
    m_dictionary["BTN_PRINT_A4"] = { {Language::Vietnamese, "In A4"}, {Language::Khmer, "បោះពុម្ព A4"}, {Language::English, "Print A4"} };
    m_dictionary["BTN_EXPORT_PDF"] = { {Language::Vietnamese, "Xuất PDF"}, {Language::Khmer, "នាំចេញ PDF"}, {Language::English, "Export PDF"} };
    m_dictionary["MSG_INPUT_READER_ID"] = { {Language::Vietnamese, "Nhập mã thẻ học viên:"}, {Language::Khmer, "បញ្ចូលលេខកូដប័ណ្ណសិស្ស:"}, {Language::English, "Enter Student ID:"} };
    m_dictionary["MSG_READER_LOCKED"] = { {Language::Vietnamese, "Học viên này đã bị khóa!"}, {Language::Khmer, "សិស្សនេះត្រូវបានបិទ!"}, {Language::English, "This student is locked!"} };
    m_dictionary["MSG_READER_ALREADY_QUEUED"] = { {Language::Vietnamese, "Học viên này đã có trong hàng đợi!"}, {Language::Khmer, "សិស្សនេះមាននៅក្នុងជួររង់ចាំរួចហើយ!"}, {Language::English, "Student already in queue!"} };
    m_dictionary["MSG_QUEUE_EMPTY_ADD"] = { {Language::Vietnamese, "Hàng đợi trống! Vui lòng thêm học viên trước."}, {Language::Khmer, "ជួររង់ចាំទទេ! សូមបន្ថែមសិស្សជាមុនសិន។"}, {Language::English, "Queue empty! Please add students first."} };
    m_dictionary["MSG_CONFIRM_ADD_BATCH"] = { {Language::Vietnamese, "Thêm %1 học viên vào hàng đợi?"}, {Language::Khmer, "បន្ថែមសិស្ស %1 នាក់ទៅជួររង់ចាំ?"}, {Language::English, "Add %1 students to queue?"} };
    m_dictionary["MSG_CONFIRM_PRINT_A4"] = { {Language::Vietnamese, "In %1 thẻ (%2 trang A4, %3 cột)?"}, {Language::Khmer, "បោះពុម្ពប័ណ្ណ %1 (%2 ទំព័រ A4, %3 ជួរឈរ)?"}, {Language::English, "Print %1 cards (%2 A4 pages, %3 cols)?"} };
    m_dictionary["MSG_QUEUE_EMPTY"] = { {Language::Vietnamese, "Hàng đợi trống!"}, {Language::Khmer, "ជួររង់ចាំទទេ!"}, {Language::English, "Queue is empty!"} };
    m_dictionary["MSG_CONFIRM_DELETE_ALL"] = { {Language::Vietnamese, "Xóa toàn bộ hàng đợi?"}, {Language::Khmer, "លុបជួររង់ចាំទាំងអស់?"}, {Language::English, "Clear entire queue?"} };
    m_dictionary["MSG_PRINT_SENT"] = { {Language::Vietnamese, "Đã gửi lệnh in!"}, {Language::Khmer, "បានផ្ញើការបោះពុម្ព!"}, {Language::English, "Print command sent!"} };
    m_dictionary["MSG_ADDED_READER"] = { {Language::Vietnamese, "Đã thêm: %1 %2 (Mã: %3)"}, {Language::Khmer, "បានបន្ថែម: %1 %2 (កូដ: %3)"}, {Language::English, "Added: %1 %2 (ID: %3)"} };
    m_dictionary["MSG_ADDED_COUNT"] = { {Language::Vietnamese, "Đã thêm %1 học viên!"}, {Language::Khmer, "បានបន្ថែមសិស្ស %1 នាក់!"}, {Language::English, "Added %1 students!"} };
    m_dictionary["MSG_SELECT_DELETE"] = { {Language::Vietnamese, "Vui lòng chọn học viên cần xóa!"}, {Language::Khmer, "សូមជ្រើសរើសសិស្សដើម្បីលុប!"}, {Language::English, "Please select student to delete!"} };

    // HopThoaiQuetMaVach
    m_dictionary["DIALOG_SCAN_TITLE"] = { {Language::Vietnamese, "Quét Mã (Barcode/QR Code)"}, {Language::Khmer, "ស្កេនកូដ (Barcode/QR Code)"}, {Language::English, "Scan Code (Barcode/QR)"} };
    m_dictionary["HEADER_SCAN_RETURN_SEARCH"] = { {Language::Vietnamese, "QUÉT MÃ TRẢ / TÌM KIẾM"}, {Language::Khmer, "ស្កេនសង / ស្វែងរក"}, {Language::English, "SCAN RETURN / SEARCH"} };
    m_dictionary["HEADER_SCAN_BOOK"] = { {Language::Vietnamese, "QUÉT MÃ SÁCH"}, {Language::Khmer, "ស្កេនកូដសៀវភៅ"}, {Language::English, "SCAN BOOK CODE"} };
    m_dictionary["HEADER_SCAN_READER"] = { {Language::Vietnamese, "QUÉT THẺ HỌC VIÊN"}, {Language::Khmer, "ស្កេនប័ណ្ណសិស្ស"}, {Language::English, "SCAN STUDENT CARD"} };
    m_dictionary["MSG_SCAN_INSTRUCTION"] = { {Language::Vietnamese, "Sử dụng máy quét Barcode/QR hoặc nhập tay mã số bên dưới."}, {Language::Khmer, "ប្រើម៉ាស៊ីនស្កេន Barcode/QR ឬបញ្ចូលកូដដោយដៃខាងក្រោម។"}, {Language::English, "Use Barcode/QR scanner or enter code manually below."} };
    m_dictionary["MSG_PLEASE_SCAN"] = { {Language::Vietnamese, "Vui lòng quét mã..."}, {Language::Khmer, "សូមស្កេនកូដ..."}, {Language::English, "Please scan code..."} };
    m_dictionary["MSG_SEARCHING"] = { {Language::Vietnamese, " Đang tìm kiếm..."}, {Language::Khmer, " កំពុងស្វែងរក..."}, {Language::English, " Searching..."} };
    m_dictionary["MSG_ENTER_CODE"] = { {Language::Vietnamese, "Vui lòng nhập mã cần quét!"}, {Language::Khmer, "សូមបញ្ចូលកូដដែលត្រូវស្កេន!"}, {Language::English, "Please enter code to scan!"} };
    m_dictionary["MSG_CODE_NOT_FOUND"] = { {Language::Vietnamese, "Không tìm thấy mã: "}, {Language::Khmer, "រកមិនឃើញកូដ: "}, {Language::English, "Code not found: "} };
    m_dictionary["MSG_BARCODE_NOT_EXIST"] = { {Language::Vietnamese, "Mã vạch không tồn tại trong hệ thống.\n\nVui lòng kiểm tra lại hoặc thử quét lại."}, {Language::Khmer, "កូដមិនមាននៅក្នុងប្រព័ន្ធ។\n\nសូមពិនិត្យម្តងទៀត ឬព្យាយាមស្កេនម្តងទៀត។"}, {Language::English, "Barcode does not exist in system.\n\nPlease check or try scanning again."} };
    m_dictionary["MSG_NO_RESULT"] = { {Language::Vietnamese, "Chưa có kết quả"}, {Language::Khmer, "មិនមានលទ្ធផល"}, {Language::English, "No result"} };
    m_dictionary["MSG_SCAN_RESET"] = { {Language::Vietnamese, "Vui lòng quét hoặc nhập mã để xem thông tin"}, {Language::Khmer, "សូមស្កេន ឬបញ្ចូលកូដដើម្បីមើលព័ត៌មាន"}, {Language::English, "Please scan or enter code to view info"} };
    
    // HopThoaiCaiDat (System Settings)
    m_dictionary["LBL_LIBRARY_NAME"] = { {Language::Vietnamese, "Tên thư viện"}, {Language::Khmer, "ឈ្មោះបណ្ណាល័យ"}, {Language::English, "Library Name"} };
    m_dictionary["LBL_ADDRESS"] = { {Language::Vietnamese, "Địa chỉ"}, {Language::Khmer, "អាសយដ្ឋាន"}, {Language::English, "Address"} };
    m_dictionary["TAB_GENERAL_INFO"] = { {Language::Vietnamese, "Thông Tin Chung"}, {Language::Khmer, "ព័ត៌មានទូទៅ"}, {Language::English, "General Info"} };
    
    m_dictionary["LBL_FINE_PER_DAY"] = { {Language::Vietnamese, "Tiền phạt mỗi ngày (VNĐ)"}, {Language::Khmer, "ប្រាក់ពិន័យប្រចាំថ្ងៃ (VNĐ)"}, {Language::English, "Fine per day (VNĐ)"} };
    m_dictionary["LBL_MAX_BORROW_DAYS"] = { {Language::Vietnamese, "Số ngày mượn tối đa"}, {Language::Khmer, "ចំនួនថ្ងៃខ្ចីអតិបរមា"}, {Language::English, "Max borrow days"} };
    m_dictionary["LBL_MAX_RENEWALS"] = { {Language::Vietnamese, "Số lần gia hạn tối đa"}, {Language::Khmer, "ចំនួនដងបន្តអតិបរមា"}, {Language::English, "Max renewals"} };
    m_dictionary["LBL_MAX_BORROW_BOOKS"] = { {Language::Vietnamese, "Số sách mượn tối đa"}, {Language::Khmer, "ចំនួនសៀវភៅខ្ចីអតិបរមា"}, {Language::English, "Max books to borrow"} };
    m_dictionary["TAB_BORROW_RULES"] = { {Language::Vietnamese, "Quy Định Mượn Trả"}, {Language::Khmer, "ច្បាប់ខ្ចី-សង"}, {Language::English, "Borrow Rules"} };

    // New keys for WidgetMuonTraSach
    m_dictionary["TABLE_HEADER_BOOK_ID"] = { {Language::Vietnamese, "Mã Sách"}, {Language::Khmer, "លេខកូដសៀវភៅ"}, {Language::English, "Book ID"} };
    m_dictionary["TABLE_HEADER_DATE_BORROW"] = { {Language::Vietnamese, "Ngày Mượn"}, {Language::Khmer, "កាលបរិច្ឆេទខ្ចី"}, {Language::English, "Borrow Date"} };
    m_dictionary["TABLE_HEADER_TITLE"] = { {Language::Vietnamese, "Tên Sách"}, {Language::Khmer, "ចំណងជើង"}, {Language::English, "Title"} };
    m_dictionary["TABLE_HEADER_STATUS"] = { {Language::Vietnamese, "Trạng Thái"}, {Language::Khmer, "ស្ថានភាព"}, {Language::English, "Status"} };
    m_dictionary["BTN_EXTEND"] = { {Language::Vietnamese, "Gia Hạn"}, {Language::Khmer, "ពន្យារ"}, {Language::English, "Extend"} };
    
    m_dictionary["LBL_PRINT_MODE"] = { {Language::Vietnamese, "Chế độ in"}, {Language::Khmer, "របៀបបោះពុម្ព"}, {Language::English, "Print Mode"} };
    m_dictionary["LBL_PRINTER_RECEIPT"] = { {Language::Vietnamese, "Máy in Bill (Receipt)"}, {Language::Khmer, "ម៉ាស៊ីនបោះពុម្ពវិក្កយបត្រ"}, {Language::English, "Receipt Printer"} };
    m_dictionary["LBL_PRINTER_REPORT"] = { {Language::Vietnamese, "Máy in Báo cáo (A4)"}, {Language::Khmer, "ម៉ាស៊ីនបោះពុម្ពរបាយការណ៍ (A4)"}, {Language::English, "Report Printer (A4)"} };
    m_dictionary["OPTION_SYSTEM_DEFAULT"] = { {Language::Vietnamese, "Mặc định hệ thống"}, {Language::Khmer, "លំនាំដើមប្រព័ន្ធ"}, {Language::English, "System Default"} };
    m_dictionary["TAB_PRINT_CONFIG"] = { {Language::Vietnamese, "Cấu Hình In Ấn"}, {Language::Khmer, "ការកំណត់ការបោះពុម្ព"}, {Language::English, "Print Config"} };

    m_dictionary["BTN_RESTORE_DEFAULT"] = { {Language::Vietnamese, "Khôi phục mặc định"}, {Language::Khmer, "ស្តារលំនាំដើម"}, {Language::English, "Restore Defaults"} };
    m_dictionary["MSG_SAVE_SETTINGS_SUCCESS"] = { {Language::Vietnamese, "Đã lưu cấu hình thành công!"}, {Language::Khmer, "បានរក្សាទុកការកំណត់ដោយជោគជ័យ!"}, {Language::English, "Settings saved successfully!"} };
    
    // MainWindow - Sidebar & Menus
    m_dictionary["SIDEBAR_TITLE"] = { {Language::Vietnamese, "THƯ VIỆN"}, {Language::Khmer, "បណ្ណាល័យ"}, {Language::English, "LIBRARY"} };
    m_dictionary["MENU_HOME"] = { {Language::Vietnamese, "Trang Chủ"}, {Language::Khmer, "ទំព័រដើម"}, {Language::English, "Home"} };
    m_dictionary["MENU_BOOKS"] = { {Language::Vietnamese, "Quản Lý Sách"}, {Language::Khmer, "គ្រប់គ្រងសៀវភៅ"}, {Language::English, "Book Mgmt"} };
    m_dictionary["MENU_READERS"] = { {Language::Vietnamese, "Quản Lý Học Viên"}, {Language::Khmer, "គ្រប់គ្រងសិស្ស"}, {Language::English, "Reader Mgmt"} };
    m_dictionary["MENU_LOANS"] = { {Language::Vietnamese, "Mượn Trả Sách"}, {Language::Khmer, "ខ្ចី-សងសៀវភៅ"}, {Language::English, "Loan/Return"} };
    m_dictionary["MENU_LOAN"] = { {Language::Vietnamese, "Mượn Trả Sách"}, {Language::Khmer, "ខ្ចី-សងសៀវភៅ"}, {Language::English, "Loan/Return"} }; // Alias for BangDieuKhien compatibility
    m_dictionary["MENU_USERS"] = { {Language::Vietnamese, "Quản Lý Người Dùng"}, {Language::Khmer, "អ្នកប្រើប្រាស់"}, {Language::English, "User Mgmt"} };
    m_dictionary["MENU_SETTINGS"] = { {Language::Vietnamese, "Cấu Hình"}, {Language::Khmer, "ការកំណត់"}, {Language::English, "Settings"} };
    m_dictionary["MENU_ABOUT"] = { {Language::Vietnamese, "Giới Thiệu"}, {Language::Khmer, "អំពីកម្មវិធី"}, {Language::English, "About"} };
    m_dictionary["MENU_EXIT"] = { {Language::Vietnamese, "Thoát"}, {Language::Khmer, "ចាកចេញ"}, {Language::English, "Exit"} };
    m_dictionary["APP_TITLE"] = { {Language::Vietnamese, "HỆ THỐNG QUẢN LÝ THƯ VIỆN"}, {Language::Khmer, "ប្រព័ន្ធគ្រប់គ្រងបណ្ណាល័យ"}, {Language::English, "LIBRARY MANAGEMENT SYSTEM"} };
    m_dictionary["APP_WINDOW_TITLE"] = { {Language::Vietnamese, "Hệ Thống Quản Lý Thư Viện"}, {Language::Khmer, "ប្រព័ន្ធគ្រប់គ្រងបណ្ណាល័យ"}, {Language::English, "Library Management System"} };
    
    // MainWindow - Buttons & Tooltips
    m_dictionary["BTN_MENU_BOOKS"] = { {Language::Vietnamese, "QUẢN LÝ SÁCH"}, {Language::Khmer, "គ្រប់គ្រងសៀវភៅ"}, {Language::English, "BOOK MGMT"} };
    m_dictionary["BTN_MENU_READERS"] = { {Language::Vietnamese, "QUẢN LÝ HỌC VIÊN"}, {Language::Khmer, "គ្រប់គ្រងសៀវភៅ"}, {Language::English, "READER MGMT"} };
    m_dictionary["BTN_MENU_LOANS"] = { {Language::Vietnamese, "MƯỢN - TRẢ SÁCH"}, {Language::Khmer, "ខ្ចី - សងសៀវភៅ"}, {Language::English, "LOAN - RETURN"} };
    m_dictionary["BTN_MENU_TOP10"] = { {Language::Vietnamese, "TOP 10 SÁCH"}, {Language::Khmer, "សៀវភៅកំពូលទាំង ១០"}, {Language::English, "TOP 10 BOOKS"} };
    m_dictionary["BTN_MENU_STATS"] = { {Language::Vietnamese, "THỐNG KÊ HỆ THỐNG"}, {Language::Khmer, "ស្ថិតិប្រព័ន្ធ"}, {Language::English, "SYSTEM STATISTICS"} };
    m_dictionary["BTN_MENU_BACKUP"] = { {Language::Vietnamese, "SAO LƯU DỮ LIỆU"}, {Language::Khmer, "បម្រុងទុកទិន្នន័យ"}, {Language::English, "BACKUP DATA"} };
    m_dictionary["BTN_MENU_OVERDUE"] = { {Language::Vietnamese, "HỌC VIÊN QUÁ HẠN"}, {Language::Khmer, "សិស្សហួសកំណត់"}, {Language::English, "OVERDUE READERS"} };
    m_dictionary["BTN_MENU_ABOUT"] = { {Language::Vietnamese, "THÔNG TIN"}, {Language::Khmer, "ព័ត៌មាន"}, {Language::English, "INFORMATION"} };
    m_dictionary["BTN_MENU_EXIT"] = { {Language::Vietnamese, "THOÁT"}, {Language::Khmer, "ចាកចេញ"}, {Language::English, "EXIT"} };

    // Dashboard - Headers & Cards
    m_dictionary["HEADER_DASHBOARD_WARNING"] = { {Language::Vietnamese, "Cảnh báo"}, {Language::Khmer, "ការព្រមាន"}, {Language::English, "Warnings"} };
    m_dictionary["HEADER_DASHBOARD_STATS"] = { {Language::Vietnamese, "Số liệu chính"}, {Language::Khmer, "តួលេខសំខាន់"}, {Language::English, "Key Metrics"} };
    m_dictionary["HEADER_DASHBOARD_QUICK"] = { {Language::Vietnamese, "Tác vụ nhanh"}, {Language::Khmer, "ភារកិច្ចរហ័ស"}, {Language::English, "Quick Actions"} };
    
    m_dictionary["CARD_TITLE_OVERDUE_BOOKS"] = { {Language::Vietnamese, "Sách Quá Hạn"}, {Language::Khmer, "សៀវភៅហួសកំណត់"}, {Language::English, "Overdue Books"} };
    m_dictionary["CARD_TITLE_WARNING_CARDS"] = { {Language::Vietnamese, "Thẻ Cảnh Báo"}, {Language::Khmer, "ប័ណ្ណព្រមាន"}, {Language::English, "Warning Cards"} };
    m_dictionary["CARD_TITLE_TOTAL_BOOKS"] = { {Language::Vietnamese, "Tổng số sách"}, {Language::Khmer, "សៀវភៅសរុប"}, {Language::English, "Total Books"} };
    m_dictionary["CARD_TITLE_TOTAL_READERS"] = { {Language::Vietnamese, "Số học viên"}, {Language::Khmer, "ចំនួនសិស្ស"}, {Language::English, "Total Readers"} };
    m_dictionary["CARD_TITLE_BORROWED"] = { {Language::Vietnamese, "Sách đang mượn"}, {Language::Khmer, "សៀវភៅកំពុងខ្ចី"}, {Language::English, "Books Borrowed"} };
    m_dictionary["CARD_TITLE_VISITS"] = { {Language::Vietnamese, "Lượt truy cập hôm nay"}, {Language::Khmer, "ការចូលមើលថ្ងៃនេះ"}, {Language::English, "Visits Today"} };
    
    // Dashboard - Details
    m_dictionary["MSG_DASHBOARD_LOADING"] = { {Language::Vietnamese, "Đang kiểm tra dữ liệu..."}, {Language::Khmer, "កំពុងពិនិត្យទិន្នន័យ..."}, {Language::English, "Checking data..."} };
    m_dictionary["MSG_DASHBOARD_OVERDUE_COUNT"] = { {Language::Vietnamese, "%1 Sách Quá Hạn"}, {Language::Khmer, "%1 សៀវភៅហួសកំណត់"}, {Language::English, "%1 Overdue Books"} };
    m_dictionary["MSG_DASHBOARD_WARNING_COUNT"] = { {Language::Vietnamese, "%1 Thẻ Cảnh Báo"}, {Language::Khmer, "%1 ប័ណ្ណព្រមាន"}, {Language::English, "%1 Warning Cards"} };
    m_dictionary["MSG_DASHBOARD_NO_OVERDUE"] = { {Language::Vietnamese, "Không có sách quá hạn."}, {Language::Khmer, "មិនមានសៀវភៅហួសកំណត់ទេ។"}, {Language::English, "No overdue books."} };
    m_dictionary["MSG_DASHBOARD_NO_WARNING"] = { {Language::Vietnamese, "Không có học viên quá hạn."}, {Language::Khmer, "មិនមានសិស្សហួសកំណត់ទេ។"}, {Language::English, "No overdue students."} };
    m_dictionary["MSG_DASHBOARD_OVERDUE_DETAIL"] = { {Language::Vietnamese, "Học viên '%1' quá hạn %2 ngày."}, {Language::Khmer, "សិស្ស '%1' ហួសកំណត់ %2 ថ្ងៃ។"}, {Language::English, "Student '%1' overdue %2 days."} };

    // Dashboard - Quick Buttons
    m_dictionary["BTN_QUICK_ADD_BOOK"] = { {Language::Vietnamese, "Thêm Sách Mới"}, {Language::Khmer, "បន្ថែមសៀវភៅថ្មី"}, {Language::English, "Add New Book"} };
    m_dictionary["BTN_QUICK_ADD_READER"] = { {Language::Vietnamese, "Đăng Ký Học Viên"}, {Language::Khmer, "ចុះឈ្មោះសិស្ស"}, {Language::English, "Register Reader"} };
    m_dictionary["BTN_QUICK_CREATE_LOAN"] = { {Language::Vietnamese, "Tạo Phiếu Mượn"}, {Language::Khmer, "បង្កើតប័ណ្ណខ្ចី"}, {Language::English, "Create Loan Slip"} };
    m_dictionary["BTN_QUICK_BACKUP"] = { {Language::Vietnamese, "Sao Lưu Dữ Liệu"}, {Language::Khmer, "បម្រុងទុកទិន្នន័យ"}, {Language::English, "Backup Data"} };
    m_dictionary["BTN_QUICK_SYSTEM_INFO"] = { {Language::Vietnamese, "Thông Tin Hệ Thống"}, {Language::Khmer, "ព័ត៌មានប្រព័ន្ធ"}, {Language::English, "System Info"} };
    m_dictionary["BTN_VIEW_DETAILS"] = { {Language::Vietnamese, "Xem chi tiết"}, {Language::Khmer, "មើលលម្អិត"}, {Language::English, "View Details"} };

    // Other System Messages
    m_dictionary["TITLE_EXIT_CONFIRM"] = { {Language::Vietnamese, "Xác nhận thoát"}, {Language::Khmer, "បញ្ជាក់ការចាកចេញ"}, {Language::English, "Confirm Exit"} };
    m_dictionary["MSG_EXIT_BACKUP_PROMPT"] = { {Language::Vietnamese, "Dữ liệu đã thay đổi sẽ được lưu tự động.\n\nBạn có muốn tạo backup trước khi thoát không?"}, {Language::Khmer, "ទិន្នន័យដែលបានផ្លាស់ប្តូរនឹងត្រូវរក្សាទុកដោយស្វ័យប្រវត្តិ។\n\nតើអ្នកចង់បង្កើតការបម្រុងទុកមុនពេលចាកចេញទេ?"}, {Language::English, "Data changes will be saved automatically.\n\nDo you want to backup before exiting?"} };
    m_dictionary["TITLE_BACKUP_SUCCESS"] = { {Language::Vietnamese, "Backup Thành Công"}, {Language::Khmer, "បម្រុងទុកជោគជ័យ"}, {Language::English, "Backup Success"} };
    m_dictionary["MSG_BACKUP_SUCCESS"] = { {Language::Vietnamese, "Đã tạo backup dữ liệu thành công!"}, {Language::Khmer, "បានបង្កើតការបម្រុងទុកទិន្នន័យដោយជោគជ័យ!"}, {Language::English, "Data backup created successfully!"} };
    m_dictionary["MSG_BACKUP_SUCCESS_EXIT"] = { {Language::Vietnamese, "Đã tạo backup dữ liệu thành công!\n\nHệ thống sẽ thoát."}, {Language::Khmer, "បានបង្កើតការបម្រុងទុកទិន្នន័យដោយជោគជ័យ!\n\nប្រព័ន្ធនឹងចាកចេញ។"}, {Language::English, "Backup successful!\n\nSystem will exit."} };
    m_dictionary["GROUP_SCAN_RESULT"] = { {Language::Vietnamese, "KẾT QUẢ QUÉT"}, {Language::Khmer, "លទ្ធផលស្កេន"}, {Language::English, "SCAN RESULT"} };
    m_dictionary["BTN_CONFIRM_SCAN"] = { {Language::Vietnamese, "XÁC NHẬN"}, {Language::Khmer, "យល់ព្រម"}, {Language::English, "CONFIRM"} };
    m_dictionary["BTN_USE"] = { {Language::Vietnamese, "SỬ DỤNG"}, {Language::Khmer, "ប្រើប្រាស់"}, {Language::English, "USE"} };
    m_dictionary["MSG_FOUND_BOOK"] = { {Language::Vietnamese, "Tìm thấy sách: "}, {Language::Khmer, "រកឃើញសៀវភៅ: "}, {Language::English, "Found book: "} };
    m_dictionary["MSG_FOUND_READER"] = { {Language::Vietnamese, "Tìm thấy học viên: "}, {Language::Khmer, "រកឃើញសិស្ស: "}, {Language::English, "Found student: "} };
    m_dictionary["STATUS_LOCKED_ICON"] = { {Language::Vietnamese, "Bị khóa"}, {Language::Khmer, "ត្រូវបានបិទ"}, {Language::English, "Locked"} };
    m_dictionary["STATUS_ACTIVE_ICON"] = { {Language::Vietnamese, "Hoạt động"}, {Language::Khmer, "សកម្ម"}, {Language::English, "Active"} };
    m_dictionary["STATUS_UNKNOWN_ICON"] = { {Language::Vietnamese, "Không xác định"}, {Language::Khmer, "មិនស្គាល់"}, {Language::English, "Unknown"} };
    m_dictionary["GENDER_MALE"] = { {Language::Vietnamese, "Nam"}, {Language::Khmer, "ប្រុស"}, {Language::English, "Male"} };
    m_dictionary["GENDER_FEMALE"] = { {Language::Vietnamese, "Nữ"}, {Language::Khmer, "ស្រី"}, {Language::English, "Female"} };
    m_dictionary["LBL_BORROWING_COUNT"] = { {Language::Vietnamese, "Số sách đang mượn:"}, {Language::Khmer, "ចំនួនសៀវភៅកំពុងខ្ចី:"}, {Language::English, "Books borrowing:"} };
    m_dictionary["TITLE_BOOK_INFO"] = { {Language::Vietnamese, "THÔNG TIN SÁCH"}, {Language::Khmer, "ព័ត៌មានសៀវភៅ"}, {Language::English, "BOOK INFORMATION"} };
    m_dictionary["TITLE_READER_INFO"] = { {Language::Vietnamese, "THÔNG TIN HỌC VIÊN"}, {Language::Khmer, "ព័ត៌មានសិស្ស"}, {Language::English, "STUDENT INFORMATION"} };
    m_dictionary["LBL_AUTHOR"] = { {Language::Vietnamese, "Tác giả:"}, {Language::Khmer, "អ្នកនិពន្ធ:"}, {Language::English, "Author:"} };
    m_dictionary["LBL_CATEGORY"] = { {Language::Vietnamese, "Thể loại:"}, {Language::Khmer, "ប្រភេទ:"}, {Language::English, "Category:"} };
    m_dictionary["LBL_PUBLISH_YEAR"] = { {Language::Vietnamese, "Năm xuất bản:"}, {Language::Khmer, "ឆ្នាំបោះពុម្ព:"}, {Language::English, "Publish Year:"} };
    m_dictionary["LBL_TOTAL_COPIES_SCAN"] = { {Language::Vietnamese, "Tổng bản sao:"}, {Language::Khmer, "ច្បាប់ចម្លងសរុប:"}, {Language::English, "Total Copies:"} };
    m_dictionary["LBL_PAGES"] = { {Language::Vietnamese, "Số trang:"}, {Language::Khmer, "ចំនួនទំព័រ:"}, {Language::English, "Pages:"} };
    m_dictionary["LBL_FULLNAME"] = { {Language::Vietnamese, "Họ tên:"}, {Language::Khmer, "ឈ្មោះពេញ:"}, {Language::English, "Full Name:"} };
    m_dictionary["LBL_CARD_ID"] = { {Language::Vietnamese, "Mã thẻ:"}, {Language::Khmer, "លេខកូដប័ណ្ណ:"}, {Language::English, "Card ID:"} };
    m_dictionary["LBL_GENDER_TEXT"] = { {Language::Vietnamese, "Giới tính:"}, {Language::Khmer, "ភេទ:"}, {Language::English, "Gender:"} };
    m_dictionary["LBL_STATUS_TEXT"] = { {Language::Vietnamese, "Trạng Thái:"}, {Language::Khmer, "ស្ថានភាព:"}, {Language::English, "Status:"} };
    m_dictionary["MSG_RETURN_SUCCESS_ICON"] = { {Language::Vietnamese, "Đã trả sách thành công!"}, {Language::Khmer, "បានសងសៀវភៅជោគជ័យ!"}, {Language::English, "Return success!"} };
    m_dictionary["MSG_RETURN_ERROR_ICON"] = { {Language::Vietnamese, "Lỗi trả sách: "}, {Language::Khmer, "កំហុសសងសៀវភៅ: "}, {Language::English, "Return Error: "} };
    m_dictionary["TITLE_RETURN_SUCCESS_HTML"] = { {Language::Vietnamese, "TRẢ SÁCH THÀNH CÔNG"}, {Language::Khmer, "សងសៀវភៅជោគជ័យ"}, {Language::English, "RETURN SUCCESSFUL"} };
    m_dictionary["LBL_RETURN_READER"] = { {Language::Vietnamese, "Học viên:"}, {Language::Khmer, "សិស្ស:"}, {Language::English, "Student:"} };
    m_dictionary["LBL_RETURN_BOOK"] = { {Language::Vietnamese, "Sách:"}, {Language::Khmer, "សៀវភៅ:"}, {Language::English, "Book:"} };
    m_dictionary["LBL_RETURN_BOOK_CODE"] = { {Language::Vietnamese, "Mã sách:"}, {Language::Khmer, "កូដសៀវភៅ:"}, {Language::English, "Book Code:"} };
    m_dictionary["LBL_BORROW_DATE"] = { {Language::Vietnamese, "Ngày mượn:"}, {Language::Khmer, "ថ្ងៃខ្ចី:"}, {Language::English, "Borrow Date:"} };
    m_dictionary["LBL_RETURN_DATE"] = { {Language::Vietnamese, "Ngày trả:"}, {Language::Khmer, "ថ្ងៃសង:"}, {Language::English, "Return Date:"} };
    m_dictionary["MSG_RETURN_SUCCESS_BODY"] = { {Language::Vietnamese, "Đã trả sách thành công!\n\nHọc viên: %1\nSách: %2"}, {Language::Khmer, "បានសងសៀវភៅជោគជ័យ!\n\nសិស្ស: %1\nសៀវភៅ: %2"}, {Language::English, "Return successful!\n\nStudent: %1\nBook: %2"} };
    m_dictionary["MSG_NO_CODE_SCANNED"] = { {Language::Vietnamese, "Chưa quét được mã nào!"}, {Language::Khmer, "មិនទាន់បានស្កេនកូដណាមួយទេ!"}, {Language::English, "No code scanned!"} };
    m_dictionary["MSG_READER_LIST_EMPTY"] = { {Language::Vietnamese, "Danh sách học viên trống!"}, {Language::Khmer, "បញ្ជីសិស្សទទេ!"}, {Language::English, "Reader list is empty!"} };
    m_dictionary["MSG_ALL_READERS_QUEUED"] = { {Language::Vietnamese, "Tất cả học viên đang hoạt động đã có trong hàng đợi!"}, {Language::Khmer, "សិស្សសកម្មទាំងអស់មាននៅក្នុងជួររង់ចាំរួចហើយ!"}, {Language::English, "All active students are already in queue!"} };
    m_dictionary["TITLE_ADD"] = { {Language::Vietnamese, "Thêm"}, {Language::Khmer, "បន្ថែម"}, {Language::English, "Add"} }; // Ensure simple ADD title exists
    m_dictionary["MSG_ERROR_SEARCH_YEAR_RANGE"] = {
        {Language::Vietnamese, "Lỗi khi tìm kiếm theo khoảng năm!"},
        {Language::Khmer, "កំហុសស្វែងរកតាមចន្លោះឆ្នាំ!"},
        {Language::English, "Error searching by year range!"}
    };
    m_dictionary["MSG_FOUND_BOOKS_YEAR_RANGE"] = {
        {Language::Vietnamese, "Tìm thấy %1 sách từ %2 đến %3"},
        {Language::Khmer, "រកឃើញសៀវភៅ %1 ចាប់ពីឆ្នាំ %2 ដល់ %3"},
        {Language::English, "Found %1 books from %2 to %3"}
    };
    m_dictionary["MSG_ENTER_AUTHOR"] = {
        {Language::Vietnamese, "Vui lòng nhập tên tác giả!"},
        {Language::Khmer, "សូមបញ្ចូលឈ្មោះអ្នកនិពន្ធ!"},
        {Language::English, "Please enter author name!"}
    };
    m_dictionary["MSG_ERROR_SEARCH_AUTHOR"] = {
        {Language::Vietnamese, "Lỗi khi tìm kiếm theo tác giả!"},
        {Language::Khmer, "កំហុសស្វែងរកតាមអ្នកនិពន្ធ!"},
        {Language::English, "Error searching by author!"}
    };
    m_dictionary["MSG_FOUND_BOOKS_AUTHOR"] = {
        {Language::Vietnamese, " Tìm thấy %1 sách của tác giả '%2'"},
        {Language::Khmer, " រកឃើញសៀវភៅ %1 របស់អ្នកនិពន្ធ '%2'"},
        {Language::English, " Found %1 books by author '%2'"}
    };
    m_dictionary["OPTION_STATUS_BORROWABLE"] = {
        {Language::Vietnamese, "0 - Có thể mượn"},
        {Language::Khmer, "0 - អាចខ្ចីបាន"},
        {Language::English, "0 - Borrowable"}
    };
    m_dictionary["OPTION_STATUS_BORROWED"] = {
        {Language::Vietnamese, "1 - Đang được mượn"},
        {Language::Khmer, "1 - កំពុងត្រូវបានខ្ចី"},
        {Language::English, "1 - Borrowed"}
    };
    m_dictionary["OPTION_STATUS_LIQUIDATED"] = {
        {Language::Vietnamese, "2 - Thanh lý"},
        {Language::Khmer, "2 - បានលុបចោល"},
        {Language::English, "2 - Liquidated"}
    };
    m_dictionary["MSG_ERROR_FILTER_STATUS"] = {
        {Language::Vietnamese, "Lỗi khi lọc theo trạng thái!"},
        {Language::Khmer, "កំហុសក្នុងការត្រងតាមស្ថានភាព!"},
        {Language::English, "Error filtering by status!"}
    };
    m_dictionary["MSG_FOUND_BOOKS_STATUS"] = {
        {Language::Vietnamese, " Tìm thấy %1 đầu sách có bản sao '%2'"},
        {Language::Khmer, " រកឃើញសៀវភៅ %1 ដែលមានស្ថានភាព '%2'"},
        {Language::English, " Found %1 books with status '%2'"}
    };
    m_dictionary["MSG_SHOW_ALL_BOOKS"] = {
        {Language::Vietnamese, "Hiển thị tất cả %1 đầu sách"},
        {Language::Khmer, "បង្ហាញសៀវភៅទាំងអស់ %1"},
        {Language::English, "Showing all %1 books"}
    };
    m_dictionary["TITLE_CANNOT_DELETE"] = {
        {Language::Vietnamese, "Không thể xóa"},
        {Language::Khmer, "មិនអាចលុបបានទេ"},
        {Language::English, "Cannot Delete"}
    };
    m_dictionary["MSG_CANNOT_DELETE_BORROWED"] = {
        {Language::Vietnamese, "Không thể xóa đầu sách này vì còn bản sao đang được mượn hoặc chờ mượn!"},
        {Language::Khmer, "មិនអាចលុបសៀវភៅនេះបានទេ ព្រោះនៅមានច្បាប់ចម្លងដែលកំពុងខ្ចី!"},
        {Language::English, "Cannot delete book because copies are still borrowed!"}
    };
    m_dictionary["MSG_CANNOT_DELETE_WAIT"] = {
        {Language::Vietnamese, "Vui lòng đợi tất cả bản sao được trả lại trước khi xóa."},
        {Language::Khmer, "សូមរង់ចាំរហូតដល់ច្បាប់ចម្លងទាំងអស់ត្រូវបានប្រគល់មកវិញ។"},
        {Language::English, "Please wait for all copies to be returned."}
    };
    m_dictionary["TABLE_HEADER_OVERDUE_DAYS_MAX"] = {
        {Language::Vietnamese, "Số Ngày Quá Hạn (Max)"},
        {Language::Khmer, "ចំនួនថ្ងៃហួសកំណត់ (Max)"},
        {Language::English, "Overdue Days (Max)"}
    };
    m_dictionary["STATUS_SERIOUS"] = {
        {Language::Vietnamese, "Nghiêm Trọng"},
        {Language::Khmer, "ធ្ងន់ធ្ងរ (Serious)"},
        {Language::English, "Serious"}
    };
    m_dictionary["STATUS_WARNING"] = {
        {Language::Vietnamese, "Cảnh Báo"},
        {Language::Khmer, "ការព្រមាន (Warning)"},
        {Language::English, "Warning"}
    };
    m_dictionary["STATUS_REMINDER"] = {
        {Language::Vietnamese, "Nhắc Nhở"},
        {Language::Khmer, "ការរំលឹក (Reminder)"},
        {Language::English, "Reminder"}
    };
    m_dictionary["MSG_NO_OVERDUE"] = {
        {Language::Vietnamese, "Không có học viên nào quá hạn trả sách!\nTất cả học viên đều trả sách đúng hạn."},
        {Language::Khmer, "គ្មានសិស្សណាហួសកំណត់ទេ!\nសិស្សទាំងអស់បានសងសៀវភៅទាន់ពេលវេលា។"},
        {Language::English, "No overdue students!\nAll students returned books on time."}
    };
    m_dictionary["MSG_OVERDUE_DETECTED"] = {
        {Language::Vietnamese, "Phát hiện %1 học viên quá hạn trả sách!\nVui lòng liên hệ để nhắc nhở các học viên này."},
        {Language::Khmer, "បានរកឃើញ %1 សិស្សហួសកំណត់!\nសូមទាក់ទងដើម្បីរំលឹកពួកគេ។"},
        {Language::English, "Detected %1 overdue students!\nPlease contact them."}
    };
    m_dictionary["STATUS_ACTIVE"] = {
        {Language::Vietnamese, "Hoạt động"},
        {Language::Khmer, "សកម្ម (Active)"},
        {Language::English, "Active"}
    };
    m_dictionary["STATUS_LOCKED"] = {
        {Language::Vietnamese, "Khóa"},
        {Language::Khmer, "បានចាក់សោ (Locked)"},
        {Language::English, "Locked"}
    };
    m_dictionary["STATUS_OVERDUE"] = {
        {Language::Vietnamese, "Quá hạn"},
        {Language::Khmer, "ហួសកំណត់ (Overdue)"},
        {Language::English, "Overdue"}
    };
    m_dictionary["GENDER_MALE"] = {
        {Language::Vietnamese, "Nam"},
        {Language::Khmer, "ប្រុស (Male)"},
        {Language::English, "Male"}
    };
    m_dictionary["GENDER_FEMALE"] = {
        {Language::Vietnamese, "Nữ"},
        {Language::Khmer, "ស្រី (Female)"},
        {Language::English, "Female"}
    };
    m_dictionary["BTN_SEARCH"] = {
        {Language::Vietnamese, "Tìm kiếm"},
        {Language::Khmer, "ស្វែងរក (Search)"},
        {Language::English, "Search"}
    };
    m_dictionary["BTN_CLEAR"] = {
        {Language::Vietnamese, "Xóa"},
        {Language::Khmer, "លុប (Clear)"},
        {Language::English, "Clear"}
    };
    m_dictionary["BTN_UNLOCK"] = {
        {Language::Vietnamese, "Mở Khóa"},
        {Language::Khmer, "ដោះសោ (Unlock)"},
        {Language::English, "Unlock"}
    };
    m_dictionary["BTN_DETAILS"] = {
        {Language::Vietnamese, "Chi Tiết"},
        {Language::Khmer, "លម្អិត (Details)"},
        {Language::English, "Details"}
    };
    m_dictionary["BTN_Back"] = {
        {Language::Vietnamese, "Quay Lại"},
        {Language::Khmer, "ត្រឡប់ក្រោយ (Back)"},
        {Language::English, "Back"}
    };
    
    // === APP TITLE ===
    m_dictionary["APP_TITLE"] = {
        {Language::Vietnamese, "HỆ THỐNG QUẢN LÝ THƯ VIỆN"},
        {Language::Khmer, "ប្រព័ន្ធគ្រប់គ្រងបណ្ណាល័យ"},
        {Language::English, "LIBRARY MANAGEMENT SYSTEM"}
    };

    m_dictionary["STATUS_AVAILABLE"] = {
        {Language::Vietnamese, "Có sẵn"},
        {Language::Khmer, "មាននៅបណ្ណាល័យ"},
        {Language::English, "Available"}
    };
    m_dictionary["STATUS_LIQUIDATED"] = {
        {Language::Vietnamese, "Đã thanh lý"},
        {Language::Khmer, "បានលុបចោល"},
        {Language::English, "Liquidated"}
    };
    m_dictionary["TABLE_HEADER_ISBN"] = {
        {Language::Vietnamese, "ISBN"},
        {Language::Khmer, "ISBN"},
        {Language::English, "ISBN"}
    };
    m_dictionary["TABLE_HEADER_BOOK_NAME"] = {
        {Language::Vietnamese, "TÊN SÁCH"},
        {Language::Khmer, "ឈ្មោះសៀវភៅ"},
        {Language::English, "BOOK TITLE"}
    };
    m_dictionary["TABLE_HEADER_AUTHOR"] = {
        {Language::Vietnamese, "TÁC GIẢ"},
        {Language::Khmer, "អ្នកនិពន្ធ"},
        {Language::English, "AUTHOR"}
    };
    m_dictionary["TABLE_HEADER_YEAR"] = {
        {Language::Vietnamese, "NĂM XB"},
        {Language::Khmer, "ឆ្នាំបោះពុម្ព"},
        {Language::English, "YEAR"}
    };
    m_dictionary["TITLE_MANAGER_BOOK"] = {
        {Language::Vietnamese, " Quản Lý Sách"},
        {Language::Khmer, " គ្រប់គ្រងសៀវភៅ"},
        {Language::English, " Book Management"}
    };
    m_dictionary["BTN_ADD_NEW_BOOK"] = {
        {Language::Vietnamese, "Thêm Sách Mới"},
        {Language::Khmer, "បន្ថែមសៀវភៅថ្មី"},
        {Language::English, "Add New Book"}
    };
    m_dictionary["PLACEHOLDER_SEARCH_BOOK"] = {
        {Language::Vietnamese, "Tìm kiếm theo ISBN, Tên sách, Tác giả..."},
        {Language::Khmer, "ស្វែងរកតាម ISBN, ឈ្មោះសៀវភៅ, អ្នកនិពន្ធ..."},
        {Language::English, "Search by ISBN, Title, Author..."}
    };
    m_dictionary["OPTION_ALL_STATUS"] = {
        {Language::Vietnamese, "Tất cả trạng thái"},
        {Language::Khmer, "ស្ថានភាពទាំងអស់"},
        {Language::English, "All Statuses"}
    };
    m_dictionary["BTN_SEARCH_SYMBOL"] = {
        {Language::Vietnamese, "Tìm kiếm"}, // Or "Tìm kiếm"
        {Language::Khmer, "ស្វែងរក"},
        {Language::English, "Search"}
    };
    m_dictionary["BTN_DELETE_SYMBOL"] = {
        {Language::Vietnamese, "Xóa"},
        {Language::Khmer, "លុប"},
        {Language::English, "Delete"}
    };
    m_dictionary["BTN_PRINT_LIST_SYMBOL"] = {
        {Language::Vietnamese, "In Danh Sách"},
        {Language::Khmer, "បោះពុម្ពបញ្ជី"},
        {Language::English, "Print List"}
    };
    m_dictionary["BTN_ADD_COPY_SYMBOL"] = {
        {Language::Vietnamese, "Thêm Bản Sao"},
        {Language::Khmer, "បន្ថែមច្បាប់ចម្លង"},
        {Language::English, "Add Copy"}
    };
    m_dictionary["TAB_BOOK_LIST_SYMBOL"] = {
        {Language::Vietnamese, "Danh Sách Sách"},
        {Language::Khmer, "បញ្ជីសៀវភៅ"},
        {Language::English, "Book List"}
    };
    m_dictionary["TAB_STATS_SYMBOL"] = {
        {Language::Vietnamese, "Thống Kê"},
        {Language::Khmer, "ស្ថិតិ"},
        {Language::English, "Statistics"}
    };
    m_dictionary["TITLE_TOP_10_BORROWED"] = {
        {Language::Vietnamese, "TOP 10 SÁCH ĐƯỢC MƯỢN NHIỀU NHẤT"},
        {Language::Khmer, "សៀវភៅ ១០ ក្បាលដែលត្រូវបានខ្ចីច្រើនបំផុត"},
        {Language::English, "TOP 10 MOST BORROWED BOOKS"}
    };
    m_dictionary["TITLE_STATS_BY_CATEGORY"] = {
        {Language::Vietnamese, "THỐNG KÊ SÁCH THEO THỂ LOẠI"},
        {Language::Khmer, "ស្ថិតិសៀវភៅតាមប្រភេទ"},
        {Language::English, "STATISTICS BY CATEGORY"}
    };
    m_dictionary["MSG_FEATURE_DEVELOPING"] = {
        {Language::Vietnamese, "Chức năng đang được phát triển"},
        {Language::Khmer, "មុខងារនេះកំពុងស្ថិតក្នុងការអភិវឌ្ឍ"},
        {Language::English, "Feature is under development"}
    };
    m_dictionary["LBL_TOTAL_BOOKS_COUNT"] = { // "Tổng số: 0 sách" format needs thinking. Using "Tổng số: %1 sách"
        {Language::Vietnamese, "Tổng số: %1 sách"},
        {Language::Khmer, "សរុប: %1 សៀវភៅ"},
        {Language::English, "Total: %1 books"}
    };
    m_dictionary["OPTION_STATUS_AVAILABLE_SYMBOL"] = {
        {Language::Vietnamese, "Có sẵn"},
        {Language::Khmer, "មាននៅបណ្ណាល័យ"},
        {Language::English, "Available"}
    };
    m_dictionary["OPTION_STATUS_LIQUIDATED_SYMBOL"] = {
        {Language::Vietnamese, "Thanh lý"}, // "Đã thanh lý" vs "Thanh lý"
        {Language::Khmer, "បានលុបចោល"},
        {Language::English, "Liquidated"}
    };
    m_dictionary["DIALOG_EDIT_BOOK_TITLE"] = {
        {Language::Vietnamese, "Chỉnh Sửa Đầu Sách"},
        {Language::Khmer, "កែសម្រួលសៀវភៅ"},
        {Language::English, "Edit Book"}
    };
    m_dictionary["TITLE_EDIT_BOOK_HEADER"] = {
        {Language::Vietnamese, "CHỈNH SỬA THÔNG TIN SÁCH"},
        {Language::Khmer, "កែសម្រួលព័ត៌មានសៀវភៅ"},
        {Language::English, "EDIT BOOK INFO"}
    };
    m_dictionary["BTN_SAVE_CHANGES"] = {
        {Language::Vietnamese, "Lưu Thay Đổi"},
        {Language::Khmer, "រក្សាទុកការផ្លាស់ប្តូរ"},
        {Language::English, "Save Changes"}
    };
    m_dictionary["BTN_SAVE"] = {
        {Language::Vietnamese, "Lưu"},
        {Language::Khmer, "រក្សាទុក"},
        {Language::English, "Save"}
    };
    m_dictionary["MSG_PROCESSING"] = {
        {Language::Vietnamese, "Đang xử lý..."},
        {Language::Khmer, "កំពុងដំណើរការ..."},
        {Language::English, "Processing..."}
    };
    m_dictionary["MSG_UPDATING"] = {
        {Language::Vietnamese, "Đang cập nhật..."},
        {Language::Khmer, "កំពុងធ្វើបច្ចុប្បន្នភាព..."},
        {Language::English, "Updating..."}
    };
    m_dictionary["MSG_UPDATE_SUCCESS_VAR"] = {
        {Language::Vietnamese, "Đã cập nhật sách '%1' thành công!"},
        {Language::Khmer, "បានធ្វើបច្ចុប្បន្នភាពសៀវភៅ '%1' ដោយជោគជ័យ!"},
        {Language::English, "Updated book '%1' successfully!"}
    };
    m_dictionary["BTN_CANCEL"] = {
        {Language::Vietnamese, "Hủy"},
        {Language::Khmer, "បោះបង់"},
        {Language::English, "Cancel"}
    };
    m_dictionary["TITLE_ERROR"] = {
        {Language::Vietnamese, "Lỗi"},
        {Language::Khmer, "កំហុស"},
        {Language::English, "Error"}
    };
    m_dictionary["MSG_BOOK_NOT_FOUND"] = {
        {Language::Vietnamese, "Không tìm thấy sách!"},
        {Language::Khmer, "រកមិនឃើញសៀវភៅទេ!"},
        {Language::English, "Book not found!"}
    };
    m_dictionary["TITLE_HELLO"] = {
        {Language::Vietnamese, "Xin chào!"},
        {Language::Khmer, "សួស្តី!"},
        {Language::English, "Hello!"}
    };
    m_dictionary["MSG_LOGIN_SUBTITLE"] = {
        {Language::Vietnamese, "Đăng nhập để quản lý thư viện"},
        {Language::Khmer, "ចូលដើម្បីគ្រប់គ្រងបណ្ណាល័យ"},
        {Language::English, "Login to manage library"}
    };
    m_dictionary["LBL_USERNAME"] = {
        {Language::Vietnamese, "Tên đăng nhập"},
        {Language::Khmer, "ឈ្មោះអ្នកប្រើ"},
        {Language::English, "Username"}
    };
    m_dictionary["LBL_PASSWORD"] = {
        {Language::Vietnamese, "Mật khẩu"},
        {Language::Khmer, "ពាក្យសម្ងាត់"},
        {Language::English, "Password"}
    };
    m_dictionary["BTN_LOGIN_NOW"] = {
        {Language::Vietnamese, "ĐĂNG NHẬP NGAY"},
        {Language::Khmer, "ចូលឥឡូវនេះ"},
        {Language::English, "LOGIN NOW"}
    };
    m_dictionary["BTN_EXIT_APP"] = {
        {Language::Vietnamese, "Thoát ứng dụng"},
        {Language::Khmer, "ចាកចេញពីកម្មវិធី"},
        {Language::English, "Exit Application"}
    };
    m_dictionary["MSG_LOGIN_MISSING_INFO"] = {
        {Language::Vietnamese, "Vui lòng nhập đầy đủ thông tin!"},
        {Language::Khmer, "សូមបញ្ចូលព័ត៌មានពេញលេញ!"},
        {Language::English, "Please enter full information!"}
    };
    m_dictionary["DIALOG_SETTINGS_TITLE"] = {
        {Language::Vietnamese, "Cài Đặt Hệ Thống"},
        {Language::Khmer, "ការកំណត់ប្រព័ន្ធ"},
        {Language::English, "System Settings"}
    };
    m_dictionary["TAB_GENERAL_INFO"] = {
        {Language::Vietnamese, "Thông Tin Chung"},
        {Language::Khmer, "ព័ត៌មានទូទៅ"},
        {Language::English, "General Info"}
    };
    m_dictionary["TAB_BORROW_RULES"] = {
        {Language::Vietnamese, "Quy Định Mượn Trả"},
        {Language::Khmer, "ច្បាប់ខ្ចី/សង"},
        {Language::English, "Borrow/Return Rules"}
    };
    m_dictionary["TAB_PRINT_CONFIG"] = {
        {Language::Vietnamese, "Cấu Hình In Ấn"},
        {Language::Khmer, "ការកំណត់ការបោះពុម្ព"},
        {Language::English, "Print Configuration"}
    };
    m_dictionary["LBL_LIBRARY_NAME"] = {
        {Language::Vietnamese, "Tên Thư Viện"},
        {Language::Khmer, "ឈ្មោះបណ្ណាល័យ"},
        {Language::English, "Library Name"}
    };
    m_dictionary["LBL_ADDRESS"] = {
        {Language::Vietnamese, "Địa Chỉ"},
        {Language::Khmer, "អាសយដ្ឋាន"},
        {Language::English, "Address"}
    };
    m_dictionary["LBL_FINE_PER_DAY"] = {
        {Language::Vietnamese, "Tiền phạt quá hạn / ngày"},
        {Language::Khmer, "ផាកពិន័យហួសកំណត់ / ថ្ងៃ"},
        {Language::English, "Overdue fine / day"}
    };
    m_dictionary["LBL_MAX_BORROW_DAYS"] = {
        {Language::Vietnamese, "Thời hạn mượn mặc định"},
        {Language::Khmer, "រយៈពេលខ្ចីលំនាំដើម"},
        {Language::English, "Default borrow duration"}
    };
    m_dictionary["LBL_MAX_RENEWALS"] = {
        {Language::Vietnamese, "Số lần gia hạn tối đa"},
        {Language::Khmer, "ចំនួនដងបន្តអតិបរមា"},
        {Language::English, "Max renewals"}
    };
    m_dictionary["LBL_MAX_BORROW_BOOKS"] = {
        {Language::Vietnamese, "Số sách mượn tối đa"},
        {Language::Khmer, "ចំនួនសៀវភៅខ្ចីអតិបរមា"},
        {Language::English, "Max books to borrow"}
    };
    m_dictionary["CB_SILENT_PRINT"] = {
        {Language::Vietnamese, "Bật chế độ in yên lặng (Không hiện hộp thoại chọn máy in)"},
        {Language::Khmer, "បើកការបោះពុម្ពស្ងាត់ (មិនបង្ហាញប្រអប់ជ្រើសរើសម៉ាស៊ីនបោះពុម្ព)"},
        {Language::English, "Enable silent printing (No printer dialog)"}
    };
    m_dictionary["LBL_PRINT_MODE"] = {
        {Language::Vietnamese, "Chế độ in"},
        {Language::Khmer, "របៀបបោះពុម្ព"},
        {Language::English, "Print Mode"}
    };
    m_dictionary["LBL_PRINTER_RECEIPT"] = {
        {Language::Vietnamese, "Máy in Hóa đơn/Tem (Receipt)"},
        {Language::Khmer, "ម៉ាស៊ីនបោះពុម្ពវិក្កយបត្រ (Receipt)"},
        {Language::English, "Receipt Printer"}
    };
    m_dictionary["LBL_PRINTER_REPORT"] = {
        {Language::Vietnamese, "Máy in Báo cáo A4 (Report)"},
        {Language::Khmer, "ម៉ាស៊ីនបោះពុម្ពរបាយការណ៍ (Report)"},
        {Language::English, "Report Printer"}
    };
    m_dictionary["OPTION_SYSTEM_DEFAULT"] = {
        {Language::Vietnamese, "-- Mặc định hệ thống --"},
        {Language::Khmer, "-- លំនាំដើមប្រព័ន្ធ --"},
        {Language::English, "-- System Default --"}
    };
    m_dictionary["BTN_RESTORE_DEFAULT"] = {
        {Language::Vietnamese, "Khôi phục mặc định"},
        {Language::Khmer, "ស្តារលំនាំដើម"},
        {Language::English, "Restore Defaults"}
    };
    m_dictionary["MSG_SAVE_SETTINGS_SUCCESS"] = {
        {Language::Vietnamese, "Đã lưu cài đặt thành công!"},
        {Language::Khmer, "ការកំណត់ត្រូវបានរក្សាទុកដោយជោគជ័យ!"},
        {Language::English, "Settings saved successfully!"}
    };
    m_dictionary["DIALOG_PRINT_MANAGER_TITLE"] = {
        {Language::Vietnamese, "Quản Lý In Ấn"},
        {Language::Khmer, "ការគ្រប់គ្រងការបោះពុម្ព"},
        {Language::English, "Print Manager"}
    };
    m_dictionary["HEADER_PRINT_LIBRARY"] = {
        {Language::Vietnamese, "IN ẤN THƯ VIỆN"},
        {Language::Khmer, "បោះពុម្ពបណ្ណាល័យ"},
        {Language::English, "LIBRARY PRINTING"}
    };
    m_dictionary["GROUP_READER_CARDS"] = {
        {Language::Vietnamese, "THẺ HỌC VIÊN"},
        {Language::Khmer, "ប័ណ្ណអ្នកអាន"},
        {Language::English, "READER CARDS"}
    };
    m_dictionary["GROUP_BOOK_STAMPS"] = {
        {Language::Vietnamese, "TEM SÁCH"},
        {Language::Khmer, "តែមសៀវភៅ"},
        {Language::English, "BOOK STAMPS"}
    };
    m_dictionary["GROUP_BOOK_MGMT"] = {
        {Language::Vietnamese, "QUẢN LÝ TEM SÁCH"},
        {Language::Khmer, "គ្រប់គ្រងតែមសៀវភៅ"},
        {Language::English, "BOOK STAMPS MANAGEMENT"}
    };
    m_dictionary["GROUP_REPORTS"] = {
        {Language::Vietnamese, "BÁO CÁO & BIÊN BẢN"},
        {Language::Khmer, "របាយការណ៍ & កំណត់ហេតុ"},
        {Language::English, "REPORTS & MINUTES"}
    };
    m_dictionary["GROUP_REPORT_STATS"] = {
        {Language::Vietnamese, "BÁO CÁO & THỐNG KÊ"},
        {Language::Khmer, "របាយការណ៍ & ស្ថិតិ"},
        {Language::English, "REPORTS & STATISTICS"}
    };
    m_dictionary["BTN_PRINT_CARD_CURRENT"] = {
        {Language::Vietnamese, "In Thẻ (Hiện Tại)"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណ (បច្ចុប្បន្ន)"},
        {Language::English, "Print Card (Current)"}
    };
    m_dictionary["BTN_PRINT_CARD"] = {
        {Language::Vietnamese, "In Thẻ"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណ (Print Card)"},
        {Language::English, "Print Card"}
    };
    m_dictionary["DESC_PRINT_CARD"] = {
        {Language::Vietnamese, "In thẻ đơn cho học viên đang chọn"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណសម្រាប់សិស្សដែលបានជ្រើសរើស"},
        {Language::English, "Print card for selected student"}
    };
    m_dictionary["DESC_PRINT_CARD_CURRENT"] = {
        {Language::Vietnamese, "In thẻ đơn cho học viên đang chọn"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណតែមួយសម្រាប់អ្នកអានដែលបានជ្រើសរើស"},
        {Language::English, "Print single card for selected reader"}
    };
    m_dictionary["BTN_PRINT_CARD_BATCH"] = {
        {Language::Vietnamese, "In Hàng Loạt"},
        {Language::Khmer, "បោះពុម្ពច្រើន"},
        {Language::English, "Batch Print"}
    };
    m_dictionary["DESC_PRINT_CARD_BATCH"] = {
        {Language::Vietnamese, "Danh sách thẻ nhiều học viên"},
        {Language::Khmer, "បញ្ជីប័ណ្ណអ្នកអានជាច្រើន"},
        {Language::English, "List of multiple reader cards"}
    };
    m_dictionary["BTN_SEARCH_PRINT_STAMP"] = {
        {Language::Vietnamese, "Tìm & In Tem"},
        {Language::Khmer, "ស្វែងរក & បោះពុម្ពតែម"},
        {Language::English, "Search & Print Stamp"}
    };
    m_dictionary["DESC_SEARCH_PRINT_STAMP"] = {
        {Language::Vietnamese, "Tìm theo ISBN/Tên để in tem dán"},
        {Language::Khmer, "ស្វែងរកតាម ISBN/ឈ្មោះ ដើម្បីបោះពុម្ពតែម"},
        {Language::English, "Search by ISBN/Name to print sticker"}
    };
    m_dictionary["BTN_PRINT_STAMP"] = {
        {Language::Vietnamese, "In Tem Sách"},
        {Language::Khmer, "បោះពុម្ពតែមសៀវភៅ"},
        {Language::English, "Print Book Stamp"}
    };
    m_dictionary["DESC_PRINT_STAMP"] = {
        {Language::Vietnamese, "In tem mã vạch dán lên sách"},
        {Language::Khmer, "បោះពុម្ពតែមបាកូដសម្រាប់បិទលើសៀវភៅ"},
        {Language::English, "Print barcode stamp for books"}
    };
    m_dictionary["BTN_LIST_READERS"] = {
        {Language::Vietnamese, "DS Học Viên"},
        {Language::Khmer, "បញ្ជីអ្នកអាន"},
        {Language::English, "Reader List"}
    };
    m_dictionary["DESC_LIST_READERS"] = {
        {Language::Vietnamese, "Danh sách toàn bộ học viên"},
        {Language::Khmer, "បញ្ជីអ្នកអានទាំងអស់"},
        {Language::English, "List of all readers"}
    };
    m_dictionary["BTN_LIST_BOOKS"] = {
        {Language::Vietnamese, "Danh Mục Sách"},
        {Language::Khmer, "បញ្ជីសៀវភៅ"},
        {Language::English, "Book Catalog"}
    };
    m_dictionary["DESC_LIST_BOOKS"] = {
        {Language::Vietnamese, "Danh sách toàn bộ sách"},
        {Language::Khmer, "បញ្ជីសៀវភៅទាំងអស់"},
        {Language::English, "List of all books"}
    };
    m_dictionary["BTN_REPORT_READER"] = {
        {Language::Vietnamese, "Báo Cáo Học Viên"},
        {Language::Khmer, "របាយការណ៍សិស្ស (Student Report)"},
        {Language::English, "Student Report"}
    };
    m_dictionary["DESC_REPORT_READER"] = {
        {Language::Vietnamese, "In danh sách toàn bộ học viên"},
        {Language::Khmer, "បោះពុម្ពបញ្ជីសិស្សទាំងអស់"},
        {Language::English, "Print all students list"}
    };
    m_dictionary["BTN_REPORT_BOOK"] = {
        {Language::Vietnamese, "Báo Cáo Sách"},
        {Language::Khmer, "របាយការណ៍សៀវភៅ (Book Report)"},
        {Language::English, "Book Report"}
    };
    m_dictionary["DESC_REPORT_BOOK"] = {
        {Language::Vietnamese, "In danh sách toàn bộ các đầu sách"},
        {Language::Khmer, "បោះពុម្ពបញ្ជីសៀវភៅទាំងអស់"},
        {Language::English, "Print list of all books"}
    };
    m_dictionary["BTN_BORROW_SLIP"] = {
        {Language::Vietnamese, "Phiếu Mượn"},
        {Language::Khmer, "ប័ណ្ណខ្ចី"},
        {Language::English, "Borrow Slip"}
    };
    m_dictionary["DESC_BORROW_SLIP"] = {
        {Language::Vietnamese, "In biên bản mượn sách"},
        {Language::Khmer, "បោះពុម្ពកំណត់ហេតុខ្ចីសៀវភៅ"},
        {Language::English, "Print borrow minutes"}
    };
    m_dictionary["BTN_RETURN_SLIP"] = {
        {Language::Vietnamese, "Phiếu Trả"},
        {Language::Khmer, "ប័ណ្ណសង"},
        {Language::English, "Return Slip"}
    };
    m_dictionary["DESC_RETURN_SLIP"] = {
        {Language::Vietnamese, "In biên bản trả sách"},
        {Language::Khmer, "បោះពុម្ពកំណត់ហេតុសងសៀវភៅ"},
        {Language::English, "Print return minutes"}
    };
    m_dictionary["BTN_PRINT_CONFIG"] = {
        {Language::Vietnamese, "Cấu Hình In Ấn"},
        {Language::Khmer, "ការកំណត់ការបោះពុម្ព"},
        {Language::English, "Print Config"}
    };
    m_dictionary["MSG_SELECT_READER_FIRST"] = {
        {Language::Vietnamese, "Vui lòng chọn học viên trước khi in thẻ!"},
        {Language::Khmer, "សូមជ្រើសរើសអ្នកអានមុននឹងបោះពុម្ពប័ណ្ណ!"},
        {Language::English, "Please select a reader before printing card!"}
    };
    m_dictionary["BTN_CONFIG_PRINT"] = {
        {Language::Vietnamese, "Cấu Hình In"},
        {Language::Khmer, "ការកំណត់ការបោះពុម្ព"},
        {Language::English, "Print Configuration"}
    };
    m_dictionary["DESC_CONFIG_PRINT"] = {
        {Language::Vietnamese, "Cấu hình khổ giấy và máy in mặc định"},
        {Language::Khmer, "កំណត់រចនាសម្ព័ន្ធក្រដាស និងម៉ាស៊ីនបោះពុម្ពលំនាំដើម"},
        {Language::English, "Configure paper size and default printer"}
    };
    m_dictionary["MSG_NO_READERS"] = {
        {Language::Vietnamese, "Không có học viên nào trong hệ thống!"},
        {Language::Khmer, "គ្មានអ្នកអាននៅក្នុងប្រព័ន្ធទេ!"},
        {Language::English, "No readers in the system!"}
    };
    m_dictionary["TITLE_CONFIRM_BATCH_PRINT"] = {
        {Language::Vietnamese, "Xác Nhận In Hàng Loạt"},
        {Language::Khmer, "បញ្ជាក់ការបោះពុម្ពច្រើន"},
        {Language::English, "Confirm Batch Print"}
    };

    // ==================== BACKUP SCREEN ====================
    m_dictionary["TITLE_BACKUP_MANAGER"] = { {Language::Vietnamese, "Quản Lý Sao Lưu"}, {Language::Khmer, "ការគ្រប់គ្រងការបម្រុងទុក"}, {Language::English, "Backup Management"} };
    m_dictionary["HEADER_BACKUP"] = { {Language::Vietnamese, "QUẢN LÝ SAO LƯU DỮ LIỆU"}, {Language::Khmer, "ការគ្រប់គ្រងការបម្រុងទុកទិន្នន័យ"}, {Language::English, "DATABASE BACKUP MANAGEMENT"} };
    m_dictionary["MSG_BACKUP_GUIDE_HTML"] = { 
        {Language::Vietnamese, "<b>Hướng dẫn:</b><br>• <b>Tạo Backup:</b> Sao lưu toàn bộ dữ liệu hiện tại<br>• <b>Phục hồi:</b> Khôi phục dữ liệu từ bản sao lưu đã chọn<br>• <b>Xóa:</b> Xóa bản sao lưu đã chọn<br>• <b>Xóa cũ:</b> Xóa các bản sao lưu cũ, giữ lại n bản mới nhất"},
        {Language::Khmer, "<b>ការណែនាំ:</b><br>• <b>បង្កើត:</b> បម្រុងទុកទិន្នន័យបច្ចុប្បន្ន<br>• <b>ស្តារ:</b> ស្តារទិន្នន័យពីការបម្រុងទុកដែលបានជ្រើសរើស<br>• <b>លុប:</b> លុបការបម្រុងទុកដែលបានជ្រើសរើស<br>• <b>សម្អាត:</b> លុបការបម្រុងទុកចាស់ រក្សាទុក n ថ្មីបំផុត"},
        {Language::English, "<b>Guide:</b><br>• <b>Create:</b> Backup current data<br>• <b>Restore:</b> Restore data from selected backup<br>• <b>Delete:</b> Delete selected backup<br>• <b>Cleanup:</b> Delete old backups, keep n latest"}
    };
    m_dictionary["BTN_CREATE_BACKUP"] = { {Language::Vietnamese, "Tạo Backup"}, {Language::Khmer, "បង្កើតការបម្រុងទុក"}, {Language::English, "Create Backup"} };
    m_dictionary["BTN_RESTORE_BACKUP"] = { {Language::Vietnamese, "Phục Hồi"}, {Language::Khmer, "ស្តារ"}, {Language::English, "Restore"} };
    m_dictionary["BTN_DELETE_BACKUP"] = { {Language::Vietnamese, "Xóa"}, {Language::Khmer, "លុប"}, {Language::English, "Delete"} };
    m_dictionary["BTN_CLEANUP_BACKUP"] = { {Language::Vietnamese, "Xóa Cũ"}, {Language::Khmer, "សម្អាតចាស់"}, {Language::English, "Cleanup Old"} };
    m_dictionary["MSG_NO_BACKUP"] = { {Language::Vietnamese, "Chưa có bản sao lưu nào"}, {Language::Khmer, "មិនមានការបម្រុងទុកទេ"}, {Language::English, "No backups found"} };
    m_dictionary["MSG_BACKUP_FOUND"] = { {Language::Vietnamese, "Tìm thấy %1 bản sao lưu"}, {Language::Khmer, "រកឃើញការបម្រុងទុក %1"}, {Language::English, "Found %1 backups"} };
    m_dictionary["MSG_CONFIRM_RESTORE"] = { {Language::Vietnamese, "Bạn có chắc muốn phục hồi từ:\n\n%1\n%2\n\nDữ liệu hiện tại sẽ bị ghi đè!"}, {Language::Khmer, "តើអ្នកប្រាកដថាចង់ស្តារពី:\n\n%1\n%2\n\nទិន្នន័យបច្ចុប្បន្ននឹងត្រូវសរសេរជាន់ពីលើ!"}, {Language::English, "Are you sure you want to restore from:\n\n%1\n%2\n\nCurrent data will be overwritten!"} };
    m_dictionary["MSG_CONFIRM_DELETE_BACKUP"] = { {Language::Vietnamese, "Bạn có chắc muốn xóa bản sao lưu:\n\n%1\n%2"}, {Language::Khmer, "តើអ្នកប្រាកដថាចង់លុបការបម្រុងទុក:\n\n%1\n%2"}, {Language::English, "Are you sure you want to delete backup:\n\n%1\n%2"} };
    m_dictionary["MSG_DELETE_SUCCESS"] = { {Language::Vietnamese, "Đã xóa bản sao lưu"}, {Language::Khmer, "បានលុបការបម្រុងទុក"}, {Language::English, "Backup deleted"} };
    m_dictionary["TITLE_CLEANUP"] = { {Language::Vietnamese, "Xóa backup cũ"}, {Language::Khmer, "សម្អាតការបម្រុងទុកចាស់"}, {Language::English, "Cleanup Old Backups"} };
    m_dictionary["MSG_CLEANUP_PROMPT"] = { {Language::Vietnamese, "Nhập số bản sao lưu muốn giữ lại (mới nhất):"}, {Language::Khmer, "បញ្ចូលចំនួនការបម្រុងទុកដែលត្រូវរក្សាទុក (ថ្មីបំផុត):"}, {Language::English, "Enter number of backups to keep (newest):"} };
    m_dictionary["MSG_CLEANUP_SUCCESS"] = { {Language::Vietnamese, "Đã xóa %1 bản cũ!\n\nGiữ lại %2 bản mới nhất."}, {Language::Khmer, "បានលុប %1 ចាស់!\n\nរក្សាទុក %2 ថ្មីបំផុត។"}, {Language::English, "Deleted %1 old backups!\n\nKept %2 newest."} };
    
    // ==================== TOP 10 SCREEN ====================
    m_dictionary["TITLE_TOP10"] = { {Language::Vietnamese, "Top 10 Sách Được Mượn Nhiều Nhất"}, {Language::Khmer, "សៀវភៅ ១០ ក្បាលដែលត្រូវបានខ្ចីច្រើនបំផុត"}, {Language::English, "Top 10 Most Borrowed Books"} };
    m_dictionary["HEADER_TOP10"] = { {Language::Vietnamese, "TOP 10 SÁCH PHỔ BIẾN NHẤT"}, {Language::Khmer, "សៀវភៅពេញនិយមបំផុតទាំង ១០"}, {Language::English, "TOP 10 POPULAR BOOKS"} };
    m_dictionary["GROUP_FILTER_TIME"] = { {Language::Vietnamese, "Bộ Lọc Thời Gian"}, {Language::Khmer, "តម្រងពេលវេលា"}, {Language::English, "Time Filter"} };
    m_dictionary["LBL_RANGE"] = { {Language::Vietnamese, "Khoảng:"}, {Language::Khmer, "ចន្លោះ:"}, {Language::English, "Range:"} };
    m_dictionary["LBL_FROM_DATE"] = { {Language::Vietnamese, "Từ ngày:"}, {Language::Khmer, "ចាប់ពីថ្ងៃ:"}, {Language::English, "From:"} };
    m_dictionary["LBL_TO_DATE"] = { {Language::Vietnamese, "Đến:"}, {Language::Khmer, "ដល់:"}, {Language::English, "To:"} };
    m_dictionary["CB_TIME_ALL"] = { {Language::Vietnamese, "Tất cả thời gian"}, {Language::Khmer, "គ្រប់ពេលវេលា"}, {Language::English, "All Time"} };
    m_dictionary["CB_TIME_THIS_MONTH"] = { {Language::Vietnamese, "Tháng này"}, {Language::Khmer, "ខែនេះ"}, {Language::English, "This Month"} };
    m_dictionary["CB_TIME_LAST_MONTH"] = { {Language::Vietnamese, "Tháng trước"}, {Language::Khmer, "ខែមុន"}, {Language::English, "Last Month"} };
    m_dictionary["CB_TIME_CUSTOM"] = { {Language::Vietnamese, "Tùy chỉnh"}, {Language::Khmer, "ផ្ទាល់ខ្លួន"}, {Language::English, "Custom"} };
    m_dictionary["CHECK_SHOW_TREND"] = { {Language::Vietnamese, "Hiển thị xu hướng"}, {Language::Khmer, "បង្ហាញនិន្នាការ"}, {Language::English, "Show Trend"} };
    m_dictionary["MSG_TOP10_NOTE"] = { {Language::Vietnamese, "<b>Thống kê:</b> Xếp hạng dựa trên tổng số lượt sách đã được mượn. Bao gồm cả sách đang mượn và đã trả."}, {Language::Khmer, "<b>ស្ថិតិ:</b> ចំណាត់ថ្នាក់ផ្អែកលើចំនួនសរុបនៃការខ្ចីសៀវភៅ។ រួមបញ្ចូលទាំងសៀវភៅដែលកំពុងខ្ចី និងបានសង។"}, {Language::English, "<b>Stats:</b> Ranking based on total borrow count. Includes both active and returned loans."} };
    m_dictionary["TABLE_HEADER_RANK"] = { {Language::Vietnamese, "Hạng"}, {Language::Khmer, "ចំណាត់ថ្នាក់"}, {Language::English, "Rank"} };
    m_dictionary["TABLE_HEADER_LOANS"] = { {Language::Vietnamese, "Số Lượt"}, {Language::Khmer, "ចំនួនដង"}, {Language::English, "Count"} };
    m_dictionary["TABLE_HEADER_RATE"] = { {Language::Vietnamese, "Tỷ Lệ"}, {Language::Khmer, "អត្រា"}, {Language::English, "Rate"} };
    m_dictionary["TABLE_HEADER_TREND"] = { {Language::Vietnamese, "Xu Hướng"}, {Language::Khmer, "និន្នាការ"}, {Language::English, "Trend"} };
    m_dictionary["BTN_EXPORT_REPORT"] = { {Language::Vietnamese, "Xuất Báo Cáo"}, {Language::Khmer, "នាំចេញរបាយការណ៍"}, {Language::English, "Export Report"} };
    m_dictionary["MSG_NO_DATA_TOP10"] = { {Language::Vietnamese, "Chưa có dữ liệu sách trong hệ thống"}, {Language::Khmer, "គ្មានទិន្នន័យសៀវភៅនៅក្នុងប្រព័ន្ធទេ"}, {Language::English, "No book data in system"} };
    m_dictionary["MSG_NO_DATA_PERIOD"] = { {Language::Vietnamese, "Chưa có sách nào được mượn trong khoảng thời gian này"}, {Language::Khmer, "គ្មានសៀវភៅណាមួយត្រូវបានខ្ចីក្នុងអំឡុងពេលនេះទេ"}, {Language::English, "No books borrowed in this period"} };
    m_dictionary["MSG_STATS_SUMMARY_HTML"] = { {Language::Vietnamese, "Khoảng: <b>%1</b> | %2 đầu sách | Tổng lượt: %3 | Hiển thị top %4"}, {Language::Khmer, "ចន្លោះ: <b>%1</b> | %2 សៀវភៅ | សរុប: %3 | បង្ហាញកំពូល %4"}, {Language::English, "Range: <b>%1</b> | %2 titles | Total: %3 | Top %4"} };
    m_dictionary["MSG_TREND_POSITIVE"] = { {Language::Vietnamese, "<b>Xu hướng tích cực:</b> Có %1 sách tăng, %2 sách giảm, %3 sách không đổi so với kỳ trước"}, {Language::Khmer, "<b>និន្នាការវិជ្ជមាន:</b> មាន %1 សៀវភៅកើនឡើង, %2 ថយចុះ, %3 ថេរ"}, {Language::English, "<b>Positive:</b> %1 up, %2 down, %3 stable"} };
    m_dictionary["MSG_TREND_NEGATIVE"] = { {Language::Vietnamese, "<b>Xu hướng giảm:</b> Có %1 sách giảm, %2 sách tăng, %3 sách không đổi so với kỳ trước"}, {Language::Khmer, "<b>និន្នាការអវិជ្ជមាន:</b> មាន %1 សៀវភៅថយចុះ, %2 កើនឡើង, %3 ថេរ"}, {Language::English, "<b>Negative:</b> %1 down, %2 up, %3 stable"} };
    m_dictionary["MSG_TREND_STABLE"] = { {Language::Vietnamese, "<b>Xu hướng ổn định:</b> Có %1 sách tăng, %2 sách giảm, %3 sách không đổi"}, {Language::Khmer, "<b>និន្នាការថេរ:</b> មាន %1 សៀវភៅកើនឡើង, %2 ថយចុះ, %3 ថេរ"}, {Language::English, "<b>Stable:</b> %1 up, %2 down, %3 stable"} };
    m_dictionary["MSG_CONFIRM_BATCH_PRINT"] = {
        {Language::Vietnamese, "In thẻ cho TẤT CẢ học viên trong hệ thống?"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណសម្រាប់អ្នកអានទាំងអស់នៅក្នុងប្រព័ន្ធ?"},
        {Language::English, "Print cards for ALL readers in the system?"}
    };
    m_dictionary["MSG_NO_BOOKS_SYSTEM"] = {
        {Language::Vietnamese, "Không có sách nào trong hệ thống!"},
        {Language::Khmer, "គ្មានសៀវភៅនៅក្នុងប្រព័ន្ធទេ!"},
        {Language::English, "No books in the system!"}
    };
    m_dictionary["MSG_READER_NO_BORROW"] = {
        {Language::Vietnamese, "Học viên này chưa mượn sách nào!"},
        {Language::Khmer, "សិស្សនេះមិនទាន់បានខ្ចីសៀវភៅទេ!"},
        {Language::English, "This reader has not borrowed any books!"}
    };
    m_dictionary["MSG_READER_NO_RETURN"] = {
        {Language::Vietnamese, "Học viên này không có sách nào đang mượn để trả!"},
        {Language::Khmer, "សិស្សនេះមិនមានសៀវភៅណាមួយកំពុងខ្ចីដើម្បីសងទេ!"},
        {Language::English, "This reader has no books to return!"}
    };
    m_dictionary["MSG_NO_BOOKS_TO_RETURN"] = {
        {Language::Vietnamese, "Không có sách nào cần trả!"},
        {Language::Khmer, "គ្មានសៀវភៅណាត្រូវសងទេ!"},
        {Language::English, "No books to return!"}
    };
    m_dictionary["TITLE_SAVE_CONFIG"] = {
        {Language::Vietnamese, "Lưu Cấu Hình"},
        {Language::Khmer, "រក្សាទុកការកំណត់"},
        {Language::English, "Save Config"}
    };
    m_dictionary["MSG_CONFIG_SAVED"] = {
        {Language::Vietnamese, "Cấu hình in đã được lưu."},
        {Language::Khmer, "ការកំណត់ការបោះពុម្ពត្រូវបានរក្សាទុក។"},
        {Language::English, "Print configuration saved."}
    };
    m_dictionary["TITLE_SEARCH_BOOK"] = {
        {Language::Vietnamese, "Tìm Sách"},
        {Language::Khmer, "ស្វែងរកសៀវភៅ"},
        {Language::English, "Search Book"}
    };
    m_dictionary["MSG_ENTER_ISBN_NAME"] = {
        {Language::Vietnamese, "Nhập ISBN hoặc tên sách:"},
        {Language::Khmer, "បញ្ចូល ISBN ឬឈ្មោះសៀវភៅ:"},
        {Language::English, "Enter ISBN or book name:"}
    };
    m_dictionary["TITLE_STAMP_QUANTITY"] = {
        {Language::Vietnamese, "Số Lượng Tem"},
        {Language::Khmer, "ចំនួនតែម"},
        {Language::English, "Stamp Quantity"}
    };
    m_dictionary["MSG_ENTER_STAMP_QUANTITY"] = {
        {Language::Vietnamese, "Số tem cần in:"},
        {Language::Khmer, "ចំនួនតែមដែលត្រូវបោះពុម្ព:"},
        {Language::English, "Number of stamps to print:"}
    };
    m_dictionary["TITLE_NOT_FOUND"] = {
        {Language::Vietnamese, "Không Tìm Thấy"},
        {Language::Khmer, "រកមិនឃើញ"},
        {Language::English, "Not Found"}
    };
    m_dictionary["DIALOG_PRINT_CENTER_TITLE"] = {
        {Language::Vietnamese, "Trung Tâm In Ấn & Báo Cáo"},
        {Language::Khmer, "មជ្ឈមណ្ឌលបោះពុម្ព & របាយការណ៍"},
        {Language::English, "Print & Report Center"}
    };
    m_dictionary["HEADER_CENTER_COMMAND"] = {
        {Language::Vietnamese, "CENTER COMMAND: PRINT & REPORT"},
        {Language::Khmer, "មជ្ឈមណ្ឌលបញ្ជា៖ បោះពុម្ព & របាយការណ៍"},
        {Language::English, "CENTER COMMAND: PRINT & REPORT"}
    };
    m_dictionary["BTN_CONFIG_PRINTER"] = {
        {Language::Vietnamese, "Cấu Hình Máy In"},
        {Language::Khmer, "ការកំណត់ម៉ាស៊ីនបោះពុម្ព"},
        {Language::English, "Printer Config"}
    };
    m_dictionary["GROUP_READER_MGMT"] = {
        {Language::Vietnamese, "QUẢN LÝ HỌC VIÊN"},
        {Language::Khmer, "ការគ្រប់គ្រងសិស្ស"},
        {Language::English, "READER MANAGEMENT"}
    };
    m_dictionary["BTN_PRINT_CARD_SINGLE"] = {
        {Language::Vietnamese, "In Thẻ Học Viên"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណសិស្ស"},
        {Language::English, "Print Reader Card"}
    };
    m_dictionary["DESC_PRINT_CARD_SINGLE"] = {
        {Language::Vietnamese, "In thẻ hiện tại"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណបច្ចុប្បន្ន"},
        {Language::English, "Print current card"}
    };
    m_dictionary["BTN_PRINT_CARD_BATCH_LIST"] = {
        {Language::Vietnamese, "In Thẻ Hàng Loạt"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណច្រើន"},
        {Language::English, "Batch Print Cards"}
    };
    m_dictionary["DESC_PRINT_CARD_BATCH_LIST"] = {
        {Language::Vietnamese, "In danh sách đang lọc"},
        {Language::Khmer, "បោះពុម្ពបញ្ជីដែលបានត្រង"},
        {Language::English, "Print filtered list"}
    };
    m_dictionary["BTN_PRINT_BORROW_SLIP"] = {
        {Language::Vietnamese, "In Phiếu Mượn"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណខ្ចី"},
        {Language::English, "Print Borrow Slip"}
    };
    m_dictionary["DESC_PRINT_BORROW_SLIP"] = {
        {Language::Vietnamese, "In phiếu mượn gần nhất"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណខ្ចីចុងក្រោយ"},
        {Language::English, "Print last borrow slip"}
    };
    m_dictionary["BTN_EXPORT_LIST_A4"] = {
        {Language::Vietnamese, "In Danh Sách"},
        {Language::Khmer, "បោះពុម្ពបញ្ជី"},
        {Language::English, "Print List"}
    };
    m_dictionary["DESC_EXPORT_LIST_A4"] = {
        {Language::Vietnamese, "Xuất ra A4"},
        {Language::Khmer, "នាំចេញទៅ A4"},
        {Language::English, "Export to A4"}
    };
    m_dictionary["GROUP_BOOK_MGMT"] = {
        {Language::Vietnamese, "QUẢN LÝ SÁCH (BOOK)"},
        {Language::Khmer, "ការគ្រប់គ្រងសៀវភៅ (BOOK)"},
        {Language::English, "BOOK MANAGEMENT"}
    };
    m_dictionary["BTN_PRINT_STAMP_SINGLE"] = {
        {Language::Vietnamese, "In Tem Mã Vạch"},
        {Language::Khmer, "បោះពុម្ពតែមបាកូដ"},
        {Language::English, "Print Barcode Stamp"}
    };
    m_dictionary["DESC_PRINT_STAMP_SINGLE"] = {
        {Language::Vietnamese, "In sách đang chọn"},
        {Language::Khmer, "បោះពុម្ពសៀវភៅដែលបានជ្រើសរើស"},
        {Language::English, "Print selected book"}
    };
    m_dictionary["BTN_PRINT_STAMP_BATCH"] = {
        {Language::Vietnamese, "In Tem Hàng Loạt"},
        {Language::Khmer, "បោះពុម្ពតែមច្រើន"},
        {Language::English, "Batch Print Stamps"}
    };
    m_dictionary["DESC_PRINT_STAMP_BATCH"] = {
        {Language::Vietnamese, "In tất cả sách trong kho"},
        {Language::Khmer, "បោះពុម្ពសៀវភៅទាំងអស់ក្នុងស្តុក"},
        {Language::English, "Print all books in stock"}
    };
    m_dictionary["BTN_PRINT_CATALOG"] = {
        {Language::Vietnamese, "In Danh Mục"},
        {Language::Khmer, "បោះពុម្ពកាតាឡុក"},
        {Language::English, "Print Catalog"}
    };
    m_dictionary["DESC_PRINT_CATALOG"] = {
        {Language::Vietnamese, "Xuất Excel/PDF"},
        {Language::Khmer, "នាំចេញ Excel/PDF"},
        {Language::English, "Export Excel/PDF"}
    };
    m_dictionary["GROUP_REPORT_STATS"] = {
        {Language::Vietnamese, "BÁO CÁO & THỐNG KÊ"},
        {Language::Khmer, "របាយការណ៍ & ស្ថិតិ"},
        {Language::English, "REPORTS & STATISTICS"}
    };
    m_dictionary["BTN_REPORT_READER"] = {
        {Language::Vietnamese, "Báo Cáo Học Viên"},
        {Language::Khmer, "របាយការណ៍អ្នកអាន"},
        {Language::English, "Reader Report"}
    };
    m_dictionary["DESC_REPORT_READER"] = {
        {Language::Vietnamese, "Số lượng, tình trạng"},
        {Language::Khmer, "បរិមាណ, ស្ថានភាព"},
        {Language::English, "Quantity, status"}
    };
    m_dictionary["BTN_REPORT_BOOK"] = {
        {Language::Vietnamese, "Báo Cáo Kho Sách"},
        {Language::Khmer, "របាយការណ៍ឃ្លាំងសៀវភៅ"},
        {Language::English, "Book Stock Report"}
    };
    m_dictionary["DESC_REPORT_BOOK"] = {
        {Language::Vietnamese, "Nhập xuất, tồn kho"},
        {Language::Khmer, "នាំចូល/ចេញ, សន្និធិ"},
        {Language::English, "Import/Export, Stock"}
    };
    m_dictionary["MSG_SELECT_AT_LEAST_ONE_READER"] = {
        {Language::Vietnamese, "Vui lòng chọn ít nhất 1 học viên từ danh sách để in hàng loạt."},
        {Language::Khmer, "សូមជ្រើសរើសយ៉ាងហោចណាស់អ្នកអាន 1 នាក់ពីបញ្ជីដើម្បីបោះពុម្ពច្រើន។"},
        {Language::English, "Please select at least 1 reader from list for batch printing."}
    };
    m_dictionary["MSG_CONFIRM_PRINT_READERS_COUNT"] = {
        {Language::Vietnamese, "Bạn có chắc muốn in thẻ cho %1 học viên đã chọn?"},
        {Language::Khmer, "តើអ្នកប្រាកដថាចង់បោះពុម្ពប័ណ្ណសម្រាប់អ្នកអាន %1 ដែលបានជ្រើសរើសទេ?"},
        {Language::English, "Are you sure you want to print cards for %1 selected readers?"}
    };
    m_dictionary["MSG_SELECT_AT_LEAST_ONE_BOOK"] = {
        {Language::Vietnamese, "Vui lòng chọn ít nhất 1 đầu sách từ danh sách."},
        {Language::Khmer, "សូមជ្រើសរើសយ៉ាងហោចណាស់សៀវភៅ 1 ក្បាលពីបញ្ជី។"},
        {Language::English, "Please select at least 1 book from list."}
    };
    m_dictionary["MSG_CONFIRM_PRINT_BOOKS_COUNT"] = {
        {Language::Vietnamese, "Bạn có chắc muốn in tem cho %1 đầu sách đã chọn?"},
        {Language::Khmer, "តើអ្នកប្រាកដថាចង់បោះពុម្ពតែមសម្រាប់សៀវភៅ %1 ដែលបានជ្រើសរើសទេ?"},
        {Language::English, "Are you sure you want to print stamps for %1 selected books?"}
    };
    m_dictionary["MSG_FEATURE_UPDATING"] = {
        {Language::Vietnamese, "Tính năng in tem hàng loạt đang được cập nhật engine. Hiện tại sẽ in sách đầu tiên trong nhóm chọn."},
        {Language::Khmer, "មុខងារបោះពុម្ពតែមច្រើនកំពុងត្រូវបានធ្វើបច្ចុប្បន្នភាព។ បច្ចុប្បន្ននឹងបោះពុម្ពសៀវភៅដំបូងក្នុងក្រុមដែលបានជ្រើសរើស។"},
        {Language::English, "Batch stamp printing engine is updating. Currently printing first book in selection."}
    };
    m_dictionary["MSG_SELECT_READER_FROM_LIST"] = {
        {Language::Vietnamese, "Vui lòng chọn học viên từ danh sách."},
        {Language::Khmer, "សូមជ្រើសរើសអ្នកអានពីបញ្ជី។"},
        {Language::English, "Please select a reader from the list."}
    };
    m_dictionary["MSG_SELECT_BOOK_FROM_LIST"] = {
        {Language::Vietnamese, "Vui lòng chọn sách từ danh sách."},
        {Language::Khmer, "សូមជ្រើសរើសសៀវភៅពីបញ្ជី។"},
        {Language::English, "Please select a book from the list."}
    };
    m_dictionary["DIALOG_PAYMENT_TITLE"] = {
        {Language::Vietnamese, "Thanh Toán Tiền Phạt"},
        {Language::Khmer, "បង់ប្រាក់ពិន័យ"},
        {Language::English, "Pay Fine"}
    };
    m_dictionary["HEADER_PAYMENT"] = {
        {Language::Vietnamese, " THANH TOÁN PHẠT"},
        {Language::Khmer, " បង់ប្រាក់ពិន័យ"},
        {Language::English, " PAY FINE"}
    };
    m_dictionary["GROUP_PAYMENT_INFO"] = {
        {Language::Vietnamese, "Thông Tin Thanh Toán"},
        {Language::Khmer, "ព័ត៌មានការទូទាត់"},
        {Language::English, "Payment Info"}
    };
    m_dictionary["LBL_READER_NAME"] = {
        {Language::Vietnamese, "Học viên"},
        {Language::Khmer, "អ្នកអាន"},
        {Language::English, "Reader"}
    };
    m_dictionary["LBL_REASON"] = {
        {Language::Vietnamese, "Lý do"},
        {Language::Khmer, "មូលហេតុ"},
        {Language::English, "Reason"}
    };
    m_dictionary["LBL_AMOUNT"] = {
        {Language::Vietnamese, "Số tiền"},
        {Language::Khmer, "ទឹកប្រាក់"},
        {Language::English, "Amount"}
    };
    m_dictionary["CURRENCY_UNIT"] = {
        {Language::Vietnamese, "VNĐ"},
        {Language::Khmer, "រៀល"},
        {Language::English, "VND"}
    };
    m_dictionary["CB_PRINT_RECEIPT"] = {
        {Language::Vietnamese, "In biên lai sau khi thanh toán"},
        {Language::Khmer, "បោះពុម្ពវិក្កយបត្របន្ទាប់ពីបង់ប្រាក់"},
        {Language::English, "Print receipt after payment"}
    };
    m_dictionary["BTN_CONFIRM_PAYMENT"] = {
        {Language::Vietnamese, "Xác Nhận Thanh Toán"},
        {Language::Khmer, "បញ្ជាក់ការទូទាត់"},
        {Language::English, "Confirm Payment"}
    };
    m_dictionary["DIALOG_ADD_COPY_TITLE"] = {
        {Language::Vietnamese, "Thêm Bản Sao"},
        {Language::Khmer, "បន្ថែមច្បាប់ចម្លង"},
        {Language::English, "Add Copies"}
    };
    m_dictionary["HEADER_ADD_COPY"] = {
        {Language::Vietnamese, "THÊM BẢN SAO"},
        {Language::Khmer, "បន្ថែមច្បាប់ចម្លង"},
        {Language::English, "ADD COPIES"}
    };
    m_dictionary["GROUP_ADD_NEW_COPY"] = {
        {Language::Vietnamese, "Thêm Bản Sao Mới"},
        {Language::Khmer, "បន្ថែមច្បាប់ចម្លងថ្មី"},
        {Language::English, "Add New Copies"}
    };
    m_dictionary["LBL_QUANTITY"] = {
        {Language::Vietnamese, "Số lượng"},
        {Language::Khmer, "បរិមាណ"},
        {Language::English, "Quantity"}
    };
    m_dictionary["PLACEHOLDER_LOCATION_EXAMPLE"] = {
        {Language::Vietnamese, "VD: Kệ A1, Phòng 201"},
        {Language::Khmer, "ឧទាហរណ៍: ធ្នើ A1, បន្ទប់ 201"},
        {Language::English, "Ex: Shelf A1, Room 201"}
    };
    m_dictionary["BTN_ADD_SYMBOL"] = {
        {Language::Vietnamese, "Thêm"},
        {Language::Khmer, "បន្ថែម"},
        {Language::English, "Add"}
    };
    m_dictionary["MSG_CHECKING_DATA"] = {
        {Language::Vietnamese, "Đang kiểm tra dữ liệu..."},
        {Language::Khmer, "កំពុងពិនិត្យទិន្នន័យ..."},
        {Language::English, "Checking data..."}
    };
    m_dictionary["MSG_ADD_COPY_SUCCESS"] = {
        {Language::Vietnamese, "Đã thêm %1 bản sao tại '%2'!"},
        {Language::Khmer, "បានបន្ថែមច្បាប់ចម្លង %1 នៅ '%2'!"},
        {Language::English, "Added %1 copies at '%2'!"}
    };
    m_dictionary["DIALOG_SUCCESS_TITLE"] = {
        {Language::Vietnamese, "Thành Công"},
        {Language::Khmer, "ជោគជ័យ"},
        {Language::English, "Success"}
    };
    m_dictionary["DIALOG_ERROR_TITLE"] = {
        {Language::Vietnamese, "Lỗi"},
        {Language::Khmer, "កំហុស"},
        {Language::English, "Error"}
    };
    m_dictionary["DIALOG_WARNING_TITLE"] = {
        {Language::Vietnamese, "Cảnh Báo"},
        {Language::Khmer, "ការព្រមាន"},
        {Language::English, "Warning"}
    };
    m_dictionary["MSG_BORROW_SUCCESS"] = {
        {Language::Vietnamese, "Đã mượn sách thành công!"},
        {Language::Khmer, "បានខ្ចីសៀវភៅដោយជោគជ័យ!"},
        {Language::English, "Book borrowed successfully!"}
    };
    m_dictionary["MSG_RETURN_SUCCESS"] = {
        {Language::Vietnamese, "Đã trả sách thành công!"},
        {Language::Khmer, "បានសងសៀវភៅដោយជោគជ័យ!"},
        {Language::English, "Book returned successfully!"}
    };
    m_dictionary["MSG_PAYMENT_FINE_SUCCESS"] = {
        {Language::Vietnamese, "Đã thanh toán tiền phạt!"},
        {Language::Khmer, "បានបង់ប្រាក់ពិន័យ!"},
        {Language::English, "Fine paid successfully!"}
    };
    m_dictionary["MSG_EXTEND_SUCCESS"] = {
        {Language::Vietnamese, "Đã gia hạn sách!"},
        {Language::Khmer, "បានបន្តសៀវភៅ!"},
        {Language::English, "Book extended!"}
    };
    m_dictionary["MSG_REPORT_LOST_SUCCESS"] = {
        {Language::Vietnamese, "Đã báo mất sách!"},
        {Language::Khmer, "បានរាយការណ៍ថាបាត់សៀវភៅ!"},
        {Language::English, "Reported lost book!"}
    };
    m_dictionary["MSG_SELECT_READER_FIRST"] = {
        {Language::Vietnamese, "Vui lòng chọn học viên trước!"},
        {Language::Khmer, "សូមជ្រើសរើសអ្នកអានជាមុនសិន!"},
        {Language::English, "Please select a reader first!"}
    };
    m_dictionary["MSG_SELECT_BOOK_RETURN"] = {
        {Language::Vietnamese, "Vui lòng chọn sách cần trả!"},
        {Language::Khmer, "សូមជ្រើសរើសសៀវភៅដែលត្រូវសង!"},
        {Language::English, "Please select a book to return!"}
    };
    m_dictionary["MSG_INVALID_SELECTION"] = {
        {Language::Vietnamese, "Dữ liệu chọn không hợp lệ!"},
        {Language::Khmer, "ទិន្នន័យដែលបានជ្រើសរើសមិនត្រឹមត្រូវ!"},
        {Language::English, "Invalid selection data!"}
    };
    m_dictionary["MSG_CONFIRM_RETURN_TITLE"] = {
        {Language::Vietnamese, "Xác Nhận Trả Sách"},
        {Language::Khmer, "បញ្ជាក់ការសងសៀវភៅ"},
        {Language::English, "Confirm Return"}
    };
    m_dictionary["MSG_CONFIRM_RETURN_BODY"] = {
        {Language::Vietnamese, "Xác nhận trả sách:\n\nMã: %1\nTên: %2%3"},
        {Language::Khmer, "បញ្ជាក់ការសងសៀវភៅ:\n\nលេខកូដ: %1\nឈ្មោះ: %2%3"},
        {Language::English, "Confirm return:\n\nCode: %1\nName: %2%3"}
    };
    m_dictionary["MSG_OVERDUE_WARNING"] = {
        {Language::Vietnamese, "\n\nQUÁ HẠN %1 NGÀY!\nTiền phạt dự kiến: %2 đ"},
        {Language::Khmer, "\n\nហួសកំណត់ %1 ថ្ងៃ!\nពិន័យប៉ាន់ស្មាន: %2 ៛"},
        {Language::English, "\n\nOVERDUE %1 DAYS!\nEstimated fine: %2"}
    };
    m_dictionary["MSG_BOOK_BORROWED_BY_OTHER"] = {
        {Language::Vietnamese, "Sách %1 đã có người mượn!"},
        {Language::Khmer, "សៀវភៅ %1 ត្រូវបានខ្ចីដោយអ្នកផ្សេង!"},
        {Language::English, "Book %1 is borrowed by someone else!"}
    };
    m_dictionary["MSG_CONFIRM_EXTEND_TITLE"] = {
        {Language::Vietnamese, "Xác Nhận Gia Hạn"},
        {Language::Khmer, "បញ្ជាក់ការបន្ត"},
        {Language::English, "Confirm Extension"}
    };
    m_dictionary["MSG_CONFIRM_EXTEND_BODY"] = {
        {Language::Vietnamese, "Gia hạn thêm 7 ngày cho sách:\n%1"},
        {Language::Khmer, "បន្តរយៈពេល 7 ថ្ងៃទៀតសម្រាប់សៀវភៅ:\n%1"},
        {Language::English, "Extend 7 days for book:\n%1"}
    };
    m_dictionary["MSG_SELECT_BOOK_EXTEND"] = {
        {Language::Vietnamese, "Vui lòng chọn sách cần gia hạn!"},
        {Language::Khmer, "សូមជ្រើសរើសសៀវភៅដែលត្រូវបន្ត!"},
        {Language::English, "Please select a book to extend!"}
    };
    m_dictionary["MSG_SELECT_BOOK_REPRINT"] = {
        {Language::Vietnamese, "Vui lòng chọn sách để in lại phiếu!"},
        {Language::Khmer, "សូមជ្រើសរើសសៀវភៅដើម្បីបោះពុម្ពប័ណ្ណឡើងវិញ!"},
        {Language::English, "Please select book to reprint slip!"}
    };
    m_dictionary["MSG_SELECT_BOOK_PAY"] = {
        {Language::Vietnamese, "Vui lòng chọn sách cần thanh toán!"},
        {Language::Khmer, "សូមជ្រើសរើសសៀវភៅដែលត្រូវបង់ប្រាក់!"},
        {Language::English, "Please select book to pay!"}
    };
    m_dictionary["MSG_SCAN_READER_FIRST"] = {
        {Language::Vietnamese, "Vui lòng quét thẻ học viên trước!"},
        {Language::Khmer, "សូមស្កេនប័ណ្ណអ្នកអានជាមុនសិន!"},
        {Language::English, "Please scan reader card first!"}
    };
    m_dictionary["MSG_SCAN_READER_NOT_FOUND"] = {
        {Language::Vietnamese, "Không tìm thấy học viên có mã thẻ: %1\n\nVui lòng kiểm tra lại mã thẻ!"},
        {Language::Khmer, "រកមិនឃើញអ្នកអានដែលមានលេខកូដប័ណ្ណ: %1\n\nសូមពិនិត្យមើលលេខកូដប័ណ្ណម្តងទៀត!"},
        {Language::English, "Reader not found with card code: %1\n\nPlease check card code again!"}
    };
    m_dictionary["MSG_CONFIRM_LOST_TITLE"] = {
        {Language::Vietnamese, "XÁC NHẬN BÁO MẤT SÁCH"},
        {Language::Khmer, "បញ្ជាក់ការបាត់សៀវភៅ"},
        {Language::English, "CONFIRM LOST BOOK"}
    };
    m_dictionary["MSG_CONFIRM_LOST_BODY"] = {
        {Language::Vietnamese, "BÁO MẤT SÁCH là thao tác NGHIÊM TRỌNG!\n\nMã: %1\nTên: %2\n\nCHI PHÍ ĐỀN BÙ:\n━━━━━━━━━━━━━━━━━━━━\nGiá sách: %3 VNĐ\nPhí phạt: %4 VNĐ\n━━━━━━━━━━━━━━━━━━━━\nTỔNG CỘNG: %5 VNĐ\n\nHọc viên sẽ bị ghi nợ số tiền này.\n\nBạn có chắc chắn?"},
        {Language::Khmer, "ការរាយការណ៍សៀវភៅបាត់គឺជាសកម្មភាពធ្ងន់ធ្ងរ!\n\nលេខកូដ: %1\nឈ្មោះ: %2\n\nតម្លៃសំណង:\n━━━━━━━━━━━━━━━━━━━━\nតម្លៃសៀវភៅ: %3 ៛\nប្រាក់ពិន័យ: %4 ៛\n━━━━━━━━━━━━━━━━━━━━\nសរុប: %5 ៛\n\nអ្នកអាននឹងត្រូវបានកត់ត្រាបំណុលនេះ।\n\nតើអ្នកប្រាកដទេ?"},
        {Language::English, "REPORTING LOST BOOK is SERIOUS!\n\nCode: %1\nName: %2\n\nCOMPENSATION COST:\n━━━━━━━━━━━━━━━━━━━━\nBook Price: %3\nFine Fee: %4\n━━━━━━━━━━━━━━━━━━━━\nTOTAL: %5\n\nReader will be charged this amount.\n\nAre you sure?"}
    };
    m_dictionary["MSG_LOST_RECORDED_TITLE"] = {
        {Language::Vietnamese, "Đã Ghi Nhận Mất Sách"},
        {Language::Khmer, "បានកត់ត្រាការបាត់សៀវភៅ"},
        {Language::English, "Lost Book Recorded"}
    };
    m_dictionary["MSG_LOST_RECORDED_BODY"] = {
        {Language::Vietnamese, "Đã ghi nhận mất sách: %1\n\nCHI TIẾT ĐỀN BÙ:\n━━━━━━━━━━━━━━━━━━━━\nGiá sách: %2 VNĐ\nPhí phạt: %3 VNĐ\n━━━━━━━━━━━━━━━━━━━━\nTỔNG CỘNG: %4 VNĐ\n\nHọc viên cần thanh toán khoản nợ này."},
        {Language::Khmer, "បានកត់ត្រាការបាត់សៀវភៅ: %1\n\nព័ត៌មានលម្អិតអំពីសំណង:\n━━━━━━━━━━━━━━━━━━━━\nតម្លៃសៀវភៅ: %2 ៛\nប្រាក់ពិន័យ: %3 ៛\n━━━━━━━━━━━━━━━━━━━━\nសរុប: %4 ៛\n\nអ្នកអានត្រូវបង់បំណុលនេះ។"},
        {Language::English, "Recorded lost book: %1\n\nCOMPENSATION DETAILS:\n━━━━━━━━━━━━━━━━━━━━\nBook Price: %2\nFine Fee: %3\n━━━━━━━━━━━━━━━━━━━━\nTOTAL: %4\n\nReader needs to pay this debt."}
    };
    m_dictionary["MSG_PAYMENT_COMPENSATION"] = {
        {Language::Vietnamese, "Đền bù sách mất: %1"},
        {Language::Khmer, "សំណងសៀវភៅបាត់: %1"},
        {Language::English, "Lost book compensation: %1"}
    };
    m_dictionary["MSG_PAYMENT_FINE_OVERDUE"] = {
        {Language::Vietnamese, "Thanh toán tiền phạt quá hạn (%1 ngày)"},
        {Language::Khmer, "បង់ប្រាក់ពិន័យហួសកំណត់ (%1 ថ្ងៃ)"},
        {Language::English, "Pay overdue fine (%1 days)"}
    };
    m_dictionary["MSG_NO_FINE_TO_PAY"] = {
        {Language::Vietnamese, "Sách này không có khoản phạt nào cần thanh toán (hoặc đã trả đúng hạn)."},
        {Language::Khmer, "សៀវភៅនេះគ្មានប្រាក់ពិន័យដែលត្រូវបង់ទេ (ឬបានសងទាន់ពេល)។"},
        {Language::English, "This book has no fines to pay (or returned on time)."}
    };
    m_dictionary["MSG_TAB_HISTORY_PLACEHOLDER"] = {
        {Language::Vietnamese, "Lịch Sử Mượn Trả\n\nChức năng xem lịch sử đang được phát triển...\n\nSẽ hiển thị:\n- Tất cả giao dịch mượn/trả\n- Lọc theo học viên\n- Lọc theo thời gian\n- Trạng thái: Đang mượn / Đã trả / Mất"},
        {Language::Khmer, "ប្រវត្តិខ្ចី-សង\n\nមុខងារមើលប្រវត្តិកំពុងត្រូវបានអភិវឌ្ឍ...\n\nនឹងបង្ហាញ:\n- រាល់ប្រតិបត្តិការខ្ចី/សង\n- ត្រងតាមអ្នកអាន\n- ត្រងតាមពេលវេលា\n- ស្ថានភាព: កំពុងខ្ចី / បានសង / បាត់"},
        {Language::English, "Borrow/Return History\n\nHistory view is under development...\n\nWill show:\n- All borrow/return transactions\n- Filter by reader\n- Filter by time\n- Status: Borrowing / Returned / Lost"}
    };
    m_dictionary["LBL_TOTAL_READERS"] = {
        {Language::Vietnamese, "Tổng: %1 học viên"},
        {Language::Khmer, "សរុប: %1 អ្នកអាន"},
        {Language::English, "Total: %1 readers"}
    };
    m_dictionary["GROUP_READER_INFO"] = {
        {Language::Vietnamese, "THÔNG TIN HỌC VIÊN"},
        {Language::Khmer, "ព័ត៌មានអ្នកអាន"},
        {Language::English, "READER INFO"}
    };
    m_dictionary["GROUP_BOOKS_BORROWED"] = {
        {Language::Vietnamese, "SÁCH ĐANG MƯỢN"},
        {Language::Khmer, "សៀវភៅកំពុងខ្ចី"},
        {Language::English, "BORROWED BOOKS"}
    };
    m_dictionary["LBL_CODE"] = {
        {Language::Vietnamese, "Mã"},
        {Language::Khmer, "កូដ"},
        {Language::English, "Code"}
    };
    m_dictionary["LBL_NAME"] = {
        {Language::Vietnamese, "Tên"},
        {Language::Khmer, "ឈ្មោះ"},
        {Language::English, "Name"}
    };
    m_dictionary["LBL_GENDER"] = {
        {Language::Vietnamese, "Giới Tính"},
        {Language::Khmer, "ភេទ"},
        {Language::English, "Gender"}
    };
    m_dictionary["LBL_STATUS_ABBR"] = {
        {Language::Vietnamese, "TT"},
        {Language::Khmer, "ស្ថានភាព"},
        {Language::English, "Sts"}
    };
    m_dictionary["LBL_BORROWING"] = {
        {Language::Vietnamese, "Đang Mượn"},
        {Language::Khmer, "កំពុងខ្ចី"},
        {Language::English, "Borrowing"}
    };
    m_dictionary["LBL_UNIT_BOOKS"] = {
        {Language::Vietnamese, "cuốn"},
        {Language::Khmer, "ក្បាល"},
        {Language::English, "books"}
    };
    m_dictionary["HEADER_MAIN_TITLE"] = {
        {Language::Vietnamese, "MƯỢN TRẢ SÁCH"},
        {Language::Khmer, "ការខ្ចី-សងសៀវភៅ"},
        {Language::English, "BORROW & RETURN BOOKS"}
    };
    m_dictionary["HEADER_QUICK_RETURN"] = {
        {Language::Vietnamese, "TRẢ SÁCH NHANH"},
        {Language::Khmer, "សងសៀវភៅរហ័ស"},
        {Language::English, "QUICK RETURN"}
    };
    m_dictionary["BTN_SCAN_SHORT"] = {
        {Language::Vietnamese, "Quét"},
        {Language::Khmer, "ស្កេន"},
        {Language::English, "Scan"}
    };
    m_dictionary["BTN_BORROW_SHORT"] = {
        {Language::Vietnamese, "MƯỢN"},
        {Language::Khmer, "ខ្ចី"},
        {Language::English, "BORROW"}
    };
    m_dictionary["BTN_RETURN_SHORT"] = {
        {Language::Vietnamese, "TRẢ"},
        {Language::Khmer, "សង"},
        {Language::English, "RETURN"}
    };
    m_dictionary["BTN_LOST_SHORT"] = {
        {Language::Vietnamese, "BÁO MẤT"},
        {Language::Khmer, "រាយការណ៍បាត់"},
        {Language::English, "REPORT LOST"}
    };
    m_dictionary["BTN_EXTEND_SHORT"] = {
        {Language::Vietnamese, "GIA HẠN"},
        {Language::Khmer, "បន្ត"},
        {Language::English, "EXTEND"}
    };
    m_dictionary["BTN_PAY_SHORT"] = {
        {Language::Vietnamese, "THANH TOÁN"},
        {Language::Khmer, "បង់ប្រាក់"},
        {Language::English, "PAY"}
    };
    m_dictionary["BTN_REPRINT_SHORT"] = {
        {Language::Vietnamese, "In Phiếu"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណ"},
        {Language::English, "Print Slip"}
    };
    m_dictionary["TOOLTIP_REFRESH"] = {
        {Language::Vietnamese, "Làm mới danh sách"},
        {Language::Khmer, "ធ្វើឱ្យបញ្ជីថ្មី"},
        {Language::English, "Refresh list"}
    };
    m_dictionary["PLACEHOLDER_SEARCH_READER"] = {
        {Language::Vietnamese, "Nhập tên, mã thẻ để tìm kiếm..."},
        {Language::Khmer, "បញ្ចូលឈ្មោះ, លេខកូដប័ណ្ណដើម្បីស្វែងរក..."},
        {Language::English, "Enter name, card ID to search..."}
    };
    m_dictionary["PLACEHOLDER_SCAN_BORROW"] = {
        {Language::Vietnamese, "Quét ISBN / Mã Sách để mượn..."},
        {Language::Khmer, "ស្កេន ISBN / លេខកូដសៀវភៅដើម្បីខ្ចី..."},
        {Language::English, "Scan ISBN / Book Code to borrow..."}
    };
    m_dictionary["LBL_INSTRUCT_QUICK_RETURN"] = {
        {Language::Vietnamese, "Quét mã sách hoặc nhập ISBN để trả tự động:"},
        {Language::Khmer, "ស្កេនលេខកូដសៀវភៅ ឬបញ្ចូល ISBN ដើម្បីសងដោយស្វ័យប្រវត្តិ:"},
        {Language::English, "Scan book code or enter ISBN to auto-return:"}
    };
    m_dictionary["PLACEHOLDER_ENTER_BOOK_CODE"] = {
        {Language::Vietnamese, "Nhập mã sách..."},
        {Language::Khmer, "បញ្ចូលលេខកូដសៀវភៅ..."},
        {Language::English, "Enter book code..."}
    };
    // Table Statuses
    m_dictionary["STATUS_RETURNED_DEBT"] = {
        {Language::Vietnamese, "ĐÃ TRẢ (NỢ %1)"},
        {Language::Khmer, "បានសង (ជំពាក់ %1)"},
        {Language::English, "RETURNED (DEBT %1)"}
    };
    m_dictionary["STATUS_RETURNED"] = {
        {Language::Vietnamese, "ĐÃ TRẢ"},
        {Language::Khmer, "បានសង"},
        {Language::English, "RETURNED"}
    };
    m_dictionary["STATUS_OVERDUE"] = {
        {Language::Vietnamese, "Quá hạn %1 ngày"},
        {Language::Khmer, "ហួសកំណត់ %1 ថ្ងៃ"},
        {Language::English, "Overdue %1 days"}
    };
    m_dictionary["STATUS_DUE_IN"] = {
        {Language::Vietnamese, "Còn hạn (%1 ngày)"},
        {Language::Khmer, "នៅសល់ (%1 ថ្ងៃ)"},
        {Language::English, "Due in (%1 days)"}
    };
    m_dictionary["STATUS_AVAILABLE"] = {
        {Language::Vietnamese, "Còn hạn"},
        {Language::Khmer, "នៅសល់"},
        {Language::English, "Available"} // Probably not used in status column but consistent
    };
    m_dictionary["STATUS_NA"] = {
        {Language::Vietnamese, "N/A"},
        {Language::Khmer, "N/A"},
        {Language::English, "N/A"}
    };

    // ==================== EDIT BOOK SCREEN ====================
    m_dictionary["TITLE_EDIT_BOOK"] = { {Language::Vietnamese, "Chỉnh Sửa Đầu Sách"}, {Language::Khmer, "កែសម្រួលសៀវភៅ"}, {Language::English, "Edit Book"} };
    m_dictionary["HEADER_EDIT_BOOK"] = { {Language::Vietnamese, "Chỉnh Sửa Thông Tin Đầu Sách"}, {Language::Khmer, "កែសម្រួលព័ត៌មានសៀវភៅ"}, {Language::English, "Edit Book Information"} };
    m_dictionary["LBL_ISBN_READONLY"] = { {Language::Vietnamese, "ISBN không thể chỉnh sửa"}, {Language::Khmer, "ISBN មិនអាចកែប្រែបានទេ"}, {Language::English, "ISBN cannot be edited"} };
    m_dictionary["LBL_REQ_NOTE"] = { {Language::Vietnamese, "Các trường đánh dấu (*) là bắt buộc"}, {Language::Khmer, "វាលដែលសម្គាល់ (*) គឺត្រូវការ"}, {Language::English, "Fields marked (*) are required"} };
    m_dictionary["MSG_UPDATE_BOOK_SUCCESS"] = { 
        {Language::Vietnamese, "Đã cập nhật thông tin đầu sách thành công!\n\nTên sách: %1\nTác giả: %2\nThể loại: %3"}, 
        {Language::Khmer, "បានធ្វើបច្ចុប្បន្នភាពព័ត៌មានសៀវភៅដោយជោគជ័យ!\n\nឈ្មោះសៀវភៅ: %1\nអ្នកនិពន្ធ: %2\nប្រភេទ: %3"}, 
        {Language::English, "Book info updated successfully!\n\nTitle: %1\nAuthor: %2\nGenre: %3"} 
    };
    m_dictionary["MSG_ERROR_UPDATE_BOOK"] = { {Language::Vietnamese, "Không thể cập nhật thông tin đầu sách. Vui lòng thử lại!"}, {Language::Khmer, "មិនអាចធ្វើបច្ចុប្បន្នភាពព័ត៌មានសៀវភៅបានទេ។ សូមព្យាយាមម្តងទៀត!"}, {Language::English, "Cannot update book info. Please try again!"} };

    // ==================== READER DETAILS SCREEN ====================
    m_dictionary["TITLE_READER_DETAILS"] = { {Language::Vietnamese, "Chi Tiết Học Viên"}, {Language::Khmer, "ព័ត៌មានលម្អិតសិស្ស"}, {Language::English, "Student Details"} };
    m_dictionary["HEADER_READER_DETAILS"] = { {Language::Vietnamese, "CHI TIẾT HỌC VIÊN"}, {Language::Khmer, "ព័ត៌មានលម្អិតសិស្ស"}, {Language::English, "STUDENT DETAILS"} };
    m_dictionary["GROUP_HISTORY"] = { {Language::Vietnamese, "Lịch Sử Mượn Trả"}, {Language::Khmer, "ប្រវត្តិខ្ចី-សង"}, {Language::English, "Loan History"} };
    m_dictionary["MSG_READER_INFO_HTML"] = { 
        {Language::Vietnamese, "<b>Họ Tên:</b> %1 %2<br><b>Mã Thẻ:</b> %3<br><b>Giới Tính:</b> %4<br><b>Trạng Thái:</b> %5<br><b>Sách Đang Mượn:</b> %6 cuốn"}, 
        {Language::Khmer, "<b>ឈ្មោះ:</b> %1 %2<br><b>លេខកូដ:</b> %3<br><b>ភេទ:</b> %4<br><b>ស្ថានភាព:</b> %5<br><b>សៀវភៅកំពុងខ្ចី:</b> %6 ក្បាល"}, 
        {Language::English, "<b>Name:</b> %1 %2<br><b>ID:</b> %3<br><b>Gender:</b> %4<br><b>Status:</b> %5<br><b>Books Borrowed:</b> %6"} 
    };
    m_dictionary["TABLE_HEADER_BORROW_DATE"] = { {Language::Vietnamese, "Ngày Mượn"}, {Language::Khmer, "ថ្ងៃខ្ចី"}, {Language::English, "Borrow Date"} };
    m_dictionary["TABLE_HEADER_DUE_DATE"] = { {Language::Vietnamese, "Hạn Trả"}, {Language::Khmer, "ថ្ងៃកំណត់សង"}, {Language::English, "Due Date"} };
    m_dictionary["BTN_RETURN_BOOK"] = { {Language::Vietnamese, "Trả Sách"}, {Language::Khmer, "សងសៀវភៅ"}, {Language::English, "Return Book"} };
    m_dictionary["BTN_REPORT_LOST"] = { {Language::Vietnamese, "Báo Mất"}, {Language::Khmer, "រាយការណ៍បាត់"}, {Language::English, "Report Lost"} };
    m_dictionary["BTN_PRINT_CARD_TV"] = { {Language::Vietnamese, "In Thẻ TV"}, {Language::Khmer, "បោះពុម្ពប័ណ្ណ"}, {Language::English, "Print Card"} };
    m_dictionary["MSG_CONFIRM_RETURN_BOOK"] = { {Language::Vietnamese, "Xác nhận trả sách: %1?"}, {Language::Khmer, "បញ្ជាក់ការសងសៀវភៅ: %1?"}, {Language::English, "Confirm return book: %1?"} };
    m_dictionary["MSG_WARN_RETURN_NOT_BORROWED"] = { {Language::Vietnamese, "Sách này không ở trạng thái đang mượn!"}, {Language::Khmer, "សៀវភៅនេះមិនមែននៅក្នុងស្ថានភាពកំពុងខ្ចីទេ!"}, {Language::English, "Book is not in borrowed status!"} };
    m_dictionary["MSG_WARN_LOST_NOT_BORROWED"] = { {Language::Vietnamese, "Chỉ có thể báo mất sách đang mượn!"}, {Language::Khmer, "អាចរាយការណ៍បានតែសៀវភៅដែលកំពុងខ្ចីប៉ុណ្ណោះ!"}, {Language::English, "Can only report lost for borrowed books!"} };
    m_dictionary["MSG_CONFIRM_LOST_HTML"] = { 
        {Language::Vietnamese, "Xác nhận BÁO MẤT sách: %1?\n\nGiá sách: %2 VNĐ\nPhí phạt: %3 VNĐ\n-------------------\nTổng đền bù: %4 VNĐ\n\n(Học viên sẽ bị ghi nợ số tiền này)"}, 
        {Language::Khmer, "បញ្ជាក់ការរាយការណ៍បាត់សៀវភៅ: %1?\n\nតម្លៃសៀវភៅ: %2 VNĐ\nប្រាក់ពិន័យ: %3 VNĐ\n-------------------\nសរុបសំណង: %4 VNĐ\n\n(សិស្សនឹងត្រូវបានកត់ត្រាបំណុលនេះ)"}, 
        {Language::English, "Confirm REPORT LOST book: %1?\n\nPrice: %2 VNĐ\nFine: %3 VNĐ\n-------------------\nTotal Compensation: %4 VNĐ\n\n(Student will be debited this amount)"} 
    };
    m_dictionary["MSG_LOST_RECORDED_SUCCESS"] = { 
        {Language::Vietnamese, "Đã ghi nhận mất sách: %1\n\nGiá sách: %2 VNĐ\nPhí phạt: %3 VNĐ\n-------------------\nTổng đền bù: %4 VNĐ\n\nHọc viên cần thanh toán khoản nợ này."}, 
        {Language::Khmer, "បានកត់ត្រាការបាត់សៀវភៅ: %1\n\nតម្លៃសៀវភៅ: %2 VNĐ\nប្រាក់ពិន័យ: %3 VNĐ\n-------------------\nសរុបសំណង: %4 VNĐ\n\nសិស្សត្រូវបង់បំណុលនេះ។"}, 
        {Language::English, "Recorded lost book: %1\n\nPrice: %2 VNĐ\nFine: %3 VNĐ\n-------------------\nTotal: %4 VNĐ\n\nStudent needs to pay this debt."} 
    };

    // ==================== ABOUT SCREEN ====================
    m_dictionary["HEADER_SYSTEM_NAME"] = { {Language::Vietnamese, "HỆ THỐNG QUẢN LÝ THƯ VIỆN"}, {Language::Khmer, "ប្រព័ន្ធគ្រប់គ្រងបណ្ណាល័យ"}, {Language::English, "LIBRARY MANAGEMENT SYSTEM"} };
    m_dictionary["TITLE_ABOUT"] = { {Language::Vietnamese, "Về Hệ Thống"}, {Language::Khmer, "អំពីប្រព័ន្ធ"}, {Language::English, "About System"} };
    m_dictionary["LBL_VERSION"] = { {Language::Vietnamese, "Phiên bản 2.1 (Premium)"}, {Language::Khmer, "កំណែ 2.1 (Premium)"}, {Language::English, "Version 2.1 (Premium)"} };
    m_dictionary["LBL_BUILD_DATE"] = { {Language::Vietnamese, "Ngày xây dựng: 20/12/2025"}, {Language::Khmer, "កាលបរិច្ឆេទសាងសង់: 20/12/2025"}, {Language::English, "Build Date: 20/12/2025"} };
    m_dictionary["MSG_ABOUT_FEATURES_HTML"] = { 
        {Language::Vietnamese, "<p style='color: #B5B5C3; font-size: 14px; line-height: 1.6;'><b> Tính năng nổi bật:</b></p><ul style='margin-left: 20px; color: #D1D3E0; line-height: 1.6;'><li> <b>Giao diện:</b> Dark Mode hiện đại, Responsive, Web-like Animations.</li><li> <b>Dữ liệu:</b> Cây AVL (Học viên), Danh sách liên kết (Mượn trả), Mảng (Sách).</li><li> <b>Báo cáo:</b> Xuất PDF, In ấn Tem/Thẻ chuyên nghiệp.</li><li> <b>Tiện ích:</b> Tự động sao lưu, tìm kiếm thông minh, phím tắt.</li></ul>"}, 
        {Language::Khmer, "<p style='color: #B5B5C3; font-size: 14px; line-height: 1.6;'><b> លក្ខណៈពិសេស:</b></p><ul style='margin-left: 20px; color: #D1D3E0; line-height: 1.6;'><li> <b>ចំណុចប្រទាក់:</b> Dark Mode ទំនើប, Responsive.</li><li> <b>ទិន្នន័យ:</b> AVL Tree (សិស្ស), Linked List (ខ្ចី-សង), Array (សៀវភៅ).</li><li> <b>របាយការណ៍:</b> នាំចេញ PDF, បោះពុម្ពត្រា/ប័ណ្ណអាជីព.</li><li> <b>ឧបករណ៍ប្រើប្រាស់:</b> បម្រុងទុកដោយស្វ័យប្រវត្តិ, ស្វែងរកឆ្លាតវៃ.</li></ul>"}, 
        {Language::English, "<p style='color: #B5B5C3; font-size: 14px; line-height: 1.6;'><b> Key Features:</b></p><ul style='margin-left: 20px; color: #D1D3E0; line-height: 1.6;'><li> <b>UI:</b> Modern Dark Mode, Responsive, Animations.</li><li> <b>Data:</b> AVL Tree (Student), Linked List (Loan), Array (Book).</li><li> <b>Reports:</b> PDF Export, Professional Stamps/Cards.</li><li> <b>Utils:</b> Auto Backup, Smart Search, Shortcuts.</li></ul>"} 
    };
    m_dictionary["MSG_ABOUT_TECH_HTML"] = { 
        {Language::Vietnamese, "<p style='color: #B5B5C3; font-size: 14px; line-height: 1.6;'><b> Công nghệ:</b></p><p style='margin-left: 20px; color: #3699FF;'>C++17 • Qt 6.10 • Custom UI Framework</p>"}, 
        {Language::Khmer, "<p style='color: #B5B5C3; font-size: 14px; line-height: 1.6;'><b> បច្ចេកវិទ្យា:</b></p><p style='margin-left: 20px; color: #3699FF;'>C++17 • Qt 6.10 • Custom UI Framework</p>"}, 
        {Language::English, "<p style='color: #B5B5C3; font-size: 14px; line-height: 1.6;'><b> Tech Stack:</b></p><p style='margin-left: 20px; color: #3699FF;'>C++17 • Qt 6.10 • Custom UI Framework</p>"} 
    };
    m_dictionary["MSG_ABOUT_CREDIT"] = { 
        {Language::Vietnamese, "Phát triển bởi: <b>Chăm Rốch Thi</b> @ 2025"}, 
        {Language::Khmer, "អភិវឌ្ឍដោយ: <b>Chăm Rốch Thi</b> @ 2025"}, 
        {Language::English, "Developed by: <b>Chăm Rốch Thi</b> @ 2025"} 
    };

    // ==================== ADD COPY SCREEN ====================
    m_dictionary["TITLE_ADD_COPY"] = { {Language::Vietnamese, "Thêm Bản Sao Sách"}, {Language::Khmer, "បន្ថែមច្បាប់ចម្លងសៀវភៅ"}, {Language::English, "Add Book Copy"} };
    m_dictionary["HEADER_ADD_COPY"] = { {Language::Vietnamese, "Thêm Bản Sao Sách"}, {Language::Khmer, "បន្ថែមច្បាប់ចម្លងសៀវភៅ"}, {Language::English, "Add Book Copy"} };
    m_dictionary["GROUP_COPY_INFO"] = { {Language::Vietnamese, "Thông Tin Bản Sao"}, {Language::Khmer, "ព័ត៌មានច្បាប់ចម្លង"}, {Language::English, "Copy Information"} };
    m_dictionary["LBL_QUANTITY"] = { {Language::Vietnamese, "Số Lượng:"}, {Language::Khmer, "ចំនួន:"}, {Language::English, "Quantity:"} };
    m_dictionary["LBL_NOTE_AUTO_CODE"] = { {Language::Vietnamese, "Mã sách sẽ được tự động tạo theo quy tắc: ISBN-XX"}, {Language::Khmer, "លេខកូដសៀវភៅនឹងត្រូវបានបង្កើតដោយស្វ័យប្រវត្តិ: ISBN-XX"}, {Language::English, "Book code will be auto-generated: ISBN-XX"} };
    m_dictionary["BTN_ADD_COPY"] = { {Language::Vietnamese, "Thêm Bản Sao"}, {Language::Khmer, "បន្ថែមច្បាប់ចម្លង"}, {Language::English, "Add Copy"} };
    m_dictionary["MSG_ADD_COPY_SUCCESS"] = { 
        {Language::Vietnamese, "Đã thêm thành công %1 bản sao!\n\nTên sách: %2\nVị trí: %3\nMã sách đầu tiên: %4"}, 
        {Language::Khmer, "បានបន្ថែមច្បាប់ចម្លង %1 ដោយជោគជ័យ!\n\nឈ្មោះសៀវភៅ: %2\nទីតាំង: %3\nលេខកូដដំបូង: %4"}, 
        {Language::English, "Added %1 copies successfully!\n\nTitle: %2\nLocation: %3\nFirst Code: %4"} 
    };
    m_dictionary["MSG_ERROR_ADD_COPY"] = { {Language::Vietnamese, "Không thể thêm bản sao. Vui lòng kiểm tra lại thông tin!"}, {Language::Khmer, "មិនអាចបន្ថែមច្បាប់ចម្លងបានទេ។ សូមពិនិត្យព័ត៌មានម្តងទៀត!"}, {Language::English, "Cannot add copy. Please check info again!"} };

    // ==================== TOP 10 READERS SCREEN ====================
    m_dictionary["TITLE_TOP10_READERS"] = { {Language::Vietnamese, "Top 10 Học Viên Hoạt Động Nhiều Nhất"}, {Language::Khmer, "សិស្សសកម្មបំផុតទាំង ១០"}, {Language::English, "Top 10 Most Active Students"} };
    m_dictionary["HEADER_TOP10_READERS"] = { {Language::Vietnamese, "TOP 10 HỌC VIÊN HOẠT ĐỘNG NHIỀU NHẤT"}, {Language::Khmer, "សិស្សសកម្មបំផុតទាំង ១០"}, {Language::English, "TOP 10 MOST ACTIVE STUDENTS"} };
    m_dictionary["MSG_STATS_READERS"] = { {Language::Vietnamese, "<b>Thống kê:</b> Dựa trên tổng số lượt mượn sách của học viên"}, {Language::Khmer, "<b>ស្ថិតិ:</b> ផ្អែកលើចំនួនដងដែលសិស្សខ្ចីសៀវភៅ"}, {Language::English, "<b>Stats:</b> Based on total book loans by student"} };
    m_dictionary["TABLE_HEADER_RANK"] = { {Language::Vietnamese, "Hạng"}, {Language::Khmer, "ចំណាត់ថ្នាក់"}, {Language::English, "Rank"} };
    m_dictionary["TABLE_HEADER_TOTAL_BORROW"] = { {Language::Vietnamese, "Tổng Mượn"}, {Language::Khmer, "សរុបខ្ចី"}, {Language::English, "Total Borrowed"} };
    m_dictionary["TABLE_HEADER_CURRENT_BORROW"] = { {Language::Vietnamese, "Đang Mượn"}, {Language::Khmer, "កំពុងខ្ចី"}, {Language::English, "Currently Borrowing"} };
    m_dictionary["MSG_NO_READERS"] = { {Language::Vietnamese, "Chưa có học viên nào trong hệ thống."}, {Language::Khmer, "មិនទាន់មានសិស្សនៅក្នុងប្រព័ន្ធទេ។"}, {Language::English, "No students in the system."} };
    m_dictionary["MSG_NO_BORROWS"] = { {Language::Vietnamese, "Chưa có học viên nào mượn sách."}, {Language::Khmer, "មិនទាន់មានសិស្សណាម្នាក់ខ្ចីសៀវភៅទេ។"}, {Language::English, "No students have borrowed books."} };

    // ==================== PREVIEW IMPORT SCREEN ====================
    m_dictionary["TITLE_PREVIEW_IMPORT"] = { {Language::Vietnamese, "Xem Trước Kết Quả Import"}, {Language::Khmer, "មើលលទ្ធផលនាំចូលជាមុន"}, {Language::English, "Preview Import Results"} };
    m_dictionary["HEADER_PREVIEW_IMPORT"] = { {Language::Vietnamese, "KẾT QUẢ XEM TRƯỚC (PREVIEW)"}, {Language::Khmer, "លទ្ធផលមើលជាមុន (PREVIEW)"}, {Language::English, "PREVIEW RESULTS"} };
    m_dictionary["LBL_PREVIEW_NOTE"] = { {Language::Vietnamese, "Các dòng bị lỗi sẽ được tô màu đỏ. Vui lòng kiểm tra kỹ trước khi Áp Dụng."}, {Language::Khmer, "ជួរដេកដែលមានកំហុសនឹងត្រូវបានបន្លិចពណ៌ក្រហម។ សូមពិនិត្យឱ្យបានច្បាស់លាស់មុនពេលអនុវត្ត។"}, {Language::English, "Rows with errors will be highlighted in red. Please check carefully before Applying."} };
    m_dictionary["TAB_COPIES"] = { {Language::Vietnamese, "Bản Sao"}, {Language::Khmer, "ច្បាប់ចម្លង"}, {Language::English, "Copies"} };
    m_dictionary["TAB_LOANS"] = { {Language::Vietnamese, "Mượn Trả"}, {Language::Khmer, "ខ្ចី-សង"}, {Language::English, "Loans"} };
    m_dictionary["BTN_APPLY_IMPORT"] = { {Language::Vietnamese, "Áp Dụng Import"}, {Language::Khmer, "អនុវត្តការនាំចូល"}, {Language::English, "Apply Import"} };
    m_dictionary["DIALOG_IMPORT_MODE_TITLE"] = { {Language::Vietnamese, "Chọn Chế Độ Import"}, {Language::Khmer, "ជ្រើសរើសរបៀបនាំចូល"}, {Language::English, "Select Import Mode"} };
    m_dictionary["DIALOG_IMPORT_MODE_MSG"] = { {Language::Vietnamese, "Xử lý dữ liệu trùng lặp (ISBN/Mã Thẻ):"}, {Language::Khmer, "ដោះស្រាយទិន្នន័យស្ទួន (ISBN/លេខកូដ):"}, {Language::English, "Handle duplicate data (ISBN/ID):"} };
    m_dictionary["OPTION_MERGE"] = { {Language::Vietnamese, "Hợp nhất (Merge)"}, {Language::Khmer, "បញ្ចូលគ្នា (Merge)"}, {Language::English, "Merge"} };
    m_dictionary["OPTION_REPLACE"] = { {Language::Vietnamese, "Ghi đè (Replace)"}, {Language::Khmer, "សរសេរជាន់លើ (Replace)"}, {Language::English, "Replace"} };
    m_dictionary["OPTION_SKIP"] = { {Language::Vietnamese, "Bỏ qua (Skip)"}, {Language::Khmer, "រំលង (Skip)"}, {Language::English, "Skip"} };
    m_dictionary["MSG_IMPORT_SUCCESS"] = { {Language::Vietnamese, "Import dữ liệu hoàn tất!"}, {Language::Khmer, "ការនាំចូលទិន្នន័យបានបញ្ចប់!"}, {Language::English, "Data import completed!"} };
    m_dictionary["MSG_ENTER_PRINT_QUANTITY"] = { {Language::Vietnamese, "Số lượng tem cần in:"}, {Language::Khmer, "ចំនួនតែមដែលត្រូវបោះពុម្ព:"}, {Language::English, "Quantity to print:"} };
    m_dictionary["TABLE_HEADER_LOCATION"] = { {Language::Vietnamese, "Vị Trí"}, {Language::Khmer, "ទីតាំង"}, {Language::English, "Location"} };
    m_dictionary["TABLE_HEADER_ACTION"] = { {Language::Vietnamese, "Thao Tác"}, {Language::Khmer, "សកម្មភាព"}, {Language::English, "Action"} };
    m_dictionary["BTN_CLOSE"] = { {Language::Vietnamese, "Đóng"}, {Language::Khmer, "បិទ"}, {Language::English, "Close"} }; // Ensure exist

    // === TOP 10 BOOKS ===
    m_dictionary["TITLE_TOP10_BOOKS"] = { {Language::Vietnamese, "Top 10 Sách Được Mượn Nhiều Nhất"}, {Language::Khmer, "សៀវភៅដែលត្រូវបានខ្ចីច្រើនបំផុតទាំង ១០"}, {Language::English, "Top 10 Most Borrowed Books"} };
    m_dictionary["HEADER_TOP10_BOOKS"] = { {Language::Vietnamese, "TOP 10 SÁCH ĐƯỢC MƯỢN NHIỀU NHẤT"}, {Language::Khmer, "សៀវភៅដែលត្រូវបានខ្ចីច្រើនបំផុតទាំង ១០"}, {Language::English, "TOP 10 MOST BORROWED BOOKS"} };
    m_dictionary["MSG_STATS_BOOKS"] = { {Language::Vietnamese, "<b>Thống kê:</b> Dựa trên số lượt mượn của từng đầu sách"}, {Language::Khmer, "<b>ស្ថិតិ:</b> ផ្អែកលើចំនួនដងដែលសៀវភៅត្រូវបានខ្ចី"}, {Language::English, "<b>Stats:</b> Based on borrow count per book"} };
    m_dictionary["TABLE_HEADER_BORROW_COUNT"] = { {Language::Vietnamese, "Số Lượt Mượn"}, {Language::Khmer, "ចំនួនដងខ្ចី"}, {Language::English, "Borrow Count"} };
    
    // === COMMON / OVERDUE ===
    m_dictionary["LBL_DAYS"] = { {Language::Vietnamese, "ngày"}, {Language::Khmer, "ថ្ងៃ"}, {Language::English, "days"} };
    m_dictionary["TABLE_HEADER_FULLNAME"] = { {Language::Vietnamese, "Họ và Tên"}, {Language::Khmer, "ឈ្មោះពេញ"}, {Language::English, "Full Name"} };
    // === ADD READER SCREEN ===
    m_dictionary["GROUP_READER_INFO"] = { {Language::Vietnamese, "Thông Tin Học Viên"}, {Language::Khmer, "ព័ត៌មានសិស្ស"}, {Language::English, "Student Info"} };
    m_dictionary["MSG_NOTE_ADD_READER"] = { 
        {Language::Vietnamese, "💡 <b>Lưu ý:</b><br>• Các trường đánh dấu (*) là bắt buộc<br>• Mã thẻ sẽ được tự động tạo<br>• Học viên mới có trạng thái hoạt động"}, 
        {Language::Khmer, "💡 <b>ចំណាំ:</b><br>• វាលដែលមានសញ្ញា (*) គឺចាំបាច់<br>• លេខកូដកាតនឹងបង្កើតដោយស្វ័យប្រវត្តិ<br>• សិស្សថ្មីមានស្ថានភាពសកម្ម"}, 
        {Language::English, "💡 <b>Note:</b><br>• Fields marked (*) are required<br>• Card code is auto-generated<br>• New students are active by default"} 
    };

    // === ADD BOOK SCREEN ===
    m_dictionary["TITLE_ADD_BOOK"] = { {Language::Vietnamese, "Thêm Đầu Sách Mới"}, {Language::Khmer, "បន្ថែមសៀវភៅថ្មី"}, {Language::English, "Add New Book"} };
    m_dictionary["HEADER_ADD_BOOK"] = { {Language::Vietnamese, "THÊM ĐẦU SÁCH MỚI"}, {Language::Khmer, "បន្ថែមសៀវភៅថ្មី"}, {Language::English, "ADD NEW BOOK"} };
    m_dictionary["GROUP_BOOK_INFO"] = { {Language::Vietnamese, "Thông Tin Sách"}, {Language::Khmer, "ព័ត៌មានសៀវភៅ"}, {Language::English, "Book Info"} };
    
    m_dictionary["PLACEHOLDER_ISBN"] = { {Language::Vietnamese, "Nhập 10 hoặc 13 chữ số (VD: 978...)"}, {Language::Khmer, "បញ្ចូលលេខ ១០ ឬ ១៣ ខ្ទង់ (ឧ. 978...)"}, {Language::English, "Enter 10 or 13 digits (e.g. 978...)"} };
    m_dictionary["PLACEHOLDER_BOOK_NAME"] = { {Language::Vietnamese, "Tên sách (tối đa 100 ký tự)"}, {Language::Khmer, "ឈ្មោះសៀវភៅ (អតិបរមា ១០០ តួអក្សរ)"}, {Language::English, "Book title (max 100 chars)"} };
    m_dictionary["PLACEHOLDER_AUTHOR"] = { {Language::Vietnamese, "Tên tác giả (tối đa 50 ký tự)"}, {Language::Khmer, "ឈ្មោះអ្នកនិពន្ធ (អតិបរមា ៥០ តួអក្សរ)"}, {Language::English, "Author name (max 50 chars)"} };
    m_dictionary["PLACEHOLDER_LOCATION"] = { {Language::Vietnamese, "VD: Kệ A1, Phòng 201"}, {Language::Khmer, "ឧ. ធ្នើ A1, បន្ទប់ 201"}, {Language::English, "e.g. Shelf A1, Room 201"} };
    m_dictionary["PLACEHOLDER_GENRE"] = { {Language::Vietnamese, "Chọn hoặc nhập thể loại"}, {Language::Khmer, "ជ្រើសរើស ឬបញ្ចូលប្រភេទ"}, {Language::English, "Select or enter genre"} };
    
    m_dictionary["TOOLTIP_ISBN"] = { {Language::Vietnamese, "ISBN phải có 10 hoặc 13 chữ số"}, {Language::Khmer, "ISBN ត្រូវតែមាន ១០ ឬ ១៣ ខ្ទង់"}, {Language::English, "ISBN must have 10 or 13 digits"} };
    m_dictionary["TOOLTIP_BOOK_NAME"] = { {Language::Vietnamese, "Nhập tên đầy đủ của sách (bắt buộc)"}, {Language::Khmer, "បញ្ចូលឈ្មោះពេញសៀវភៅ (ចាំបាច់)"}, {Language::English, "Enter full book title (required)"} };
    m_dictionary["TOOLTIP_AUTHOR"] = { {Language::Vietnamese, "Nhập tên tác giả (bắt buộc)"}, {Language::Khmer, "បញ្ចូលឈ្មោះអ្នកនិពន្ធ (ចាំបាច់)"}, {Language::English, "Enter author name (required)"} };
    m_dictionary["TOOLTIP_YEAR"] = { {Language::Vietnamese, "Năm xuất bản sách (1000 - hiện tại)"}, {Language::Khmer, "ឆ្នាំបោះពុម្ពសៀវភៅ (1000 - បច្ចុប្បន្ន)"}, {Language::English, "Publish year (1000 - current)"} };
    m_dictionary["TOOLTIP_PAGES"] = { {Language::Vietnamese, "Số trang của sách (1-10000)"}, {Language::Khmer, "ចំនួនទំព័រសៀវភៅ (1-10000)"}, {Language::English, "Number of pages (1-10000)"} };
    m_dictionary["TOOLTIP_COPIES"] = { {Language::Vietnamese, "Số lượng bản sao muốn thêm cùng lúc (1-100)"}, {Language::Khmer, "ចំនួនច្បាប់ចម្លង (1-100)"}, {Language::English, "Number of copies (1-100)"} };
    m_dictionary["TOOLTIP_LOCATION"] = { {Language::Vietnamese, "Vị trí đặt sách trong thư viện (không bắt buộc)"}, {Language::Khmer, "ទីតាំងសៀវភៅនៅក្នុងបណ្ណាល័យ (មិនចាំបាច់)"}, {Language::English, "Book location in library (optional)"} };
    
    m_dictionary["SUFFIX_YEAR"] = { {Language::Vietnamese, " năm"}, {Language::Khmer, " ឆ្នាំ"}, {Language::English, " year"} };
    m_dictionary["SUFFIX_PAGES"] = { {Language::Vietnamese, " trang"}, {Language::Khmer, " ទំព័រ"}, {Language::English, " pages"} };
    
    // === EDIT DIALOGS ===
    m_dictionary["TITLE_EDIT_BOOK"] = { {Language::Vietnamese, "Chỉnh Sửa Sách"}, {Language::Khmer, "កែប្រែសៀវភៅ"}, {Language::English, "Edit Book"} };
    m_dictionary["HEADER_EDIT_BOOK"] = { {Language::Vietnamese, "CHỈNH SỬA THÔNG TIN SÁCH"}, {Language::Khmer, "កែប្រែព័ត៌មានសៀវភៅ"}, {Language::English, "EDIT BOOK INFO"} };
    m_dictionary["TITLE_EDIT_READER"] = { {Language::Vietnamese, "Chỉnh Sửa Học Viên"}, {Language::Khmer, "កែប្រែសិស្ស"}, {Language::English, "Edit Student"} };
    
    m_dictionary["GROUP_STATS"] = { {Language::Vietnamese, "Thống Kê"}, {Language::Khmer, "ស្ថិតិ"}, {Language::English, "Statistics"} };
    m_dictionary["MSG_NOTE_CANNOT_LOCK"] = { {Language::Vietnamese, "💡 Không thể khóa học viên đang mượn sách"}, {Language::Khmer, "💡 មិនអាចចាក់សោសិស្សដែលកំពុងខ្ចីសៀវភៅបានទេ"}, {Language::English, "💡 Cannot lock student with borrowed books"} };
    m_dictionary["LBL_ISBN_READONLY"] = { {Language::Vietnamese, "Mã ISBN không thể thay đổi"}, {Language::Khmer, "មិនអាចប្តូរលេខកូដ ISBN បានទេ"}, {Language::English, "ISBN cannot be changed"} };
    // === BACKUP SCREEN (Missing) ===
    m_dictionary["MSG_ERROR_CREATE_BACKUP_FAILED"] = { {Language::Vietnamese, "Không thể tạo backup: %1"}, {Language::Khmer, "មិនអាចបង្កើតការបម្រុងទុក: %1"}, {Language::English, "Cannot create backup: %1"} };
    m_dictionary["MSG_RESTORE_SUCCESS"] = { {Language::Vietnamese, "Đã phục hồi dữ liệu thành công!\nVui lòng tải lại chương trình."}, {Language::Khmer, "បានស្តារទិន្នន័យដោយជោគជ័យ!\nសូមផ្ទុកកម្មវិធីឡើងវិញ។"}, {Language::English, "Restore successful!\nPlease restart application."} };
    m_dictionary["MSG_ERROR_RESTORE_FAILED"] = { {Language::Vietnamese, "Không thể phục hồi:\n%1"}, {Language::Khmer, "មិនអាចស្តារ:\n%1"}, {Language::English, "Cannot restore:\n%1"} };
    m_dictionary["MSG_ERROR_DELETE_FAILED"] = { {Language::Vietnamese, "Không thể xóa:\n%1"}, {Language::Khmer, "មិនអាចលុប:\n%1"}, {Language::English, "Cannot delete:\n%1"} };
    m_dictionary["MSG_ERROR_CLEANUP_FAILED"] = { {Language::Vietnamese, "Không thể xóa các bản cũ!"}, {Language::Khmer, "មិនអាចលុបឯកសារចាស់!"}, {Language::English, "Cannot clean up old backups!"} };

    // === SETTINGS SCREEN ===
    m_dictionary["TITLE_SETTINGS"] = { {Language::Vietnamese, "Cấu Hình Hệ Thống"}, {Language::Khmer, "ការកំណត់ប្រព័ន្ធ"}, {Language::English, "System Settings"} };
    m_dictionary["HEADER_LIBRARY_INFO"] = { {Language::Vietnamese, "Thông Tin Thư Viện"}, {Language::Khmer, "ព័ត៌មានបណ្ណាល័យ"}, {Language::English, "Library Info"} };
    m_dictionary["HEADER_PRINTER_CONFIG"] = { {Language::Vietnamese, "Cấu Hình Máy In"}, {Language::Khmer, "ការកំណត់ម៉ាស៊ីនបោះពុម្ព"}, {Language::English, "Printer Configuration"} };
    m_dictionary["HEADER_STORAGE_PATHS"] = { {Language::Vietnamese, "Đường Dẫn Lưu Trữ"}, {Language::Khmer, "ផ្លូវផ្ទុក"}, {Language::English, "Storage Paths"} };
    m_dictionary["HEADER_LOAN_RULES"] = { {Language::Vietnamese, "Quy Định Mượn Trả"}, {Language::Khmer, "ច្បាប់ខ្ចី"}, {Language::English, "Loan Rules"} };
    
    m_dictionary["LBL_LIB_NAME"] = { {Language::Vietnamese, "Tên thư viện:"}, {Language::Khmer, "ឈ្មោះបណ្ណាល័យ:"}, {Language::English, "Library Name:"} };
    m_dictionary["LBL_LIB_SHORT_NAME"] = { {Language::Vietnamese, "Tên viết tắt:"}, {Language::Khmer, "ឈ្មោះកាត់:"}, {Language::English, "Short Name:"} };
    m_dictionary["LBL_PRINT_COLOR"] = { {Language::Vietnamese, "In màu"}, {Language::Khmer, "បោះពុម្ពពណ៌"}, {Language::English, "Print Color"} };
    m_dictionary["LBL_SHOW_LOGO"] = { {Language::Vietnamese, "Hiện logo"}, {Language::Khmer, "បង្ហាញរូបសញ្ញា"}, {Language::English, "Show Logo"} };
    m_dictionary["LBL_LOGO"] = { {Language::Vietnamese, "Logo:"}, {Language::Khmer, "រូបសញ្ញា:"}, {Language::English, "Logo:"} };
    
    m_dictionary["LBL_PRINTER_RECEIPT"] = { {Language::Vietnamese, "Máy in hóa đơn:"}, {Language::Khmer, "ម៉ាស៊ីនបោះពុម្ពវិក្កយបត្រ:"}, {Language::English, "Receipt Printer:"} };
    m_dictionary["LBL_PRINTER_REPORT"] = { {Language::Vietnamese, "Máy in A4:"}, {Language::Khmer, "ម៉ាស៊ីនបោះពុម្ព A4:"}, {Language::English, "A4 Printer:"} };
    m_dictionary["LBL_SILENT_PRINT"] = { {Language::Vietnamese, "Tự động in (Không hỏi)"}, {Language::Khmer, "បោះពុម្ពដោយស្វ័យប្រវត្តិ"}, {Language::English, "Silent Print (No Confirm)"} };
    
    m_dictionary["LBL_QR_DIR"] = { {Language::Vietnamese, "Thư mục QR:"}, {Language::Khmer, "ថត QR:"}, {Language::English, "QR Folder:"} };
    m_dictionary["LBL_PDF_DIR"] = { {Language::Vietnamese, "Thư mục PDF:"}, {Language::Khmer, "ថត PDF:"}, {Language::English, "PDF Folder:"} };
    
    m_dictionary["LBL_LOAN_DAYS"] = { {Language::Vietnamese, "Thời hạn mượn:"}, {Language::Khmer, "រយៈពេលខ្ចី:"}, {Language::English, "Loan Duration:"} };
    m_dictionary["LBL_FINE_PER_DAY"] = { {Language::Vietnamese, "Tiền phạt quá hạn:"}, {Language::Khmer, "ប្រាក់ពិន័យហួសកាលកំណត់:"}, {Language::English, "Overdue Fine:"} };
    m_dictionary["LBL_MAX_EXTENSIONS"] = { {Language::Vietnamese, "Số lần gia hạn tối đa:"}, {Language::Khmer, "ចំនួនដងបន្តអតិបរមា:"}, {Language::English, "Max Extensions:"} };
    
    m_dictionary["PLACEHOLDER_LIB_NAME"] = { {Language::Vietnamese, "VD: HỌC VIỆN PHẬT GIÁO..."}, {Language::Khmer, "ឧ. បណ្ឌិត្យសភា..."}, {Language::English, "e.g. BUDDHIST ACADEMY..."} };
    m_dictionary["PLACEHOLDER_LIB_SHORT_NAME"] = { {Language::Vietnamese, "VD: HVPG"}, {Language::Khmer, "ឧ. HVPG"}, {Language::English, "e.g. BA"} };
    
    m_dictionary["SUFFIX_DAYS"] = { {Language::Vietnamese, " ngày"}, {Language::Khmer, " ថ្ងៃ"}, {Language::English, " days"} };
    m_dictionary["SUFFIX_MONEY_DAY"] = { {Language::Vietnamese, " VNĐ/ngày"}, {Language::Khmer, " រៀល/ថ្ងៃ"}, {Language::English, " VND/day"} };
    m_dictionary["SUFFIX_TIMES"] = { {Language::Vietnamese, " lần"}, {Language::Khmer, " ដង"}, {Language::English, " times"} };
    
    m_dictionary["BTN_BROWSE"] = { {Language::Vietnamese, "Chọn..."}, {Language::Khmer, "ជ្រើសរើស..."}, {Language::English, "Browse..."} };
    m_dictionary["BTN_SAVE_CONFIG"] = { {Language::Vietnamese, "Lưu Cấu Hình"}, {Language::Khmer, "រក្សាទុកការកំណត់"}, {Language::English, "Save Config"} };
    m_dictionary["COMBO_SELECT_PRINTER"] = { {Language::Vietnamese, "-- Chọn máy in --"}, {Language::Khmer, "-- ជ្រើសរើសម៉ាស៊ីនបោះពុម្ព --"}, {Language::English, "-- Select Printer --"} };
    
    m_dictionary["MSG_SAVE_CONFIG_SUCCESS"] = { {Language::Vietnamese, "Đã lưu cấu hình in ấn!"}, {Language::Khmer, "បានរក្សាទុកការកំណត់!"}, {Language::English, "Configuration saved!"} };
    m_dictionary["MSG_ERROR_NOT_FOUND"] = { {Language::Vietnamese, "Không tìm thấy dữ liệu!"}, {Language::Khmer, "រកមិនឃើញទិន្នន័យទេ!"}, {Language::English, "Data not found!"} };
    m_dictionary["SUFFIX_COPIES"] = { {Language::Vietnamese, " bản sao"}, {Language::Khmer, " ច្បាប់ចម្លង"}, {Language::English, " copies"} };
    
    m_dictionary["MSG_CONFIRM_NORMALIZE"] = { {Language::Vietnamese, "Hệ thống sẽ quét mã sách và chuẩn hóa.\nDữ liệu sẽ thay đổi!\nBạn có chắc chắn?"}, {Language::Khmer, "ប្រព័ន្ធនឹងស្កេននិងធ្វើឱ្យលេខកូដសៀវភៅមានលក្ខណៈស្តង់ដារ។\nទិន្នន័យនឹងផ្លាស់ប្តូរ!\nតើអ្នកប្រាកដទេ?"}, {Language::English, "System will normalize book codes.\nData will change!\nAre you sure?"} };
    m_dictionary["MSG_NORMALIZE_COMPLETE"] = { {Language::Vietnamese, "Hoàn Tất: %1 xử lý, %2 đã sửa."}, {Language::Khmer, "បានបញ្ចប់: %1 ដំណើរការ, %2 បានជួសជុល។"}, {Language::English, "Complete: %1 processed, %2 fixed."} };
    m_dictionary["MSG_NORMALIZE_NO_CHANGE"] = { {Language::Vietnamese, "Không có gì thay đổi."}, {Language::Khmer, "មិនមានអ្វីផ្លាស់ប្តូរទេ។"}, {Language::English, "No changes."} };
    m_dictionary["TITLE_CONFIRM"] = { {Language::Vietnamese, "Xác Nhận"}, {Language::Khmer, "បញ្ជាក់"}, {Language::English, "Confirm"} };
    m_dictionary["TITLE_COMPLETE"] = { {Language::Vietnamese, "Hoàn Tất"}, {Language::Khmer, "បានបញ្ចប់"}, {Language::English, "Complete"} };
    
    m_dictionary["TITLE_COMMIT_ERROR"] = { {Language::Vietnamese, "Lỗi Commit"}, {Language::Khmer, "កំហុស Commit"}, {Language::English, "Commit Error"} };
    m_dictionary["MSG_COMMIT_FAILED"] = { {Language::Vietnamese, "Không thể lưu thay đổi vào Database!"}, {Language::Khmer, "មិនអាចរក្សាទុកការផ្លាស់ប្តូរទៅ Database បានទេ!"}, {Language::English, "Cannot save changes to Database!"} };
    
    // === ADD COPY DIALOG ===
    m_dictionary["TITLE_ADD_COPY"] = { {Language::Vietnamese, "Thêm Bản Sao"}, {Language::Khmer, "បន្ថែមច្បាប់ចម្លង"}, {Language::English, "Add Copy"} };
    m_dictionary["HEADER_ADD_COPY"] = { {Language::Vietnamese, "THÊM BẢN SAO SÁCH"}, {Language::Khmer, "បន្ថែមច្បាប់ចម្លងសៀវភៅ"}, {Language::English, "ADD BOOK COPY"} };
    m_dictionary["GROUP_COPY_INFO"] = { {Language::Vietnamese, "Chi Tiết Bản Sao"}, {Language::Khmer, "ព័ត៌មានលម្អិតច្បាប់ចម្លង"}, {Language::English, "Copy Details"} };
    m_dictionary["LBL_QUANTITY"] = { {Language::Vietnamese, "Số lượng:"}, {Language::Khmer, "បរិមាណ:"}, {Language::English, "Quantity:"} };
    m_dictionary["LBL_NOTE_AUTO_CODE"] = { {Language::Vietnamese, "💡 Mã sách sẽ được tạo tự động dựa trên ISBN và số lượng."}, {Language::Khmer, "💡 លេខកូដសៀវភៅនឹងត្រូវបង្កើតដោយស្វ័យប្រវត្តិដោយផ្អែកលើ ISBN និងបរិមាណ។"}, {Language::English, "💡 Book codes generated automatically based on ISBN and quantity."} };
    m_dictionary["BTN_ADD_COPY"] = { {Language::Vietnamese, "Thêm Bản Sao"}, {Language::Khmer, "បន្ថែមច្បាប់ចម្លង"}, {Language::English, "Add Copy"} };
    m_dictionary["MSG_ADD_COPY_SUCCESS"] = { {Language::Vietnamese, "✅ Đã thêm thành công %1 bản sao!\n📚 Tên: %2\n📍 Vị trí: %3\n🔖 Mã đầu: %4"}, {Language::Khmer, "✅ បានបន្ថែមច្បាប់ចម្លង %1 ដោយជោគជ័យ!\n📚 ឈ្មោះ: %2\n📍 ទីតាំង: %3\n🔖 កូដដំបូង: %4"}, {Language::English, "✅ Added %1 copies successfully!\n📚 Title: %2\n📍 Location: %3\n🔖 First Code: %4"} };
    m_dictionary["MSG_ERROR_ADD_COPY_FAILED"] = { {Language::Vietnamese, "Không thể thêm bản sao. Vui lòng kiểm tra lại!"}, {Language::Khmer, "មិនអាចបន្ថែមច្បាប់ចម្លងទេ។ សូមពិនិត្យម្តងទៀត!"}, {Language::English, "Cannot add copy. Please check again!"} };
    
    // === ABOUT DIALOG ===
    m_dictionary["HEADER_SYSTEM_NAME"] = { {Language::Vietnamese, "HỆ THỐNG QUẢN LÝ THƯ VIỆN"}, {Language::Khmer, "ប្រព័ន្ធគ្រប់គ្រងបណ្ណាល័យ"}, {Language::English, "LIBRARY MANAGEMENT SYSTEM"} };
    m_dictionary["TITLE_ABOUT"] = { {Language::Vietnamese, "Về Hệ Thống"}, {Language::Khmer, "អំពីប្រព័ន្ធ"}, {Language::English, "About System"} };
    m_dictionary["LBL_VERSION"] = { {Language::Vietnamese, "Phiên bản: 2.1 (Premium)"}, {Language::Khmer, "កំណែ: 2.1 (Premium)"}, {Language::English, "Version: 2.1 (Premium)"} };
    m_dictionary["LBL_BUILD_DATE"] = { {Language::Vietnamese, "Ngày xây dựng: 20/12/2025"}, {Language::Khmer, "កាលបរិច្ឆេទសាងសង់: 20/12/2025"}, {Language::English, "Build Date: 20/12/2025"} };
    m_dictionary["MSG_ABOUT_FEATURES_HTML"] = { 
        {Language::Vietnamese, "<b>Tính năng tiêu biểu (Premium):</b><ul style='margin-left:15px;'><li><b>Tam Tạng:</b> Quản lý Luật - Kinh - Vi Diệu Pháp chuyên sâu.</li><li><b>Địa phương hóa:</b> Tích hợp Phật lịch (B.E.), Đa ngôn ngữ Việt - Khmer.</li><li><b>Dữ liệu:</b> Cây AVL (Tăng sinh), DSLK (Mượn trả), Mảng (Sách).</li><li><b>Tiện ích:</b> Tự động sao lưu, In ấn chuyên nghiệp, Xuất PDF.</li></ul>"}, 
        {Language::Khmer, "<b>លក្ខណៈពិសេស (Premium):</b><ul style='margin-left:15px;'><li><b>ព្រះត្រៃបិដក:</b> គ្រប់គ្រង វិន័យ សុត្ត អភិធម្ម.</li><li><b>ការធ្វើមូលដ្ឋានីយកម្ម:</b> រួមបញ្ចូលពុទ្ធសករាជ (ព.ស).</li><li><b>ទិន្នន័យ:</b> Tree AVL (សមណៈសិស្ស), DSLK, Array.</li><li><b>ឧបករណ៍:</b> បម្រុងទុកដោយស្វ័យប្រវត្តិ, នាំចេញ PDF.</li></ul>"}, 
        {Language::English, "<b>Premium Features:</b><ul style='margin-left:15px;'><li><b>Tripitaka:</b> Specialized Vinaya - Sutta - Abhidhamma management.</li><li><b>Localization:</b> Buddhist Era (B.E.) integration.</li><li><b>Data Structures:</b> AVL Tree (Monks), Linked List, Array.</li><li><b>Utilities:</b> Auto Backup, Professional Printing, PDF Export.</li></ul>"} 
    };
    m_dictionary["MSG_ABOUT_TECH_HTML"] = { 
        {Language::Vietnamese, "<b>Công nghệ sử dụng:</b><br>- Qt 6.8 C++ Framework (LTS)<br>- SQLite Database System<br>- Modern UI/UX Design & Animations"}, 
        {Language::Khmer, "<b>បច្ចេកវិទ្យាដែលបានប្រើ:</b><br>- Qt 6.8 C++ Framework (LTS)<br>- SQLite Database System<br>- Modern UI/UX Design & Animations"}, 
        {Language::English, "<b>Tech Stack:</b><br>- Qt 6.8 C++ Framework (LTS)<br>- SQLite Database System<br>- Modern UI/UX Design & Animations"} 
    };
    m_dictionary["MSG_ABOUT_CREDIT"] = { {Language::Vietnamese, "Phát triển bởi Chăm Rốch Thi @ 2025"}, {Language::Khmer, "បង្កើតដោយ Chăm Rốch Thi @ 2025"}, {Language::English, "Developed by Chăm Rốch Thi @ 2025"} };
    
    // === STATISTICS DIALOG ===
    m_dictionary["DIALOG_STATS_TITLE"] = { {Language::Vietnamese, "Thống Kê Tổng Quan"}, {Language::Khmer, "ស្ថិតិទូទៅ"}, {Language::English, "General Statistics"} };
    m_dictionary["TITLE_STATS_HEADER"] = { {Language::Vietnamese, "BẢNG THỐNG KÊ HOẠT ĐỘNG"}, {Language::Khmer, "តារាងស្ថិតិសកម្មភាព"}, {Language::English, "ACTIVITY STATISTICS"} };
    m_dictionary["MSG_STATS_NOTE_HTML"] = { {Language::Vietnamese, "<b>Lưu ý:</b> Dữ liệu được cập nhật theo thời gian thực."}, {Language::Khmer, "<b>ចំណាំ:</b> ទិន្នន័យត្រូវបានធ្វើបច្ចុប្បន្នភាពតាមពេលវេលាជាក់ស្តែង។"}, {Language::English, "<b>Note:</b> Data is updated in real-time."} };
    m_dictionary["GROUP_STATS_BOOKS"] = { {Language::Vietnamese, "Thống Kê Sách"}, {Language::Khmer, "ស្ថិតិសៀវភៅ"}, {Language::English, "Book Stats"} };
    m_dictionary["GROUP_STATS_READERS"] = { {Language::Vietnamese, "Thống Kê Học Viên"}, {Language::Khmer, "ស្ថិតិសិស្ស"}, {Language::English, "Reader Stats"} };
    
    m_dictionary["LBL_TOTAL_TITLES"] = { {Language::Vietnamese, "Tổng đầu sách:"}, {Language::Khmer, "ចំណងជើងសរុប:"}, {Language::English, "Total Titles:"} };
    m_dictionary["LBL_TOTAL_COPIES"] = { {Language::Vietnamese, "Tổng bản sao:"}, {Language::Khmer, "ច្បាប់ចម្លងសរុប:"}, {Language::English, "Total Copies:"} };
    m_dictionary["LBL_AVAILABLE_COPIES"] = { {Language::Vietnamese, "Có thể mượn:"}, {Language::Khmer, "អាចខ្ចីបាន:"}, {Language::English, "Available:"} };
    m_dictionary["LBL_BORROWED_COPIES"] = { {Language::Vietnamese, "Đang được mượn:"}, {Language::Khmer, "កំពុងខ្ចី:"}, {Language::English, "Borrowed:"} };
    m_dictionary["LBL_LIQUIDATED_COPIES"] = { {Language::Vietnamese, "Đã thanh lý:"}, {Language::Khmer, "បានលុបចោល:"}, {Language::English, "Liquidated:"} };
    m_dictionary["LBL_BORROW_RATE"] = { {Language::Vietnamese, "Tỷ lệ cho mượn:"}, {Language::Khmer, "អត្រាខ្ចី:"}, {Language::English, "Borrow Rate:"} };
    m_dictionary["LBL_TOTAL_CATEGORIES"] = { {Language::Vietnamese, "Số thể loại:"}, {Language::Khmer, "ចំនួនប្រភេទ:"}, {Language::English, "Categories:"} };
    
    m_dictionary["LBL_TOTAL_READERS"] = { {Language::Vietnamese, "Tổng học viên:"}, {Language::Khmer, "សិស្សសរុប:"}, {Language::English, "Total Readers:"} };
    m_dictionary["LBL_ACTIVE_READERS"] = { {Language::Vietnamese, "Đang hoạt động:"}, {Language::Khmer, "កំពុងសកម្ម:"}, {Language::English, "Active:"} };
    m_dictionary["LBL_LOCKED_READERS"] = { {Language::Vietnamese, "Bị khóa:"}, {Language::Khmer, "ត្រូវបានចាក់សោ:"}, {Language::English, "Locked:"} };
    m_dictionary["LBL_READERS_BORROWING"] = { {Language::Vietnamese, "Đang mượn sách:"}, {Language::Khmer, "កំពុងខ្ចីសៀវភៅ:"}, {Language::English, "Borrowing:"} };
    m_dictionary["LBL_TOTAL_BORROWED_BOOKS"] = { {Language::Vietnamese, "Tổng sách đang mượn:"}, {Language::Khmer, "សៀវភៅខ្ចីសរុប:"}, {Language::English, "Total Books Borrowed:"} };
    m_dictionary["LBL_READERS_OVERDUE"] = { {Language::Vietnamese, "Có sách quá hạn:"}, {Language::Khmer, "មានសៀវភៅហួសកំណត់:"}, {Language::English, "Overdue Readers:"} };
    
    // === IMPORT PREVIEW DIALOG ===
    m_dictionary["TITLE_PREVIEW_IMPORT"] = { {Language::Vietnamese, "Xem Trước Dữ Liệu Import"}, {Language::Khmer, "មើលទិន្នន័យនាំចូលជាមុន"}, {Language::English, "Import Preview"} };
    m_dictionary["HEADER_PREVIEW_IMPORT"] = { {Language::Vietnamese, "KIỂM TRA DỮ LIỆU TRƯỚC KHI IMPORT"}, {Language::Khmer, "ពិនិត្យទិន្នន័យមុនពេលនាំចូល"}, {Language::English, "VERIFY DATA BEFORE IMPORT"} };
    m_dictionary["LBL_PREVIEW_NOTE"] = { {Language::Vietnamese, "Vui lòng kiểm tra kỹ các tab dữ liệu bên dưới"}, {Language::Khmer, "សូមពិនិត្យមើលផ្ទាំងទិន្នន័យខាងក្រោម"}, {Language::English, "Please check data tabs below"} };
    
    m_dictionary["TAB_BOOKS"] = { {Language::Vietnamese, "Đầu Sách"}, {Language::Khmer, "សៀវភៅ"}, {Language::English, "Titles"} };
    m_dictionary["TAB_COPIES"] = { {Language::Vietnamese, "Bản Sao"}, {Language::Khmer, "ច្បាប់ចម្លង"}, {Language::English, "Copies"} };
    m_dictionary["TAB_READERS"] = { {Language::Vietnamese, "Học Viên"}, {Language::Khmer, "សិស្ស"}, {Language::English, "Students"} };
    m_dictionary["TAB_LOANS"] = { {Language::Vietnamese, "Mượn Trả"}, {Language::Khmer, "ខ្ចី/សង"}, {Language::English, "Loans"} };
    
    m_dictionary["BTN_APPLY_IMPORT"] = { {Language::Vietnamese, "Xác Nhận Import"}, {Language::Khmer, "បញ្ជាក់ការនាំចូល"}, {Language::English, "Confirm Import"} };
    m_dictionary["OPTION_MERGE"] = { {Language::Vietnamese, "Gộp dữ liệu"}, {Language::Khmer, "បញ្ចូលទិន្នន័យ (Merge)"}, {Language::English, "Merge Data"} };
    m_dictionary["OPTION_REPLACE"] = { {Language::Vietnamese, "Thay thế"}, {Language::Khmer, "ជំនួស (Replace)"}, {Language::English, "Replace"} };
    m_dictionary["OPTION_SKIP"] = { {Language::Vietnamese, "Bỏ qua trùng lặp"}, {Language::Khmer, "រំលងស្ទួន (Skip)"}, {Language::English, "Skip Duplicates"} };
    
    m_dictionary["DIALOG_IMPORT_MODE_TITLE"] = { {Language::Vietnamese, "Chế Độ Import"}, {Language::Khmer, "របៀបនាំចូល"}, {Language::English, "Import Mode"} };
    m_dictionary["MSG_IMPORT_SUCCESS"] = { {Language::Vietnamese, "Import dữ liệu hoàn tất!"}, {Language::Khmer, "ការនាំចូលទិន្នន័យបានបញ្ចប់!"}, {Language::English, "Import completed!"} };
    
    // === TOP 10 DIALOG ===
    m_dictionary["TITLE_TOP10"] = { {Language::Vietnamese, "Top 10 Sách Hot"}, {Language::Khmer, "សៀវភៅពេញនិយមទាំង 10"}, {Language::English, "Top 10 Books"} };
    m_dictionary["HEADER_TOP10"] = { {Language::Vietnamese, "BẢNG XẾP HẠNG TOP 10 SÁCH"}, {Language::Khmer, "តារាងចំណាត់ថ្នាក់សៀវភៅទាំង 10"}, {Language::English, "TOP 10 BOOKS RANKING"} };
    m_dictionary["GROUP_FILTER_TIME"] = { {Language::Vietnamese, "Bộ Lọc Thời Gian"}, {Language::Khmer, "តម្រងពេលវេលា"}, {Language::English, "Time Filter"} };
    m_dictionary["LBL_RANGE"] = { {Language::Vietnamese, "Khoảng thời gian:"}, {Language::Khmer, "រយៈពេល:"}, {Language::English, "Time Range:"} };
    m_dictionary["CB_TIME_ALL"] = { {Language::Vietnamese, "Tất cả"}, {Language::Khmer, "ទាំងអស់"}, {Language::English, "All Time"} };
    m_dictionary["CB_TIME_THIS_MONTH"] = { {Language::Vietnamese, "Tháng này"}, {Language::Khmer, "ខែនេះ"}, {Language::English, "This Month"} };
    m_dictionary["CB_TIME_LAST_MONTH"] = { {Language::Vietnamese, "Tháng trước"}, {Language::Khmer, "ខែមុន"}, {Language::English, "Last Month"} };
    m_dictionary["CB_TIME_CUSTOM"] = { {Language::Vietnamese, "Tùy chỉnh"}, {Language::Khmer, "ផ្ទាល់ខ្លួន"}, {Language::English, "Custom"} };
    m_dictionary["LBL_FROM_DATE"] = { {Language::Vietnamese, "Từ ngày:"}, {Language::Khmer, "ចាប់ពី:"}, {Language::English, "From:"} };
    m_dictionary["LBL_TO_DATE"] = { {Language::Vietnamese, "Đến ngày:"}, {Language::Khmer, "ដល់:"}, {Language::English, "To:"} };
    m_dictionary["CHECK_SHOW_TREND"] = { {Language::Vietnamese, "Hiển thị xu hướng so với kỳ trước"}, {Language::Khmer, "បង្ហាញនិន្នាការធៀបនឹងគ្រាមុន"}, {Language::English, "Show trend vs previous period"} };
    m_dictionary["MSG_TOP10_NOTE"] = { {Language::Vietnamese, "Bảng xếp hạng dựa trên số lượt mượn thực tế."}, {Language::Khmer, "ចំណាត់ថ្នាក់ផ្អែកលើការខ្ចីជាក់ស្តែង។"}, {Language::English, "Ranking based on actual loans."} };
    m_dictionary["MSG_NO_DATA_TOP10"] = { {Language::Vietnamese, "Chưa có dữ liệu thống kê!"}, {Language::Khmer, "មិនទាន់មានទិន្នន័យស្ថិតិទេ!"}, {Language::English, "No statistics data yet!"} };
    m_dictionary["MSG_NO_DATA_PERIOD"] = { {Language::Vietnamese, "Không có dữ liệu trong khoảng thời gian này."}, {Language::Khmer, "គ្មានទិន្នន័យក្នុងកំឡុងពេលនេះទេ។"}, {Language::English, "No data in this period."} };
    m_dictionary["MSG_STATS_SUMMARY_HTML"] = { {Language::Vietnamese, "Thống kê <b>%1</b>: Có <b>%2</b> đầu sách được mượn với tổng <b>%3</b> lượt. Hiển thị Top <b>%4</b>."}, {Language::Khmer, "ស្ថិតិ <b>%1</b>: មានសៀវភៅ <b>%2</b> ត្រូវបានខ្ចីសរុប <b>%3</b> ដង។ បង្ហាញ Top <b>%4</b>។"}, {Language::English, "Stats for <b>%1</b>: <b>%2</b> titles borrowed <b>%3</b> times total. Showing Top <b>%4</b>."} };
    m_dictionary["MSG_TREND_POSITIVE"] = { {Language::Vietnamese, "📈 Xu hướng TỐT: %1 sách tăng, %2 sách giảm, %3 không đổi."}, {Language::Khmer, "📈 និន្នាការល្អ: %1 កើនឡើង, %2 ថយចុះ, %3 មិនផ្លាស់ប្តូរ។"}, {Language::English, "📈 POSITIVE Trend: %1 up, %2 down, %3 stable."} };
    m_dictionary["MSG_TREND_NEGATIVE"] = { {Language::Vietnamese, "📉 Xu hướng GIẢM: %1 sách giảm, %2 sách tăng, %3 không đổi."}, {Language::Khmer, "📉 និន្នាការធ្លាក់ចុះ: %1 ថយចុះ, %2 កើនឡើង, %3 មិនផ្លាស់ប្តូរ។"}, {Language::English, "📉 NEGATIVE Trend: %1 down, %2 up, %3 stable."} };
    m_dictionary["MSG_TREND_STABLE"] = { {Language::Vietnamese, "📊 Xu hướng ỔN ĐỊNH: %1 tăng, %2 giảm, %3 không đổi."}, {Language::Khmer, "📊 និន្នាការថេរ: %1 កើនឡើង, %2 ថយចុះ, %3 មិនផ្លាស់ប្តូរ។"}, {Language::English, "📊 STABLE Trend: %1 up, %2 down, %3 stable."} };
    
    m_dictionary["TABLE_HEADER_RANK"] = { {Language::Vietnamese, "Hạng"}, {Language::Khmer, "ចំណាត់ថ្នាក់"}, {Language::English, "Rank"} };
    m_dictionary["TABLE_HEADER_LOANS"] = { {Language::Vietnamese, "Lượt Mượn"}, {Language::Khmer, "ចំនួនខ្ចី"}, {Language::English, "Loans"} };
    m_dictionary["TABLE_HEADER_RATE"] = { {Language::Vietnamese, "Tỷ Lệ"}, {Language::Khmer, "អត្រា"}, {Language::English, "Rate"} };
    m_dictionary["TABLE_HEADER_TREND"] = { {Language::Vietnamese, "Xu Hướng"}, {Language::Khmer, "និន្នាការ"}, {Language::English, "Trend"} };
    m_dictionary["BTN_EXPORT_REPORT"] = { {Language::Vietnamese, "Xuất Báo Cáo"}, {Language::Khmer, "នាំចេញរបាយការណ៍"}, {Language::English, "Export Report"} };
    m_dictionary["BTN_REFRESH"] = { {Language::Vietnamese, "Làm Mới"}, {Language::Khmer, "ផ្ទុកឡើងវិញ"}, {Language::English, "Refresh"} };

    // === OVERDUE DIALOG ===
    m_dictionary["TITLE_OVERDUE_LIST_DIALOG"] = { {Language::Vietnamese, "Học Viên Mượn Quá Hạn"}, {Language::Khmer, "សិស្សខ្ចីហួសកំណត់"}, {Language::English, "Overdue Readers List"} };
    m_dictionary["HEADER_OVERDUE_LIST"] = { {Language::Vietnamese, "DANH SÁCH HỌC VIÊN MƯỢN QUÁ HẠN"}, {Language::Khmer, "បញ្ជីសិស្សខ្ចីហួសកំណត់"}, {Language::English, "OVERDUE READERS LIST"} };
    m_dictionary["GROUP_REVENUE_STATS"] = { {Language::Vietnamese, "Thống Kê Doanh Thu"}, {Language::Khmer, "ស្ថិតិប្រាក់ចំណូល"}, {Language::English, "Revenue Statistics"} };
    m_dictionary["GROUP_ACTIVE_READERS"] = { {Language::Vietnamese, "Học Viên Hoạt Động"}, {Language::Khmer, "សិស្សសកម្ម"}, {Language::English, "Active Readers"} };
    m_dictionary["MSG_OVERDUE_NOTE"] = { {Language::Vietnamese, "📌 Danh sách học viên có sách mượn quá hạn. Phí phạt: 5,000đ/ngày sau 7 ngày mượn."}, {Language::Khmer, "📌 បញ្ជីសិស្សដែលមានសៀវភៅហួសកំណត់។ ពិន័យ: 5,000 រៀល/ថ្ងៃ បន្ទាប់ពី 7 ថ្ងៃ។"}, {Language::English, "📌 List of overdue readers. Fine: 5,000 VND/day after 7 days."} };
    m_dictionary["MSG_NO_DATA_SYSTEM"] = { {Language::Vietnamese, "Chưa có dữ liệu học viên trong hệ thống"}, {Language::Khmer, "មិនទាន់មានទិន្នន័យសិស្សនៅក្នុងប្រព័ន្ធទេ"}, {Language::English, "No reader data in system"} };
    m_dictionary["MSG_NO_DATA_OVERDUE"] = { {Language::Vietnamese, "Không có học viên nào mượn quá hạn!"}, {Language::Khmer, "គ្មានសិស្សណាម្នាក់ខ្ចីហួសកំណត់ទេ!"}, {Language::English, "No overdue readers!"} };
    m_dictionary["MSG_OVERDUE_SUMMARY"] = { {Language::Vietnamese, "Có %1 học viên mượn quá hạn | Tổng %2 cuốn sách quá hạn | Cần xử lý ngay!"}, {Language::Khmer, "មានសិស្ស %1 នាក់ហួសកំណត់ | សៀវភៅហួសកំណត់ %2 ក្បាល | ត្រូវការដោះស្រាយជាបន្ទាន់!"}, {Language::English, "There are %1 overdue readers | Total %2 overdue books | Action needed!"} };
    m_dictionary["MSG_REVENUE_STATS_HTML"] = { {Language::Vietnamese, "<b>Tổng phí phạt:</b> %1 đ<br><b>Nghiêm trọng:</b> %2 học viên<br><b>Mức cao:</b> %3 học viên<br><i>Phí phạt: 5,000đ/ngày sau 7 ngày mượn</i>"}, {Language::Khmer, "<b>ពិន័យសរុប:</b> %1<br><b>ធ្ងន់ធ្ងរ:</b> %2<br><b>កម្រិតខ្ពស់:</b> %3<br><i>ពិន័យ: 5,000/ថ្ងៃ</i>"}, {Language::English, "<b>Total Fine:</b> %1<br><b>Severe:</b> %2<br><b>High:</b> %3<br><i>Fine: 5k/day</i>"} };
    m_dictionary["MSG_ACTIVE_STATS_HTML"] = { {Language::Vietnamese, "<b>Tổng học viên:</b> %1<br><b>Hoạt động 7 ngày:</b> %2<br><b>Hoạt động 30 ngày:</b> %3<br><b>Tỷ lệ hoạt động:</b> %4%"}, {Language::Khmer, "<b>សិស្សសរុប:</b> %1<br><b>សកម្ម 7 ថ្ងៃ:</b> %2<br><b>សកម្ម 30 ថ្ងៃ:</b> %3<br><b>អត្រាសកម្មភាព:</b> %4%"}, {Language::English, "<b>Total Readers:</b> %1<br><b>Active 7d:</b> %2<br><b>Active 30d:</b> %3<br><b>Activity Rate:</b> %4%"} };
    
    m_dictionary["TABLE_HEADER_CARD_ID"] = { {Language::Vietnamese, "Mã Thẻ"}, {Language::Khmer, "កូដកាត"}, {Language::English, "Card ID"} };
    m_dictionary["TABLE_HEADER_GENDER"] = { {Language::Vietnamese, "Giới Tính"}, {Language::Khmer, "ភេទ"}, {Language::English, "Gender"} };
    m_dictionary["TABLE_HEADER_OVERDUE_BOOKS"] = { {Language::Vietnamese, "Số Sách QH"}, {Language::Khmer, "សៀវភៅ QH"}, {Language::English, "Overdue Books"} };
    m_dictionary["TABLE_HEADER_TOTAL_DAYS"] = { {Language::Vietnamese, "Tổng Ngày"}, {Language::Khmer, "ថ្ងៃសរុប"}, {Language::English, "Total Days"} };
    m_dictionary["TABLE_HEADER_MAX_DAYS"] = { {Language::Vietnamese, "Ngày Max"}, {Language::Khmer, "ថ្ងៃអតិបរមា"}, {Language::English, "Max Days"} };
    m_dictionary["TABLE_HEADER_FINE"] = { {Language::Vietnamese, "Phí Phạt"}, {Language::Khmer, "ប្រាក់ពិន័យ"}, {Language::English, "Fine"} };
    m_dictionary["TABLE_HEADER_SEVERITY"] = { {Language::Vietnamese, "Mức Độ"}, {Language::Khmer, "កម្រិត"}, {Language::English, "Severity"} };
    
    m_dictionary["SEVERITY_SEVERE"] = { {Language::Vietnamese, "Nghiêm Trọng"}, {Language::Khmer, "ធ្ងន់ធ្ងរ"}, {Language::English, "Severe"} };
    m_dictionary["SEVERITY_HIGH"] = { {Language::Vietnamese, "Cao"}, {Language::Khmer, "ខ្ពស់"}, {Language::English, "High"} };
    m_dictionary["SEVERITY_MEDIUM"] = { {Language::Vietnamese, "Trung Bình"}, {Language::Khmer, "មធ្យម"}, {Language::English, "Medium"} };
    m_dictionary["SEVERITY_LOW"] = { {Language::Vietnamese, "Nhẹ"}, {Language::Khmer, "ទាប"}, {Language::English, "Low"} };
    
    m_dictionary["BTN_VIEW_DETAIL"] = { {Language::Vietnamese, "Xem Chi Tiết"}, {Language::Khmer, "មើលលម្អិត"}, {Language::English, "View Detail"} };

    // === GENRES ===
    m_dictionary["GENRE_LITERATURE"] = { {Language::Vietnamese, "Văn học"}, {Language::Khmer, "អក្សរសិល្ប៍"}, {Language::English, "Literature"} };
    m_dictionary["GENRE_SCIENCE"] = { {Language::Vietnamese, "Khoa học"}, {Language::Khmer, "វិទ្យាសាស្ត្រ"}, {Language::English, "Science"} };
    m_dictionary["GENRE_HISTORY"] = { {Language::Vietnamese, "Lịch sử"}, {Language::Khmer, "ប្រវត្តិសាស្ត្រ"}, {Language::English, "History"} };
    m_dictionary["GENRE_PHILOSOPHY"] = { {Language::Vietnamese, "Triết học"}, {Language::Khmer, "ទស្សនវិជ្ជា"}, {Language::English, "Philosophy"} };
    m_dictionary["GENRE_PSYCHOLOGY"] = { {Language::Vietnamese, "Tâm lý học"}, {Language::Khmer, "ចិត្តវិទ្យា"}, {Language::English, "Psychology"} };
    m_dictionary["GENRE_ECONOMICS"] = { {Language::Vietnamese, "Kinh tế"}, {Language::Khmer, "សេដ្ឋកិច្ច"}, {Language::English, "Economics"} };
    m_dictionary["GENRE_TECHNOLOGY"] = { {Language::Vietnamese, "Công nghệ"}, {Language::Khmer, "បច្ចេកវិទ្យា"}, {Language::English, "Technology"} };
    m_dictionary["GENRE_EDUCATION"] = { {Language::Vietnamese, "Giáo dục"}, {Language::Khmer, "ការអប់រំ"}, {Language::English, "Education"} };
    m_dictionary["GENRE_MEDICINE"] = { {Language::Vietnamese, "Y học"}, {Language::Khmer, "ឱសថ"}, {Language::English, "Medicine"} };
    m_dictionary["GENRE_ART"] = { {Language::Vietnamese, "Nghệ thuật"}, {Language::Khmer, "សិល្បៈ"}, {Language::English, "Art"} };
    m_dictionary["GENRE_SPORTS"] = { {Language::Vietnamese, "Thể thao"}, {Language::Khmer, "កីឡា"}, {Language::English, "Sports"} };
    m_dictionary["GENRE_TRAVEL"] = { {Language::Vietnamese, "Du lịch"}, {Language::Khmer, "ទេសចរណ៍"}, {Language::English, "Travel"} };
    m_dictionary["GENRE_COOKING"] = { {Language::Vietnamese, "Nấu ăn"}, {Language::Khmer, "ការធ្វើម្ហូប"}, {Language::English, "Cooking"} };
    m_dictionary["GENRE_KIDS"] = { {Language::Vietnamese, "Thiếu nhi"}, {Language::Khmer, "កុមារ"}, {Language::English, "Kids"} };
    m_dictionary["GENRE_OTHER"] = { {Language::Vietnamese, "Khác"}, {Language::Khmer, "ផ្សេងៗ"}, {Language::English, "Other"} };

    m_dictionary["BTN_CANCEL"] = { {Language::Vietnamese, "Hủy"}, {Language::Khmer, "បោះបង់"}, {Language::English, "Cancel"} };
    m_dictionary["BTN_CLEAR_FORM"] = { {Language::Vietnamese, "Xóa Form"}, {Language::Khmer, "សម្អាត"}, {Language::English, "Clear Form"} };
    
    m_dictionary["MSG_ERROR_ISBN_FORMAT"] = { {Language::Vietnamese, "ISBN phải có 10 hoặc 13 chữ số!"}, {Language::Khmer, "ISBN ត្រូវតែមាន ១០ ឬ ១៣ ខ្ទង់!"}, {Language::English, "ISBN must be 10 or 13 digits!"} };
    m_dictionary["MSG_ERROR_ISBN_EXISTS"] = { {Language::Vietnamese, "ISBN này đã tồn tại trong hệ thống!"}, {Language::Khmer, "ISBN នេះមានរួចហើយនៅក្នុងប្រព័ន្ធ!"}, {Language::English, "ISBN already exists in system!"} };
    m_dictionary["MSG_ERROR_MISSING_REQUIRED"] = { {Language::Vietnamese, "Vui lòng điền đầy đủ các trường bắt buộc!"}, {Language::Khmer, "សូមបំពេញវាលចាំបាច់ទាំងអស់!"}, {Language::English, "Please fill all required fields!"} };
    m_dictionary["MSG_ERROR_NAME_SHORT"] = { {Language::Vietnamese, "Tên sách quá ngắn!"}, {Language::Khmer, "ឈ្មោះសៀវភៅខ្លីពេក!"}, {Language::English, "Book title too short!"} };
    m_dictionary["MSG_ERROR_AUTHOR_SHORT"] = { {Language::Vietnamese, "Tên tác giả quá ngắn!"}, {Language::Khmer, "ឈ្មោះអ្នកនិពន្ធខ្លីពេក!"}, {Language::English, "Author name too short!"} };
    
    m_dictionary["MSG_SUCCESS_ADD_BOOK_TITLE"] = { {Language::Vietnamese, "Thêm đầu sách thành công"}, {Language::Khmer, "បន្ថែមសៀវភៅជំនោគជ័យ"}, {Language::English, "Book Added Successfully"} };
    m_dictionary["MSG_SUCCESS_ADD_BOOK_BODY_HTML"] = { 
        {Language::Vietnamese, "✅ Đã thêm thành công đầu sách:\n\n📖 %1\n✍️ Tác giả: %2\n📚 %3 bản sao"}, 
        {Language::Khmer, "✅ បានបន្ថែមសៀវភៅដោយជោគជ័យ:\n\n📖 %1\n✍️ អ្នកនិពន្ធ: %2\n📚 %3 ច្បាប់ចម្លង"}, 
        {Language::English, "✅ Book added successfully:\n\n📖 %1\n✍️ Author: %2\n📚 %3 copies"} 
    };
    
    m_dictionary["MSG_CONFIRM_CLEAR_FORM_TITLE"] = { {Language::Vietnamese, "Xác nhận xóa form"}, {Language::Khmer, "បញ្ជាក់ការសម្អាត"}, {Language::English, "Confirm Clear"} };
    m_dictionary["MSG_CONFIRM_CLEAR_FORM_BODY"] = { {Language::Vietnamese, "Bạn có chắc muốn xóa tất cả dữ liệu đã nhập?"}, {Language::Khmer, "តើអ្នកពិតជារចង់លុបទិន្នន័យដែលបានបញ្ចូលទាំងអស់ឬ?"}, {Language::English, "Are you sure you want to clear all input?"} };
    m_dictionary["MSG_FORM_CLEARED"] = { {Language::Vietnamese, "Form đã được xóa"}, {Language::Khmer, "ទម្រង់ត្រូវបានសម្អាត"}, {Language::English, "Form cleared"} };

    // === DASHBOARD (NEW) ===
    m_dictionary["DASH_WELCOME"] = {
        {Language::Vietnamese, "Xin chào, Quản trị viên 👋"},
        {Language::Khmer, "សួស្តី អ្នកគ្រប់គ្រង 👋"},
        {Language::English, "Welcome, Administrator 👋"}
    };
    m_dictionary["DASH_SUBTITLE"] = {
        {Language::Vietnamese, "Đây là số liệu thống kê mới nhất hôm nay."},
        {Language::Khmer, "នេះគឺជាស្ថិតិចុងក្រោយសម្រាប់ថ្ងៃនេះ។"},
        {Language::English, "Here are the latest statistics for today."}
    };
    m_dictionary["DASH_SEARCH_PLACEHOLDER"] = {
        {Language::Vietnamese, "Tìm kiếm nhanh sách, độc giả..."},
        {Language::Khmer, "ស្វែងរករហ័ស សៀវភៅ, អ្នកអាន..."},
        {Language::English, "Quick search books, readers..."}
    };
    m_dictionary["DASH_RECENT_ACTIVITY"] = {
        {Language::Vietnamese, "Hoạt động gần đây"},
        {Language::Khmer, "សកម្មភាពថ្មីៗ"},
        {Language::English, "Recent Activity"}
    };
    m_dictionary["DASH_COL_TIME"] = {
        {Language::Vietnamese, "Thời gian"},
        {Language::Khmer, "ពេលវេលា"},
        {Language::English, "Time"}
    };
    m_dictionary["DASH_COL_USER"] = {
        {Language::Vietnamese, "Người thực hiện"},
        {Language::Khmer, "អ្នកអនុវត្ត"},
        {Language::English, "Performed By"}
    };
    m_dictionary["DASH_COL_ACTION"] = {
        {Language::Vietnamese, "Hành động"},
        {Language::Khmer, "សកម្មភាព"},
        {Language::English, "Action"}
    };
    m_dictionary["DASH_COL_DETAILS"] = {
        {Language::Vietnamese, "Chi tiết"},
        {Language::Khmer, "ព័ត៌មានលម្អិត"},
        {Language::English, "Details"}
    };
    m_dictionary["DASH_USER_SYSTEM"] = {
        {Language::Vietnamese, "Hệ thống"},
        {Language::Khmer, "ប្រព័ន្ធ"},
        {Language::English, "System"}
    };
    m_dictionary["DASH_ACTION_BORROW"] = {
        {Language::Vietnamese, "Mượn"},
        {Language::Khmer, "ខ្ចី"},
        {Language::English, "Borrow"}
    };
    m_dictionary["DASH_ACTION_RETURN"] = {
        {Language::Vietnamese, "Trả"},
        {Language::Khmer, "សង"},
        {Language::English, "Return"}
    };
    m_dictionary["DASH_ACTION_LOST"] = {
        {Language::Vietnamese, "Mất"},
        {Language::Khmer, "បាត់"},
        {Language::English, "Lost"}
    };
    m_dictionary["DASH_ACTION_ADD"] = {
        {Language::Vietnamese, "Thêm"},
        {Language::Khmer, "បន្ថែម"},
        {Language::English, "Add"}
    };
    m_dictionary["DASH_ACTION_EDIT"] = {
        {Language::Vietnamese, "Sửa"},
        {Language::Khmer, "កែសម្រួល"},
        {Language::English, "Edit"}
    };
    m_dictionary["DASH_ACTION_DELETE"] = {
        {Language::Vietnamese, "Xóa"},
        {Language::Khmer, "លុប"},
        {Language::English, "Delete"}
    };
    m_dictionary["DASH_ACTION_RENEW"] = {
        {Language::Vietnamese, "Gia hạn"},
        {Language::Khmer, "ពន្យារពេល"},
        {Language::English, "Renew"}
    };
    m_dictionary["DASH_ACTION_LOG"] = {
        {Language::Vietnamese, "Nhật ký"},
        {Language::Khmer, "កំណត់ហេតុ"},
        {Language::English, "Log"}
    };
    m_dictionary["DASH_STATUS_BORROWING"] = {
        {Language::Vietnamese, "Đang Mượn"},
        {Language::Khmer, "កំពុងខ្ចី"},
        {Language::English, "Borrowing"}
    };
    m_dictionary["DASH_STATUS_RETURNED"] = {
        {Language::Vietnamese, "Đã Trả"},
        {Language::Khmer, "បានសង"},
        {Language::English, "Returned"}
    };
    m_dictionary["DASH_STATUS_LOST"] = {
        {Language::Vietnamese, "Mất"},
        {Language::Khmer, "បាត់"},
        {Language::English, "Lost"}
    };
    m_dictionary["STATUS_LOST"] = {
        {Language::Vietnamese, "Báo Mất"},
        {Language::Khmer, "រាយការណ៍ថាបាត់"},
        {Language::English, "Reported Lost"}
    };
    m_dictionary["REASON_PAY_FINE"] = {
        {Language::Vietnamese, "Nộp tiền phạt cho sách: %1"},
        {Language::Khmer, "បង់ប្រាក់ពិន័យសម្រាប់សៀវភៅ: %1"},
        {Language::English, "Pay fine for book: %1"}
    };
    m_dictionary["DASH_READER_PREFIX"] = {
        {Language::Vietnamese, "Học viên: "},
        {Language::Khmer, "សិស្ស: "},
        {Language::English, "Reader: "}
    };
    m_dictionary["DASH_PANEL_ALERTS"] = {
        {Language::Vietnamese, "Cảnh báo & Tác vụ"},
        {Language::Khmer, "ការព្រមាន & ភារកិច្ច"},
        {Language::English, "Alerts & Tasks"}
    };
    m_dictionary["DASH_TASK_PRINT"] = {
        {Language::Vietnamese, "In Ấn Thư Viện"},
        {Language::Khmer, "ការបោះពុម្ពបណ្ណាល័យ"},
        {Language::English, "Library Printing"}
    };
    m_dictionary["DASH_DESC_PRINT"] = {
        {Language::Vietnamese, "In thẻ, tem, báo cáo"},
        {Language::Khmer, "បោះពុម្ពប័ណ្ណ តែម របាយការណ៍"},
        {Language::English, "Print cards, stamps, reports"}
    };
    m_dictionary["DASH_TASK_SCAN"] = {
        {Language::Vietnamese, "Quét mã vạch"},
        {Language::Khmer, "ស្កេនបាកូដ"},
        {Language::English, "Scan Barcode"}
    };
    m_dictionary["DASH_DESC_SCAN"] = {
        {Language::Vietnamese, "Quét sách/học viên"},
        {Language::Khmer, "ស្កេនសៀវភៅ/សិស្ស"},
        {Language::English, "Scan books/readers"}
    };
    m_dictionary["DASH_TASK_BACKUP"] = {
        {Language::Vietnamese, "Sao lưu dữ liệu"},
        {Language::Khmer, "បម្រុងទុកទិន្នន័យ"},
        {Language::English, "Backup Data"}
    };
    m_dictionary["DASH_DESC_BACKUP"] = {
        {Language::Vietnamese, "Backup CSDL an toàn"},
        {Language::Khmer, "បម្រុងទុកទិន្នន័យដោយសុវត្ថិភាព"},
        {Language::English, "Safe database backup"}
    };
    m_dictionary["DASH_ALERT_CARDS"] = {
        {Language::Vietnamese, "Cảnh báo Thẻ"},
        {Language::Khmer, "ការព្រមានប័ណ្ណ"},
        {Language::English, "Card Alerts"}
    };
    m_dictionary["DASH_ALERT_EXPIRED"] = {
        {Language::Vietnamese, "Hết hạn"},
        {Language::Khmer, "ហួសកំណត់"},
        {Language::English, "Expired"}
    };
    m_dictionary["DASH_ALERT_NEAR_EXPIRY"] = {
        {Language::Vietnamese, "Sắp hết"},
        {Language::Khmer, "ជិតហួសកំណត់"},
        {Language::English, "Near Expiry"}
    };
    m_dictionary["DASH_ALERT_OVERDUE"] = {
        {Language::Vietnamese, "Sách quá hạn"},
        {Language::Khmer, "សៀវភៅហួសកំណត់"},
        {Language::English, "Overdue Books"}
    };
    m_dictionary["DASH_ALERT_LOW_STOCK"] = {
        {Language::Vietnamese, "Sách sắp hết"},
        {Language::Khmer, "សៀវភៅជិតអស់"},
        {Language::English, "Low Stock Books"}
    };
    m_dictionary["DASH_DESC_LOW_STOCK"] = {
        {Language::Vietnamese, "SL khả dụng <= 2"},
        {Language::Khmer, "ចំនួនដែលអាចប្រើបាន <= 2"},
        {Language::English, "Available copies <= 2"}
    };
    m_dictionary["DASH_CHART_BOOKS_RATIO"] = {
        {Language::Vietnamese, "Tỉ lệ sách"},
        {Language::Khmer, "សមាមាត្រសៀវភៅ"},
        {Language::English, "Books Ratio"}
    };
    m_dictionary["DASH_CHART_AVAILABLE"] = {
        {Language::Vietnamese, "Sẵn có"},
        {Language::Khmer, "មាន"},
        {Language::English, "Available"}
    };
    m_dictionary["DASH_CHART_BORROWED"] = {
        {Language::Vietnamese, "Đang mượn"},
        {Language::Khmer, "កំពុងខ្ចី"},
        {Language::English, "Borrowed"}
    };
    m_dictionary["DASH_CHART_TREND"] = {
        {Language::Vietnamese, "Xu hướng mượn sách (7 ngày qua)"},
        {Language::Khmer, "និន្នាការខ្ចីសៀវភៅ (៧ ថ្ងៃចុងក្រោយ)"},
        {Language::English, "Book Loan Trend (Last 7 Days)"}
    };
    m_dictionary["DASH_CHART_LOANS"] = {
        {Language::Vietnamese, "Lượt mượn"},
        {Language::Khmer, "ចំនួនការខ្ចី"},
        {Language::English, "Loans"}
    };
    m_dictionary["DASH_SEARCH_RESULTS_TITLE"] = {
        {Language::Vietnamese, "Kết Quả Tìm Kiếm"},
        {Language::Khmer, "លទ្ធផលស្វែងរក"},
        {Language::English, "Search Results"}
    };
    m_dictionary["DASH_SEARCH_NO_RESULTS"] = {
        {Language::Vietnamese, "Không tìm thấy kết quả nào cho: "},
        {Language::Khmer, "រកមិនឃើញលទ្ធផលសម្រាប់: "},
        {Language::English, "No results found for: "}
    };
    m_dictionary["DASH_SEARCH_FOUND_COUNT"] = {
        {Language::Vietnamese, "Tìm thấy %1 kết quả cho '%2'"},
        {Language::Khmer, "រកឃើញ %1 លទ្ធផលសម្រាប់ '%2'"},
        {Language::English, "Found %1 results for '%2'"}
    };

    // === LOAN / RETURN MODULE EXTRA ===
    m_dictionary["LBL_CODE"] = { {Language::Vietnamese, "Mã:"}, {Language::Khmer, "កូដ:"}, {Language::English, "Code:"} };
    m_dictionary["LBL_NAME"] = { {Language::Vietnamese, "Tên:"}, {Language::Khmer, "ឈ្មោះ:"}, {Language::English, "Name:"} };
    m_dictionary["LBL_GENDER"] = { {Language::Vietnamese, "Giới Tính:"}, {Language::Khmer, "ភេទ:"}, {Language::English, "Gender:"} };
    m_dictionary["LBL_STATUS_SHORT"] = { {Language::Vietnamese, "TT:"}, {Language::Khmer, "ស្ថានភាព:"}, {Language::English, "Sts:"} };
    m_dictionary["LBL_BORROWING"] = { {Language::Vietnamese, "Đang Mượn:"}, {Language::Khmer, "កំពុងខ្ចី:"}, {Language::English, "Borrowing:"} };
    m_dictionary["LBL_DEBT"] = { {Language::Vietnamese, "(Nợ: %1 đ)"}, {Language::Khmer, "(បំណុល: %1 đ)"}, {Language::English, "(Debt: %1 đ)"} };
    m_dictionary["BOOK_UNIT"] = { {Language::Vietnamese, "cuốn"}, {Language::Khmer, "ក្បាល"}, {Language::English, "books"} };
    
    m_dictionary["STATUS_ACTIVE"] = { {Language::Vietnamese, "Hoạt động"}, {Language::Khmer, "សកម្ម"}, {Language::English, "Active"} };
    m_dictionary["STATUS_LOCKED"] = { {Language::Vietnamese, "Bị Khóa"}, {Language::Khmer, "ត្រូវបានបិទ"}, {Language::English, "Locked"} };
    m_dictionary["GENDER_MALE"] = { {Language::Vietnamese, "Nam"}, {Language::Khmer, "ប្រុស"}, {Language::English, "Male"} };
    m_dictionary["GENDER_FEMALE"] = { {Language::Vietnamese, "Nữ"}, {Language::Khmer, "ស្រី"}, {Language::English, "Female"} };

    m_dictionary["MSG_NO_READER_SELECTED"] = { {Language::Vietnamese, "<i>(Chưa chọn học viên)</i>"}, {Language::Khmer, "<i>(មិនទាន់ជ្រើសរើសសិស្ស)</i>"}, {Language::English, "<i>(No reader selected)</i>"} };
    m_dictionary["MSG_SELECT_READER_FIRST"] = { {Language::Vietnamese, "Vui lòng chọn học viên trước!"}, {Language::Khmer, "សូមជ្រើសរើសសិស្សជាមុនសិន!"}, {Language::English, "Please select a reader first!"} };
    m_dictionary["MSG_PLEASE_SELECT_RETURN_BOOK"] = { {Language::Vietnamese, "Vui lòng chọn sách cần trả!"}, {Language::Khmer, "សូមជ្រើសរើសសៀវភៅដែលត្រូវសង!"}, {Language::English, "Please select a book to return!"} };
    m_dictionary["MSG_INVALID_SELECTION"] = { {Language::Vietnamese, "Dữ liệu chọn không hợp lệ!"}, {Language::Khmer, "ទិន្នន័យដែលបានជ្រើសរើសមិនត្រឹមត្រូវ!"}, {Language::English, "Invalid selection data!"} };
    
    m_dictionary["TITLE_NOT_FOUND"] = { {Language::Vietnamese, "Không Tìm Thấy"}, {Language::Khmer, "រកមិនឃើញ"}, {Language::English, "Not Found"} };
    m_dictionary["MSG_LOAN_INFO_NOT_FOUND"] = { {Language::Vietnamese, "Không tìm thấy thông tin mượn cho: %1\n\n- Nếu nhập Mã Sách: Sách chưa được mượn hoặc mã sai.\n- Nếu nhập ISBN: Có thể không có ai mượn hoặc có nhiều người cùng mượn (vui lòng nhập mã cụ thể)."}, {Language::Khmer, "រកមិនឃើញព័ត៌មានខ្ចីសម្រាប់: %1"}, {Language::English, "Loan info not found for: %1"} };
    m_dictionary["TITLE_LOGIC_ERROR"] = { {Language::Vietnamese, "Lỗi Logic"}, {Language::Khmer, "កំហុសតក្កវិជ្ជា"}, {Language::English, "Logic Error"} };
    m_dictionary["MSG_LOGIC_ERROR_CONTENT"] = { {Language::Vietnamese, "Tìm thấy học viên %1 nhưng không tìm thấy sách %2 trong danh sách mượn (Lỗi Đồng Bộ?)"}, {Language::Khmer, "រកឃើញសិស្ស %1 ប៉ុន្តែរកមិនឃើញសៀវភៅ %2"}, {Language::English, "Found reader %1 but not book %2 in loan list"} };
    
    m_dictionary["TITLE_CONFIRM_RETURN"] = { {Language::Vietnamese, "Xác Nhận Trả Sách"}, {Language::Khmer, "បញ្ជាក់ការសងសៀវភៅ"}, {Language::English, "Confirm Return"} };
    m_dictionary["MSG_CONFIRM_RETURN_BODY"] = { {Language::Vietnamese, "Xác nhận trả sách:\n\nMã: %1\nTên: %2%3"}, {Language::Khmer, "បញ្ជាក់ការសងសៀវភៅ:\n\nកូដ: %1\nឈ្មោះ: %2%3"}, {Language::English, "Confirm return:\n\nCode: %1\nTitle: %2%3"} };
    
    m_dictionary["MSG_OVERDUE_WARNING"] = { {Language::Vietnamese, "\n\n⚠️ QUÁ HẠN %1 NGÀY!\nTiền phạt dự kiến: %2 đ"}, {Language::Khmer, "\n\n⚠️ ហួសកំណត់ %1 ថ្ងៃ!\nប្រាក់ពិន័យប៉ាន់ស្មាន: %2 đ"}, {Language::English, "\n\n⚠️ OVERDUE %1 DAYS!\nEstimated fine: %2 đ"} };
    m_dictionary["MSG_OVERDUE_DAYS"] = { {Language::Vietnamese, "Quá hạn %1 ngày"}, {Language::Khmer, "ហួសកំណត់ %1 ថ្ងៃ"}, {Language::English, "Overdue %1 days"} };
    
    m_dictionary["MSG_RETURN_PAY_SUCCESS"] = { {Language::Vietnamese, "Đã trả sách và thanh toán tiền phạt."}, {Language::Khmer, "បានសងសៀវភៅនិងបង់ប្រាក់ពិន័យ។"}, {Language::English, "Book returned and fine paid."} };
    m_dictionary["MSG_RETURN_DEBT_RECORDED"] = { {Language::Vietnamese, "Đã trả sách. Ghi nợ tiền phạt."}, {Language::Khmer, "បានសងសៀវភៅ។ កត់ត្រាប្រាក់ពិន័យ។"}, {Language::English, "Book returned. Fine recorded as debt."} };
    m_dictionary["MSG_RETURN_SUCCESS"] = { {Language::Vietnamese, "Đã trả sách thành công!"}, {Language::Khmer, "បានសងសៀវភៅដោយជោគជ័យ!"}, {Language::English, "Book returned successfully!"} };
    m_dictionary["MSG_BORROW_SUCCESS"] = { {Language::Vietnamese, "Đã mượn sách thành công!"}, {Language::Khmer, "បានខ្ចីសៀវភៅដោយជោគជ័យ!"}, {Language::English, "Book borrowed successfully!"} };

    m_dictionary["DASH_SEARCH_TYPE_BOOK"] = {
        {Language::Vietnamese, "Sách"},
        {Language::Khmer, "សៀវភៅ"},
        {Language::English, "Book"}
    };
    m_dictionary["DASH_SEARCH_TYPE_READER"] = {
        {Language::Vietnamese, "Học Viên"},
        {Language::Khmer, "សិស្ស"},
        {Language::English, "Reader"}
    };
    m_dictionary["DASH_SEARCH_HINT"] = {
        {Language::Vietnamese, "💡 Nhấp đúp để chọn và đi tới"},
        {Language::Khmer, "💡 ចុចពីរដងដើម្បីជ្រើសរើស និងទៅកាន់"},
        {Language::English, "💡 Double-click to select and go to"}
    };
    m_dictionary["DASH_NAV_TITLE"] = {
        {Language::Vietnamese, "Điều Hướng"},
        {Language::Khmer, "ការរុករក"},
        {Language::English, "Navigation"}
    };
    m_dictionary["DASH_NAV_COPY_ISBN"] = {
        {Language::Vietnamese, "Đã copy ISBN <b>%1</b>.<br>Vui lòng Paste (Ctrl+V) vào ô tìm kiếm sách."},
        {Language::Khmer, "បានចម្លង ISBN <b>%1</b>។<br>សូមបិទភ្ជាប់ (Ctrl+V) ក្នុងប្រអប់ស្វែងរកសៀវភៅ។"},
        {Language::English, "Copied ISBN <b>%1</b>.<br>Please Paste (Ctrl+V) into the book search box."}
    };
    m_dictionary["DASH_NAV_COPY_READER"] = {
        {Language::Vietnamese, "Đã copy Mã Thẻ <b>%1</b>.<br>Vui lòng Paste (Ctrl+V) vào ô tìm kiếm học viên."},
        {Language::Khmer, "បានចម្លងលេខកូដប័ណ្ណ <b>%1</b>។<br>សូមបិទភ្ជាប់ (Ctrl+V) ក្នុងប្រអប់ស្វែងរកសិស្ស។"},
        {Language::English, "Copied Reader ID <b>%1</b>.<br>Please Paste (Ctrl+V) into the reader search box."}
    };
    m_dictionary["TITLE_INPUT_CARD"] = {
        {Language::Vietnamese, "Nhập Mã Thẻ Học Viên"},
        {Language::Khmer, "បញ្ចូលលេខកូដកាតសិស្ស"},
        {Language::English, "Enter Reader Card ID"}
    };
    m_dictionary["LBL_INPUT_CARD_PRINT"] = {
        {Language::Vietnamese, "Mã thẻ học viên cần in:"},
        {Language::Khmer, "លេខកូដកាតសិស្សដែលត្រូវបោះពុម្ព:"},
        {Language::English, "Reader Card ID to print:"}
    };
    m_dictionary["TITLE_CONFIRM_BULK_PRINT"] = {
        {Language::Vietnamese, "Xác Nhận In Hàng Loạt"},
        {Language::Khmer, "បញ្ជាក់ការបោះពុម្ពច្រើន"},
        {Language::English, "Confirm Bulk Print"}
    };
    m_dictionary["MSG_CONFIRM_BULK_PRINT_BODY"] = {
        {Language::Vietnamese, "In thẻ cho TẤT CẢ học viên trong hệ thống?\n\nQuá trình này có thể mất vài phút."},
        {Language::Khmer, "បោះពុម្ពកាតសម្រាប់សិស្សទាំងអស់ក្នុងប្រព័ន្ធ?\n\nដំណើរការនេះអាចចំណាយពេលពីរបីនាទី។"},
        {Language::English, "Print cards for ALL readers in the system?\n\nThis process may take a few minutes."}
    };
    m_dictionary["TITLE_INPUT_ISBN"] = {
        {Language::Vietnamese, "Nhập ISBN Sách"},
        {Language::Khmer, "បញ្ចូល ISBN សៀវភៅ"},
        {Language::English, "Enter Book ISBN"}
    };
    m_dictionary["LBL_INPUT_ISBN_PRINT"] = {
        {Language::Vietnamese, "ISBN của sách cần in tem:"},
        {Language::Khmer, "ISBN នៃសៀវភៅដែលត្រូវបោះពុម្ពតែម:"},
        {Language::English, "ISBN of book to print stamps:"}
    };
    m_dictionary["LBL_INPUT_ISBN_PRINT_COUNT"] = {
        {Language::Vietnamese, "ISBN của sách cần in %1 tem:"},
        {Language::Khmer, "ISBN នៃសៀវភៅដែលត្រូវបោះពុម្ព %1 តែម:"},
        {Language::English, "ISBN of book to print %1 stamps:"}
    };
    m_dictionary["TITLE_INPUT_TEM_COUNT"] = {
        {Language::Vietnamese, "Nhập Số Lượng Tem"},
        {Language::Khmer, "បញ្ចូលចំនួនតែម"},
        {Language::English, "Enter Stamp Count"}
    };
    m_dictionary["LBL_INPUT_TEM_COUNT"] = {
        {Language::Vietnamese, "Số lượng tem cần in:"},
        {Language::Khmer, "ចំនួនតែមដែលត្រូវបោះពុម្ព:"},
        {Language::English, "Number of stamps to print:"}
    };
    m_dictionary["MSG_NO_READER_SYSTEM"] = {
        {Language::Vietnamese, "Không có học viên nào trong hệ thống!"},
        {Language::Khmer, "មិនមានសិស្សនៅក្នុងប្រព័ន្ធទេ!"},
        {Language::English, "No readers in the system!"}
    };
    m_dictionary["MSG_READER_NOT_FOUND"] = {
        {Language::Vietnamese, "Không tìm thấy học viên với mã thẻ: %1"},
        {Language::Khmer, "រកមិនឃើញសិស្សដែលមានលេខកូដកាត: %1"},
        {Language::English, "Reader not found with card ID: %1"}
    };
    m_dictionary["MSG_BOOK_NOT_FOUND_ISBN"] = {
        {Language::Vietnamese, "Không tìm thấy sách với ISBN: %1"},
        {Language::Khmer, "រកមិនឃើញសៀវភៅដែលមាន ISBN: %1"},
        {Language::English, "Book not found with ISBN: %1"}
    };
    m_dictionary["MSG_NO_BOOK_SYSTEM"] = {
        {Language::Vietnamese, "Không có sách nào trong hệ thống!"},
        {Language::Khmer, "មិនមានសៀវភៅនៅក្នុងប្រព័ន្ធទេ!"},
        {Language::English, "No books in the system!"}
    };
    m_dictionary["LBL_INPUT_CARD_LOAN"] = {
        {Language::Vietnamese, "Mã thẻ học viên có sách đang mượn:"},
        {Language::Khmer, "លេខកូដកាតសិស្សដែលមានសៀវភៅកំពុងខ្ចី:"},
        {Language::English, "Card ID of reader having borrowed books:"}
    };
    m_dictionary["LBL_INPUT_CARD_RETURN"] = {
        {Language::Vietnamese, "Mã thẻ học viên vừa trả sách:"},
        {Language::Khmer, "លេខកូដកាតសិស្សដែលទើបតែសងសៀវភៅ:"},
        {Language::English, "Card ID of reader who just returned books:"}
    };
    m_dictionary["MSG_READER_NO_LOAN"] = {
        {Language::Vietnamese, "Học viên này chưa mượn sách nào!"},
        {Language::Khmer, "សិស្សនេះមិនទាន់បានខ្ចីសៀវភៅទេ!"},
        {Language::English, "This reader hasn't borrowed any books!"}
    };
    m_dictionary["MSG_RETURN_SUCCESS_PRINT"] = {
        {Language::Vietnamese, "Đã xác nhận trả sách cho học viên: %1 %2\nIn biên lai?"},
        {Language::Khmer, "បានបញ្ជាក់ការសងសៀវភៅសម្រាប់សិស្ស: %1 %2\nបោះពុម្ពបង្កាន់ដៃ?"},
        {Language::English, "Confirmed return for reader: %1 %2\nPrint receipt?"}
    };
    m_dictionary["LBL_CONFIG_BARCODE_TITLE"] = {
        {Language::Vietnamese, "CÀI ĐẶT MÃ VẠCH"},
        {Language::Khmer, "ការកំណត់បាកូដ"},
        {Language::English, "BARCODE SETTINGS"}
    };
    m_dictionary["LBL_BARCODE_PRESETS"] = {
        {Language::Vietnamese, "Khổ giấy mẫu (Presets)"},
        {Language::Khmer, "ទម្រង់ក្រដាសគំរូ (Presets)"},
        {Language::English, "Paper Presets"}
    };
    m_dictionary["LBL_BARCODE_WIDTH"] = {
        {Language::Vietnamese, "Chiều rộng tem (mm):"},
        {Language::Khmer, "ទទឹងតែម (mm):"},
        {Language::English, "Label Width (mm):"}
    };
    m_dictionary["LBL_BARCODE_HEIGHT"] = {
        {Language::Vietnamese, "Chiều cao tem (mm):"},
        {Language::Khmer, "កម្ពស់តែម (mm):"},
        {Language::English, "Label Height (mm):"}
    };
    m_dictionary["LBL_BARCODE_COLS"] = {
        {Language::Vietnamese, "Số cột (tem/hàng):"},
        {Language::Khmer, "ចំនួនជួរឈរ (តែម/ជួរ):"},
        {Language::English, "Columns (labels/row):"}
    };
    m_dictionary["CB_SHOW_LIB_NAME"] = {
        {Language::Vietnamese, "Hiện tên thư viện"},
        {Language::Khmer, "បង្ហាញឈ្មោះបណ្ណាល័យ"},
        {Language::English, "Show Library Name"}
    };
    m_dictionary["CB_SHOW_BOOK_TITLE"] = {
        {Language::Vietnamese, "Hiện tên sách"},
        {Language::Khmer, "បង្ហាញចំណងជើងសៀវភៅ"},
        {Language::English, "Show Book Title"}
    };
    m_dictionary["CB_SHOW_PRICE"] = {
        {Language::Vietnamese, "Hiện giá tiền"},
        {Language::Khmer, "បង្ហាញតម្លៃ"},
        {Language::English, "Show Price"}
    };
    m_dictionary["CB_SHOW_BORDER"] = {
        {Language::Vietnamese, "Hiện viền cắt"},
        {Language::Khmer, "បង្ហាញស៊ុមសម្រាប់កាត់"},
        {Language::English, "Show Cut Border"}
    };
    m_dictionary["LBL_LIVE_PREVIEW"] = {
        {Language::Vietnamese, "Xem Trước (Live Preview)"},
        {Language::Khmer, "មើលជាមុន (Live Preview)"},
        {Language::English, "Live Preview"}
    };
    m_dictionary["PRESET_CUSTOM"] = {
        {Language::Vietnamese, "Tùy chỉnh (Custom)"},
        {Language::Khmer, "តាមបំណង (Custom)"},
        {Language::English, "Custom"}
    };
    m_dictionary["PRESET_A4_3COL"] = {
        {Language::Vietnamese, "A4 - 3 Cột (Tiêu chuẩn)"},
        {Language::Khmer, "A4 - 3 ជួរឈរ (ស្តង់ដារ)"},
        {Language::English, "A4 - 3 Cols (Standard)"}
    };
    m_dictionary["PRESET_A4_4COL"] = {
        {Language::Vietnamese, "A4 - 4 Cột (Nhỏ)"},
        {Language::Khmer, "A4 - 4 ជួរឈរ (តូច)"},
        {Language::English, "A4 - 4 Cols (Small)"}
    };
    m_dictionary["PRESET_SINGLE"] = {
        {Language::Vietnamese, "Tem đơn (Máy in tem)"},
        {Language::Khmer, "តែមតែមួយ (ម៉ាស៊ីនបោះពុម្ពតែម)"},
        {Language::English, "Single Label (Label Printer)"}
    };
    m_dictionary["TITLE_EXPORT_LOG"] = {
        {Language::Vietnamese, "Xuất Báo Cáo Hệ Thống"},
        {Language::Khmer, "នាំចេញរបាយការណ៍ប្រព័ន្ធ"},
        {Language::English, "Export System Report"}
    };
    m_dictionary["MSG_EXPORT_SUCCESS"] = {
        {Language::Vietnamese, "Đã xuất báo cáo thành công!"},
        {Language::Khmer, "បាននាំចេញរបាយការណ៍ដោយជោគជ័យ!"},
        {Language::English, "Report exported successfully!"}
    };

    m_dictionary["TABLE_HEADER_PUBLISHER"] = { {Language::Vietnamese, "Nhà Xuất Bản"}, {Language::Khmer, "អ្នកបោះពុម្ព"}, {Language::English, "Publisher"} };
    // === READER MODULE ===
    m_dictionary["TABLE_HEADER_STT"] = { {Language::Vietnamese, "STT"}, {Language::Khmer, "ល.រ"}, {Language::English, "No."} };
    m_dictionary["TABLE_HEADER_BORROWED"] = { {Language::Vietnamese, "Sách Đang Mượn"}, {Language::Khmer, "សៀវភៅកំពុងខ្ចី"}, {Language::English, "Borrowed Books"} };
    m_dictionary["TABLE_HEADER_OVERDUE"] = { {Language::Vietnamese, "Quá Hạn"}, {Language::Khmer, "ហួសកំណត់"}, {Language::English, "Overdue"} };
    m_dictionary["TABLE_HEADER_TOTAL_LOAN"] = { {Language::Vietnamese, "Tổng Mượn"}, {Language::Khmer, "សរុបការខ្ចី"}, {Language::English, "Total Loans"} };
    m_dictionary["TABLE_HEADER_CODE"] = { {Language::Vietnamese, "Mã Thẻ"}, {Language::Khmer, "លេខកូដ"}, {Language::English, "Card ID"} };
    m_dictionary["TABLE_HEADER_LASTNAME"] = { {Language::Vietnamese, "Họ Đệm"}, {Language::Khmer, "នាមត្រកូល"}, {Language::English, "Last Name"} };
    m_dictionary["TABLE_HEADER_FIRSTNAME"] = { {Language::Vietnamese, "Tên"}, {Language::Khmer, "ឈ្មោះ"}, {Language::English, "First Name"} };
    m_dictionary["TABLE_HEADER_CLASS"] = { {Language::Vietnamese, "Lớp"}, {Language::Khmer, "ថ្នាក់"}, {Language::English, "Class"} };
    m_dictionary["TABLE_HEADER_PHONE"] = { {Language::Vietnamese, "SĐT"}, {Language::Khmer, "ទូរស័ព្ទ"}, {Language::English, "Phone"} };
    m_dictionary["TABLE_HEADER_STATUS"] = { {Language::Vietnamese, "Trạng Thái"}, {Language::Khmer, "ស្ថានភាព"}, {Language::English, "Status"} };
    m_dictionary["TABLE_HEADER_GENDER"] = { {Language::Vietnamese, "Giới Tính"}, {Language::Khmer, "ភេទ"}, {Language::English, "Gender"} };
    m_dictionary["TABLE_HEADER_BOOKS"] = { {Language::Vietnamese, "Sách"}, {Language::Khmer, "សៀវភៅ"}, {Language::English, "Books"} };
    m_dictionary["TABLE_HEADER_BOOK_CODE"] = { {Language::Vietnamese, "Mã Sách"}, {Language::Khmer, "លេខកូដសៀវភៅ"}, {Language::English, "Book Code"} };
    m_dictionary["TABLE_HEADER_DATE_BORROW"] = { {Language::Vietnamese, "Ngày Mượn"}, {Language::Khmer, "ថ្ងៃខ្ចី"}, {Language::English, "Borrow Date"} };
    m_dictionary["GENDER_MALE"] = { {Language::Vietnamese, "Nam"}, {Language::Khmer, "ប្រុស"}, {Language::English, "Male"} };
    m_dictionary["GENDER_FEMALE"] = { {Language::Vietnamese, "Nữ"}, {Language::Khmer, "ស្រី"}, {Language::English, "Female"} };

    m_dictionary["LBL_DAYS_COUNT"] = { {Language::Vietnamese, "%1 ngày"}, {Language::Khmer, "%1 ថ្ងៃ"}, {Language::English, "%1 days"} };
    m_dictionary["LBL_HINT_READER_DBLCLICK"] = { {Language::Vietnamese, "Mẹo: Nhấp đúp vào học viên để xem chi tiết & in thẻ"}, {Language::Khmer, "គន្លឹះ: ចុចពីរដងលើសិស្សដើម្បីមើលព័ត៌មានលម្អិត និងបោះពុម្ពកាត"}, {Language::English, "Tip: Double-click reader to view details & print card"} };

    m_dictionary["BTN_ADD_READER"] = { {Language::Vietnamese, "Thêm Học Viên"}, {Language::Khmer, "បន្ថែមសិស្ស"}, {Language::English, "Add Reader"} };
    m_dictionary["BTN_IMPORT_EXCEL"] = { {Language::Vietnamese, "Nhập Excel"}, {Language::Khmer, "នាំចូល Excel"}, {Language::English, "Import Excel"} };
    m_dictionary["BTN_DETAILS"] = { {Language::Vietnamese, "Chi Tiết"}, {Language::Khmer, "លម្អិត"}, {Language::English, "Details"} };
    m_dictionary["BTN_LOCK_CARD"] = { {Language::Vietnamese, "Khóa Thẻ"}, {Language::Khmer, "បិទកាត"}, {Language::English, "Lock Card"} };
    m_dictionary["BTN_UNLOCK_CARD"] = { {Language::Vietnamese, "Mở Khóa"}, {Language::Khmer, "បើកកាត"}, {Language::English, "Unlock Card"} };
    m_dictionary["BTN_HISTORY"] = { {Language::Vietnamese, "Lịch Sử"}, {Language::Khmer, "ប្រវត្តិ"}, {Language::English, "History"} };
    
    m_dictionary["CTX_VIEW_DETAILS"] = { {Language::Vietnamese, "Xem Chi Tiết"}, {Language::Khmer, "មើលលម្អិត"}, {Language::English, "View Details"} };
    m_dictionary["CTX_EDIT_READER"] = { {Language::Vietnamese, "Sửa Học Viên"}, {Language::Khmer, "កែសម្រួលសិស្ស"}, {Language::English, "Edit Reader"} };
    m_dictionary["CTX_PRINT_CARD"] = { {Language::Vietnamese, "In Thẻ TV"}, {Language::Khmer, "បោះពុម្ពកាត"}, {Language::English, "Print Card"} };
    m_dictionary["CTX_DELETE_READER"] = { {Language::Vietnamese, "Xóa Học Viên"}, {Language::Khmer, "លុបសិស្ស"}, {Language::English, "Delete Reader"} };

    m_dictionary["PLACEHOLDER_SEARCH_READER_FULL"] = { {Language::Vietnamese, "Nhập mã thẻ, họ, tên..."}, {Language::Khmer, "បញ្ចូលលេខកូដកាត នាមត្រកូល ឈ្មោះ..."}, {Language::English, "Enter ID, Last Name, First Name..."} };
    m_dictionary["FILTER_ALL_GENDER"] = { {Language::Vietnamese, "Tất cả giới tính"}, {Language::Khmer, "ភេទទាំងអស់"}, {Language::English, "All Genders"} };
    
    m_dictionary["MSG_CANNOT_DELETE"] = { {Language::Vietnamese, "Không Thể Xóa"}, {Language::Khmer, "មិនអាចលុបបាន"}, {Language::English, "Cannot Delete"} };
    m_dictionary["MSG_CANNOT_DELETE_HAS_BOOKS"] = { {Language::Vietnamese, "Không thể xóa học viên này!\n\nHọc viên đang mượn %1 cuốn sách.\nVui lòng trả hết sách trước khi xóa."}, {Language::Khmer, "មិនអាចលុបសិស្សនេះបានទេ!\n\nសិស្សកំពុងខ្ចីសៀវភៅ %1 ក្បាល។\nសូមសងសៀវភៅទាំងអស់មុនពេលលុប។"}, {Language::English, "Cannot delete this reader!\n\nReader is borrowing %1 books.\nPlease return all books before deleting."} };
    m_dictionary["MSG_DELETE_CONFIRM_DETAILS"] = { {Language::Vietnamese, "Họ tên: %1 %2\nMã thẻ: %3\nTrạng thái: %4\nGiới tính: %5\n\nSố sách đang mượn: %6"}, {Language::Khmer, "ឈ្មោះ: %1 %2\nលេខកូដ: %3\nស្ថានភាព: %4\nភេទ: %5\n\nសៀវភៅកំពុងខ្ចី: %6"}, {Language::English, "Name: %1 %2\nID: %3\nStatus: %4\nGender: %5\n\nBorrowed Books: %6"} };
    m_dictionary["MSG_ALREADY_LOCKED"] = { {Language::Vietnamese, "Thẻ học viên này đã bị khóa rồi!"}, {Language::Khmer, "កាតសិស្សនេះត្រូវបានបិទហើយ!"}, {Language::English, "This reader card is already locked!"} };
    m_dictionary["TITLE_CONFIRM_LOCK"] = { {Language::Vietnamese, "Xác Nhận Khóa Thẻ"}, {Language::Khmer, "បញ្ជាក់ការបិទកាត"}, {Language::English, "Confirm Lock Card"} };
    m_dictionary["MSG_CONFIRM_LOCK"] = { {Language::Vietnamese, "Bạn có chắc muốn khóa thẻ học viên:\n%1 %2 (Mã: %3)?\n\nHọc viên sẽ không thể mượn sách sau khi bị khóa."}, {Language::Khmer, "តើអ្នកពិតជាចង់បិទកាតសិស្ស:\n%1 %2 (លេខកូដ: %3)?\n\nសិស្សនឹងមិនអាចខ្ចីសៀវភៅបានទេបន្ទាប់ពីបិទ។"}, {Language::English, "Are you sure you want to lock reader card:\n%1 %2 (ID: %3)?\n\nReader cannot borrow books after locking."} };
    m_dictionary["MSG_LOCK_SUCCESS"] = { {Language::Vietnamese, "Đã khóa thẻ học viên!"}, {Language::Khmer, "បានបិទកាតសិស្ស!"}, {Language::English, "Reader card locked!"} };
    m_dictionary["MSG_ALREADY_ACTIVE"] = { {Language::Vietnamese, "Thẻ học viên này đang hoạt động bình thường!"}, {Language::Khmer, "កាតសិស្សនេះកំពុងដំណើរការធម្មតា!"}, {Language::English, "This reader card is active!"} };
    m_dictionary["TITLE_CONFIRM_UNLOCK"] = { {Language::Vietnamese, "Xác Nhận Mở Khóa"}, {Language::Khmer, "បញ្ជាក់ការបើកកាត"}, {Language::English, "Confirm Unlock Card"} };
    m_dictionary["MSG_CONFIRM_UNLOCK"] = { {Language::Vietnamese, "Bạn có chắc muốn mở khóa thẻ học viên:\n%1 %2 (Mã: %3)?"}, {Language::Khmer, "តើអ្នកពិតជាចង់បើកកាតសិស្ស:\n%1 %2 (លេខកូដ: %3)?"}, {Language::English, "Are you sure you want to unlock reader card:\n%1 %2 (ID: %3)?"} };
    m_dictionary["MSG_UNLOCK_SUCCESS"] = { {Language::Vietnamese, "Đã mở khóa thẻ học viên!"}, {Language::Khmer, "បានបើកកាតសិស្ស!"}, {Language::English, "Reader card unlocked!"} };
    
    m_dictionary["TITLE_HISTORY"] = { {Language::Vietnamese, "Lịch Sử"}, {Language::Khmer, "ប្រវត្តិ"}, {Language::English, "History"} };
    m_dictionary["MSG_FEATURE_DEVELOPING"] = { {Language::Vietnamese, "Chức năng xem lịch sử đang được phát triển..."}, {Language::Khmer, "មុខងារនេះកំពុងត្រូវបានអភិវឌ្ឍ..."}, {Language::English, "History feature is under development..."} };
    
    m_dictionary["MSG_READER_BORROWING_INFO"] = { {Language::Vietnamese, "Học viên %1 %2 đang mượn %3 cuốn sách.\n\nChức năng xem chi tiết đang được phát triển..."}, {Language::Khmer, "សិស្ស %1 %2 កំពុងខ្ចីសៀវភៅ %3 ក្បាល។\n\nមុខងារនេះកំពុងត្រូវបានអភិវឌ្ឍ..."}, {Language::English, "Reader %1 %2 is borrowing %3 books.\n\nDetail feature is under development..."} };
    m_dictionary["MSG_SELECTED_READER"] = { {Language::Vietnamese, "Đã chọn: %1 %2 (Mã: %3)"}, {Language::Khmer, "បានជ្រើសរើស: %1 %2 (លេខកូដ: %3)"}, {Language::English, "Selected: %1 %2 (ID: %3)"} };
    m_dictionary["MSG_LOADED_READERS"] = { {Language::Vietnamese, "Đã tải %1 học viên"}, {Language::Khmer, "បានផ្ទុកសិស្ស %1"}, {Language::English, "Loaded %1 readers"} };
    
    m_dictionary["TITLE_SELECT_EXCEL"] = { {Language::Vietnamese, "Chọn file Excel để nhập"}, {Language::Khmer, "ជ្រើសរើសឯកសារ Excel"}, {Language::English, "Select Excel file to import"} };
    m_dictionary["MSG_PROCESSING_EXCEL"] = { {Language::Vietnamese, "Đang xử lý tập tin Excel..."}, {Language::Khmer, "កំពុងដំណើរការឯកសារ Excel..."}, {Language::English, "Processing Excel file..."} };
    m_dictionary["MSG_ADD_READER_SUCCESS"] = { {Language::Vietnamese, "Đã thêm học viên mới thành công!"}, {Language::Khmer, "បានបន្ថែមសិស្សថ្មីដោយជោគជ័យ!"}, {Language::English, "New reader added successfully!"} };
    m_dictionary["MSG_UPDATE_READER_SUCCESS"] = { {Language::Vietnamese, "Đã cập nhật thông tin học viên!"}, {Language::Khmer, "បានធ្វើបច្ចុប្បន្នភាពព័ត៌មានសិស្ស!"}, {Language::English, "Reader info updated!"} };
    m_dictionary["MSG_DELETE_SUCCESS"] = { {Language::Vietnamese, "Đã xóa học viên thành công!"}, {Language::Khmer, "បានលុបសិស្សដោយជោគជ័យ!"}, {Language::English, "Reader deleted successfully!"} };
    m_dictionary["TITLE_WARNING"] = { {Language::Vietnamese, "Cảnh Báo"}, {Language::Khmer, "ការព្រមាន"}, {Language::English, "Warning"} };
    m_dictionary["TITLE_INFO"] = { {Language::Vietnamese, "Thông Báo"}, {Language::Khmer, "ជូនដំណឹង"}, {Language::English, "Information"} };
    m_dictionary["FILTER_ALL"] = { {Language::Vietnamese, "Tất cả"}, {Language::Khmer, "ទាំងអស់"}, {Language::English, "All"} };
    m_dictionary["BTN_SEARCH"] = { {Language::Vietnamese, "Tìm"}, {Language::Khmer, "ស្វែងរក"}, {Language::English, "Search"} };
    m_dictionary["BTN_REFRESH"] = { {Language::Vietnamese, "Làm mới"}, {Language::Khmer, "ផ្ទុកឡើងវិញ"}, {Language::English, "Refresh"} };
    m_dictionary["LBL_STATUS_ACTIVE_LONG"] = { {Language::Vietnamese, "Đang hoạt động"}, {Language::Khmer, "កំពុងដំណើរការ"}, {Language::English, "Active"} };
    m_dictionary["FILTER_EXCEL_FILES"] = { {Language::Vietnamese, "Tập tin Excel (*.xlsx);;Tất cả (*)"}, {Language::Khmer, "ឯកសារ Excel (*.xlsx);;ទាំងអស់ (*)"}, {Language::English, "Excel Files (*.xlsx);;All Files (*)"} };
    // === BOOK MANAGEMENT ===
    m_dictionary["TABLE_HEADER_ISBN"] = { {Language::Vietnamese, "Mã ISBN"}, {Language::Khmer, "ISBN"}, {Language::English, "ISBN"} };
    m_dictionary["TABLE_HEADER_TITLE"] = { {Language::Vietnamese, "Tên Sách"}, {Language::Khmer, "ចំណងជើងសៀវភៅ"}, {Language::English, "Book Title"} };
    m_dictionary["TABLE_HEADER_AUTHOR"] = { {Language::Vietnamese, "Tác Giả"}, {Language::Khmer, "អ្នកនិពន្ធ"}, {Language::English, "Author"} };
    m_dictionary["TABLE_HEADER_CATEGORY"] = { {Language::Vietnamese, "Thể Loại"}, {Language::Khmer, "ប្រភេទ"}, {Language::English, "Category"} };
    m_dictionary["TABLE_HEADER_YEAR"] = { {Language::Vietnamese, "Năm XB"}, {Language::Khmer, "ឆ្នាំ"}, {Language::English, "Year"} };
    m_dictionary["TABLE_HEADER_PAGES"] = { {Language::Vietnamese, "Số Trang"}, {Language::Khmer, "ទំព័រ"}, {Language::English, "Pages"} };
    m_dictionary["TABLE_HEADER_ACTIONS"] = { {Language::Vietnamese, "Thao Tác"}, {Language::Khmer, "សកម្មភាព"}, {Language::English, "Actions"} };
    m_dictionary["TABLE_HEADER_LOCATION"] = { {Language::Vietnamese, "Vị Trí"}, {Language::Khmer, "ទីតាំង"}, {Language::English, "Location"} };
    m_dictionary["TABLE_HEADER_PRICE"] = { {Language::Vietnamese, "Giá Tiền"}, {Language::Khmer, "តម្លៃ"}, {Language::English, "Price"} };

    m_dictionary["BTN_ADD_BOOK"] = { {Language::Vietnamese, "Thêm Sách"}, {Language::Khmer, "បន្ថែមសៀវភៅ"}, {Language::English, "Add Book"} };
    m_dictionary["BTN_IMPORT"] = { {Language::Vietnamese, "Nhập Excel"}, {Language::Khmer, "នាំចូល Excel"}, {Language::English, "Import Excel"} };
    m_dictionary["BTN_EXPORT_PRINT"] = { {Language::Vietnamese, "In Ấn ▾"}, {Language::Khmer, "បោះពុម្ព ▾"}, {Language::English, "Print ▾"} };
    
    m_dictionary["PLACEHOLDER_SEARCH_BOOK"] = { {Language::Vietnamese, "Nhập ISBN, tên sách, tác giả..."}, {Language::Khmer, "បញ្ចូល ISBN ចំណងជើង អ្នកនិពន្ធ..."}, {Language::English, "Enter ISBN, Title, Author..."} };
    
    m_dictionary["FILTER_ALL_CATEGORIES"] = { {Language::Vietnamese, "Tất cả thể loại"}, {Language::Khmer, "ប្រភេទទាំងអស់"}, {Language::English, "All Categories"} };
    m_dictionary["FILTER_ALL_STATUS"] = { {Language::Vietnamese, "Tất cả trạng thái"}, {Language::Khmer, "ស្ថានភាពទាំងអស់"}, {Language::English, "All Statuses"} };
    m_dictionary["STATUS_AVAILABLE"] = { {Language::Vietnamese, "Còn sách"}, {Language::Khmer, "មានសៀវភៅ"}, {Language::English, "Available"} };
    m_dictionary["STATUS_OUT_OF_STOCK"] = { {Language::Vietnamese, "Hết sách"}, {Language::Khmer, "អស់ពីស្តុក"}, {Language::English, "Out of Stock"} };
    
    m_dictionary["LBL_HINT_BOOK_DBLCLICK"] = { {Language::Vietnamese, "Mẹo: Nhấp đúp vào sách để xem chi tiết & in tem"}, {Language::Khmer, "គន្លឹះ: ចុចពីរដងលើសៀវភៅដើម្បីមើលព័ត៌មានលម្អិត"}, {Language::English, "Tip: Double-click book for details & print label"} };
    
    m_dictionary["CTX_EDIT_BOOK"] = { {Language::Vietnamese, "Sửa Sách"}, {Language::Khmer, "កែសម្រួលសៀវភៅ"}, {Language::English, "Edit Book"} };
    m_dictionary["CTX_PRINT_BARCODE"] = { {Language::Vietnamese, "In Tem Mã Vạch"}, {Language::Khmer, "បោះពុម្ពបាកូដ"}, {Language::English, "Print Barcode"} };
    m_dictionary["CTX_DELETE_BOOK"] = { {Language::Vietnamese, "Xóa Sách"}, {Language::Khmer, "លុបសៀវភៅ"}, {Language::English, "Delete Book"} };

    m_dictionary["MSG_SELECT_BOOK_DELETE"] = { {Language::Vietnamese, "Vui lòng chọn một đầu sách để xóa!"}, {Language::Khmer, "សូមជ្រើសរើសសៀវភៅដើម្បីលុប!"}, {Language::English, "Please select a book to delete!"} };
    m_dictionary["MSG_CANNOT_DELETE_BOOK"] = { {Language::Vietnamese, "Không thể xóa"}, {Language::Khmer, "មិនអាចលុបបាន"}, {Language::English, "Cannot Delete"} };
    m_dictionary["MSG_CANNOT_DELETE_BOOK_HAS_COPIES"] = { {Language::Vietnamese, "Không thể xóa đầu sách này!\nVẫn còn bản sao đang được mượn hoặc chưa thanh lý."}, {Language::Khmer, "មិនអាចលុបសៀវភៅនេះបានទេ!\nនៅតែមានច្បាប់ចម្លងដែលកំពុងខ្ចី។"}, {Language::English, "Cannot delete this book!\nCopies are still borrowed or not liquidated."} };
    
    m_dictionary["MSG_DELETE_BOOK_CONFIRM_DETAILS"] = { {Language::Vietnamese, "Tên sách: %1\nTác giả: %2\nISBN: %3\n\nTổng bản sao: %4\nCó thể mượn: %5\nĐang mượn: %6\nĐã thanh lý: %7"}, {Language::Khmer, "ឈ្មោះ: %1\nអ្នកនិពន្ធ: %2\nISBN: %3\n\nសរុប: %4\nអាចខ្ចី: %5\nកំពុងខ្ចី: %6\nដកចេញ: %7"}, {Language::English, "Title: %1\nAuthor: %2\nISBN: %3\n\nTotal: %4\nAvailable: %5\nBorrowed: %6\nLiquidated: %7"} };
    m_dictionary["MSG_DELETE_BOOK_SUCCESS"] = { {Language::Vietnamese, "Đã xóa đầu sách và đồng bộ cơ sở dữ liệu!"}, {Language::Khmer, "បានលុបសៀវភៅនិងធ្វើសមកាលកម្មទិន្នន័យ!"}, {Language::English, "Book deleted and DB synced!"} };
    m_dictionary["MSG_DELETE_BOOK_NOTIFICATION"] = { {Language::Vietnamese, "Đã xóa đầu sách!"}, {Language::Khmer, "បានលុបសៀវភៅ!"}, {Language::English, "Book deleted!"} };
    
    m_dictionary["MSG_SELECT_BOOK_EDIT"] = { {Language::Vietnamese, "Vui lòng chọn một đầu sách để sửa!"}, {Language::Khmer, "សូមជ្រើសរើសសៀវភៅដើម្បីកែសម្រួល!"}, {Language::English, "Please select a book to edit!"} };
    m_dictionary["MSG_UPDATE_BOOK_SUCCESS"] = { {Language::Vietnamese, "Đã cập nhật thông tin sách!"}, {Language::Khmer, "បានធ្វើបច្ចុប្បន្នភាពព័ត៌មានសៀវភៅ!"}, {Language::English, "Book info updated!"} };
    
    m_dictionary["MSG_SELECT_BOOK_ADD_COPY"] = { {Language::Vietnamese, "Vui lòng chọn một đầu sách để thêm bản sao!"}, {Language::Khmer, "សូមជ្រើសរើសសៀវភៅដើម្បីបន្ថែមច្បាប់ចម្លង!"}, {Language::English, "Please select a book to add copy!"} };
    m_dictionary["MSG_ADD_COPY_SUCCESS"] = { {Language::Vietnamese, "Đã thêm bản sao thành công!"}, {Language::Khmer, "បានបន្ថែមច្បាប់ចម្លងដោយជោគជ័យ!"}, {Language::English, "Copy added successfully!"} };
    
    m_dictionary["MSG_SELECT_BOOK_DETAILS"] = { {Language::Vietnamese, "Vui lòng chọn một đầu sách để xem chi tiết!"}, {Language::Khmer, "សូមជ្រើសរើសសៀវភៅដើម្បីមើលព័ត៌មានលម្អិត!"}, {Language::English, "Please select a book for details!"} };
    m_dictionary["MSG_UPDATE_COPY_SUCCESS"] = { {Language::Vietnamese, "Đã cập nhật thông tin bản sao!"}, {Language::Khmer, "បានធ្វើបច្ចុប្បន្នភាពព័ត៌មានច្បាប់ចម្លង!"}, {Language::English, "Copy info updated!"} };
    
    m_dictionary["MSG_LOADED_BOOKS"] = { {Language::Vietnamese, "Đã tải %1 đầu sách"}, {Language::Khmer, "បានផ្ទុកសៀវភៅ %1"}, {Language::English, "Loaded %1 books"} };
    m_dictionary["MSG_SEARCH_RESULT_BOOKS"] = { {Language::Vietnamese, "📋 Tìm thấy <b>%1</b> kết quả"}, {Language::Khmer, "📋 រកឃើញ <b>%1</b> លទ្ធផល"}, {Language::English, "📋 Found <b>%1</b> results"} };
    
    m_dictionary["STATUS_READY"] = { {Language::Vietnamese, "Sẵn sàng"}, {Language::Khmer, "រួចរាល់"}, {Language::English, "Ready"} };
    m_dictionary["STATUS_BORROWED"] = { {Language::Vietnamese, "Đang mượn"}, {Language::Khmer, "កំពុងខ្ចី"}, {Language::English, "Borrowed"} };
    m_dictionary["STATUS_LIQUIDATED"] = { {Language::Vietnamese, "Thanh lý"}, {Language::Khmer, "លក់ចេញ"}, {Language::English, "Liquidated"} };
    m_dictionary["STATUS_UNKNOWN"] = { {Language::Vietnamese, "Không rõ"}, {Language::Khmer, "មិនស្គាល់"}, {Language::English, "Unknown"} };
    
    m_dictionary["LBL_COPY_INFO"] = { {Language::Vietnamese, "Bản sao: %1 (Có thể mượn: %2)"}, {Language::Khmer, "ច្បាប់ចម្លង: %1 (អាចខ្ចី: %2)"}, {Language::English, "Copies: %1 (Available: %2)"} };
    m_dictionary["MSG_IMPORT_ERROR"] = { {Language::Vietnamese, "Không thể đọc file Excel:\n"}, {Language::Khmer, "មិនអាចអានឯកសារ Excel:\n"}, {Language::English, "Cannot read Excel file:\n"} };
    
    // === DIALOG ADD BOOK VALIDATION ===
    m_dictionary["LBL_MUST_ENTER"] = { {Language::Vietnamese, "(*) Các trường bắt buộc"}, {Language::Khmer, "(*) វាលដែលត្រូវការ"}, {Language::English, "(*) Required fields"} };
    m_dictionary["MSG_READY_TO_SAVE"] = { {Language::Vietnamese, "✓ Sẵn sàng lưu"}, {Language::Khmer, "✓ រួចរាល់ដើម្បីរក្សាទុក"}, {Language::English, "✓ Ready to save"} };
    m_dictionary["MSG_CHECKING_DATA"] = { {Language::Vietnamese, "⚙ Đang kiểm tra dữ liệu..."}, {Language::Khmer, "⚙ កំពុងពិនិត្យទិន្នន័យ..."}, {Language::English, "⚙ Checking data..."} };
    m_dictionary["MSG_SAVING_TO_DB"] = { {Language::Vietnamese, "Đang lưu vào cơ sở dữ liệu..."}, {Language::Khmer, "កំពុងរក្សាទុកទៅក្នុងទិន្នន័យ..."}, {Language::English, "Saving to database..."} };
    m_dictionary["MSG_ADD_BOOK_SUCCESS_TITLE"] = { {Language::Vietnamese, "Đã thêm sách '%1' thành công!"}, {Language::Khmer, "បានបន្ថែមសៀវភៅ '%1' ដោយជោគជ័យ!"}, {Language::English, "Added book '%1' successfully!"} };
    m_dictionary["MSG_ADD_BOOK_FAILED"] = { {Language::Vietnamese, "Thêm sách thất bại: %1"}, {Language::Khmer, "ការបន្ថែមសៀវភៅបរាជ័យ: %1"}, {Language::English, "Failed to add book: %1"} };
    m_dictionary["ERR_ISBN_LENGTH"] = { {Language::Vietnamese, "ISBN phải có 10 hoặc 13 chữ số"}, {Language::Khmer, "ISBN ត្រូវមាន 10 ឬ 13 ខ្ទង់"}, {Language::English, "ISBN must be 10 or 13 digits"} };
    m_dictionary["ERR_ISBN_EXISTS"] = { {Language::Vietnamese, "ISBN này đã tồn tại trong hệ thống"}, {Language::Khmer, "ISBN នេះមានរួចហើយនៅក្នុងប្រព័ន្ធ"}, {Language::English, "ISBN already exists"} };
    m_dictionary["ERR_FILL_REQUIRED"] = { {Language::Vietnamese, "Vui lòng điền đầy đủ các trường bắt buộc!"}, {Language::Khmer, "សូមបំពេញវាលដែលត្រូវការទាំងអស់!"}, {Language::English, "Please fill all required fields!"} };
    m_dictionary["BTN_PROCESSING"] = { {Language::Vietnamese, "Đang xử lý..."}, {Language::Khmer, "កំពុងដំណើរការ..."}, {Language::English, "Processing..."} };
    
    // === DIALOG EDIT BOOK ===
    m_dictionary["TITLE_EDIT_BOOK_HEADER"] = { {Language::Vietnamese, "CHỈNH SỬA THÔNG TIN SÁCH"}, {Language::Khmer, "កែសម្រួលព័ត៌មានសៀវភៅ"}, {Language::English, "EDIT BOOK INFORMATION"} };
    m_dictionary["DIALOG_EDIT_BOOK_TITLE"] = { {Language::Vietnamese, "Sửa Sách"}, {Language::Khmer, "កែសម្រួលសៀវភៅ"}, {Language::English, "Edit Book"} };
    m_dictionary["LBL_PRICE"] = { {Language::Vietnamese, "Giá Tiền"}, {Language::Khmer, "តម្លៃ"}, {Language::English, "Price"} };
    m_dictionary["LBL_LOCATION"] = { {Language::Vietnamese, "Vị Trí"}, {Language::Khmer, "ទីតាំង"}, {Language::English, "Location"} };
    m_dictionary["TABLE_HEADER_COPIES"] = { {Language::Vietnamese, "Số Lượng Sao"}, {Language::Khmer, "ចំនួនច្បាប់ចម្លង"}, {Language::English, "Copies"} };
    m_dictionary["BTN_SAVE_CHANGES"] = { {Language::Vietnamese, "Lưu Thay Đổi"}, {Language::Khmer, "រក្សាទុកការផ្លាស់ប្តូរ"}, {Language::English, "Save Changes"} };
    m_dictionary["BTN_SAVE"] = { {Language::Vietnamese, "Lưu"}, {Language::Khmer, "រក្សាទុក"}, {Language::English, "Save"} };
    m_dictionary["BTN_CANCEL"] = { {Language::Vietnamese, "Hủy"}, {Language::Khmer, "បោះបង់"}, {Language::English, "Cancel"} };
    
    m_dictionary["MSG_BOOK_NOT_FOUND"] = { {Language::Vietnamese, "Không tìm thấy thông tin đầu sách!"}, {Language::Khmer, "រកមិនឃើញព័ត៌មានសៀវភៅ!"}, {Language::English, "Book info not found!"} };
    m_dictionary["MSG_UPDATING"] = { {Language::Vietnamese, "⟳ Đang cập nhật..."}, {Language::Khmer, "⟳ កំពុងធ្វើបច្ចុប្បន្នភាព..."}, {Language::English, "⟳ Updating..."} };
    m_dictionary["MSG_UPDATE_SUCCESS_VAR"] = { {Language::Vietnamese, "Đã cập nhật sách '%1' thành công!"}, {Language::Khmer, "បានធ្វើបច្ចុប្បន្នភាពសៀវភៅ '%1' ដោយជោគជ័យ!"}, {Language::English, "Updated book '%1' successfully!"} };
    m_dictionary["MSG_ERROR_UPDATE_FAILED"] = { {Language::Vietnamese, "Cập nhật thất bại"}, {Language::Khmer, "ការធ្វើបច្ចុប្បន្នភាពបរាជ័យ"}, {Language::English, "Update failed"} };
    
    // === DIALOG READER VALIDATION ===
    m_dictionary["DIALOG_ADD_READER_TITLE"] = { {Language::Vietnamese, "Thêm Học Viên Mới"}, {Language::Khmer, "បន្ថែមសិស្សថ្មី"}, {Language::English, "Add New Reader"} };
    m_dictionary["DIALOG_EDIT_READER_TITLE"] = { {Language::Vietnamese, "Sửa Thông Tin Học Viên"}, {Language::Khmer, "កែសម្រួលព័ត៌មានសិស្ស"}, {Language::English, "Edit Reader Info"} };
    m_dictionary["GROUP_READER_CODE"] = { {Language::Vietnamese, "Mã Thẻ Học Viên"}, {Language::Khmer, "លេខកូដកាតសិស្ស"}, {Language::English, "Reader ID Card"} };
    m_dictionary["GROUP_READER_INFO"] = { {Language::Vietnamese, "Thông Tin Học Viên"}, {Language::Khmer, "ព័ត៌មានសិស្ស"}, {Language::English, "Reader Information"} };
    m_dictionary["LBL_AUTO_CODE_HINT"] = { {Language::Vietnamese, "* Mã thẻ được tạo tự động"}, {Language::Khmer, "* លេខកូដត្រូវបានបង្កើតដោយស្វ័យប្រវត្តិ"}, {Language::English, "* ID is auto-generated"} };
    m_dictionary["LBL_HO_DEM"] = { {Language::Vietnamese, "Họ đệm *:"}, {Language::Khmer, "នាមត្រកូល *:"}, {Language::English, "Last Name *:"} };
    m_dictionary["LBL_TEN"] = { {Language::Vietnamese, "Tên *:"}, {Language::Khmer, "ឈ្មោះ *:"}, {Language::English, "First Name *:"} };
    m_dictionary["PLACEHOLDER_HO"] = { {Language::Vietnamese, "Nhập họ đệm..."}, {Language::Khmer, "បញ្ចូលនាមត្រកូល..."}, {Language::English, "Enter last name..."} };
    m_dictionary["PLACEHOLDER_TEN"] = { {Language::Vietnamese, "Nhập tên..."}, {Language::Khmer, "បញ្ចូលឈ្មោះ..."}, {Language::English, "Enter first name..."} };
    m_dictionary["BTN_ADD_READER"] = { {Language::Vietnamese, "Thêm Học Viên"}, {Language::Khmer, "បន្ថែមអ្នកអាន"}, {Language::English, "Add Reader"} };
    m_dictionary["MSG_ADD_READER_PROCESSING"] = { {Language::Vietnamese, "⚙ Đang thêm học viên..."}, {Language::Khmer, "⚙ កំពុងបន្ថែមសិស្ស..."}, {Language::English, "⚙ Adding reader..."} };
    m_dictionary["MSG_ADD_READER_SUCCESS_VAR"] = { {Language::Vietnamese, "Đã thêm học viên '%1 %2' thành công!"}, {Language::Khmer, "បានបន្ថែមសិស្ស '%1 %2' ដោយជោគជ័យ!"}, {Language::English, "Added reader '%1 %2' successfully!"} };
    m_dictionary["MSG_ADD_READER_SUCCESS_LBL"] = { {Language::Vietnamese, "✓ Đã thêm học viên: %1 %2"}, {Language::Khmer, "✓ បានបន្ថែមសិស្ស: %1 %2"}, {Language::English, "✓ Added reader: %1 %2"} };
    
    m_dictionary["TITLE_EDIT_READER_HEADER"] = { {Language::Vietnamese, "CHỈNH SỬA THÔNG TIN HỌC VIÊN"}, {Language::Khmer, "កែសម្រួលព័ត៌មានសិស្ស"}, {Language::English, "EDIT READER INFORMATION"} };
    m_dictionary["LBL_READER_ID"] = { {Language::Vietnamese, "Mã thẻ:"}, {Language::Khmer, "លេខកូដ:"}, {Language::English, "ID:"} };
    m_dictionary["LBL_BORROWED_COUNT_HINT"] = { {Language::Vietnamese, "➤ Số sách đang mượn: %1"}, {Language::Khmer, "➤ សៀវភៅកំពុងខ្ចី: %1"}, {Language::English, "➤ Borrowing: %1"} };
    m_dictionary["MSG_READER_NOT_FOUND"] = { {Language::Vietnamese, "Không tìm thấy học viên!"}, {Language::Khmer, "រកមិនឃើញសិស្ស!"}, {Language::English, "Reader not found!"} };
    m_dictionary["STATUS_ACTIVE"] = { {Language::Vietnamese, "Hoạt động"}, {Language::Khmer, "សកម្ម"}, {Language::English, "Active"} };
    m_dictionary["STATUS_LOCKED"] = { {Language::Vietnamese, "Khóa"}, {Language::Khmer, "បិទ"}, {Language::English, "Locked"} };
    // === STATISTICS DIALOG ===
    m_dictionary["STATS_MAIN_TITLE"] = { {Language::Vietnamese, "THỐNG KÊ TỔNG QUAN THƯ VIỆN"}, {Language::Khmer, "ស្ថិតិបណ្ណាល័យសរុប"}, {Language::English, "LIBRARY STATISTICS OVERVIEW"} };
    m_dictionary["STATS_BOOKS_GROUP"] = { {Language::Vietnamese, "Thống Kê Sách"}, {Language::Khmer, "ស្ថិតិសៀវភៅ"}, {Language::English, "Book Statistics"} };
    m_dictionary["STATS_READERS_GROUP"] = { {Language::Vietnamese, "Thống Kê Học Viên"}, {Language::Khmer, "ស្ថិតិសិស្ស"}, {Language::English, "Reader Statistics"} };
    
    // Books
    m_dictionary["LBL_TOTAL_TITLES"] = { {Language::Vietnamese, "Tổng đầu sách:"}, {Language::Khmer, "ចំនួនសៀវភៅសរុប:"}, {Language::English, "Total Titles:"} };
    m_dictionary["LBL_TOTAL_COPIES_STAT"] = { {Language::Vietnamese, "Tổng bản sao:"}, {Language::Khmer, "ច្បាប់ចម្លងសរុប:"}, {Language::English, "Total Copies:"} };
    m_dictionary["LBL_AVAILABLE_STAT"] = { {Language::Vietnamese, "Có thể mượn:"}, {Language::Khmer, "អាចខ្ចីបាន:"}, {Language::English, "Available:"} };
    m_dictionary["LBL_BORROWED_STAT"] = { {Language::Vietnamese, "Đang mượn:"}, {Language::Khmer, "កំពុងខ្ចី:"}, {Language::English, "Borrowed:"} };
    m_dictionary["LBL_LIQUIDATED_STAT"] = { {Language::Vietnamese, "Đã thanh lý:"}, {Language::Khmer, "បានលុបចោល:"}, {Language::English, "Liquidated:"} };
    m_dictionary["LBL_BORROW_RATE"] = { {Language::Vietnamese, "Tỷ lệ cho mượn:"}, {Language::Khmer, "អត្រាការខ្ចី:"}, {Language::English, "Borrow Rate:"} };
    m_dictionary["LBL_CATEGORIES_STAT"] = { {Language::Vietnamese, "Số thể loại:"}, {Language::Khmer, "ចំនួនប្រភេទ:"}, {Language::English, "Categories:"} };
    
    // Readers
    m_dictionary["LBL_TOTAL_READERS_STAT"] = { {Language::Vietnamese, "Tổng học viên:"}, {Language::Khmer, "សិស្សសរុប:"}, {Language::English, "Total Readers:"} };
    m_dictionary["LBL_ACTIVE_STAT"] = { {Language::Vietnamese, "Đang hoạt động:"}, {Language::Khmer, "កំពុងសកម្ម:"}, {Language::English, "Active:"} };
    m_dictionary["LBL_LOCKED_STAT"] = { {Language::Vietnamese, "Bị khóa thẻ:"}, {Language::Khmer, "បានចាក់សោ:"}, {Language::English, "Locked:"} };
    m_dictionary["LBL_READERS_BORROWING_STAT"] = { {Language::Vietnamese, "Đang mượn sách:"}, {Language::Khmer, "កំពុងខ្ចីសៀវភៅ:"}, {Language::English, "Borrowing Books:"} };
    m_dictionary["LBL_TOTAL_BORROWED_BOOKS_STAT"] = { {Language::Vietnamese, "Tổng sách mượn:"}, {Language::Khmer, "ចំនួនសៀវភៅដែលខ្ចី:"}, {Language::English, "Total Borrowed:"} };
    m_dictionary["LBL_READERS_OVERDUE_STAT"] = { {Language::Vietnamese, "Có sách quá hạn:"}, {Language::Khmer, "មានសៀវភៅហួសកំណត់:"}, {Language::English, "Overdue Readers:"} };

    // Charts
    m_dictionary["CHART_BOOK_STATUS"] = { {Language::Vietnamese, "Phân Bố Trạng Thái Sách"}, {Language::Khmer, "ការចែកចាយស្ថានភាពសៀវភៅ"}, {Language::English, "Book Status Distribution"} };
    m_dictionary["CHART_READER_STATUS"] = { {Language::Vietnamese, "Trạng Thái Học Viên"}, {Language::Khmer, "ស្ថានភាពសិស្ស"}, {Language::English, "Reader Status"} };
    
    m_dictionary["BTN_CLOSE"] = { {Language::Vietnamese, "Đóng"}, {Language::Khmer, "បិទ"}, {Language::English, "Close"} };
    
    // Mission Keys for Phase 11
    m_dictionary["FILTER_ACTIVE"] = { {Language::Vietnamese, "Đang hoạt động"}, {Language::Khmer, "កំពុងសកម្ម"}, {Language::English, "Active"} };
    m_dictionary["FILTER_LOCKED"] = { {Language::Vietnamese, "Bị khóa"}, {Language::Khmer, "ត្រូវបានបិទ"}, {Language::English, "Locked"} };
    m_dictionary["LBL_CARD_VALIDITY_MONTHS"] = { {Language::Vietnamese, "Thời hạn thẻ (tháng):"}, {Language::Khmer, "សុពលភាពប័ណ្ណ (ខែ):"}, {Language::English, "Card Validity (months):"} };
    m_dictionary["LBL_LOST_BOOK_FEE"] = { {Language::Vietnamese, "Phí mất sách:"}, {Language::Khmer, "ថ្លៃបាត់សៀវភៅ:"}, {Language::English, "Lost Book Fee:"} };
    m_dictionary["STATUS_LOST_DEBT"] = { {Language::Vietnamese, "Đã báo mất (Nợ: %1)"}, {Language::Khmer, "បានរាយការណ៍ថាបាត់ (ជំពាក់: %1)"}, {Language::English, "Reported Lost (Debt: %1)"} };
    m_dictionary["STATUS_LOST_PAID"] = { {Language::Vietnamese, "Đã mất (Đã đền bù)"}, {Language::Khmer, "បានបាត់ (សងរួច)"}, {Language::English, "Lost (Compensated)"} };

    // === MISSING KEYS (RUNTIME LOGS) ===
    m_dictionary["PLACEHOLDER_BOOK_TITLE"] = { {Language::Vietnamese, "Nhập tên sách..."}, {Language::Khmer, "បញ្ចូលឈ្មោះសៀវភៅ..."}, {Language::English, "Enter book title..."} };
    m_dictionary["PLACEHOLDER_CATEGORY"] = { {Language::Vietnamese, "VD: Luật Tạng, Kinh Tạng, Pali, Khmer..."}, {Language::Khmer, "ឧទាហរណ៍: វិន័យបិដក, សុត្តន្តបិដក..."}, {Language::English, "Ex: Vinaya, Sutta, Pali..."} };
    m_dictionary["LBL_COPY_COUNT"] = { {Language::Vietnamese, "Số lượng bản sao:"}, {Language::Khmer, "ចំនួនច្បាប់ចម្លង:"}, {Language::English, "Copy Count:"} };
    
    m_dictionary["READER_TYPE_STUDENT"] = { {Language::Vietnamese, "Học viên"}, {Language::Khmer, "សិស្ស"}, {Language::English, "Student"} };
    m_dictionary["READER_TYPE_TEACHER"] = { {Language::Vietnamese, "Giảng viên"}, {Language::Khmer, "គ្រូបង្រៀន"}, {Language::English, "Teacher"} };
    
    m_dictionary["PLACEHOLDER_CLASS"] = { {Language::Vietnamese, "Nhập lớp..."}, {Language::Khmer, "បញ្ចូលថ្នាក់..."}, {Language::English, "Enter class..."} };
    m_dictionary["PLACEHOLDER_PHONE"] = { {Language::Vietnamese, "Nhập SĐT..."}, {Language::Khmer, "បញ្ចូលលេខទូរស័ព្ទ..."}, {Language::English, "Enter phone..."} };
    m_dictionary["PLACEHOLDER_EMAIL"] = { {Language::Vietnamese, "Nhập email..."}, {Language::Khmer, "បញ្ចូលអ៊ីមែល..."}, {Language::English, "Enter email..."} };
    
    m_dictionary["LBL_READER_TYPE"] = { {Language::Vietnamese, "Phân loại:"}, {Language::Khmer, "ប្រភេទ:"}, {Language::English, "Type:"} };
    m_dictionary["LBL_CLASS"] = { {Language::Vietnamese, "Lớp:"}, {Language::Khmer, "ថ្នាក់:"}, {Language::English, "Class:"} };
    m_dictionary["LBL_PHONE"] = { {Language::Vietnamese, "SĐT:"}, {Language::Khmer, "ទូរស័ព្ទ:"}, {Language::English, "Phone:"} };
    m_dictionary["LBL_EMAIL"] = { {Language::Vietnamese, "Email:"}, {Language::Khmer, "អ៊ីមែល:"}, {Language::English, "Email:"} };
    m_dictionary["LBL_DOB"] = { {Language::Vietnamese, "Ngày sinh:"}, {Language::Khmer, "ថ្ងៃកំណើត:"}, {Language::English, "Date of Birth:"} };
    
    // Advanced Features
    m_dictionary["GROUP_HISTORY"] = { {Language::Vietnamese, "Lịch sử Mượn Trả"}, {Language::Khmer, "ប្រវត្តិនៃការខ្ចីនិងសង"}, {Language::English, "Borrowing History"} };
    m_dictionary["GROUP_STATS_READERS"] = { {Language::Vietnamese, "Thống Kê Xu Hướng"}, {Language::Khmer, "ស្ថិតិនិន្នាការ"}, {Language::English, "Trend Statistics"} };
    m_dictionary["BTN_EXPORT_PDF"] = { {Language::Vietnamese, "Xuất PDF"}, {Language::Khmer, "នាំចេញជា PDF"}, {Language::English, "Export PDF"} };
}
