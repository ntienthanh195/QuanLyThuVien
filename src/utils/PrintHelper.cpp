/**
 * @file PrintHelper.cpp
 * @path src/utils/PrintHelper.cpp
 * @description Hỗ trợ in ấn và xuất báo cáo (PDF/Printer).
 * Xử lý: tạo layout trang in, in danh sách học viên, danh sách sách, và phiếu mượn/trả.
 * @project Library Management System (QuanLyThuVien)
 */
#include "utils/PrintHelper.h"

#include "utils/FilePathManager.h"
#include <QPainter>
#include "qrcode/Code128.h"
#include "utils/QrCodeGenerator.h"
#include "utils/SettingsManager.h"
#include "utils/LanguageManager.h"  // Missing
#include "utils/CustomPrintDialog.h"  // Custom dark-themed print preview
#include <QPrinter>
#include <QPrintDialog>
#include <QPageSize>
#include <QPageLayout>
#include <QDateTime>
#include <QMap>
#include <QAbstractItemModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QApplication>
#include "models/UserManager.h"
#include <QMarginsF>
#include <QInputDialog> // Missing
#include <QTextDocument>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <QApplication>
#include <QDate>
#include <QtMath>
#include <QDesktopServices>
#include <QUrl>
#include <QTime>
#include <QFileDialog>
#include <QDir>
#include <QRegularExpression>
#include <functional> // For std::function
#include <QBuffer>
#include <QDateTime>
#include <QRandomGenerator>
#include <QPair>

// Static config
PrintHelper::PrintConfig PrintHelper::s_config;
static bool s_configLoaded = false; // Flag to track initialization

// Caching for Performance (Phase 10)
static QMap<QString, QString> s_barcodeCache;
static QMap<QString, QString> s_qrCache;

// Forward declarations
// static QImage generateCode39Image(const QString &text, int height);

// helper to ensure directories exist
/* static bool ensureDirExists(const QString &path) {
    QDir dir(path);
    if (dir.exists()) return true;
    return dir.mkpath(".");
} */

// ==================== PUBLIC API ====================

bool PrintHelper::inTheDocGia(PTRDG docGia, QWidget* parent) {
    if (!docGia) return false;
    
    // Generate card HTML
    QString cardHtml = taoHTMLTheDocGia(docGia);
    
    // Wrap card with centered container for single-card printing
    // This ensures the card appears in the center of A4 page instead of top-left
    QString html = QString(
        "<html><head><style>"
        "body { "
        "  margin: 0; "
        "  padding: 0; "
        "  background: white; "
        "  display: flex; "
        "  align-items: center; /* Canh giữa dọc */"
        "  justify-content: center; /* Canh giữa ngang */"
        "  min-height: 297mm; /* A4 height */"
        "  width: 210mm; /* A4 width */"
        "}"
        "</style></head><body>"
        "%1"
        "</body></html>"
    ).arg(cardHtml);
    
    QTextDocument doc;
    doc.setHtml(html);
    
    QString title = QString("Xem trước: Thẻ học viên - %1 %2")
        .arg(QString::fromStdString(docGia->data.Ho))
        .arg(QString::fromStdString(docGia->data.Ten));
    
    // Auto filename for PDF: The_Hoc_Vien_[Name]_[Code].pdf
    QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
    QString defaultFile = taoTenFileTheDocGia(docGia->data.MaThe, hoTen);
    
    return showPrintPreview(&doc, title, parent, defaultFile);
}

bool PrintHelper::inTheDocGiaTuDong(PTRDG docGia, QWidget* parent) {
    if (!docGia) return false;

    return inTheDocGia(docGia, parent);
}

bool PrintHelper::inNhieuTheDocGia(const QList<PTRDG>& danhSach, QWidget* parent) {
    if (danhSach.isEmpty()) return false;
    
    // Load Config
    PrintConfig cfg = getCauHinh();
    
    // Config-driven Grid Layout for student cards (A4 paper)
    // Allows customization via SettingsManager
    // TABLE-BASED LAYOUT (Compatible with QTextDocument)
    // Replaces CSS Grid/Flexbox which are not supported
    QString htmlToanBo = "<html><head><style>"
                         "body { margin: 0; padding: 10mm; background: white; }"
                         "</style></head><body>";
    
    // Start Table
    // Use fixed width table or 100%? 100% is safer for centering if columns match
    htmlToanBo += "<table width='100%' border='0' cellspacing='0' cellpadding='5'>"; // cellpadding roughly half of gap
    
    for (int i = 0; i < danhSach.size(); ++i) {
        // Start new row if needed
        if (i % cfg.cardCols == 0) {
            htmlToanBo += "<tr>";
        }
        
        // Table Cell
        double colWidthPercent = 100.0 / cfg.cardCols;
        htmlToanBo += QString("<td width='%1%' align='center' valign='top'>").arg(colWidthPercent);
        
        // Render Card
        htmlToanBo += taoHTMLTheDocGia(danhSach[i]);
        
        htmlToanBo += "</td>";
        
        // Close row if it's the last column OR last item
        bool isLastColumn = ((i + 1) % cfg.cardCols == 0);
        bool isLastItem = (i == danhSach.size() - 1);
        
        if (isLastColumn || isLastItem) {
             htmlToanBo += "</tr>";
        }
    }
    
    htmlToanBo += "</table></body></html>";
    
    QTextDocument doc;
    doc.setHtml(htmlToanBo);
    
    QString title = QString("Xem trước: %1 thẻ học viên").arg(danhSach.size());
    QString ngayFile = QDate::currentDate().toString("yyyyMMdd");
    QString defaultFile = QString("The_Hoc_Vien_Hang_Loat_%1the_%2.pdf").arg(danhSach.size()).arg(ngayFile);
    
    return showPrintPreview(&doc, title, parent, defaultFile);
}

bool PrintHelper::inA4NhieuThe(const QList<PTRDG>& danhSach, int soCot, QWidget* parent) {
    // Student cards always use 2-column grid layout for consistency
    // Ignore soCot parameter - always force 2 columns
    Q_UNUSED(soCot);
    
    return inNhieuTheDocGia(danhSach, parent);
}

bool PrintHelper::inHangLoatTheDocGia(PTRDG root, QWidget* parent) {
    if (!root) return false;

    // Flatten tree
    QList<PTRDG> list;
    std::function<void(PTRDG)> traverse = [&](PTRDG p) {
        if (!p) return;
        traverse(p->left);
        list.append(p);
        traverse(p->right);
    };
    traverse(root);

    return inA4NhieuThe(list, 2, parent); // Default 2 cols
}

bool PrintHelper::inTemMaVachSach(const QString& maSach, const QString& tenSach, QWidget* parent) {
    QString html = taoHTMLTemSach(maSach, tenSach);
    QTextDocument doc;
    doc.setHtml(html);
    
    QString question = QString("In tem cho sách: %1").arg(tenSach);
    return xuLyYeuCauIn(&doc, "In Tem Sách", question, taoTenFileTemSach(maSach, 1), parent, s_config.printerReceipt);

}

bool PrintHelper::inTemSachTuDong(PTRDS dauSach, int soLuong, QWidget* parent) {
    if (!dauSach) return false;
    
    int qty = soLuong;
    if (qty <= 0) {
        int totalCopies = dauSach->tongBanSao;
        if (totalCopies <= 0) totalCopies = 1;

        bool ok;
        qty = QInputDialog::getInt(parent, 
            LanguageManager::instance().getText("DIALOG_PRINT_LABELS"), 
            LanguageManager::instance().getText("MSG_ENTER_PRINT_QUANTITY"), 
            totalCopies, 1, 1000, 1, &ok);
        
        if (!ok) return false;
    }

    QList<QPair<PTRDS, int>> list;
    list.append(qMakePair(dauSach, qty));
    return inNhieuTemSachBatch(list, parent);
}

bool PrintHelper::inNhieuTemSach(PTRDS dauSach, int soLuong, QWidget* parent) {
    if (!dauSach || soLuong <= 0) return false;
    
    // Redirect to Batch function to ensure we use the high-quality grid layout (A4)
    QList<QPair<PTRDS, int>> list;
    list.append(qMakePair(dauSach, soLuong));
    return inNhieuTemSachBatch(list, parent);
}

bool PrintHelper::inTemSachTuyChon(PTRDS dauSach, const QStringList& codes, QWidget* parent) {
    if (!dauSach || codes.isEmpty()) return false;

    PrintConfig cfg = getCauHinh();
    QString tenSach = QString::fromStdString(dauSach->tenSach);
    
    // AUTO-CALCULATE GRID
    double pageWidth = 190.0;
    if (cfg.pageSize.toUpper() == "A4") {
        pageWidth = (cfg.orientation.toLower() == "landscape" ? 277.0 : 190.0);
    }

    int cols = cfg.labelCols;
    if (cols <= 0) {
        cols = qFloor((pageWidth + cfg.labelGapMm) / (cfg.labelWidth + cfg.labelGapMm));
        if (cols < 1) cols = 1;
    }

    QString html = "<html><head><style>body { margin: 0; padding: 0; background: white; }</style></head><body>";
    html += QString("<table width='100%' border='0' style='border-collapse: separate; border-spacing: %1mm; padding: 0;'>").arg(cfg.labelGapMm);
    
    int currentCell = 0;
    bool rowOpen = false;
    
    for (int k = 0; k < codes.size(); ++k) {
        if (currentCell % cols == 0) { 
            html += "<tr>"; 
            rowOpen = true; 
        }
        
        double colWidth = 100.0 / cols;
        html += QString("<td width='%1%' align='center' valign='top' style='padding:0;'>").arg(colWidth);
        
        QString priceStr = "";
        if (cfg.showPrice) {
             QLocale locale(QLocale::Vietnamese, QLocale::Vietnam);
             priceStr = locale.toCurrencyString((double)dauSach->giaTien, "đ");
        }

        html += taoHTMLTemSach(codes[k], tenSach, priceStr);
        html += "</td>";
        
        if ((currentCell + 1) % cols == 0) { 
            html += "</tr>"; 
            rowOpen = false; 
        }
        currentCell++;
    }
    
    if (rowOpen) {
        for (int m = currentCell % cols; m < cols; ++m) {
             html += "<td></td>";
        }
        html += "</tr>";
    }
    
    html += "</table></body></html>";
    
    QTextDocument doc;
    doc.setHtml(html);
    
    QString title = QString("In %1 tem tự chọn - %2").arg(codes.size()).arg(tenSach);
    QString defaultFile = taoTenFileTemSach(QString::fromStdString(dauSach->ISBN), codes.size());
    
    return showPrintPreview(&doc, title, parent, defaultFile);
}

