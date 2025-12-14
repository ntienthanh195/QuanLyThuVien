#include "utils/PrintHelper.h"
#include <QPainter>
#include "qrcode/Code128.h"
#include "utils/QrCodeGenerator.h"
#include "utils/SettingsManager.h"
#include "utils/CustomPrintDialog.h"
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <QApplication>
#include <QDate>
#include <QTime>
#include <QFileDialog>
#include <QDir>
#include <QRegularExpression>
#include <functional> // For std::function
#include <QBuffer>

// Static config
// Static config
PrintHelper::PrintConfig PrintHelper::s_config;
static bool s_configLoaded = false; // Flag to track initialization

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
    
    QString html = taoHTMLTheDocGia(docGia);
    QTextDocument doc;
    doc.setHtml(html);
    
    QString question = QString("In thẻ cho: %1 %2 (Mã: %3)")
        .arg(QString::fromStdString(docGia->data.Ho))
        .arg(QString::fromStdString(docGia->data.Ten))
        .arg(docGia->data.MaThe);
        
    return xuLyYeuCauIn(&doc, "In Thẻ Học Viên", question, taoTenFileTheDocGia(docGia->data.MaThe), parent, s_config.printerReport);

}

bool PrintHelper::inTheDocGiaTuDong(PTRDG docGia, QWidget* parent) {
    if (!docGia) return false;

    return inTheDocGia(docGia, parent);
}

bool PrintHelper::inNhieuTheDocGia(const QList<PTRDG>& danhSach, QWidget* parent) {
    if (danhSach.isEmpty()) return false;
    
    QString htmlToanBo;
    for (int i = 0; i < danhSach.size(); ++i) {
        htmlToanBo += taoHTMLTheDocGia(danhSach[i]);
        if (i < danhSach.size() - 1) {
            htmlToanBo += "<div style='page-break-after: always;'></div>";
        }
    }
    
    QTextDocument doc;
    doc.setHtml(htmlToanBo);
    return inDocument(&doc, parent);
}

