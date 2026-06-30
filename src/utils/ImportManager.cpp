#include "utils/ImportManager.h"
#include "utils/FilePathManager.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "models/ThaoTacFile.h"
#include "models/NhapLieu.h" 
#include "utils/DatabaseManager.h"
#include "utils/EventBus.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>
#include <QDate>
#include <QRegularExpression>
#include <QList>
#include <QWidget>
#include <functional> // For std::function in duplicate detection
#include <QSet>
#include <algorithm> // For std::sort

// QXlsx Include
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
#include "xlsxcell.h"
using namespace QXlsx;

ImportManager::ImportManager(QObject *parent) : QObject(parent) {
}

ImportManager& ImportManager::instance() {
    static ImportManager _instance;
    return _instance;
}

#include "utils/XuLyChuoi.h"
#include <QMap>

// ⚡ SMART HELPERS: Normalize headers for matching
static QString normalizeHeader(const QString &header) {
    // Remove spaces to ensure "Tên Sách" matches "tensach", "Giá Tiền" matches "giatien"
    return QString::fromStdString(BoDauVaThuong(header.toStdString())).remove(" ").remove("-").remove("_").trimmed();
}

static QString detectTypeFromHeaders(const QStringList &headers) {
    int sachScore = 0;
    int docGiaScore = 0;
    
    // Strong indicators get more points
    QMap<QString, int> sachKeywords = {
        {"isbn", 5}, {"tensach", 3}, {"dausach", 3}, {"tacgia", 2}, {"nxb", 2}, {"giatien", 2}, {"soluong", 2}
    };
    QMap<QString, int> docGiaKeywords = {
        {"hodem", 3}, {"ngaysinh", 3}, {"msv", 5}, {"mssv", 5}, {"lop", 2}, {"sdt", 4}, {"email", 4}, {"hocvien", 5}
    };
    
    for (const QString &h : headers) {
        QString n = normalizeHeader(h);
        if (n.isEmpty()) continue;
        for (auto it = sachKeywords.begin(); it != sachKeywords.end(); ++it) if (n.contains(it.key())) sachScore += it.value();
        for (auto it = docGiaKeywords.begin(); it != docGiaKeywords.end(); ++it) if (n.contains(it.key())) docGiaScore += it.value();
    }
    
    qDebug() << "[IMPORT] Type Detection Scores - Sach:" << sachScore << "DocGia:" << docGiaScore;
    if (sachScore > docGiaScore && sachScore > 0) return "SACH";
    if (docGiaScore > sachScore && docGiaScore > 0) return "DOCGIA";
    return "";
}

static QMap<QString, int> buildColumnMap(const QStringList &headers, const QString &type) {
    QMap<QString, int> mapping;
    QMap<QString, QStringList> schema;
    
    if (type == "SACH") {
        schema["ISBN"] = {"isbn", "masach", "ma", "code", "bookid", "id"};
        schema["TEN"] = {"tensach", "dausach", "tieude", "ten", "title", "bookname", "name"};
        schema["TACGIA"] = {"tacgia", "author", "writer", "nguoiviet"};
        schema["NAM"] = {"namxb", "namxuatban", "nam", "year", "publishingyear", "namphathanh"};
        schema["THELOAI"] = {"theloai", "category", "genre", "loai", "type", "chude"};
        schema["GIA"] = {"giatien", "gia", "price", "dongia", "cost", "tien", "amount"};
        schema["SOLUONG"] = {"soluong", "sl", "quantity", "stock", "total", "tong", "qty"};
        schema["NXB"] = {"nhaxuatban", "nxb", "publisher", "nhaxb", "pub"};
        schema["VITRI"] = {"vitri", "location", "shelf", "ke", "day", "hang", "pos"};
    } else {
        schema["HO"] = {"hodem", "ho", "last", "surname", "family", "lastname"};
        schema["TEN"] = {"ten", "first", "given", "name", "firstname"};
        schema["PHAI"] = {"phai", "gioitinh", "sex", "gender", "gioi"};
        schema["NGAYSINH"] = {"ngaysinh", "dob", "birthday", "namsinh", "ngay"};
        schema["LOP"] = {"lop", "class", "grade", "khoa"};
        schema["SDT"] = {"sdt", "phone", "tel", "dienthoai", "mobile", "contact", "sodienthoai"};
        schema["EMAIL"] = {"email", "thu", "mail", "diachi"};
    }

    // ✨ FIX: Advanced Matching Algorithm (Best Match First)
    // Prevents "TacGia" being mapped to "GIA" just because it contains "gia"!
    struct Match {
        QString key;
        int colIdx;
        int score; // Length of matched keyword
    };
    QList<Match> candidates;

    for (int i = 0; i < headers.size(); ++i) {
        QString n = normalizeHeader(headers[i]);
        if (n.isEmpty()) continue;
        
        for (auto it = schema.begin(); it != schema.end(); ++it) {
            for (const QString &kw : it.value()) {
                if (n.contains(kw)) {
                    // Score = keyword length. Longer (more specific) keyword matches are better.
                    // E.g: "tacgia" (6) > "gia" (3)
                    candidates.append({it.key(), i + 1, (int)kw.length()});
                }
            }
        }
    }

    // Sort: Higher score (longer keyword) first
    std::sort(candidates.begin(), candidates.end(), [](const Match& a, const Match& b){
        return a.score > b.score;
    });

    QSet<int> usedCols; // Ensure one column only maps to one field
    
    for (const auto& m : candidates) {
        // If this Key isn't mapped yet AND this Column isn't used yet
        if (!mapping.contains(m.key) && !usedCols.contains(m.colIdx)) {
            mapping[m.key] = m.colIdx;
            usedCols.insert(m.colIdx);
        }
    }

    return mapping;
}