QString PrintHelper::taoHTMLTemSach(const QString& maSach, const QString& tenSach, const QString& giaSach) {
    PrintConfig cfg = getCauHinh();
    
    int numBlocks = cfg.blocks.size();
    if (numBlocks == 0) {
        qWarning() << "[PrintHelper] ⚠️ Label layout is empty (no blocks defined)";
        return "";
    }

    // 1. Calculate space usage
    // We assume 1pt text roughly takes 0.6mm (safer than 0.35mm to account for padding/wrapping)
    double totalTextHeightMm = 0;
    int textBlockCount = 0;
    for (const auto& block : cfg.blocks) {
        if (block.type == "text") {
            totalTextHeightMm += (block.fontSize * 0.6); 
            textBlockCount++;
        }
    }

    double remainingHeightMm = cfg.labelHeight - totalTextHeightMm - 1.5; // 1.5mm safety
    if (remainingHeightMm < 6.0) remainingHeightMm = 6.0; // Force min barcode height (Scannable)

    double padding = (cfg.labelWidth < 25) ? 0.3 : 1.0; // Minimal padding for micro labels

    // 2. Start HTML
    // We use attributes like width='...' instead of style='width: ...' where possible
    // because QTextDocument (Qt's HTML renderer) respects attributes better than CSS.
    // We also use 'table-layout: fixed' to force dimensions.
    
    double usableWidthMm = cfg.labelWidth - (padding * 2.0);

    QString html = QString(
        "<div width='%1mm' height='%2mm' style='"
        " width: %1mm; height: %2mm;"
        " border: %3;" 
        " margin: 0; padding: %4mm;"
        " font-family: \"%5\", sans-serif;"
        " background: #fff;"
        " box-sizing: border-box;"
        "'>"
        "<table width='%6mm' border='0' cellspacing='0' cellpadding='0' style='table-layout: fixed; width: %6mm;'>"
    ).arg(cfg.labelWidth).arg(cfg.labelHeight)
     .arg(cfg.showBorder ? "0.1mm solid #000" : "none")
     .arg(padding)
     .arg(cfg.fontFamily)
     .arg(usableWidthMm);

    for (const auto& block : cfg.blocks) {
        QString content = "";
        
        if (block.type == "text") {
            if (block.field == "tenDonVi") content = cfg.tenVietTat.isEmpty() ? cfg.tenThuVien : cfg.tenVietTat;
            else if (block.field == "tenSach") content = tenSach;
            else if (block.field == "maBanSao") content = maSach;
            else if (block.field == "giaTien") content = giaSach;
            
            // DYNAMIC FONT SCALING for small labels
            int fontSize = block.fontSize;
            if (cfg.labelHeight < 12) fontSize = qMax(5, fontSize - 2);
            else if (cfg.labelHeight < 15) fontSize = qMax(6, fontSize - 1);

            // Removed white-space: nowrap to prevent horizontal expansion
            // Instead we use overflow: hidden (though partially ignored) and line-height control
            QString adjustedStyle = QString("font-size: %1pt; %2 text-align: %3; line-height: 1.1; overflow: hidden;")
                            .arg(fontSize).arg(block.bold ? "font-weight: bold;" : "").arg(block.textAlign);

            html += QString("<tr><td style='%1' width='%2mm'>%3</td></tr>")
                    .arg(adjustedStyle).arg(usableWidthMm).arg(content);
        } 
        else if (block.type == "barcode") {
            // FIXED: Ensure barcode has a dedicated row with fixed height to prevent layout collapse
            double hBarcode = remainingHeightMm; 
            double barcodeWidthMm = usableWidthMm * cfg.barcodeWidthRatio;
            
            // Safety: Barcode must have minimum height (5mm) for scanners
            if (hBarcode < 5.0) hBarcode = 5.0;

            QString barcodeBase64 = generateBarcodeBase64(maSach, hBarcode, cfg.barcodeDPI, barcodeWidthMm);
            
            // FIXED: Convert MM to Logical Pixels for HTML attributes (1mm ~ 3.78px at 96 DPI)
            // This ensures exact sizing because QTextDocument respects integer attributes better than CSS units.
            int wPx = qRound(barcodeWidthMm * 3.7795); 
            int hPx = qRound(hBarcode * 3.7795);

            html += QString(
                "<tr><td align='%1' valign='middle' style='padding: 1.0mm 0;' width='%2mm'>"
                "  <img src='data:image/png;base64,%3' width='%4' height='%5' style='display: block; margin: 0 auto; image-rendering: pixelated;' />"
                "</td></tr>"
            ).arg(block.textAlign).arg(usableWidthMm).arg(barcodeBase64).arg(wPx).arg(hPx);
        }
    }

    html += "</table></div>";
    return html;
}

bool PrintHelper::inNhieuTemSachBatch(const QList<QPair<PTRDS, int>>& list, QWidget* parent) {
    if (list.isEmpty()) return false;

    PrintConfig cfg = getCauHinh();
    
    // 1. AUTO-CALCULATE GRID
    // Assume A4 (210mm) if not specified. Subtract 20mm for margins (10mm each side).
    double pageWidth = 190.0; // Printable width default
    if (cfg.pageSize.toUpper() == "A4") {
        pageWidth = (cfg.orientation.toLower() == "landscape" ? 277.0 : 190.0);
    } else if (cfg.pageSize.toUpper() == "A5") {
        pageWidth = (cfg.orientation.toLower() == "landscape" ? 190.0 : 128.0);
    }

    int cols = cfg.labelCols;
    if (cols <= 0) {
        // cols = floor( (PageWidth + Gap) / (LabelWidth + Gap) )
        cols = qFloor((pageWidth + cfg.labelGapMm) / (cfg.labelWidth + cfg.labelGapMm));
        if (cols < 1) cols = 1;
    }

    // 2. Start HTML
    QString html = "<html><head><style>"
                   "body { margin: 0; padding: 0mm; background: white; }"
                   "td.end-marker { font-weight: bold; font-size: 10pt; color: #333; height: %1mm; }"
                   "</style></head><body>";
    html = html.arg(cfg.labelHeight);
                   
    html += QString("<table border='0' style='border-collapse: separate; border-spacing: %1mm; padding: 10mm 5mm;'>").arg(cfg.labelGapMm);
    
    int currentCell = 0;
    bool rowOpen = false;
    
    for (int i = 0; i < list.size(); ++i) {
        PTRDS dauSach = list[i].first;
        int soLuong = list[i].second;
        
        QString maSach = QString::fromStdString(dauSach->ISBN);
        QString tenSach = QString::fromStdString(dauSach->tenSach);
        
        QList<QString> copyCodes;
        if (dauSach->dms) {
            PTRDMS p = dauSach->dms;
            while (p && copyCodes.size() < soLuong) {
                copyCodes.append(QString::fromStdString(p->maSach));
                p = p->next;
            }
        }
        
        // Print Labels for current book
        for (int k = 0; k < soLuong; ++k) {
             QString codeToPrint = (k < copyCodes.size()) ? copyCodes[k] : maSach;

            if (currentCell % cols == 0) {
                html += "<tr>";
                rowOpen = true;
            }
            
            html += QString("<td width='%1mm' align='center' valign='top' style='padding:0; width:%1mm;'>").arg(cfg.labelWidth);
            
            QString priceStr = "";
            if (cfg.showPrice) {
                 QLocale locale(QLocale::Vietnamese, QLocale::Vietnam);
                 priceStr = locale.toCurrencyString((double)dauSach->giaTien, "đ");
            }

            html += taoHTMLTemSach(codeToPrint, tenSach, priceStr);
            html += "</td>";
            
            if ((currentCell + 1) % cols == 0) {
                html += "</tr>";
                rowOpen = false;
            }
            currentCell++;
        }

        // ADD "-- HẾT --" MARKER AFTER EACH BOOK BATCH (Matching sample image)
        if (currentCell % cols == 0) {
            html += "<tr>";
            rowOpen = true;
        }
        
        html += QString("<td width='%1mm' align='center' valign='middle' class='end-marker' style='width:%1mm;'>-- HẾT --</td>").arg(cfg.labelWidth);
        
        if ((currentCell + 1) % cols == 0) {
            html += "</tr>";
            rowOpen = false;
        }
        currentCell++;
    }
    
    // Final closing of the last row if still open
    if (rowOpen) {
        for (int m = currentCell % cols; m < cols; ++m) {
             html += "<td></td>";
        }
        html += "</tr>";
    }
    
    html += "</table></body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    
    QString title = "Xem trước: Tem mã vạch hàng loạt";
    
    // Auto filename with timestamp: Tem_Ma_Vach_Hang_Loat_[Date].pdf
    QString dateStr = QDateTime::currentDateTime().toString("yyyyMMdd_HHmm");
    QString defaultFile = QString("Tem_Ma_Vach_Hang_Loat_%1.pdf").arg(dateStr);
    
    return showPrintPreview(&doc, title, parent, defaultFile);
}

