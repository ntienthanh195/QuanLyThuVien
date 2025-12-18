#include "utils/PrintHelper.h"
#include <QPainter>
#include "qrcode/Code128.h"
#include "utils/QrCodeGenerator.h"
#include "utils/SettingsManager.h"
#include "utils/CustomPrintDialog.h"  // Custom dark-themed print preview
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
#include <QDateTime>
#include <QPair>

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
    
    QString title = QString("Xem trước: Thẻ học viên - %1 %2")
        .arg(QString::fromStdString(docGia->data.Ho))
        .arg(QString::fromStdString(docGia->data.Ten));
    
    // Auto filename for PDF: TheHocVien_[MaThe].pdf
    QString defaultFile = taoTenFileTheDocGia(docGia->data.MaThe);
        
    return showPrintPreview(&doc, title, parent, defaultFile);
}

bool PrintHelper::inTheDocGiaTuDong(PTRDG docGia, QWidget* parent) {
    if (!docGia) return false;

    return inTheDocGia(docGia, parent);
}

bool PrintHelper::inNhieuTheDocGia(const QList<PTRDG>& danhSach, QWidget* parent) {
    if (danhSach.isEmpty()) return false;
    
    // Unified Grid/Flex Layout for Cards (Similar to Labels)
    QString htmlToanBo = "<html><head><style>"
                         "body { margin: 0; padding: 0.5cm; background: white; }"
                         ".card-container { "
                         "  display: flex; "
                         "  flex-wrap: wrap; "
                         "  justify-content: space-between; " // Distribute space (Left - Right)
                         "  align-content: flex-start; "
                         "  width: 100%; "
                         "}"
                         "</style></head><body>"
                         " <div class='card-container'>";
                         
    for (int i = 0; i < danhSach.size(); ++i) {
        htmlToanBo += taoHTMLTheDocGia(danhSach[i]);
        // No manual page break - Flexbox handles wrapping
    }
    
    htmlToanBo += "</div></body></html>";
    
    QTextDocument doc;
    doc.setHtml(htmlToanBo);
    
    QString title = QString("Xem trước: %1 thẻ học viên").arg(danhSach.size());
    QString ngayFile = QDate::currentDate().toString("yyyyMMdd");
    QString defaultFile = QString("TheHocVien_Batch_%1the_%2.pdf").arg(danhSach.size()).arg(ngayFile);
    
    return showPrintPreview(&doc, title, parent, defaultFile);
}

bool PrintHelper::inA4NhieuThe(const QList<PTRDG>& danhSach, int soCot, QWidget* parent) {
    if (danhSach.isEmpty()) return false;
    
    QString html = taoHTMLA4NhieuThe(danhSach, soCot);
    QTextDocument doc;
    doc.setHtml(html);
    
    QString title = QString("Xem trước: %1 thẻ học viên (A4)").arg(danhSach.size());
    
    // Auto filename
    QString ngayFile = QDate::currentDate().toString("yyyyMMdd");
    QString defaultFile = QString("TheHocVien_A4_%1the_%2.pdf").arg(danhSach.size()).arg(ngayFile);
    
    return showPrintPreview(&doc, title, parent, defaultFile);
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
    
    QString title = QString("Xem trước: %1 tem - %2").arg(soLuong).arg(tenSach);
    
    // Auto filename: TemSach_[ISBN]_sl[N].pdf
    QString defaultFile = taoTenFileTemSach(QString::fromStdString(dauSach->ISBN), soLuong);
    
    return showPrintPreview(&doc, title, parent, defaultFile);
}