// ⚡ SMART HELPERS: Fuzzy Number Parsing
// Handles: "20.000", "20,000", "50k", "100.00 VNĐ", "50.5k" -> Raw Number
static long long parseSmartNumber(QString input) {
    input = input.trimmed().toLower();
    if (input.isEmpty()) return 0;
    
    // ✨ FIX: Remove "vnd" text FIRST (before removing symbols)
    input.remove("vnd", Qt::CaseInsensitive);
    input = input.trimmed();
    
    // Then remove currency symbols (VNĐ, đ, USD, $, etc.)
    input.remove(QRegularExpression("[đĐ$€£¥₫]")); // ✨ FIX: Added 'Đ' for VNĐ
    input = input.trimmed();
    
    // ✨ FIX: Handle multipliers FIRST (before decimal detection)
    double multiplier = 1.0;
    if (input.endsWith("k")) {
        multiplier = 1000.0;
        input.chop(1);
        input = input.trimmed();
    } else if (input.endsWith("m")) {
        multiplier = 1000000.0;
        input.chop(1);
        input = input.trimmed();
    } else if (input.endsWith("tr")) {
        multiplier = 1000000.0;
        input.chop(2);
        input = input.trimmed();
    }
    
    // NOW detect decimal (after removing multiplier)
    // "50.5k" becomes "50.5" after chopping "k"
    int lastDot = input.lastIndexOf('.');
    int lastComma = input.lastIndexOf(',');
    int dotCount = input.count('.');
    int commaCount = input.count(',');
    
    bool hasDecimal = false;
    int decimalPos = -1;
    
    // Heuristic: If separator appears only once and is in last 3 chars, it's decimal
    if (dotCount == 1 && input.length() - lastDot <= 3) {
        hasDecimal = true;
        decimalPos = lastDot;
    } else if (commaCount == 1 && input.length() - lastComma <= 3) {
        hasDecimal = true;
        decimalPos = lastComma;
    }
    
    QString cleaned;
    if (hasDecimal) {
        // Keep last separator as decimal point
        for (int i = 0; i < input.length(); ++i) {
            QChar c = input[i];
            if (c.isDigit()) cleaned.append(c);
            else if (i == decimalPos) cleaned.append('.'); // Normalize to dot
        }
        // Parse as double, multiply, round to nearest integer
        return static_cast<long long>(cleaned.toDouble() * multiplier + 0.5);
    } else {
        // Integer: Remove all non-digits (commas/dots are thousands separators)
        for (QChar c : input) {
            if (c.isDigit()) cleaned.append(c);
        }
        if (cleaned.isEmpty()) return 0;
        return static_cast<long long>(cleaned.toLongLong() * multiplier);
    }
}