bool PrintHelper::inDanhSachSach(PTRDS* dsDauSach, int soLuong, QWidget* parent) {
    if (!dsDauSach || soLuong <= 0) return false;
    
    // Data Table (Black & White - Match Sample)
    QString noiDungBang = "<table border='1' cellspacing='0' cellpadding='8' width='100%'>"
                          "<thead>"
                          "<tr>"
                          "<th>STT</th>"
                          "<th>ISBN</th>"
                          "<th>Tên sách</th>"
                          "<th>Tác giả</th>"
                          "<th>Số bản</th>"
                          "</tr>"
                          "</thead>"
                          "<tbody>";
    
    for (int i = 0; i < soLuong; ++i) {
        PTRDS p = dsDauSach[i];
        if (!p) continue;
        
        QString isbn = QString::fromStdString(p->ISBN);
        QString ten = QString::fromStdString(p->tenSach);
        QString tacGia = QString::fromStdString(p->tacGia);
        int soBan = p->tongBanSao;
        
        noiDungBang += QString("<tr>"
                               "<td class='text-center'>%1</td>"
                               "<td class='text-center nowrap'><b>%2</b></td>"
                               "<td>%3</td>"
                               "<td>%4</td>"
                               "<td class='text-center'>%5</td>"
                               "</tr>")
                          .arg(i + 1).arg(isbn).arg(ten).arg(tacGia).arg(soBan);
    }
    
    noiDungBang += "</tbody></table>";
    
    // Wrap and Preview
    QString html = taoHTMLDanhSach("DANH SÁCH ĐẦU SÁCH", noiDungBang);
    QTextDocument doc;
    doc.setHtml(html);
    
    QString title = QString("Báo cáo: %1 đầu sách").arg(soLuong);
    return xuLyYeuCauIn(&doc, "DANH SÁCH SÁCH", title, taoTenFileBaoCao("DSSach"), parent, s_config.printerReport);
}

bool PrintHelper::inDanhMucSach(PTRDS* dsDauSach, int soLuong, QWidget* parent) {
    return inDanhSachSach(dsDauSach, soLuong, parent);
}

bool PrintHelper::inBaoCaoThongKe(const QString& tieuDe, const QString& noiDungHTML, QWidget* parent) {
    QString html = taoHTMLDanhSach(tieuDe, noiDungHTML);
    QTextDocument doc;
    doc.setHtml(html);
    return xuLyYeuCauIn(&doc, tieuDe, "In báo cáo thống kê", taoTenFileBaoCao("ThongKe"), parent);

}

bool PrintHelper::inPhieuMuonTra(PTRDG docGia, const QList<QString>& danhSachSach, bool laMuon, QWidget* parent) {
    if (!docGia) return false;
    
    QString loai = laMuon ? "PHIẾU MƯỢN SÁCH" : "PHIẾU TRẢ SÁCH";
    QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
    
    QString html = QString(
        "<html><head>"
        "<style>"
        "body { font-family: 'Siemreap', 'Khmer OS Siemreap', 'Times New Roman', serif; margin: 20px; }"
        ".header { text-align: center; font-size: 18pt; font-weight: bold; margin-bottom: 20px; }"
        "table { width: 100%%; border-collapse: collapse; margin-top: 20px; }"
        "td, th { border: 1px solid black; padding: 8px; }"
        ".info { margin: 10px 0; }"
        "</style>"
        "</head><body>"
        "<div class='header'>%1</div>"
        "<div class='info'><b>Họ tên:</b> %2</div>"
        "<div class='info'><b>Mã thẻ:</b> %3</div>"
        "<div class='info'><b>Ngày:</b> %4</div>"
        "<table><tr><th>STT</th><th>Tên sách</th></tr>"
    ).arg(loai).arg(hoTen).arg(docGia->data.MaThe).arg(QDate::currentDate().toString("dd/MM/yyyy"));
    
    for (int i = 0; i < danhSachSach.size(); ++i) {
        html += QString("<tr><td>%1</td><td>%2</td></tr>").arg(i + 1).arg(danhSachSach[i]);
    }
    
    html += "</table></body></html>";
    
    QTextDocument doc;
    doc.setHtml(html);
    return inDocument(&doc, parent);
}

bool PrintHelper::inDanhSachDocGia(PTRDG root, QWidget* parent) {
    if (!root) return false;
    
    // 1. Flatten Tree to List for Table
    QList<PTRDG> list;
    std::function<void(PTRDG)> traverse = [&](PTRDG p) {
        if (!p) return;
        traverse(p->left);
        list.append(p);
        traverse(p->right);
    };
    traverse(root);
    
    if (list.isEmpty()) return false;
    
    // 2. Summary Statistics
    int totalActive = 0, totalInactive = 0;
    int totalMale = 0, totalFemale = 0;
    int totalWithBooks = 0;
    
    for (PTRDG p : list) {
        if (p->data.TrangThai == 1) totalActive++; else totalInactive++;
        if (p->data.Phai == 0) totalMale++; else totalFemale++;
        if (p->data.soSachDangMuon > 0) totalWithBooks++;
    }
    
    // 3. Build HTML Content
    QString noiDungBang = "";
    
    // Summary Section (Clean Table Style)
    noiDungBang += QString(
        "<div style='margin-bottom: 20px; font-size: 11pt;'>"
        "<table width='100%' border='0' cellspacing='0' cellpadding='4'>"
        "<tr>"
        "  <td><b>Tổng số:</b> %1</td>"
        "  <td><b>Nam:</b> %2</td>"
        "  <td><b>Nữ:</b> %3</td>"
        "</tr>"
        "<tr>"
        "  <td><b>Hoạt động:</b> %4</td>"
        "  <td><b>Bị khóa:</b> %5</td>"
        "  <td><b>Đang mượn:</b> %6</td>"
        "</tr>"
        "</table>"
        "</div>"
    ).arg(list.size()).arg(totalMale).arg(totalFemale).arg(totalActive).arg(totalInactive).arg(totalWithBooks);

    // Data Table (Match Book Report Style)
    noiDungBang += "<table border='1' cellspacing='0' cellpadding='8' width='100%'>"
                   "<thead>"
                   "<tr>"
                   "<th>STT</th>"
                   "<th>Mã Thẻ</th>"
                   "<th>Họ Tên</th>"
                   "<th>Phái</th>"
                   "<th>Trạng Thái</th>"
                   "<th>Đang Mượn</th>"
                   "</tr>"
                   "</thead>"
                   "<tbody>";
                        
    for (int i = 0; i < list.size(); i++) {
        PTRDG p = list[i];
        QString hoTen = QString::fromStdString(p->data.Ho + " " + p->data.Ten);
        QString phai = p->data.Phai ? "Nữ" : "Nam";
        QString trangThai = (p->data.TrangThai == 1) ? "Hoạt động" : "Bị khóa";
        
        noiDungBang += QString("<tr>"
                               "<td class='text-center'>%1</td>"
                               "<td class='text-center nowrap'><b>%2</b></td>"
                               "<td>%3</td>"
                               "<td class='text-center'>%4</td>"
                               "<td class='text-center'>%5</td>"
                               "<td class='text-center'>%6</td>"
                               "</tr>")
            .arg(i + 1)
            .arg(p->data.MaThe)
            .arg(hoTen)
            .arg(phai)
            .arg(trangThai)
            .arg(p->data.soSachDangMuon > 0 ? QString::number(p->data.soSachDangMuon) + " cuốn" : "-");
    }
    noiDungBang += "</tbody></table>";
    
    // 4. Wrap with Standard Master Template
    QString html = taoHTMLDanhSach("DANH SÁCH HỌC VIÊN", noiDungBang);
    
    // 5. Build Document and Execute Print
    QTextDocument doc;
    doc.setHtml(html);
    
    QString statusMsg = QString("In báo cáo: %1 học viên").arg(list.size());
    return xuLyYeuCauIn(&doc, "BÁO CÁO HỌC VIÊN", statusMsg, taoTenFileBaoCao("DSHocVien"), parent, s_config.printerReport);
}

bool PrintHelper::inBienLaiPhat(PTRDG docGia, const QString& lyDo, long long soTien, QWidget* parent) {
    if (!docGia) return false;
    QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
    
    QString html = QString(
        "<html><head><style>"
        "body { font-family: 'Siemreap', 'Khmer OS Siemreap', 'Times New Roman', serif; padding: 20px; }"
        ".header { text-align: center; font-size: 16pt; font-weight: bold; margin-bottom: 20px; }"
        ".row { display: flex; justify-content: space-between; margin-bottom: 10px; }"
        "</style></head><body>"
        "<div class='header'>BIÊN LAI THU TIỀN PHẠT</div>"
        "<div><b>Họ tên học viên:</b> %1</div>"
        "<div><b>Mã thẻ:</b> %2</div>"
        "<div><b>Lý do phạt:</b> %3</div>"
        "<div><b>Số tiền:</b> %4 VNĐ</div>"
        "<br><br>"
        "<div style='text-align: right;'>Ngày %5 tháng %6 năm %7</div>"
        "<div style='display: flex; justify-content: space-between; margin-top: 30px;'>"
        "  <div style='text-align: center;'><b>Người nộp tiền</b><br>(Ký tên)</div>"
        "  <div style='text-align: center;'><b>Người thu tiền</b><br>(Ký tên)</div>"
        "</div>"
        "</body></html>"
    ).arg(hoTen)
     .arg(docGia->data.MaThe)
     .arg(lyDo)
     .arg(QLocale(QLocale::Vietnamese).toString(soTien))
     .arg(QDate::currentDate().day())
     .arg(QDate::currentDate().month())
     .arg(QDate::currentDate().year());
     
    QTextDocument doc;
    doc.setHtml(html);

    // Silent Audit Log (Phase 10)
    logReceiptToPDF(&doc, "BienLaiPhat");

    QString question = QString("In biên lai phạt cho: %1\nSố tiền: %2").arg(hoTen).arg(soTien);
    return xuLyYeuCauIn(&doc, "Biên Lai Phạt", question, taoTenFileBaoCao("Phat"), parent, s_config.printerReceipt);

}

