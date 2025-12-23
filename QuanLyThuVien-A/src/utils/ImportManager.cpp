#include "utils/ImportManager.h"
#include "utils/FilePathManager.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "models/ThaoTacFile.h"
#include "models/NhapLieu.h" 
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>

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
    return QString::fromStdString(BoDauVaThuong(header.toStdString())).trimmed();
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
        schema["ISBN"] = {"isbn"};
        schema["TEN"] = {"tensach", "dausach", "tieude", "ten", "title"};
        schema["TACGIA"] = {"tacgia", "author"};
        schema["NAM"] = {"namxb", "namxuatban", "nam", "year"};
        schema["THELOAI"] = {"theloai", "category", "genre"};
        schema["GIA"] = {"giatien", "gia", "price", "dongia"};
        schema["SOLUONG"] = {"soluong", "sl", "quantity", "stock", "amount"};
        schema["NXB"] = {"nhaxuatban", "nxb", "publisher"};
        schema["VITRI"] = {"vitri", "location", "shelf", "ke"};
    } else {
        schema["HO"] = {"ho", "hodem", "last", "surname", "family"};
        schema["TEN"] = {"ten", "first", "given", "name"};
        schema["PHAI"] = {"phai", "gioitinh", "sex", "gender"};
        schema["NGAYSINH"] = {"ngaysinh", "dob", "birthday"};
        schema["LOP"] = {"lop", "class", "grade"};
        schema["SDT"] = {"sdt", "phone", "tel", "dienthoai"};
        schema["EMAIL"] = {"email", "thu"};
    }

    for (int i = 0; i < headers.size(); ++i) {
        QString n = normalizeHeader(headers[i]);
        if (n.isEmpty()) continue;
        for (auto it = schema.begin(); it != schema.end(); ++it) {
            for (const QString &kw : it.value()) {
                if (n.contains(kw) && !mapping.contains(it.key())) {
                    mapping[it.key()] = i + 1;
                    break;
                }
            }
        }
    }
    return mapping;
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
             out << "HoDem,Ten,GioiTinh,NgaySinh,Lop,SDT,Email,Valid\n";
        } else {
             // Book: ISBN, TenSach, ...
             out << "ISBN,TenSach,TacGia,NamXB,TheLoai,GiaTien,SoLuong,NhaXuatBan,ViTri,Valid\n";
        }

        for (const QStringList &row : data) {
            // Join with quotes
            QStringList quotedRow;
            for (const QString &field : row) {
                quotedRow << "\"" + field + "\"";
            }
            out << quotedRow.join(",") << "\n";
        }
        file.close();
        return path;
    }
    return "";
}