// ⚡ SMART HELPERS: Fuzzy Gender Parsing
// Handles: "Nam", "Nữ", "Male", "Female", "M", "F", "1" (Nu), "0" (Nam)
static bool parseSmartGender(QString input) {
    input = input.trimmed().toLower();
    // Common female indicators
    if (input == "nu" || input == "nữ" || input == "female" || input == "f" || input == "girl" || input == "1") {
        return true; 
    }
    // Default is Male (0)
    return false;
}

// ⚡ SMART HELPERS: Fuzzy Date Parsing
// Handles: "2020", "20/11/1990", "1990-11-20", Excel serial "44927"
// mode=0: Full Date (DD/MM/YYYY), mode=1: Just Year (YYYY)
static QString parseSmartDate(QString input, int mode) {
    input = input.trimmed();
    if (input.isEmpty()) return (mode == 1 ? "2000" : "");
    
    // Mode 1: Year only
    if (mode == 1) {
        // ✨ FIX: Check regex year FIRST (before Excel serial)
        // Otherwise "2020" will be treated as serial!
        QRegularExpression re("(19|20)\\d{2}");
        auto match = re.match(input);
        if (match.hasMatch()) {
            return match.captured(0);
        }
        
        // Fallback: If integer, assume year (but not realistic year range like 2000-2100)
        bool ok;
        int y = input.toInt(&ok);
        if (ok && y >= 1900 && y <= 2100) return QString::number(y);
        
        // Now check Excel serial (5-digit numbers like 44927)
        if (ok && y > 10000 && y < 100000) {
            QDate excelDate = QDate(1899, 12, 30).addDays(y);
            if (excelDate.isValid()) {
                qDebug() << "[PARSE] Excel serial" << y << "-> Year:" << excelDate.year();
                return QString::number(excelDate.year());
            }
        }
        
        return "2000"; // Default
    }
    
    // Mode 0: Full Date
    // ✨ FIX: Check Excel serial for full date mode
    bool isNumeric;
    int serial = input.toInt(&isNumeric);
    if (isNumeric && serial > 10000 && serial < 100000) {
        // Excel serial: Days since 1899-12-30
        QDate excelDate = QDate(1899, 12, 30).addDays(serial);
        if (excelDate.isValid()) {
            qDebug() << "[PARSE] Excel serial" << serial << "-> Date:" << excelDate.toString("dd/MM/yyyy");
            return excelDate.toString("dd/MM/yyyy");
        }
    }
    
    // Try QDate parsing with multiple formats
    QStringList formats = {"d/M/yyyy", "dd/MM/yyyy", "yyyy-MM-dd", "d-M-yyyy", "dd-MM-yyyy", "dd.MM.yyyy"};
    for (const auto& fmt : formats) {
        QDate d = QDate::fromString(input, fmt);
        if (d.isValid()) return d.toString("dd/MM/yyyy");
    }
    return input; // Return original if unknown
}