// ===== HELPER FUNCTIONS =====
// Helper to generate barcode image (Code128 preferred)
// Parameters now use mm for physical dimensions, converted to px based on target DPI
// Helper to generate barcode image (Pure Pixel approach for 100% compatibility)
static QImage generateBarcodeImage(const QString &text, int height = 150) {
    QVector<bool> pattern = Code128::encode(text);
    if (pattern.isEmpty()) return QImage();

    // Industry Standard Reference: 3px per module, 10x quiet zone
    int moduleW = 3; 
    int quietZone = 10 * moduleW;
    int totalWidth = (pattern.size() * moduleW) + (2 * quietZone);
    
    // Use ARGB32 - Most compatible format for Qt renderers
    QImage image(totalWidth, height, QImage::Format_ARGB32);
    image.fill(Qt::white);
    
    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    
    int x = quietZone;
    for (bool isBar : pattern) {
        if (isBar) {
            painter.drawRect(x, 0, moduleW, height);
        }
        x += moduleW;
    }
    painter.end();
    
    return image;
}

// Helper to expose Barcode Base64 generation
QString PrintHelper::generateBarcodeBase64(const QString &text, double heightMm, int targetDPI, double maxWidthMm) {
    Q_UNUSED(heightMm); Q_UNUSED(targetDPI); Q_UNUSED(maxWidthMm);
    QString cleanText = text.trimmed();
    if (cleanText.isEmpty()) return "";

    if (s_barcodeCache.contains(cleanText)) return s_barcodeCache[cleanText];

    // Always generate high-res (150px) for cache
    QImage barcodeImg = generateBarcodeImage(cleanText, 150);
    if (barcodeImg.isNull()) return "";

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    barcodeImg.save(&buffer, "PNG");
    
    QString base64 = byteArray.toBase64();
    s_barcodeCache[cleanText] = base64;
    return base64;
}

QString PrintHelper::taoMaVachHTML(const QString& maSo, int fontSize) {
    QString base64 = generateBarcodeBase64(maSo, 0);
    if (base64.isEmpty()) return "";

    // Standard display height 40-50px. CSS will scale the 150px image down.
    int displayH = (fontSize > 0) ? fontSize : 40;
    
    return QString("<img src='data:image/png;base64,%1' style='height:%2px; max-width:100%%; image-rendering: pixelated; display: block; margin: 0 auto;' />")
            .arg(base64).arg(displayH);
}

QString PrintHelper::taoMaQRHTML(const QString& maSo, int size) {
    QString cacheKey = maSo + "_" + QString::number(size);
    QString base64;
    
    if (s_qrCache.contains(cacheKey)) {
        base64 = s_qrCache[cacheKey];
    } else {
        QImage qr = QrCodeGenerator::generateQrImage(maSo, size);
        if (qr.isNull()) return "";
        
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        qr.save(&buffer, "PNG");
        base64 = byteArray.toBase64();
        s_qrCache[cacheKey] = base64;
    }
    
    return QString("<img src='data:image/png;base64,%1' width='%2' height='%2' />")
           .arg(base64).arg(size);
}

// Helper to create file names (Sanitized for filesystem)
static QString sanitizeFileName(const QString& name) {
    QString safe = name;
    safe.replace(" ", "_");
    // Simple ASCII and common characters only
    safe.remove(QRegularExpression("[^a-zA-Z0-9_.-]"));
    return safe;
}

QString PrintHelper::taoTenFilePhieuMuon(int maThe, const QString& hoTen, const QString& maSach) {
    QString name = sanitizeFileName(hoTen);
    return QString("Phieu_Muon_%1_%2_%3_%4.pdf")
        .arg(name)
        .arg(maThe)
        .arg(maSach)
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmm"));
}

QString PrintHelper::taoTenFilePhieuTra(int maThe, const QString& hoTen, const QString& maSach) {
    QString name = sanitizeFileName(hoTen);
    return QString("Phieu_Tra_%1_%2_%3_%4.pdf")
        .arg(name)
        .arg(maThe)
        .arg(maSach)
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmm"));
}

QString PrintHelper::taoTenFileTheDocGia(int maThe, const QString& hoTen) {
    QString name = sanitizeFileName(hoTen);
    return QString("The_Hoc_Vien_%1_%2.pdf").arg(name).arg(maThe);
}

QString PrintHelper::taoTenFileTemSach(const QString& isbn, int soLuong) {
    return QString("Tem_Ma_Vach_Sach_%1_sl%2.pdf").arg(isbn).arg(soLuong);
}

QString PrintHelper::taoTenFileBaoCao(const QString& loaiBaoCao) {
    QString loai = sanitizeFileName(loaiBaoCao);
    return QString("Bao_Cao_%1_%2.pdf").arg(loai).arg(QDate::currentDate().toString("yyyyMMdd"));
}

bool PrintHelper::xuatPDF(QTextDocument* doc, const QString& tenFile, QWidget* parent) {
    if (!doc) return false;
    
    PrintConfig cfg = getCauHinh();

    // Ensure Directory Exists
    QFileInfo fileInfo(tenFile);
    if (!QDir().mkpath(fileInfo.absolutePath())) {
        qWarning() << "Failed to create directory:" << fileInfo.absolutePath();
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(tenFile);
    
    // 1. Page Size Mapping
    QPageSize::PageSizeId paperId = QPageSize::A4;
    QString ps = cfg.pageSize.toUpper();
    if (ps == "A3") paperId = QPageSize::A3;
    else if (ps == "A5") paperId = QPageSize::A5;
    else if (ps == "LETTER") paperId = QPageSize::Letter;
    else if (ps == "LEGAL") paperId = QPageSize::Legal;
    
    printer.setPageSize(QPageSize(paperId));
    
    // 2. Orientation Mapping
    QPageLayout::Orientation orient = QPageLayout::Portrait;
    if (cfg.orientation.toLower() == "landscape") {
        orient = QPageLayout::Landscape;
    }
    printer.setPageOrientation(orient);
    
    // 3. Margins (mm) - Read from config or defaults
    QSettings settings("HVPGCT", "QuanLyThuVien_V2");
    qreal mL = settings.value("printer_margin_left", 10).toDouble();
    qreal mR = settings.value("printer_margin_right", 10).toDouble();
    qreal mT = settings.value("printer_margin_top", 10).toDouble();
    qreal mB = settings.value("printer_margin_bottom", 10).toDouble();
    
    printer.setPageMargins(QMarginsF(mL, mT, mR, mB), QPageLayout::Millimeter);
    
    // 4. Color Mode
    printer.setColorMode(cfg.inMauSac ? QPrinter::Color : QPrinter::GrayScale);

    doc->print(&printer);
    
    QString absPath = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    QMessageBox::information(parent, "Xuất PDF", 
        "Đã lưu file thành công tại:\n" + absPath);
        
    QDesktopServices::openUrl(QUrl::fromLocalFile(absPath));
    return true;
}

bool PrintHelper::logReceiptToPDF(QTextDocument* doc, const QString& type) {
    if (!doc) return false;
    
    // Base log directory (Phase 11: Dynamic Path)
    QString dateFolder = QDate::currentDate().toString("yyyyMM");
    QString fullLogPath = FilePathManager::getPrintLogPath() + "/" + dateFolder;
    
    if (!QDir().mkpath(fullLogPath)) return false;
    
    QString fileName = QString("%1_%2_%3.pdf")
        .arg(type)
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"))
        .arg(QRandomGenerator::global()->bounded(1000)); // Add randomness to avoid collisions
        
    QString fullPath = fullLogPath + "/" + fileName;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fullPath);
    
    // Compact log format
    printer.setPageSize(QPageSize(QPageSize::A6));
    printer.setPageMargins(QMarginsF(5, 5, 5, 5), QPageLayout::Millimeter);

    doc->print(&printer);
    return true;
}

void PrintHelper::setCauHinh(const PrintConfig& config) {
    s_config = config;
    s_configLoaded = true; // Mark as loaded when set manually
}

void PrintHelper::reloadCauHinh() {
    qDebug() << "[PrintHelper] 🔄 Reloading configuration and clearing caches...";
    s_configLoaded = false; 
    s_barcodeCache.clear(); // CRITICAL: Clear cache so new dimensions/text take effect
    s_qrCache.clear();
    getCauHinh(); // Practical reload
}

// Flag moved to top