bool PrintHelper::inA4NhieuThe(const QList<PTRDG>& danhSach, int soCot, QWidget* parent) {
    if (danhSach.isEmpty()) return false;
    
    QString html = taoHTMLA4NhieuThe(danhSach, soCot);
    QTextDocument doc;
    doc.setHtml(html);
    
    QString question = QString("Sẵn sàng in %1 thẻ học viên").arg(danhSach.size());
    QString ngayFile = QDate::currentDate().toString("yyyyMMdd");
    QString tenFile = QString("TheHocVien_Batch_%1the_%2.pdf").arg(danhSach.size()).arg(ngayFile);
    
    return xuLyYeuCauIn(&doc, "In Hàng Loạt", question, tenFile, parent);

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

bool PrintHelper::inTemSachTuDong(PTRDS dauSach, QWidget* parent) {
    if (!dauSach) return false;
    QString tenSach = QString::fromStdString(dauSach->tenSach);
    QString maSach = QString::fromStdString(dauSach->ISBN); // Default to ISBN
    if (dauSach->dms) {
        maSach = QString::fromStdString(dauSach->dms->maSach); // Use first copy code if available
    }

    return inTemMaVachSach(maSach, tenSach, parent);

}

bool PrintHelper::inNhieuTemSach(PTRDS dauSach, int soLuong, QWidget* parent) {
    if (!dauSach || soLuong <= 0) return false;
    
    QString tenSach = QString::fromStdString(dauSach->tenSach);
    QString htmlToanBo;
    
    // Lặp qua danh mục sách để in nhiều tem
    PTRDMS p = dauSach->dms;
    int count = 0;
    while (p && count < soLuong) {
        QString maSach = QString::fromStdString(p->maSach);
        htmlToanBo += taoHTMLTemSach(maSach, tenSach);
        if (count < soLuong - 1) {
            htmlToanBo += "<div style='page-break-after: always;'></div>";
        }
        p = p->next;
        ++count;
    }
    
    QTextDocument doc;
    doc.setHtml(htmlToanBo);
    
    QString question = QString("In %1 tem cho: %2").arg(soLuong).arg(tenSach);
    QString tenFile = taoTenFileTemSach(QString::fromStdString(dauSach->ISBN), soLuong);
    return xuLyYeuCauIn(&doc, "In Tem Sách Hàng Loạt", question, tenFile, parent, s_config.printerReceipt);

}

bool PrintHelper::inDanhSachSach(PTRDS* dsDauSach, int soLuong, QWidget* parent) {
    if (!dsDauSach || soLuong <= 0) return false;
    
    QString noiDungBang = "<table border='1' cellpadding='5' cellspacing='0' style='width:100%; border-collapse:collapse;'>"
                          "<tr style='background:#f0f0f0;'>"
                          "<th>ISBN</th><th>Tên sách</th><th>Tác giả</th><th>Số bản</th>"
                          "</tr>";
    
    for (int i = 0; i < soLuong; ++i) {
        PTRDS p = dsDauSach[i];
        if (!p) continue;
        
        QString isbn = QString::fromStdString(p->ISBN);
        QString ten = QString::fromStdString(p->tenSach);
        QString tacGia = QString::fromStdString(p->tacGia);
        int soBan = p->tongBanSao;
        
        noiDungBang += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
                          .arg(isbn).arg(ten).arg(tacGia).arg(soBan);
    }
    
    noiDungBang += "</table>";
    
    QString html = taoHTMLDanhSach("DANH MỤC SÁCH", noiDungBang);
    QTextDocument doc;
    doc.setHtml(html);
    return xuLyYeuCauIn(&doc, "In Danh Mục Sách", QString("In danh sách %1 đầu sách").arg(soLuong), taoTenFileBaoCao("DMSach"), parent, s_config.printerReport);
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
    
    // Convert Tree to List for Table
    QList<PTRDG> list;
    std::function<void(PTRDG)> traverse = [&](PTRDG p) {
        if (!p) return;
        traverse(p->left);
        list.append(p);
        traverse(p->right);
    };
    traverse(root);
    
    if (list.isEmpty()) return false;
    
    QString bangHTML = "<table border='1' cellpadding='5' cellspacing='0' style='width:100%; border-collapse:collapse;'>"
                       "<tr style='background:#f0f0f0;'>"
                       "<th>STT</th><th>Mã Thẻ</th><th>Họ Tên</th>"
                       "<th>Giới Tính</th><th>Trạng Thái</th><th>Đang Mượn</th>"
                       "</tr>";
                       
    for (int i = 0; i < list.size(); i++) {
        PTRDG p = list[i];
        QString hoTen = QString::fromStdString(p->data.Ho + " " + p->data.Ten);
        QString phai = p->data.Phai ? "Nữ" : "Nam";
        QString trangThai = (p->data.TrangThai == 1) ? "Hoạt động" : "Khóa";
        
        bangHTML += QString("<tr>"
                           "<td>%1</td><td>%2</td><td>%3</td>"
                           "<td>%4</td><td>%5</td><td>%6</td>"
                           "</tr>")
            .arg(i + 1)
            .arg(p->data.MaThe)
            .arg(hoTen)
            .arg(phai)
            .arg(trangThai)
            .arg(p->data.soSachDangMuon);
    }
    bangHTML += "</table>";
    
    QString html = taoHTMLDanhSach("DANH SÁCH HỌC VIÊN", bangHTML);
    QTextDocument doc;
    doc.setHtml(html);
    
    QString question = QString("Tìm thấy %1 học viên.").arg(list.size());
    return xuLyYeuCauIn(&doc, "In Danh Sách Học Viên", question, taoTenFileBaoCao("DSHocVien"), parent, s_config.printerReport);

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
    QString question = QString("In biên lai phạt cho: %1\nSố tiền: %2").arg(hoTen).arg(soTien);
    return xuLyYeuCauIn(&doc, "Biên Lai Phạt", question, taoTenFileBaoCao("Phat"), parent, s_config.printerReceipt);

}

// ===== HELPER FUNCTIONS =====
// Helper to generate barcode image (Code128 preferred)
static QImage generateBarcodeImage(const QString &text, int height) {
    // 1. Generate Code128 Pattern
    QVector<bool> pattern = Code128::encode(text);
    if (pattern.isEmpty()) return QImage();

    // 2. High DPI Settings
    // Minimum 3px per module for good print sharpness (3px * 128 chars ~= 400px width)
    int moduleW = 3; 
    int quietZone = 10 * moduleW;
    
    int totalWidth = (pattern.size() * moduleW) + (2 * quietZone);
    // Ensure height is reasonable (passed 200 usually)
    
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
    
    return image;
}

QString PrintHelper::taoMaVachHTML(const QString& maSo, int fontSize) {
    // New Logic: Use Code128 & High-Height Image
    // Ignore fontSize arg for image generation height (use fixed high-res 150px)
    // fontSize is used for CSS display height if desired, or we just rely on auto-scaling.
    
    QImage barcodeImg = generateBarcodeImage(maSo, 150);
    
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    barcodeImg.save(&buffer, "PNG");
    QString base64 = byteArray.toBase64();
    
    // Use pixelated rendering style and max-height constraints
    // Display height matches 'fontSize' roughly (e.g. 50px)
    int displayHeight = (fontSize > 0) ? fontSize : 50;
    
    return QString("<img src='data:image/png;base64,%1' style='height:%2px; max-width:100%%;' />")
            .arg(base64).arg(displayHeight);
}

QString PrintHelper::taoMaQRHTML(const QString& maSo, int size) {
    QImage qr = QrCodeGenerator::generateQrImage(maSo, size);
    if (qr.isNull()) return "";
    
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    qr.save(&buffer, "PNG");
    QString base64 = byteArray.toBase64();
    
    return QString("<img src='data:image/png;base64,%1' width='%2' height='%2' />")
           .arg(base64).arg(size);
}

// Helper to create file names
QString PrintHelper::taoTenFilePhieuMuon(int maThe, const QString& maSach) {
    return QString("PhieuMuon_%1_%2_%3.pdf").arg(maThe).arg(maSach).arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmm"));
}
QString PrintHelper::taoTenFilePhieuTra(int maThe, const QString& maSach) {
    return QString("PhieuTra_%1_%2_%3.pdf").arg(maThe).arg(maSach).arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmm"));
}
QString PrintHelper::taoTenFileTheDocGia(int maThe) {
    return QString("TheHocVien_%1.pdf").arg(maThe);
}
QString PrintHelper::taoTenFileTemSach(const QString& isbn, int soLuong) {
    return QString("TemSach_%1_sl%2.pdf").arg(isbn).arg(soLuong);
}
QString PrintHelper::taoTenFileBaoCao(const QString& loaiBaoCao) {
    return QString("BaoCao_%1_%2.pdf").arg(loaiBaoCao).arg(QDate::currentDate().toString("yyyyMMdd"));
}

bool PrintHelper::xuatPDF(QTextDocument* doc, const QString& tenFile, QWidget* parent) {
    if (!doc) return false;
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(tenFile);
    
    // Load Settings
    // NOTE: Cần include "utils/SettingsManager.h" nếu chưa có, hoặc dùng QSettings trực tiếp
    // Giả sử dùng QSettings để đơn giản hoá dependency nếu chưa include SettingsManager
    QSettings settings("HVPGCT", "QuanLyThuVien_V2");
    
    // 1. Page Size
    int paperId = settings.value("printer_paper_size", QPageSize::A4).toInt();
    printer.setPageSize(QPageSize(static_cast<QPageSize::PageSizeId>(paperId)));
    
    // 2. Orientation
    int orientId = settings.value("printer_orientation", QPageLayout::Portrait).toInt();
    printer.setPageOrientation(static_cast<QPageLayout::Orientation>(orientId));
    
    // 3. Margins (mm)
    qreal mL = settings.value("printer_margin_left", 10).toDouble();
    qreal mR = settings.value("printer_margin_right", 10).toDouble();
    qreal mT = settings.value("printer_margin_top", 10).toDouble();
    qreal mB = settings.value("printer_margin_bottom", 10).toDouble();
    
    printer.setPageMargins(QMarginsF(mL, mT, mR, mB), QPageLayout::Millimeter);
    
    // 4. Color Mode
    if (s_config.inMauSac) {
        printer.setColorMode(QPrinter::Color);
    } else {
        printer.setColorMode(QPrinter::GrayScale);
    }

    doc->print(&printer);
    QMessageBox::information(parent, "Xuất PDF", "Đã lưu file thành công:\n" + tenFile);
    return true;
}

void PrintHelper::setCauHinh(const PrintConfig& config) {
    s_config = config;
    s_configLoaded = true; // Mark as loaded when set manually
}

// Flag moved to top

PrintHelper::PrintConfig PrintHelper::getCauHinh() {
    if (!s_configLoaded) {
        // Lazy Load from SettingsManager
        PrintHelper::PrintConfig cfg;
        cfg.tenThuVien = SettingsManager::instance().getString("Print/TenThuVien", "THƯ VIỆN");
        cfg.tenVietTat = SettingsManager::instance().getString("Print/TenVietTat", "TV");
        cfg.inMauSac = SettingsManager::instance().getBool("Print/Color", false);
        cfg.hienThiLogo = SettingsManager::instance().getBool("Print/ShowLogo", false);
        cfg.duongDanLogo = SettingsManager::instance().getString("Print/LogoPath", "");
        cfg.qrOutputDir = SettingsManager::instance().getString("Print/QrPath", "");
        cfg.pdfOutputDir = SettingsManager::instance().getString("Print/PdfPath", "");
        cfg.silentPrint = SettingsManager::instance().getBool("Print/Silent", false);
        cfg.printerReceipt = SettingsManager::instance().getString("Print/Receipt", "");
        cfg.printerReport = SettingsManager::instance().getString("Print/Report", "");
        
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

QString PrintHelper::taoHTMLTemSach(const QString& maSach, const QString& tenSach) {
    // Standard Label Size: 50mm x 30mm (Common for Library Barcodes)
    // We use mm units for print consistency
    
    // Barcode: Code39 font needs * start/stop chars if using font.
    // If using Image (QrCodeGenerator equivalent for Barcode?), current logic uses Code39 Font DIV.
    
    // Use the short name or abbreviated full name
    // Use the short name or abbreviated full name
    QString libName = SettingsManager::instance().getString("Print/TenVietTat", s_config.tenVietTat);
    if (libName.isEmpty()) libName = SettingsManager::instance().getString("Print/TenThuVien", "THƯ VIỆN");
    
    // Ensure text fits
    QString safeTen = tenSach;
    if (safeTen.length() > 35) safeTen = safeTen.left(32) + "...";
    
    QString barcode = taoMaVachHTML(maSach, 40); // Height 40px
    
    return QString(
        "<div style='"
        " width: 50mm; height: 30mm;"
        " border: 1px solid #000;" // Solid black border
        " margin: 2mm;" // Gap between labels
        " padding: 2px;"
        " font-family: 'Segoe UI', 'Arial', sans-serif;"
        " text-align: center;"
        " background: #fff;"
        " page-break-inside: avoid;"
        " display: inline-block;"
        " overflow: hidden;"
        "'>"
        
        // Header (Library Name)
        "  <div style='"
        "    font-size: 8pt; font-weight: bold; text-transform: uppercase;"
        "    border-bottom: 1px solid #000; padding-bottom: 1px; margin-bottom: 2px;"
        "    white-space: nowrap; overflow: hidden; text-overflow: ellipsis;"
        "  '>%1</div>"
        
        // Body (Book Title)
        "  <div style='"
        "    font-size: 9pt; font-weight: bold;"
        "    height: 32px; overflow: hidden; line-height: 1.1;"
        "    margin-bottom: 2px;"
        "  '>%2</div>"
        
        // Barcode Container
        "  <div style='margin: 0 auto;'>%3</div>"
        
        // Footer (Readable Code)
        "  <div style='"
        "    font-size: 8pt; letter-spacing: 2px; font-weight: bold;"
        "  '>%4</div>"
        
        "</div>"
    ).arg(libName).arg(safeTen).arg(barcode).arg(maSach);
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
        
        html += "<td>";
        html += taoHTMLTheDocGia(danhSach[i]);
        html += "</td>";
        
        if (count % soCot == soCot - 1) html += "</tr>"; // End row
        count++;
    }
    
    // Close last row if needed
    if (count % soCot != 0) {
        html += "</tr>";
    }
    
    html += "</table></body></html>";
    return html;
}

QString PrintHelper::taoHTMLDanhSach(const QString& tieuDe, const QString& noiDungBang) {
    return QString(
        "<html><head><style>"
        "body { font-family: 'Siemreap', 'Khmer OS Siemreap', 'Times New Roman', serif; font-size: 11pt; color: #000; margin: 20px; }"
        ".page-header { text-align: center; margin-bottom: 20px; }"
        ".org-name { font-weight: bold; font-size: 12pt; text-transform: uppercase; margin-bottom: 5px; }"
        ".report-title { font-weight: 900; font-size: 16pt; margin-top: 15px; text-transform: uppercase; }"
        "table { width: 100%; border-collapse: collapse; margin-top: 10px; font-size: 10pt; }"
        "th { background-color: #eee; border: 1px solid #000; padding: 8px; font-weight: bold; text-align: center; }"
        "td { border: 1px solid #000; padding: 6px; }"
        "tr:nth-child(even) { background-color: #f9f9f9; }" // Zebra Striping for readability
        ".footer { margin-top: 30px; text-align: right; font-style: italic; font-size: 10pt; }"
        ".signature-box { margin-top: 40px; display: flex; justify-content: space-between; text-align: center; }"
        ".date-time { margin-bottom: 50px; }"
        "</style></head><body>"
        
        "<div class='page-header'>"
        "  <div class='org-name'>%1</div>"
        "  <div style='font-size: 9pt;'>Thư Viện Học Viện</div>"
        "  <div class='report-title'>%2</div>"
        "  <div style='font-size: 10pt;'>Ngày xuất báo cáo: %3</div>"
        "</div>"
        
        "%4" // Table Content
        
        "<div class='footer'>"
        "  <div class='date-time'>Cần Thơ, ngày %5 tháng %6 năm %7</div>"
        "  <div style='width: 200px; float: right; text-align: center;'>"
        "    <b>Người Lập Biểu</b><br>"
        "    <i>(Ký và ghi rõ họ tên)</i>"
        "  </div>"
        "  <div style='clear: both;'></div>"
        "</div>"
        
        "</body></html>"
    ).arg(SettingsManager::instance().getString("Print/TenThuVien", s_config.tenThuVien))
     .arg(tieuDe)
     .arg(QDate::currentDate().toString("dd/MM/yyyy HH:mm"))
     .arg(noiDungBang)
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
    QString question = QString("In phiếu mượn cho: %1\nSách: %2").arg(QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten)).arg(tenSach);
    return xuLyYeuCauIn(&doc, "In Phiếu Mượn", question, taoTenFilePhieuMuon(docGia->data.MaThe, maSach), parent, s_config.printerReceipt, forcePreview);
}

bool PrintHelper::inPhieuTraTuDong(PTRDG docGia, const QString& maSach, const QString& tenSach, const QString& ngayTra, QWidget* parent, bool forcePreview) {
    QString html = taoHTMLPhieuMuon(docGia, maSach, tenSach, ngayTra);
    QTextDocument doc;
    doc.setHtml(html);
    QString question = QString("In phiếu trả cho: %1\nSách: %2").arg(QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten)).arg(tenSach);
    return xuLyYeuCauIn(&doc, "In Phiếu Trả", question, taoTenFilePhieuTra(docGia->data.MaThe, maSach), parent, s_config.printerReceipt, forcePreview);
}