ImportManager::PreviewResult ImportManager::runPreview(const QString &excelFile) {
    PreviewResult result;
    result.success = false;

    Document doc(excelFile);
    if (!doc.load()) {
        result.message = "Không thể mở file Excel. Vui lòng kiểm tra lại đường dẫn.";
        return result;
    }

    if (!doc.selectSheet("Sheet1")) {
         QStringList sheets = doc.sheetNames();
         if (!sheets.isEmpty()) doc.selectSheet(sheets.first());
    }

    // 1. READ HEADERS
    QStringList headers;
    for (int col = 1; col <= 30; ++col) {
        auto cell = doc.cellAt(1, col);
        if (!cell) break;
        headers << cell->value().toString().trimmed();
    }
    
    result.detectedType = detectTypeFromHeaders(headers);
    
    // 2. BUILD MAPPINGS FOR BOTH
    QMap<QString, int> colMapSach = buildColumnMap(headers, "SACH");
    QMap<QString, int> colMapDocGia = buildColumnMap(headers, "DOCGIA");
    
    QStringList keysSach = {"ISBN", "TEN", "TACGIA", "NAM", "THELOAI", "GIA", "SOLUONG", "NXB", "VITRI"};
    QStringList keysDocGia = {"HO", "TEN", "PHAI", "NGAYSINH", "LOP", "SDT", "EMAIL"};

    // 3. READ DATA FOR BOTH PATHS
    QList<QStringList> previewDataSach;
    QList<QStringList> previewDataDocGia;
    int row = 2;
    int maxRow = doc.dimension().lastRow();
    
    while (row <= maxRow) {
        auto cellA = doc.cellAt(row, 1);
        auto cellB = doc.cellAt(row, 2);
        if ((!cellA || cellA->value().toString().trimmed().isEmpty()) && 
            (!cellB || cellB->value().toString().trimmed().isEmpty())) { 
            row++; continue; 
        }

        // Align for Sach
        QStringList rowSach;
        for (const QString &key : keysSach) {
            int colIdx = colMapSach.value(key, -1);
            QString val = (colIdx != -1) ? doc.read(row, colIdx).toString().trimmed() : "";
            val.replace("\"", "\"\""); rowSach << val;
        }
        rowSach << "TRUE"; previewDataSach.append(rowSach);

        // Align for DocGia
        QStringList rowDocGia;
        for (const QString &key : keysDocGia) {
            int colIdx = colMapDocGia.value(key, -1);
            QString val = (colIdx != -1) ? doc.read(row, colIdx).toString().trimmed() : "";
            val.replace("\"", "\"\""); rowDocGia << val;
        }
        rowDocGia << "TRUE"; previewDataDocGia.append(rowDocGia);

        row++;
        if (previewDataSach.size() >= 100) break;
    }

    QString outDir = QDir(FilePathManager::getRootPath()).filePath("tools");
    QDir().mkpath(outDir);

    result.dauSachCsv = createTempCsv(QDir(outDir).filePath("preview_dausach.csv"), previewDataSach, "SACH");
    result.docGiaCsv = createTempCsv(QDir(outDir).filePath("preview_docgia.csv"), previewDataDocGia, "DOCGIA"); 
    
    result.success = true;
    result.message = QString("Đọc xong. Gợi ý: %1").arg(result.detectedType == "SACH" ? "Sách" : (result.detectedType == "DOCGIA" ? "Độc giả" : "Chưa rõ"));
    return result;
}

QString ImportManager::createTempCsv(const QString &path, const QList<QStringList> &data, const QString &type) {
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            out.setEncoding(QStringConverter::Utf8);
        #else
            out.setCodec("UTF-8");
        #endif
        // Add specific headers expected by UI?
        // ManHinhPreviewImport likely expects specific headers.
        // Let's look at the old Python script logic or UI.
        // Or simpler: Just write raw data. The UI likely maps data by index 0,1,2...
        
        // We add a generic header if list is not empty, or assume row 1 was header?
        // We skipped row 1 in loop. Let's add standard headers to make UI happy.
        // Book: ISBN, TenSach, TacGia, Nam, TheLoai, Gia, SoLuong, NXB, ViTri
        // Write Headers based on Type
        if (type == "DOCGIA") {
             // Reader: Ho, Ten, Gioi Tinh, Ngay Sinh, Lop, SDT, Email
             out << "HoDem|Ten|GioiTinh|NgaySinh|Lop|SDT|Email|Valid\n";
        } else {
             // Book: ISBN, TenSach, ...
             out << "ISBN|TenSach|TacGia|NamXB|TheLoai|GiaTien|SoLuong|NhaXuatBan|ViTri|Valid\n";
        }

        for (const QStringList &row : data) {
            // Join with pipe to avoid comma issues
            // Pipe is safe because we don't expect it in typical book/reader data
            // If it exists, we can replace it.
            QStringList cleanRow;
            for (QString field : row) {
                // Remove pipes from content just in case
                field.replace("|", " "); 
                // Remove newlines
                field.replace("\n", " ");
                cleanRow << field;
            }
            out << cleanRow.join("|") << "\n";
        }
        file.close();
        return path;
    }
    return "";
}

bool ImportManager::runApply(const QString &excelFile, const QString &mode, QWidget *parent) {
    // Deprecated or redirect to Preview -> Apply flow logic if needed.
    // For now, return false to enforce Preview workflow.
    return false; 
}