PrintHelper::PrintConfig PrintHelper::getCauHinh() {
    if (!s_configLoaded) {
        PrintHelper::PrintConfig cfg;
        
        cfg.tenThuVien = SettingsManager::instance().getString("Print/TenThuVien", "HỌC VIỆN PHẬT GIÁO NAM TÔNG KHMER");
        cfg.tenVietTat = SettingsManager::instance().getString("Print/TenVietTat", "HVPG NAMTONG");
        cfg.loiChao = SettingsManager::instance().getString("Print/LoiChao", "Xin cảm ơn!");
        
        cfg.inMauSac = SettingsManager::instance().getBool("Print/Color", false);
        cfg.hienThiLogo = SettingsManager::instance().getBool("Print/ShowLogo", false);
        cfg.duongDanLogo = SettingsManager::instance().getString("Print/LogoPath", "");
        cfg.silentPrint = SettingsManager::instance().getBool("Print/Silent", false);
        
        cfg.printerReceipt = SettingsManager::instance().getString("Print/Receipt", "");
        cfg.printerReport = SettingsManager::instance().getString("Print/Report", "");
        cfg.pdfOutputDir = SettingsManager::instance().getString("Print/PdfPath", "");
        cfg.qrOutputDir = SettingsManager::instance().getString("Print/QrPath", "");
        cfg.duongDanThuMucIn = SettingsManager::instance().getString("Print/InDir", "in");
        cfg.thuMucPdf = SettingsManager::instance().getString("Print/PdfSubDir", "pdf");
        cfg.thuMucQR = SettingsManager::instance().getString("Print/QrSubDir", "qr");
        
        cfg.pageSize = SettingsManager::instance().getString("Print/PageSize", "A4");
        cfg.orientation = SettingsManager::instance().getString("Print/Orientation", "Portrait");

        // ═══ TEM SÁCH ═══
        cfg.labelWidth = SettingsManager::instance().getInt("Print/BarcodeWidth", 38);
        cfg.labelHeight = SettingsManager::instance().getInt("Print/BarcodeHeight", 20);
        cfg.labelCols = SettingsManager::instance().getInt("Print/BarcodeCols", 4); 
        cfg.labelGapMm = SettingsManager::instance().getInt("Print/LabelGapMm", 5);
        cfg.showLibName = SettingsManager::instance().getBool("Print/ShowLibName", true);
        cfg.showBookTitle = SettingsManager::instance().getBool("Print/ShowBookTitle", true);
        cfg.showPrice = SettingsManager::instance().getBool("Print/ShowPrice", false);
        cfg.showBorder = SettingsManager::instance().getBool("Print/ShowBorder", true);
        cfg.fontFamily = SettingsManager::instance().getString("Print/FontFamily", "Times New Roman");
        cfg.barcodeWidthRatio = SettingsManager::instance().getDouble("Print/BarcodeWidthRatio", 0.9);

        // ═══ DYNAMIC LAYOUT RECONSTRUCTION ═══
        // Force rebuild of blocks list to ensure settings like showLibName take effect!
        cfg.blocks.clear();
        QString blocksJson = SettingsManager::instance().getString("Print/LabelBlocks", "");
        
        if (blocksJson.isEmpty() || blocksJson == "[]" || blocksJson == "{}") {
            qDebug() << "[PrintHelper] 🧩 Building dynamic layout (LibName:" << cfg.showLibName << ", Title:" << cfg.showBookTitle << ", Price:" << cfg.showPrice << ")";
            
            if (cfg.showLibName) cfg.blocks.append({"text", "tenDonVi", "top", 7, true, "center"});
            if (cfg.showBookTitle) cfg.blocks.append({"text", "tenSach", "middle", 9, true, "center"});
            
            // Barcode (Mandatory Line 3)
            cfg.blocks.append({"barcode", "maBanSao", "middle", 0, false, "center"});
            // Human Code (Line 4)
            cfg.blocks.append({"text", "maBanSao", "bottom", 8, true, "center"});
            
            if (cfg.showPrice) cfg.blocks.append({"text", "giaTien", "bottom", 8, true, "center"});
        } else {
            // Future JSON parsing logic here
        }
        
        // FAIL-SAFE: If after all logic we STILL don't have a barcode, force add it.
        bool hasBarcode = false;
        for (const auto& b : cfg.blocks) if (b.type == "barcode") { hasBarcode = true; break; }
        if (!hasBarcode) {
            qDebug() << "[PrintHelper] ⚠️ Barcode block missing from config, forcing add.";
            cfg.blocks.append({"barcode", "maBanSao", "middle", 0, false, "center"});
        }

        // ═══ THẺ HỌC VIÊN ═══
        cfg.cardWidth = SettingsManager::instance().getDouble("Print/CardWidth", 80.0);
        cfg.cardHeight = SettingsManager::instance().getDouble("Print/CardHeight", 50.0);
        cfg.cardCols = SettingsManager::instance().getInt("Print/CardCols", 2);
        cfg.cardGapMm = SettingsManager::instance().getInt("Print/CardGapMm", 5);
        cfg.cardValidityMonths = SettingsManager::instance().getInt("Print/CardValidityMonths", 12);
        
        cfg.barcodeDPI = SettingsManager::instance().getInt("Print/BarcodeDPI", 300);
        cfg.barcodeModuleWidthMm = SettingsManager::instance().getDouble("Print/BarcodeModuleWidthMm", 0.33);
        cfg.barcodeQuietZoneMm = SettingsManager::instance().getDouble("Print/BarcodeQuietZoneMm", 4.0);
        cfg.defaultBarcodeHeight = SettingsManager::instance().getInt("Print/DefaultBarcodeHeight", 14);

        cfg.reportShowSummary = SettingsManager::instance().getBool("Print/ReportShowSummary", true);

        s_config = cfg;
        s_configLoaded = true;
    }
    return s_config;
}

// ==================== MISSING IMPLEMENTATIONS (ADDED TO FIX LINKER ERRORS) ====================

bool PrintHelper::inDocument(QTextDocument* doc, QWidget* parent, const QString& printerName) {
    if (!doc) return false;
    
    QPrinter printer(QPrinter::HighResolution);
    
    // Check Config for color
    if (s_config.inMauSac) {
        #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        printer.setColorMode(QPrinter::Color);
        #else
        printer.setColorMode(QPrinter::Color);
        #endif
    } else {
        #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        printer.setColorMode(QPrinter::GrayScale);
        #else
        printer.setColorMode(QPrinter::GrayScale);
        #endif
    }
    
    // Set Printer Name if specified (for Silent Print or Default Dialog)
    if (!printerName.isEmpty()) {
        printer.setPrinterName(printerName);
    }
    
    // Check Silent Print OR if Printer Name is already explicitly provided (implies selection made)
    if (s_config.silentPrint || !printerName.isEmpty()) {
        // Silent print or Pre-selected print skips native QPrintDialog
        doc->print(&printer);
        return true;
    } else {
        QPrintDialog dialog(&printer, parent);
        dialog.setWindowTitle("Chọn Máy In");
        if (dialog.exec() != QDialog::Accepted) {
            return false;
        }
    }
    
    doc->print(&printer);
    return true;
}

QString PrintHelper::taoHTMLA4NhieuThe(const QList<PTRDG>& danhSach, int soCot) {
    if (danhSach.isEmpty()) return "";
    
    // Start Table
    QString html = "<html><head><style>"
                   "body { font-family: 'Siemreap', 'Khmer OS Siemreap', 'Segoe UI', sans-serif; }"
                   "table { width: 100%; border-collapse: collapse; }"
                   "td { padding: 5px; vertical-align: top; }"
                   "</style></head><body>";
                   
    html += "<table>";
    
    int count = 0;
    for (int i = 0; i < danhSach.size(); ++i) {
        if (count % soCot == 0) html += "<tr>"; // New row
        
        html += "<td style='padding: 5px;'>"; // Use padding between cards on A4
        html += taoHTMLTheDocGia(danhSach[i]);
        html += "</td>";
        
        if (count % soCot == soCot - 1) html += "</tr>"; // End row
        count++;
    }
    
    // Close last row if needed
    if (count % soCot != 0) {
        // Fill empty cells
        for (int k = count % soCot; k < soCot; ++k) {
            html += "<td></td>";
        }
        html += "</tr>";
    }
    
    html += "</table></body></html>";
    return html;
}

QString PrintHelper::taoHTMLDanhSach(const QString& tieuDe, const QString& noiDungBang) {
    PrintConfig cfg = getCauHinh();
    QString currentTime = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
    
    // Process noiDungBang to ensure tables have the report-table class if possible
    QString content = noiDungBang;
    if (!content.contains("class=")) {
        content.replace("<table>", "<table class='report-table'>");
    }

    return QString(
        "<html><head><style>"
        "body { font-family: '%1', 'Siemreap', 'Segoe UI', serif; font-size: 11pt; color: #000; margin: 15mm; }"
        "table { border-collapse: collapse; width: 100%%; margin-bottom: 20px; }"
        "table[border='1'] { border: 1px solid #000; }"
        "table[border='1'] th { background-color: #ffffff; border: 1px solid #000; padding: 10px 5px; font-weight: bold; text-align: center; color: #000; }"
        "table[border='1'] td { border: 1px solid #000; padding: 8px 5px; vertical-align: middle; background-color: #ffffff; color: #000; }"
        ".no-border, .no-border td, .no-border th { border: none !important; border-width: 0px !important; }"
        ".header-container { display: table-header-group; }"
        ".header-content { border-bottom: 2px solid #000; padding-bottom: 10px; margin-bottom: 15px; }"
        ".org-name { font-weight: bold; font-size: 13pt; text-transform: uppercase; color: #000; }"
        ".report-title { font-weight: bold; font-size: 18pt; margin: 20px 0; text-transform: uppercase; text-align: center; color: #000; }"
        ".footer { margin-top: 40px; text-align: right; font-size: 10.5pt; page-break-inside: avoid; }"
        ".signature-box { margin-top: 25px; text-align: center; float: right; width: 250px; color: #000; }"
        ".nowrap { white-space: nowrap; }"
        ".text-center { text-align: center; }"
        "</style></head><body>"
        
        "<table class='no-border' border='0' style='width: 100%%;'>"
        "  <thead class='header-container'>"
        "    <tr><td style='border: none;'>"
        "      <div class='header-content'>"
        "        <table width='100%%' style='border: none;'>"
        "          <tr>"
        "            <td style='border: none; padding: 0;'>"
        "              <div class='org-name'>%2</div>"
        "              <div style='font-size: 9pt; color: #555;'>Hệ thống Quản lý Thư viện • %3</div>"
        "            </td>"
        "            <td style='border: none; padding: 0; text-align: right; font-size: 8.5pt; color: #777; vertical-align: bottom;'>"
        "              Ngày in: %4"
        "            </td>"
        "          </tr>"
        "        </table>"
        "      </div>"
        "    </td></tr>"
  "  </thead>"
        "  <tbody>"
        "    <tr><td style='border: none;'>"
        "      <div class='report-title'>%5</div>"
        "      <div class='report-body'>%6</div>"
        "      <div class='footer'>"
        "        <div>Cần Thơ, ngày %7 tháng %8 năm %9</div>"
        "        <div class='signature-box'>"
        "          <b>Người Lập Biểu</b><br>"
        "          <div style='height: 80px;'></div>"
        "          <i>(Ký và ghi rõ họ tên)</i>"
        "        </div>"
        "        <div style='clear: both;'></div>"
        "      </div>"
        "    </td></tr>"
        "  </tbody>"
        "</table>"
        "</body></html>"
    ).arg(cfg.fontFamily)
     .arg(cfg.tenThuVien)
     .arg("Báo cáo Nghiệp vụ")
     .arg(currentTime)
     .arg(tieuDe)
     .arg(content)
     .arg(QDate::currentDate().day())
     .arg(QDate::currentDate().month())
     .arg(QDate::currentDate().year());
}