bool ImportManager::runApply(const QString &excelFile, const QString &mode, QWidget *parent) {
    Document doc(excelFile);
    if (!doc.load()) return false;

    if (!doc.selectSheet("Sheet1")) {
         QStringList sheets = doc.sheetNames();
         if (!sheets.isEmpty()) doc.selectSheet(sheets.first());
    }

    // 1. Scan Headers to build map
    QStringList headers;
    for (int col = 1; col <= 30; ++col) {
        auto cell = doc.cellAt(1, col);
        if (!cell) break;
        headers << cell->value().toString().trimmed();
    }
    QMap<QString, int> colMap = buildColumnMap(headers, mode);
    qDebug() << "[IMPORT] Apply Mapping:" << colMap;

    int row = 2;
    int maxRow = doc.dimension().lastRow();

    if (mode == "SACH") {
        int iISBN = colMap.value("ISBN", 1);
        int iTEN = colMap.value("TEN", 2);
        int iTACGIA = colMap.value("TACGIA", 3);
        int iNAM = colMap.value("NAM", 4);
        int iTHELOAI = colMap.value("THELOAI", 5);
        int iGIA = colMap.value("GIA", 6);
        int iSOLUONG = colMap.value("SOLUONG", 7);
        int iNXB = colMap.value("NXB", 8);
        int iVITRI = colMap.value("VITRI", 9);

        while (row <= maxRow) {
            QString isbn = doc.read(row, iISBN).toString().trimmed();
            QString ten = doc.read(row, iTEN).toString().trimmed();
            if (ten.isEmpty()) { row++; continue; }

            QString tg = doc.read(row, iTACGIA).toString().trimmed();
            int nam = doc.read(row, iNAM).toInt();
            QString tl = doc.read(row, iTHELOAI).toString().trimmed();
            long long gia = doc.read(row, iGIA).toLongLong();
            int sl = doc.read(row, iSOLUONG).toInt();
            QString nxb = doc.read(row, iNXB).toString().trimmed();
            QString vt = doc.read(row, iVITRI).toString().trimmed();

            ThemDauSachMoi(isbn.toStdString(), ten.toStdString(), 100, tg.toStdString(), nam, tl.toStdString(), sl, vt.toStdString(), gia, nxb.toStdString());
            row++;
        }
    } 
    else if (mode == "DOCGIA") {
        int iHO = colMap.value("HO", 1);
        int iTEN = colMap.value("TEN", 2);
        int iPHAI = colMap.value("PHAI", 3);
        int iNGAYSINH = colMap.value("NGAYSINH", 4);
        int iLOP = colMap.value("LOP", 5);
        int iSDT = colMap.value("SDT", 6);
        int iEMAIL = colMap.value("EMAIL", 7);

        while (row <= maxRow) {
            QString ho = doc.read(row, iHO).toString().trimmed();
            QString ten = doc.read(row, iTEN).toString().trimmed();
            if (ten.isEmpty()) { row++; continue; }

            QString phaiStr = doc.read(row, iPHAI).toString().toLower();
            bool phai = (phaiStr == "nu" || phaiStr == "female" || phaiStr == "1" || phaiStr.contains("nữ"));
            QString ngaySinh = doc.read(row, iNGAYSINH).toString().trimmed();
            QString lop = doc.read(row, iLOP).toString().trimmed();
            QString sdt = doc.read(row, iSDT).toString().trimmed();
            QString email = doc.read(row, iEMAIL).toString().trimmed();
            
            PTRDG node = taoDocGia(ho.toStdString(), ten.toStdString(), phai, 1, rootDocGia);
            node->data.NgaySinh = ngaySinh.toStdString();
            node->data.Lop = lop.toStdString();
            node->data.SDT = sdt.toStdString();
            node->data.Email = email.toStdString();
            InsertDocGia(rootDocGia, node);
            row++;
        }
    }
    DanhDauDuLieuThayDoi();
    return true;
}

bool ImportManager::runApplyData(const QList<QStringList> &data, const QString &type, const QString &mode, QWidget *parent) {
    if (data.isEmpty()) return false;
    
    int countSuccess = 0;
    
    // Iterate rows
    for (const QStringList &row : data) {
        if (row.isEmpty()) continue;
        
        // CHECK VALIDITY (Last Column)
        if (row.last().toUpper() == "FALSE" || row.last() == "0") {
            continue; // Skip invalid
        }
        
        if (type == "SACH") {
             // 0:ISBN, 1:Ten, 2:TacGia, 3:Nam, 4:TheLoai, 5:Gia, 6:Qty, 7:NXB, 8:ViTri
             if (row.size() < 9) continue;
             QString isbn = row[0];
             QString ten = row[1];
             QString tg = row[2];
             int nam = row[3].toInt();
             QString tl = row[4];
             long long gia = row[5].toLongLong();
             int sl = row[6].toInt();
             QString nxb = row[7];
             QString vt = row[8];
             
             if (ten.isEmpty()) continue;

             ThemDauSachMoi(isbn.toStdString(), ten.toStdString(), 100, 
                            tg.toStdString(), nam, tl.toStdString(), 
                            sl, vt.toStdString(), gia, nxb.toStdString());
             countSuccess++;
        }
        else if (type == "DOCGIA") {
             // 0:Ho, 1:Ten, 2:GT, 3:DOB, 4:Lop, 5:SDT, 6:Email
             if (row.size() < 7) continue;
             QString ho = row[0];
             QString ten = row[1];
             QString phaiStr = row[2].toLower();
             bool phai = (phaiStr.contains("nu") || phaiStr.contains("female") || phaiStr == "1");
             QString ngaySinh = row[3];
             QString lop = row[4];
             QString sdt = row[5];
             QString email = row[6];
             
             if (ten.isEmpty()) continue;
             
             PTRDG node = taoDocGia(ho.toStdString(), ten.toStdString(), phai, 1, rootDocGia);
             node->data.NgaySinh = ngaySinh.toStdString();
             node->data.Lop = lop.toStdString();
             node->data.SDT = sdt.toStdString();
             node->data.Email = email.toStdString();
             
             InsertDocGia(rootDocGia, node);
             countSuccess++;
        }
    }
    
    if (type == "SACH") {
        DanhDauDuLieuThayDoi();
    } else if (type == "DOCGIA") {
        saveDocGia(rootDocGia);
    }
    
    return true;
}