bool ImportManager::runApplyData(const QList<QStringList> &data, const QString &type, const QString &mode, QWidget *parent) {
    if (data.isEmpty()) return false;
    
    int countSuccess = 0;
    
    // START BATCH TRANSACTION
    DatabaseManager::instance().beginTransaction();
    
    // Iterate rows
    for (const QStringList &row : data) {
        if (row.isEmpty()) continue;
        
        // CHECK VALIDITY (Last Column)
        if (row.last().toUpper() == "FALSE" || row.last() == "0") {
            continue; // Skip invalid
        }
        
        if (type == "SACH") {
             if (row.size() < 9) continue;
             QString isbn = row[0].trimmed();
             
             // Validate ISBN
             if (isbn.isEmpty()) {
                 qWarning() << "[IMPORT] Skip row: ISBN empty";
                 continue;
             }
             
             // Standardize Title/Author to Title Case
             std::string stdTen, stdTg, stdTl, stdNxb, stdVt, err;
             ChuanHoaTenUnicode(row[1].toStdString(), 1, 200, stdTen);
             ChuanHoaTenUnicode(row[2].toStdString(), 1, 100, stdTg);
             ChuanHoaTenUnicode(row[4].toStdString(), 1, 100, stdTl);
             stdNxb = row[7].toStdString(); // Publisher name can be generic, keep or standardize? Standardize safe.
             
             // Year: Extract YYYY (handles Excel serial now!)
             int nam = parseSmartDate(row[3], 1).toInt();
             
             long long gia = parseSmartNumber(row[5]);
             int sl = (int)parseSmartNumber(row[6]);
             
             // Validate Quantity
             if (sl <= 0) {
                 qWarning() << "[IMPORT] Skip row: Invalid quantity" << sl << "for ISBN:" << isbn;
                 continue;
             }
             if (sl > 100) {
                 qWarning() << "[IMPORT] Quantity" << sl << "exceeds max 100 for ISBN:" << isbn << "- Capping to 100";
                 sl = 100;
             }
             
             // Use standardized values
             QString ten = QString::fromStdString(stdTen.empty() ? row[1].toStdString() : stdTen);
             QString tg = QString::fromStdString(stdTg.empty() ? row[2].toStdString() : stdTg);
             QString tl = QString::fromStdString(stdTl.empty() ? row[4].toStdString() : stdTl);
             QString nxb = row[7]; // Keep original or std?
             QString vt = row[8];

             if (ten.isEmpty()) {
                 qWarning() << "[IMPORT] Skip row: Title empty for ISBN:" << isbn;
                 continue;
             }

             // ✅ FIX: Check error from backend
             std::string errorMsg = ThemDauSachMoi(isbn.toStdString(), ten.toStdString(), 100, 
                                                    tg.toStdString(), nam, tl.toStdString(), 
                                                    sl, vt.toStdString(), gia, nxb.toStdString(), false); // false = don't emit signal yet
             
             if (errorMsg.empty()) {
                 countSuccess++;
                 qDebug() << "[IMPORT] ✓ Success:" << isbn << "-" << ten << "(" << sl << "copies)";
             } else {
                 qWarning() << "[IMPORT] ✗ Failed:" << isbn << "-" << QString::fromStdString(errorMsg);
             }
        }
        else if (type == "DOCGIA") {
             // 0:Ho, 1:Ten, 2:GT, 3:DOB, 4:Lop, 5:SDT, 6:Email
             if (row.size() < 7) continue;
             
             std::string stdHo, stdTen;
             ChuanHoaTenUnicode(row[0].toStdString(), 1, 50, stdHo);
             ChuanHoaTenUnicode(row[1].toStdString(), 1, 50, stdTen);
             
             QString ho = QString::fromStdString(stdHo.empty() ? row[0].toStdString() : stdHo);
             QString ten = QString::fromStdString(stdTen.empty() ? row[1].toStdString() : stdTen);
             
             if (ten.isEmpty()) {
                 qWarning() << "[IMPORT] Skip reader: Name empty";
                 continue;
             }
             
             bool phai = parseSmartGender(row[2]);
             QString ngaySinh = parseSmartDate(row[3], 0);
             QString lop = row[4].trimmed().toUpper(); // Normalize class
             QString sdt = row[5];
             sdt.remove(" ").remove(".").remove("-").remove("(").remove(")"); // Clean phone
             QString email = row[6].toLower().trimmed();
             
             // ✨ NEW: Smart Duplicate Detection
             // Strategy: Check if existing reader matches 2+ of these criteria:
             // 1. Full name (Ho + Ten)
             // 2. Birthday (NgaySinh)
             // 3. Class (Lop)
             
             std::string fullNameStd = BoDauVaThuong(ho.toStdString() + " " + ten.toStdString());
             std::string ngaySinhStd = ngaySinh.toStdString();
             std::string lopStd = lop.toStdString();
             
             PTRDG existingReader = nullptr;
             int matchScore = 0;
             
             // Traverse AVL tree to find duplicates
             std::function<void(PTRDG)> traverse = [&](PTRDG node) {
                 if (!node) return;
                 traverse(node->left);
                 
                 // Build comparable name
                 std::string existingName = BoDauVaThuong(node->data.Ho + " " + node->data.Ten);
                 int score = 0;
                 
                 // Match 1: Name similarity (exact match after normalize)
                 if (existingName == fullNameStd) score += 2; // Strong match
                 
                 // Match 2: Birthday
                 if (!node->data.NgaySinh.empty() && node->data.NgaySinh == ngaySinhStd) score += 2;
                 
                 // Match 3: Class
                 if (!node->data.Lop.empty() && node->data.Lop == lopStd) score += 1;
                 
                 if (score >= 3 && score > matchScore) { // Threshold: need at least 3 points
                     matchScore = score;
                     existingReader = node;
                 }
                 
                 traverse(node->right);
             };
             
             extern PTRDG rootDocGia;
             traverse(rootDocGia);
             
             if (existingReader) {
                 // ✅ MERGE: Update missing info
                 bool updated = false;
                 QString updateLog = QString("[IMPORT] Duplicate found: %1 (MaThe: %2). Merging: ")
                     .arg(QString::fromStdString(existingReader->data.Ho + " " + existingReader->data.Ten))
                     .arg(existingReader->data.MaThe);
                 
                 if (existingReader->data.NgaySinh.empty() && !ngaySinhStd.empty()) {
                     existingReader->data.NgaySinh = ngaySinhStd;
                     updateLog += "Birthday, ";
                     updated = true;
                 }
                 
                 if (existingReader->data.SDT.empty() && !sdt.isEmpty()) {
                     existingReader->data.SDT = sdt.toStdString();
                     updateLog += QString("Phone(%1), ").arg(sdt);
                     updated = true;
                 }
                 
                 if (existingReader->data.Email.empty() && !email.isEmpty()) {
                     existingReader->data.Email = email.toStdString();
                     updateLog += QString("Email(%1), ").arg(email);
                     updated = true;
                 }
                 
                 if (existingReader->data.Lop.empty() && !lop.isEmpty()) {
                     existingReader->data.Lop = lopStd;
                     updateLog += QString("Class(%1), ").arg(lop);
                     updated = true;
                 }
                 
                 if (updated) {
                     // Sync to DB
                     TheDocGia updatedData = existingReader->data;
                     hieuChinhDocGia(rootDocGia, updatedData);
                     qDebug() << updateLog;
                     countSuccess++; // Count as success (merged)
                 } else {
                     qDebug() << "[IMPORT] Duplicate skipped (no new info):" 
                              << QString::fromStdString(existingReader->data.Ho + " " + existingReader->data.Ten);
                 }
             } else {
                 // ✅ NEW: Insert new reader
                 PTRDG node = taoDocGia(ho.toStdString(), ten.toStdString(), phai, 1, rootDocGia);
                 node->data.NgaySinh = ngaySinhStd;
                 node->data.Lop = lopStd;
                 node->data.SDT = sdt.toStdString();
                 node->data.Email = email.toStdString();
                 InsertDocGia(rootDocGia, node, true, false); // true = syncDB, false = don't emit signal yet
                 
                 qDebug() << "[IMPORT] ✓ New reader:" << (ho + " " + ten) << "- Class:" << lop;
                 countSuccess++;
             }
        }
    }
    
    if (type == "SACH") {
        DanhDauDuLieuThayDoi();
        // COMMIT BATCH TRANSACTION
        DatabaseManager::instance().commit();
        // EMIT SINGLE REFRESH SIGNAL
        EventBus::instance().dauSachAdded("");
    } else if (type == "DOCGIA") {
        saveDocGia(rootDocGia);
        // COMMIT BATCH TRANSACTION
        DatabaseManager::instance().commit();
         // Refresh readers
         EventBus::instance().docGiaAdded(0);
    }
    
    return true;
}