bool PrintHelper::kiemTraFontMaVach() {
    return true; 
}

bool PrintHelper::inBaoCaoTuyChinh(const QString& tieuDe, const QString& html, QWidget* parent) {
    QTextDocument doc;
    doc.setHtml(html);
    QString safeTitle = tieuDe;
    safeTitle.replace(" ", "");
    // Use generic printer or report printer if defined
    return xuLyYeuCauIn(&doc, tieuDe, QString("In báo cáo: %1").arg(tieuDe), taoTenFileBaoCao(safeTitle), parent, s_config.printerReport);
}

#include <QPrintPreviewDialog>
#include <QPainter>
#include <QMap>

// Helper to generate Code39 Image
/* static QImage generateCode39Image(const QString &text, int height = 50) {
    // Code 39 Table (1 = Wide, 0 = Narrow). Pattern: B S B S B S B S B (Bar Space...)
    static QMap<QChar, QString> code39Table = {
        {'0', "000110100"}, {'1', "100100001"}, {'2', "001100001"}, {'3', "101100000"},
        {'4', "000110001"}, {'5', "100110000"}, {'6', "001110000"}, {'7', "000100101"},
        {'8', "100100100"}, {'9', "001100100"}, {'A', "100001001"}, {'B', "001001001"},
        {'C', "101001000"}, {'D', "000011001"}, {'E', "100011000"}, {'F', "001011000"},
        {'G', "000001101"}, {'H', "100001100"}, {'I', "001001100"}, {'J', "000011100"},
        {'K', "100000011"}, {'L', "001000011"}, {'M', "101000010"}, {'N', "000010011"},
        {'O', "100010010"}, {'P', "001010010"}, {'Q', "000000111"}, {'R', "100000110"},
        {'S', "001000110"}, {'T', "000010110"}, {'U', "110000001"}, {'V', "011000001"},
        {'W', "111000000"}, {'X', "010010001"}, {'Y', "110010000"}, {'Z', "011010000"},
        {'-', "010000101"}, {'.', "110000100"}, {' ', "011000100"}, {'*', "010010100"},
        {'$', "010101000"}, {'/', "010100010"}, {'+', "010001010"}, {'%', "000101010"}
    };

    QString safeText = "*" + text.toUpper() + "*"; // Add start/stop chars
    
    // Calculate width
    int narrowW = 2; // px
    int wideW = narrowW * 2.5; // ratio 2.5:1
    int gap = narrowW; // char gap
    
    int totalWidth = 0;
    for(QChar c : safeText) {
        if (!code39Table.contains(c)) continue;
        QString pattern = code39Table[c];
        for(int i=0; i<9; i++) {
            totalWidth += (pattern[i] == '1') ? wideW : narrowW;
        }
        totalWidth += gap;
    }
    
    QImage image(totalWidth + 20, height, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    
    int x = 10;
    for(QChar c : safeText) {
        if (!code39Table.contains(c)) continue;
        QString pattern = code39Table[c];
        for(int i=0; i<9; i++) {
            int w = (pattern[i] == '1') ? wideW : narrowW;
            if (i % 2 == 0) { // Bar
                painter.drawRect(x, 0, w, height);
            }
            // else Space (skip)
            x += w;
        }
        x += gap;
    }
    
    return image;
} */

bool PrintHelper::xuLyYeuCauIn(QTextDocument* doc, const QString& tieuDe, const QString& cauHoi, const QString& tenFilePdfMacDinh, QWidget* parent, const QString& printerName, bool forcePreview) {
    Q_UNUSED(cauHoi); // Mark unused parameter
    if (!doc) return false;

    // 1. Silent Print (In tự động không hỏi) - ONLY if Not Forced Preview
    if (s_config.silentPrint && !forcePreview) {
        return inDocument(doc, parent, printerName);
    }

    // 2. Use Custom Print Dialog with meaningful filename
    CustomPrintDialog dialog(doc, tieuDe, tenFilePdfMacDinh, printerName, parent);
    int result = dialog.exec();
    
    if (result == CustomPrintDialog::PRINT_ACCEPTED) {
        // Force printer selection from dialog
        return inDocument(doc, parent, dialog.selectedPrinterName());
    } else if (result == CustomPrintDialog::PDF_EXPORT) {
        return xuatPDF(doc, dialog.exportPath(), parent);
    }

    return false;
}

bool PrintHelper::inPhieuMuonTuDong(PTRDG docGia, const QString& maSach, const QString& tenSach, const QString& ngayMuon, QWidget* parent, bool forcePreview) {
    QString html = taoHTMLPhieuMuon(docGia, maSach, tenSach, ngayMuon);
    QTextDocument doc;
    doc.setHtml(html);
    
    // Silent Audit Log (Phase 10)
    logReceiptToPDF(&doc, "PhieuMuon");

    QString question = QString("In phiếu mượn cho: %1\nSách: %2").arg(QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten)).arg(tenSach);
    QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
    return xuLyYeuCauIn(&doc, "In Phiếu Mượn", question, taoTenFilePhieuMuon(docGia->data.MaThe, hoTen, maSach), parent, s_config.printerReceipt, forcePreview);
}

bool PrintHelper::inPhieuTraTuDong(PTRDG docGia, const QString& maSach, const QString& tenSach, const QString& ngayTra, QWidget* parent, bool forcePreview) {
    QString html = taoHTMLPhieuMuon(docGia, maSach, tenSach, ngayTra);
    QTextDocument doc;
    doc.setHtml(html);

    // Silent Audit Log (Phase 10)
    logReceiptToPDF(&doc, "PhieuTra");

    QString question = QString("In phiếu trả cho: %1\nSách: %2").arg(QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten)).arg(tenSach);
    QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
    return xuLyYeuCauIn(&doc, "In Phiếu Trả", question, taoTenFilePhieuTra(docGia->data.MaThe, hoTen, maSach), parent, s_config.printerReceipt, forcePreview);
}

// ===== PRIVATE HELPERS =====

QString PrintHelper::taoHTMLTheDocGia(PTRDG docGia) {
    if (!docGia) return "";
    
    // Load Config
    PrintConfig cfg = getCauHinh();
    
    // Generate QR with fixed size
    QString qrCode = taoMaQRHTML(QString::number(docGia->data.MaThe), 110);
    
    // Config: Validity Duration
    QDate issueDate = QDate::currentDate();
    QDate expiryDate = issueDate.addMonths(cfg.cardValidityMonths);
    
    // CARD DESIGN: Config-driven dimensions
    // Pure B&W design for professional printing
    // CARD DESIGN: Config-driven dimensions
    // Pure B&W design for professional printing
    // REFACTOR: Use HTML Tables instead of Flexbox for internal layout (Qt requirement)
    return QString(
        "<div style='"
        " width: %1mm; height: %2mm;"
        " border: 2px solid #000;"         // Strong border
        " padding: 3mm;"
        " font-family: 'Arial', sans-serif;"
        " background-color: #fff;"
        " box-sizing: border-box;"
        " page-break-inside: avoid;"
        "'>"
        
        // --- HEADER (B&W) ---
        "  <div style='text-align: center; border-bottom: 2px solid #000; padding-bottom: 2mm; margin-bottom: 2mm;'>"
        "    <div style='font-size: 6pt; font-weight: bold; text-transform: uppercase; color: #000;'>%3</div>"
        "    <div style='font-size: 12pt; font-weight: bold; text-transform: uppercase; color: #000; letter-spacing: 1px;'>THẺ HỌC VIÊN</div>"
        "  </div>"

        // --- BODY (Table Layout) ---
        "  <table width='100%' border='0' cellspacing='0' cellpadding='0' style='height: 65%;'>"
        "    <tr>"
        
        // LEFT CELL: QR Code & ID
        "      <td width='40%' align='center' valign='middle' style='padding-right: 2mm;'>"
        "        <div style='margin-bottom: 2mm;'>%4</div>"
        "        <div style='font-size: 10pt; font-weight: bold; color: #000;'>%5</div>"
        "      </td>"

        // RIGHT CELL: Student Details
        "      <td width='60%' valign='middle' style='padding-left: 3mm;'>"
        "        <div style='font-size: 10pt; font-weight: bold; color: #000; text-transform: uppercase; margin-bottom: 2mm; line-height: 1.2;'>%6 %7</div>"
        "        <div style='font-size: 8pt; margin-bottom: 1.5mm; color: #000;'>Giới tính: <b>%8</b></div>"
        "        <div style='font-size: 8pt; margin-bottom: 1.5mm; color: #000;'>Ngày cấp: <b>%9</b></div>"
        "        <div style='font-size: 8pt; font-weight: bold; color: #000; border-top: 1px solid #000; padding-top: 1mm; margin-top: 1mm;'>Hết hạn: %10</div>"
        "      </td>"
        
        "    </tr>"
        "  </table>"
        
        "</div>"
    ).arg(cfg.cardWidth)
     .arg(cfg.cardHeight)
     .arg(cfg.tenThuVien)
     .arg(qrCode)
     .arg(docGia->data.MaThe)
     .arg(QString::fromStdString(docGia->data.Ho))
     .arg(QString::fromStdString(docGia->data.Ten))
     .arg(docGia->data.Phai ? "Nữ" : "Nam")
     .arg(issueDate.toString("dd/MM/yyyy"))
     .arg(expiryDate.toString("dd/MM/yyyy"));
}