bool PrintHelper::inNhieuTemSachBatch(const QList<QPair<PTRDS, int>>& list, QWidget* parent) {
    if (list.isEmpty()) return false;

    // Load Config
    PrintConfig cfg = getCauHinh();
    
    // Calculate page styles based on config if needed, but for now stick to A4 default container
    QString html = "<html><head><style>"
                   "body { margin: 0; padding: 0.5cm; background: white; }"
                   // Container using Flexbox to flow labels
                   ".label-container { "
                   "  display: flex; "
                   "  flex-wrap: wrap; "
                   "  align-content: flex-start; "
                   "  width: 100%; " 
                   "}"
                   "</style></head><body>"
                   " <div class='label-container'>";

    for (const auto& pair : list) {
        PTRDS sach = pair.first;
        int count = pair.second;
        if (!sach) continue;

        QString tenStr = QString::fromStdString(sach->tenSach);
        QString maGoc = QString::fromStdString(sach->ISBN); // Default
        
        // Loop through copies
        PTRDMS p = sach->dms;
        int actualPrinted = 0;
        
        while (p && actualPrinted < count) {
            html += taoHTMLTemSach(QString::fromStdString(p->maSach), tenStr);
            p = p->next;
            actualPrinted++;
        }
        
        // If we ran out of specific copies but still need count (e.g. printing generic ISBN labels)
        while (actualPrinted < count) {
             html += taoHTMLTemSach(maGoc, tenStr);
             actualPrinted++;
        }

        // Add "HET" divider (full width) - forced break
        // Flexbox divider needs width 100%
        html += QString(
            "<div style='flex-basis: 100%; height: 0; margin: 0;'></div>" // Force break
            "<div style='width: 100%; text-align: center; margin: 15px 0; clear: both;'>"
            "  <div style='border-top: 1px dashed #ccc; margin-bottom: 5px;'></div>"
            "  <span style='font-family: Arial; font-size: 9pt; font-weight: bold; color: #666; background: #fff; padding: 0 10px;'>"
            "    -- HẾT: %1 --"
            "  </span>"
            "</div>"
        ).arg(tenStr);
    }
    
    html += "</div></body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    
    QString title = "Xem trước: Tem mã vạch hàng loạt";
    
    // Auto filename with timestamp
    QString dateStr = QDateTime::currentDateTime().toString("yyyyMMdd_HHmm");
    QString defaultFile = QString("Batch_Barcodes_%1.pdf").arg(dateStr);
    
    return showPrintPreview(&doc, title, parent, defaultFile);
}