// ===== PRIVATE HELPERS =====

QString PrintHelper::taoHTMLTheDocGia(PTRDG docGia) {
    if (!docGia) return "";
    
    QString qrCode = taoMaQRHTML(QString::number(docGia->data.MaThe), 100);
    
    // Config: Validity Duration (Default 12 months)
    int durationMonths = SettingsManager::instance().getInt("Print/HanTheMonths", 12);
    QDate issueDate = QDate::currentDate();
    QDate expiryDate = issueDate.addMonths(durationMonths);
    
    // Card Style: ID-1 (85.6mm x 53.98mm)
    return QString(
        "<div style='"
        " width: 85mm; height: 54mm; "
        " border: 2px solid #000; " // Border for cutting
        " box-sizing: border-box; "
        " font-family: 'Siemreap', 'Khmer OS Siemreap', 'Times New Roman', serif; "
        " position: relative; "
        " overflow: hidden; "
        " margin: 5px; " // Margin for separation if batch
        " page-break-inside: avoid; "
        " color: #000; "
        " background: #fff; " // White background
        " float: left;"
        "'>"
        
        // Header
        "  <div style='text-align: center; border-bottom: 2px solid #000; padding: 2px 0;'>"
        "    <div style='font-size: 8pt; font-weight: bold; text-transform: uppercase;'>%1</div>"
        "    <div style='font-size: 11pt; font-weight: 900; text-transform: uppercase; margin-top: 2px;'>THẺ HỌC VIÊN</div>"
        "  </div>"
        
        // Body
        "  <div style='padding: 5px; height: 35mm;'>"
        "    <table style='width: 100%; height: 100%; border-collapse: collapse;'>"
        "      <tr>"
        "        <td style='width: 30mm; text-align: center; vertical-align: top; padding-top: 5px;'>"
        "           %2" // QR Code
        "           <div style='font-size: 8pt; font-weight: bold; margin-top: 2px;'>%3</div>" // Ma The text
        "        </td>"
        "        <td style='vertical-align: top; padding-left: 5px;'>"
        "           <div style='font-size: 12pt; font-weight: bold; margin-bottom: 5px;'>%4 %5</div>" // Ten
        "           <div style='font-size: 9pt;'>Giới tính: <b>%6</b></div>"
        "           <div style='font-size: 9pt;'>Ngày cấp: %7</div>"
        "           <div style='font-size: 9pt; color: #cc0000; margin-top: 5px;'><b>Hết hạn: %8</b></div>"
        "        </td>"
        "      </tr>"
        "    </table>"
        "  </div>"
        
        "</div>"
    ).arg(SettingsManager::instance().getString("Print/TenThuVien", s_config.tenThuVien))
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
    
    QString qrCode = taoMaQRHTML(maSach, 80);
    
    // Receipt Style (K80 / A6) - Minimalist Black & White
    // Width ~70-80mm for K80 printers.
    return QString(
        "<html><head><style>"
        "body { font-family: 'Courier New', monospace; font-size: 10pt; margin: 0; padding: 5px; width: 72mm; color: #000; background: #fff; }"
        ".header { text-align: center; border-bottom: 2px solid #000; padding-bottom: 5px; margin-bottom: 10px; }"
        ".title { font-size: 14pt; font-weight: 900; margin: 5px 0; text-transform: uppercase; }"
        ".row { margin-bottom: 5px; clear: both; overflow: hidden; }"
        ".label { float: left; width: 35%; font-weight: bold; }"
        ".value { float: right; width: 65%; text-align: right; }"
        ".divider { border-top: 1px dashed #000; margin: 10px 0; }"
        ".footer { text-align: center; font-size: 8pt; font-style: italic; margin-top: 10px; }"
        ".qr { text-align: center; margin-top: 10px; }"
        "</style></head><body>"
        
        "<div class='header'>"
        "  <div style='font-weight:bold; font-size:9pt;'>%1</div>"
        "  <div class='title'>PHIẾU MƯỢN/TRẢ</div>"
        "  <div style='font-size:8pt;'>GD: %2</div>"
        "</div>"
        
        "<div class='content'>"
        "  <div class='row'><span class='label'>Độc giả:</span><span class='value'>%3 %4</span></div>"
        "  <div class='row'><span class='label'>Mã thẻ:</span><span class='value'>%5</span></div>" 
        "  <div class='divider'></div>"
        "  <div class='row'><span class='label'>Tên sách:</span><span class='value'>%6</span></div>"
        "  <div class='row'><span class='label'>Mã sách:</span><span class='value'>%7</span></div>"
        "</div>"
        
        "<div class='qr'>"
        "  %8"
        "  <div><small>%7</small></div>"
        "</div>"
        
        "<div class='footer'>%9<br>%10</div>"
        "</body></html>"
    ).arg(SettingsManager::instance().getString("Print/TenThuVien", s_config.tenThuVien))
     .arg(ngayGiaoDich)
     .arg(QString::fromStdString(docGia->data.Ho))
     .arg(QString::fromStdString(docGia->data.Ten))
     .arg(docGia->data.MaThe)
     .arg(tenSach)
     .arg(maSach)
     .arg(qrCode)
     .arg(SettingsManager::instance().getString("Print/TenThuVien", s_config.tenThuVien))
     .arg(SettingsManager::instance().getString("Print/LoiChao", "Xin cảm ơn!"));
}