QString PrintHelper::taoHTMLPhieuMuon(PTRDG docGia, const QString& maSach, const QString& tenSach, const QString& ngayGiaoDich) {
    if (!docGia) return "";
    
    // Use Code128 barcode instead of QR for faster scanning
    // Reduced height to match new compact params
    QString barcodeBook = taoMaVachHTML(maSach, 50);      
    QString barcodeCard = taoMaVachHTML(QString::number(docGia->data.MaThe), 40); 
    
    // Receipt Style (K80)
    // REFACTOR: Use Tables instead of Flexbox/Grid for compatibility
    return QString(
        "<html><head><style>"
        "body { font-family: 'Segoe UI', 'Arial', sans-serif; font-size: 10pt; margin: 0; padding: 10mm 5mm; width: 72mm; color: #000; background: #fff; }"
        ".header { text-align: center; border-bottom: 2px solid #000; padding-bottom: 8px; margin-bottom: 12px; }"
        ".org-name { font-weight: bold; font-size: 10pt; text-transform: uppercase; }"
        ".title { font-size: 16pt; font-weight: 900; margin: 8px 0; text-transform: uppercase; }"
        ".datetime { font-size: 9pt; color: #333; }"
        ".section { margin: 15px 0; }"
        "table { width: 100%; border-collapse: collapse; margin-bottom: 8px; }"
        "td { vertical-align: top; }"
        ".label { font-weight: bold; color: #000; text-align: left; }"
        ".value { text-align: right; color: #000; }"
        ".divider { border-top: 1px dashed #000; margin: 15px 0; }"
        ".barcode-section { text-align: center; margin: 15px 0; background: #fff; padding: 5px; border: 1px solid #eee; }"
        ".barcode-label { font-size: 9pt; color: #333; margin-bottom: 6px; font-weight: bold; }"
        ".footer { text-align: center; font-size: 9pt; font-style: italic; margin-top: 20px; color: #666; border-top: 1px solid #000; padding-top: 10px; }"
        ".cutter-mark { border-top: 1px dashed #ccc; margin-top: 20mm; text-align: center; font-size: 8pt; color: #999; }"
        "</style></head><body>"
        
        // Header
        "<div class='header'>"
        "  <div class='org-name'>%1</div>"
        "  <div class='title'>PHIẾU MƯỢN/TRẢ</div>"
        "  <div class='datetime'>GD: %2</div>"
        "</div>"
        
        // Student Info
        "<div class='section'>"
        "  <table border='0'><tr>"
        "    <td class='label'>Độc giả:</td>"
        "    <td class='value'>%3 %4</td>"
        "  </tr></table>"
        "  <div class='barcode-section'>"
        "    <div class='barcode-label'>MÃ THẺ</div>"
        "    %5"
        "    <div style='font-size: 9pt; font-weight: bold; margin-top: 2px;'>%6</div>"
        "  </div>"
        "</div>"
        
        "<div class='divider'></div>"
        
        // Book Info
        "<div class='section'>"
        "  <div style='font-weight: bold; margin-bottom: 4px;'>Tên sách:</div>"
        "  <div style='font-size: 9pt; margin-bottom: 15px; font-weight: 600;'>%7</div>"
        "  <div class='barcode-section'>"
        "    <div class='barcode-label'>MÃ SÁCH</div>"
        "    %8"
        "    <div style='font-size: 9pt; font-weight: bold; margin-top: 2px;'>%9</div>"
        "  </div>"
        "</div>"
        
        // Footer
        "<div class='footer'>"
        "  <div>%10</div>"
        "  <div style='margin-top: 4px;'>%11</div>"
        "</div>"
        
        "<div class='cutter-mark'>- - - DẤU CẮT / CUT HERE - - -</div>"
        
        "</body></html>"
    ).arg(SettingsManager::instance().getString("Print/TenThuVien", s_config.tenThuVien))
     .arg(ngayGiaoDich)
     .arg(QString::fromStdString(docGia->data.Ho))
     .arg(QString::fromStdString(docGia->data.Ten))
     .arg(barcodeCard)
     .arg(docGia->data.MaThe)
     .arg(tenSach)
     .arg(barcodeBook)
     .arg(maSach)
     .arg(SettingsManager::instance().getString("Print/TenThuVien", s_config.tenThuVien))
     .arg(SettingsManager::instance().getString("Print/LoiChao", "Xin cảm ơn!"));
}

// ==================== BATCH RECEIPT PRINTING ====================
bool PrintHelper::inPhieuMuonBatch(PTRDG docGia, const QList<ThongTinSachDangMuon_DTO>& danhSachSach, QWidget* parent) {
    if (!docGia || danhSachSach.isEmpty()) return false;
    
    // Receipt Header
    QString html = QString(
        "<html><head><style>"
        "body { font-family: 'Segoe UI', 'Arial', sans-serif; font-size: 10pt; margin: 0; padding: 8px; width: 72mm; color: #000; background: #fff; }"
        ".header { text-align: center; border-bottom: 2px solid #000; padding-bottom: 8px; margin-bottom: 12px; }"
        ".org-name { font-weight: bold; font-size: 9pt; text-transform: uppercase; }"
        ".title { font-size: 15pt; font-weight: 900; margin: 6px 0; text-transform: uppercase; }"
        ".datetime { font-size: 8pt; color: #555; }"
        ".section { margin: 10px 0; }"
        ".row { margin-bottom: 6px; display: flex; justify-content: space-between; }"
        ".label { font-weight: bold; color: #333; }"
        ".value { text-align: right; color: #000; }"
        ".divider { border-top: 1px dashed #000; margin: 12px 0; }"
        ".book-item { margin-bottom: 8px; padding-bottom: 8px; border-bottom: 1px dotted #ccc; }"
        ".book-title { font-weight: 600; font-size: 9pt; display: block; margin-bottom: 3px; }"
        ".book-code { font-family: 'Consolas', monospace; font-size: 8pt; color: #444; }"
        ".footer { text-align: center; font-size: 8pt; font-style: italic; margin-top: 15px; color: #666; border-top: 1px solid #ddd; padding-top: 8px; }"
        "</style></head><body>"
        
        // Header
        "<div class='header'>"
        "  <div class='org-name'>%1</div>"
        "  <div class='title'>PHIẾU MƯỢN SÁCH</div>"
        "  <div class='datetime'>Ngày: %2</div>"
        "</div>"
        
        // Reader Info
        "<div class='section'>"
        "  <div class='row'><span class='label'>Độc giả:</span><span class='value'>%3 %4</span></div>"
        "  <div class='row'><span class='label'>Mã thẻ:</span><span class='value'>%5</span></div>"
        "</div>"
        
        "<div class='divider'></div>"
        "<div><b>DANH SÁCH SÁCH MƯỢN (%6 cuốn)</b></div><br>"
    ).arg(SettingsManager::instance().getString("Print/TenThuVien", s_config.tenThuVien))
     .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"))
     .arg(QString::fromStdString(docGia->data.Ho))
     .arg(QString::fromStdString(docGia->data.Ten))
     .arg(docGia->data.MaThe)
     .arg(danhSachSach.size());

    // Loop through books
    for (const auto& sach : danhSachSach) {
        QString maSach = QString::fromStdString(sach.maSach);
        QString tenSach = QString::fromStdString(sach.tenSach);
        
        html += QString(
            "<div class='book-item'>"
            "  <div class='book-title'>%1</div>"
            "  <div class='book-code'>Mã: %2</div>"
            "</div>"
        ).arg(tenSach).arg(maSach);
    }

    // Footer
    html += QString(
        "<div class='footer'>"
        "  <div>%1</div>"
        "  <div style='margin-top: 4px;'>%2</div>"
        "</div>"
        "</body></html>"
    ).arg(SettingsManager::instance().getString("Print/TenThuVien", s_config.tenThuVien))
     .arg(SettingsManager::instance().getString("Print/LoiChao", "Xin cảm ơn!"));

    QTextDocument doc;
    doc.setHtml(html);
    
    // Silent Audit Log (Phase 10)
    logReceiptToPDF(&doc, "PhieuMuonBatch");

    QString title = QString("Xem trước: Phiếu mượn (%1 sách)").arg(danhSachSach.size());
    QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
    QString defaultFile = QString("Phieu_Muon_Hang_Loat_%1_%2.pdf").arg(sanitizeFileName(hoTen)).arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmm"));
    
    return showPrintPreview(&doc, title, parent, defaultFile);
}

// ==================== NEW: PRINT PREVIEW FUNCTION ====================

bool PrintHelper::showPrintPreview(QTextDocument* doc, const QString& title, QWidget* parent, const QString& defaultFileName) {
    if (!doc) {
        qWarning() << "showPrintPreview: Document is null!";
        return false;
    }
    
    // Use custom dark-themed print dialog with inline preview (much prettier!)
    CustomPrintDialog dialog(doc, title, defaultFileName, s_config.printerReport, parent);
    
    int result = dialog.exec();
    
    // Check result
    if (result == CustomPrintDialog::PRINT_ACCEPTED) {
        qDebug() << "User printed document";
        return true;
    } else if (result == CustomPrintDialog::PDF_EXPORT) {
        qDebug() << "User exported to PDF:" << dialog.exportPath();
        return xuatPDF(doc, dialog.exportPath(), parent);
    } else {
        qDebug() << "User cancelled";
        return false;
    }
}