bool PrintHelper::inTemSachTuyChon(PTRDS dauSach, const QStringList& codes, QWidget* parent) {
    if (!dauSach || codes.isEmpty()) return false;

    QString html = "<html><head><style>"
                   "body { margin: 0; padding: 10px; background: white; }"
                   "</style></head><body>"
                   " <div style='display: block; width: 100%;'>";

    QString tenStr = QString::fromStdString(dauSach->tenSach);
    for (const QString& ma : codes) {
        html += taoHTMLTemSach(ma, tenStr);
    }

    html += "</div></body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    
    QString title = QString("Xem trước: %1 tem tùy chọn - %2")
        .arg(codes.size())
        .arg(QString::fromStdString(dauSach->tenSach));
    
    // Auto filename
    QString dateStr = QDateTime::currentDateTime().toString("yyyyMMdd_HHmm");
    QString defaultFile = QString("Labels_%1_%2.pdf")
        .arg(QString::fromStdString(dauSach->ISBN))
        .arg(dateStr);
    
    return showPrintPreview(&doc, title, parent, defaultFile);
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
    // Minimum 2px per module for better fit on small labels (2px * 128 chars ~= 250px width)
    int moduleW = 2; 
    int quietZone = 5 * moduleW;
    
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

// Helper to expose Barcode Base64 generation
QString PrintHelper::generateBarcodeBase64(const QString &text, int height) {
    QImage barcodeImg = generateBarcodeImage(text, height);
    if (barcodeImg.isNull()) return "";

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    barcodeImg.save(&buffer, "PNG");
    return byteArray.toBase64();
}

QString PrintHelper::taoMaVachHTML(const QString& maSo, int fontSize) {
    // New Logic: Use Code128 & High-Height Image
    // Ignore fontSize arg for image generation height (use fixed high-res 150px)
    // fontSize is used for CSS display height if desired, or we just rely on auto-scaling.
    
    QString base64 = generateBarcodeBase64(maSo, 150);
    
    // Use pixelated rendering style and max-height constraints
    // Display height matches 'fontSize' roughly (e.g. 50px)
    int displayHeight = (fontSize > 0) ? fontSize : 50;
    
    return QString("<img src='data:image/png;base64,%1' style='height:%2px; max-width:100%%; image-rendering: pixelated; -ms-interpolation-mode: nearest-neighbor;' />")
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
        
        cfg.labelWidth = SettingsManager::instance().getInt("Print/BarcodeWidth", 140);
        cfg.labelHeight = SettingsManager::instance().getInt("Print/BarcodeHeight", 88);
        cfg.labelCols = SettingsManager::instance().getInt("Print/BarcodeCols", 3);
        cfg.showLibName = SettingsManager::instance().getBool("Print/ShowLibName", true);
        cfg.showBookTitle = SettingsManager::instance().getBool("Print/ShowBookTitle", true);
        cfg.showPrice = SettingsManager::instance().getBool("Print/ShowPrice", false);
        cfg.showBorder = SettingsManager::instance().getBool("Print/ShowBorder", true);

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
    // 1. Load Settings (Refresh to ensure latest)
    PrintConfig cfg = getCauHinh();
    
    // 2. Prepare Data
    QString libName = cfg.showLibName ? (cfg.tenVietTat.isEmpty() ? cfg.tenThuVien : cfg.tenVietTat) : "";
    QString safeTen = cfg.showBookTitle ? tenSach : "";
    if (safeTen.length() > 50) safeTen = safeTen.left(47) + "...";
    
    // 3. Layout Calculations
    // Total Height = cfg.labelHeight
    int hTotal = cfg.labelHeight;
    int wTotal = cfg.labelWidth;
    
    // Estimations for content height
    int hLib = cfg.showLibName ? 14 : 0; // Approx 14px for lib name
    int hTitle = cfg.showBookTitle ? 26 : 0; // Approx 2 lines of text
    int hCode = 12; // Bottom text code
    
    // Barcode takes remaining space
    int hBarcode = hTotal - hLib - hTitle - hCode - 8; // 8px padding
    if (hBarcode < 20) hBarcode = 20; // Min safe height
    
    // 4. Generate Real Barcode Image
    QString barcodeBase64 = generateBarcodeBase64(maSach, 150); // High res source
    
    QString borderStyle = cfg.showBorder ? "1px solid #000" : "1px dashed #eee";
    
    // 5. Build HTML
    QString html = QString(
        "<div style='"
        " width: %1px; height: %2px;"
        " border: %3;" 
        " margin: 0; padding: 2px;"
        " font-family: 'Segoe UI', Arial, sans-serif;"
        " text-align: center;"
        " background: #fff;"
        " float: left;" // Important for flow
        " position: relative;"
        " overflow: hidden;"
        " box-sizing: border-box;"
        " page-break-inside: avoid;"
        "'>"
    ).arg(wTotal).arg(hTotal).arg(borderStyle);

    // Library Name
    if (cfg.showLibName) {
        html += QString("<div style='font-size: 7pt; font-weight: bold; text-transform: uppercase; white-space: nowrap; overflow: hidden; height: %1px; line-height: %1px;'>%2</div>").arg(hLib).arg(libName);
    }
    
    // Book Title
    if (cfg.showBookTitle) {
        html += QString("<div style='font-size: 8pt; font-weight: 600; height: %1px; overflow: hidden; line-height: 1.1; display: -webkit-box; -webkit-line-clamp: 2; -webkit-box-orient: vertical;'>%2</div>").arg(hTitle).arg(safeTen);
    }
    
    // Barcode Image
    html += QString(
        "  <div style='height: %1px; display: flex; align-items: center; justify-content: center; overflow: hidden; margin: 1px 0;'>"
        "    <img src='data:image/png;base64,%2' style='height: 100%%; max-width: 98%%; image-rendering: pixelated;' />"
        "  </div>"
    ).arg(hBarcode).arg(barcodeBase64);
    
    // Readable Code
    html += QString("<div style='font-size: 8pt; font-weight: bold; height: %1px;'>%2</div>").arg(hCode).arg(maSach);
    
    html += "</div>";
    
    return html;
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
    
    // Generate QR with fixed size
    QString qrCode = taoMaQRHTML(QString::number(docGia->data.MaThe), 110);
    
    // Config: Validity Duration (Default 12 months)
    int durationMonths = SettingsManager::instance().getInt("Print/HanTheMonths", 12);
    QDate issueDate = QDate::currentDate();
    QDate expiryDate = issueDate.addMonths(durationMonths);
    
    // CARD DESIGN: ID-1 Standard (85.6mm x 54mm)
    // Using a clean, horizontal landscape layout to avoid overlapping
    return QString(
        "<div style='"
        " width: 85.6mm; height: 53.98mm;" // Standard Credit Card Size
        " border: 1px solid #333;"         // Solid border for cutting
        " padding: 3mm;"
        " margin: 2mm;"                    // Spacing between cards in batch
        " font-family: 'Segoe UI', Arial, sans-serif;"
        " background-color: #ffffff;"
        " position: relative;"
        " overflow: hidden;"
        " box-sizing: border-box;"
        " page-break-inside: avoid;"
        " float: left;"                    // Float for batch printing flow
        "'>"
        
        // --- HEADER ---
        "  <div style='text-align: center; border-bottom: 2px solid #0056b3; padding-bottom: 2px; margin-bottom: 4px;'>"
        "    <div style='font-size: 7pt; font-weight: bold; text-transform: uppercase; color: #555;'>%1</div>"
        "    <div style='font-size: 14pt; font-weight: 900; text-transform: uppercase; color: #0056b3; letter-spacing: 1px; margin-top: 1px;'>THẺ HỌC VIÊN</div>"
        "  </div>"

        // --- BODY (Flex Row) ---
        "  <div style='display: flex; flex-direction: row; height: 35mm;'>"
        
        // LEFT: QR Code & ID
        "    <div style='width: 32mm; text-align: center; display: flex; flex-direction: column; align-items: center; justify-content: flex-start; padding-right: 2px; border-right: 1px dotted #ccc;'>"
        "      <div style='margin-bottom: 2px;'>%2</div>" // QR Image
        "      <div style='font-size: 11pt; font-weight: bold; color: #333; margin-top: 2px;'>%3</div>" // ID Number
        "    </div>"

        // RIGHT: Student Details
        "    <div style='flex: 1; padding-left: 8px; display: flex; flex-direction: column; justify-content: center;'>"
        "      <div style='font-size: 11pt; font-weight: bold; color: #000; text-transform: uppercase; margin-bottom: 3px; line-height: 1.2;'>%4 %5</div>" // Name
        "      <div style='font-size: 9pt; margin-bottom: 2px; color: #444;'>Giới tính: <b>%6</b></div>"
        "      <div style='font-size: 9pt; margin-bottom: 2px; color: #444;'>Ngày cấp: %7</div>"
        "      <div style='font-size: 9pt; color: #d9534f; font-weight: bold; margin-top: 2px;'>Hết hạn: %8</div>"
        "    </div>"
        
        "  </div>" // End Body
        
        "</div>" // End Card
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
    
    // Use Code128 barcode instead of QR for faster scanning
    // Reduced height to match new compact params
    QString barcodeBook = taoMaVachHTML(maSach, 50);      
    QString barcodeCard = taoMaVachHTML(QString::number(docGia->data.MaThe), 40); 
    
    // Receipt Style (K80)
    return QString(
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
        ".barcode-section { text-align: center; margin: 12px 0; background: #f9f9f9; padding: 8px; border-radius: 4px; }"
        ".barcode-label { font-size: 8pt; color: #666; margin-bottom: 4px; font-weight: bold; }"
        ".footer { text-align: center; font-size: 8pt; font-style: italic; margin-top: 15px; color: #666; border-top: 1px solid #ddd; padding-top: 8px; }"
        "</style></head><body>"
        
        // Header
        "<div class='header'>"
        "  <div class='org-name'>%1</div>"
        "  <div class='title'>PHIẾU MƯỢN/TRẢ</div>"
        "  <div class='datetime'>GD: %2</div>"
        "</div>"
        
        // Student Info
        "<div class='section'>"
        "  <div class='row'><span class='label'>Độc giả:</span><span class='value'>%3 %4</span></div>"
        "  <div class='barcode-section'>"
        "    <div class='barcode-label'>MÃ THẺ</div>"
        "    %5"
        "    <div style='font-size: 9pt; font-weight: bold; margin-top: 2px;'>%6</div>"
        "  </div>"
        "</div>"
        
        "<div class='divider'></div>"
        
        // Book Info
        "<div class='section'>"
        "  <div class='row'><span class='label'>Tên sách:</span></div>"
        "  <div style='font-size: 9pt; margin-bottom: 8px; font-weight: 600;'>%7</div>"
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
    
    QString title = QString("Xem trước: Phiếu mượn (%1 sách)").arg(danhSachSach.size());
    QString defaultFile = QString("PhieuMuon_Batch_%1_%2.pdf").arg(docGia->data.MaThe).arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmm"));
    
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
        return true;
    } else {
        qDebug() << "User cancelled";
        return false;
    }
}