bool PrintHelper::xuatLichSuMuonTraToPDF(PTRDG docGia, QWidget* parent) {
    if (!docGia) return false;

    // 1. Build Reader Info HTML
    QString infoHtml = QString(
        "<h2>Thông tin Học viên</h2>"
        "<p><b>Mã số:</b> %1</p>"
        "<p><b>Họ tên:</b> %2 %3</p>"
        "<p><b>Lớp:</b> %4</p>"
    ).arg(docGia->data.MaThe)
     .arg(QString::fromStdString(docGia->data.Ho))
     .arg(QString::fromStdString(docGia->data.Ten))
     .arg(QString::fromStdString(docGia->data.Lop));

    // 2. Build History Table HTML
    QString tableHtml = 
        "<br><h2>Lịch sử Mượn Trả</h2>"
        "<table border='1' cellspacing='0' cellpadding='5' width='100%' style='border-collapse: collapse;'>"
        "<tr style='background-color: #f2f2f2;'>"
        "<th>Mã Sách</th><th>Ngày Mượn</th><th>Ngày Trả</th><th>Trạng Thái</th><th>Ghi Chú</th>"
        "</tr>";

    MUONTRA p = docGia->data.dsmt;
    while (p) {
        QString statusStr = "";
        int st = p->data.TrangThai;
        if (st == 0) statusStr = "Dang muon";
        else if (st == 1) statusStr = "Da tra";
        else if (st == 2) statusStr = "Mat";

        // Calculate overdue on the fly
        QDate hanTra = QDate::fromString(QString::fromStdString(p->data.NgayTra), "dd/MM/yyyy");
        int soNgayQH = 0;
        if (st == 0) {
            QDate today = QDate::currentDate();
            if (today > hanTra) {
                soNgayQH = hanTra.daysTo(today);
            }
        }

        QString note = "";
        if (soNgayQH > 0) note = QString("Qua han %1 ngay").arg(soNgayQH);
        if (p->data.tienPhat > 0) {
            if (!note.isEmpty()) note += " ";
            note += QString("(Phat: %1)").arg(p->data.tienPhat);
        }

        tableHtml += QString(
            "<tr>"
            "<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td>"
            "</tr>"
        ).arg(p->data.banSaoSach ? QString::fromStdString(p->data.banSaoSach->maSach) : (p->data.maSachCached.empty() ? "???" : QString::fromStdString(p->data.maSachCached)))
         .arg(QString::fromStdString(p->data.NgayMuon))
         .arg(QString::fromStdString(p->data.NgayTra))
         .arg(statusStr)
         .arg(note);

        p = p->next;
    }
    tableHtml += "</table>";

    // 3. Combine
    QString finalHtml = QString(
        "<html><head><style>body { font-family: 'Times New Roman'; }</style></head><body>"
        "<h1 style='text-align: center;'>BAO CAO LICH SU MUON TRA</h1>"
        "%1"
        "<hr>"
        "%2"
        "</body></html>"
    ).arg(infoHtml).arg(tableHtml);

    QTextDocument doc;
    doc.setHtml(finalHtml);

    QString fileName = QString("LichSu_%1_%2").arg(QString::fromStdString(docGia->data.Ho)).arg(docGia->data.MaThe);
    return showPrintPreview(&doc, "Xuất Lịch Sử PDF", parent, fileName);
}

bool PrintHelper::inBaoCaoTaiChinh(const QString& start, const QString& end, const QString& tieuDe, QAbstractItemModel* model, QWidget* parent) {
    if (!model) return false;

    QString html = "<html><head><style>"
                   "table { border-collapse: collapse; width: 100%; font-size: 11px; }"
                   "th, td { border: 1px solid #333; padding: 6px; text-align: left; }"
                   "th { background-color: #f2f2f2; font-weight: bold; }"
                   "h1 { text-align: center; color: #1a1a1a; }"
                   ".header-info { margin-bottom: 20px; font-size: 12px; }"
                   ".footer { margin-top: 30px; text-align: right; font-style: italic; }"
                   "</style></head><body>";

    html += QString("<h1>%1</h1>").arg(tieuDe);
    html += QString("<div class='header-info'>"
                    "<b>Thời gian:</b> Từ %1 đến %2<br>"
                    "<b>Ngày lập:</b> %3"
                    "</div>")
            .arg(start).arg(end).arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));

    html += "<table><tr>";
    for (int i = 0; i < model->columnCount(); ++i) {
        html += QString("<th>%1</th>").arg(model->headerData(i, Qt::Horizontal).toString());
    }
    html += "</tr>";

    long long total = 0;
    for (int r = 0; r < model->rowCount(); ++r) {
        html += "<tr>";
        for (int c = 0; c < model->columnCount(); ++c) {
            QString val = model->data(model->index(r, c)).toString();
            html += QString("<td>%1</td>").arg(val);
            if (c == 4) { // Assuming Fine column
                total += model->data(model->index(r, c)).toLongLong();
            }
        }
        html += "</tr>";
    }
    html += "</table>";

    html += QString("<div class='footer'><b>Tổng cộng: %1 VND</b></div>").arg(QLocale(QLocale::Vietnamese).toString(total));
    html += "</body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    return showPrintPreview(&doc, tieuDe, parent, "BaoCo_TaiChinh_" + QDate::currentDate().toString("yyyyMMdd"));
}

bool PrintHelper::inBaoCaoKiemKe(const QStringList& missing, const QStringList& unknown, const QStringList& borrowedAnomalies,
                               const QStringList& borrowed, const QMap<QString, int>& stats, QWidget* parent) {

    QString html = "<html><head><style>"
                   "h1 { text-align: center; color: #1a1a1a; margin-top: 0; }"
                   "h2 { color: #d32f2f; border-bottom: 2px solid #ccc; padding-bottom: 5px; font-size: 16px; margin-top: 25px; }"
                   ".audit-info { margin-bottom: 25px; font-size: 13px; line-height: 1.6; }"
                   "table { border-collapse: collapse; width: 100%; margin-bottom: 20px; font-size: 11px; }"
                   "th, td { border: 1px solid #ddd; padding: 10px; text-align: left; }"
                   "th { background-color: #f5f6fa; font-weight: bold; color: #333; }"
                   ".summary-table { width: auto; min-width: 350px; border: 2px solid #333; margin-bottom: 30px; }"
                   ".summary-table th { background-color: #333; color: white; }"
                   ".summary-table td { font-weight: bold; font-size: 14px; text-align: center; }"
                   "</style></head><body>";

    html += "<h1>BÁO CÁO KIỂM KÊ KHO SÁCH TỔNG QUÁT</h1>";
    
    html += "<div class='audit-info'>";
    html += QString("• <b>Ngày kiểm kê:</b> %1<br>").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss"));
    html += QString("• <b>Người kiểm kê:</b> %1<br>").arg(UserManager::instance().currentUser().fullName.isEmpty() ? 
                                                       UserManager::instance().currentUser().username : 
                                                       UserManager::instance().currentUser().fullName);
    html += "</div>";

    // --- SUMMARY SECTION ---
    html += "<h2>1. TÓM TẮT CHỈ SỐ HỆ THỐNG</h2>";
    html += "<table class='summary-table'>";
    html += "<tr><th>Chỉ số</th><th>Số lượng</th></tr>";
    html += QString("<tr><td>Tổng số đầu sách</td><td>%1</td></tr>").arg(stats.value("titles", 0));
    html += QString("<tr><td>Tổng số cuốn (Bản sao)</td><td>%1</td></tr>").arg(stats.value("copies", 0));
    html += QString("<tr><td>Sách đang cho mượn (Theo hợp đồng)</td><td>%1</td></tr>").arg(stats.value("borrowed", 0));
    html += QString("<tr><td>Sách thực tế phải có trên kệ</td><td>%1</td></tr>").arg(stats.value("stock", 0));
    html += "</table>";

    // --- COMPARISON SECTION ---
    html += "<h2>2. KẾT QUẢ ĐỐI SOÁT THỰC TẾ</h2>";
    
    // Missing (In System but not on shelf)
    if (!missing.isEmpty()) {
        html += QString("<p style='color: #d32f2f; font-weight: bold;'>⚠️ DANH SÁCH SÁCH THẤT LẠC - THIẾU (%1 CUỐN)</p>").arg(missing.size());
        html += "<table><tr><th>STT</th><th>Mã cá biệt</th><th>Vị trí (Lý thuyết)</th><th>Ghi chú</th></tr>";
        for (int i = 0; i < missing.size(); ++i) {
            html += QString("<tr><td>%1</td><td>%2</td><td>-</td><td>Có trong DB nhưng không tìm thấy</td></tr>").arg(i+1).arg(missing[i]);
        }
        html += "</table>";
    }

    // Surplus (Scanned but not in System)
    if (!unknown.isEmpty()) {
        html += QString("<p style='color: #f39c12; font-weight: bold;'>❔ DANH SÁCH MÃ LẠ - DƯ (%1 CUỐN)</p>").arg(unknown.size());
        html += "<table><tr><th>STT</th><th>Mã quét được</th><th>Ghi chú</th></tr>";
        for (int i = 0; i < unknown.size(); ++i) {
            html += QString("<tr><td>%1</td><td>%2</td><td>Dữ liệu không tồn tại</td></tr>").arg(i+1).arg(unknown[i]);
        }
        html += "</table>";
    }

    // Anomalies (Scanned but DB says Borrowed)
    if (!borrowedAnomalies.isEmpty()) {
        html += QString("<p style='color: #a78bfa; font-weight: bold;'>⚡ DANH SÁCH SAI TRẠNG THÁI (%1 CUỐN)</p>").arg(borrowedAnomalies.size());
        html += "<table><tr><th>STT</th><th>Mã cá biệt</th><th>Trạng thái DB</th><th>Ghi chú</th></tr>";
        for (int i = 0; i < borrowedAnomalies.size(); ++i) {
            html += QString("<tr><td>%1</td><td>%2</td><td>Đang mượn</td><td>Có trên kệ nhưng chưa trả trên máy</td></tr>").arg(i+1).arg(borrowedAnomalies[i]);
        }
        html += "</table>";
    }

    if (missing.isEmpty() && unknown.isEmpty() && borrowedAnomalies.isEmpty()) {
        html += "<p style='text-align: center; font-size: 18px; color: #27ae60; padding: 20px; border: 2px dashed #27ae60;'><b>✓ TẤT CẢ DỮ LIỆU KHỚP TUYỆT ĐỐI!</b></p>";
    }

    html += "<div style='margin-top: 50px; text-align: right;'>"
            "<p>......, Ngày .... tháng .... năm ........</p>"
            "<p style='margin-right: 50px;'><b>Người lập biểu</b></p>"
            "<br><br><br>"
            "</div>";

    html += "</body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    return showPrintPreview(&doc, "Báo cáo kiểm kê tổng quát", parent, "KiemKe_Audit_" + QDate::currentDate().toString("yyyyMMdd"));
}
