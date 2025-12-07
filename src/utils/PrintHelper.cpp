#include "utils/PrintHelper.h"
#include "utils/QrCodeGenerator.h"
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

// Static config
PrintHelper::PrintConfig PrintHelper::s_config;

// helper to ensure directories exist
static bool ensureDirExists(const QString &path) {
    QDir dir(path);
    if (dir.exists()) return true;
    return dir.mkpath(".");
}

// ==================== PUBLIC API ====================

bool PrintHelper::inTheDocGia(PTRDG docGia, QWidget* parent) {
    if (!docGia) return false;
    
    QString html = taoHTMLTheDocGia(docGia);
    QTextDocument doc;
    doc.setHtml(html);
    
    // Hỏi người dùng
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("In Thẻ Độc Giả");
    msgBox.setText(QString("In thẻ cho: %1 %2 (Mã: %3)")
        .arg(QString::fromStdString(docGia->data.Ho))
        .arg(QString::fromStdString(docGia->data.Ten))
        .arg(docGia->data.MaThe));
    QPushButton* btnIn = msgBox.addButton("🖨️ In Ngay", QMessageBox::AcceptRole);
    QPushButton* btnPDF = msgBox.addButton("💾 Lưu PDF", QMessageBox::ActionRole);
    msgBox.addButton("❌ Hủy", QMessageBox::RejectRole);
    msgBox.exec();
    
    QAbstractButton* clicked = msgBox.clickedButton();
    if (clicked == btnIn) {
        return inDocument(&doc, parent);
    } else if (clicked == btnPDF) {
        QString tenFileMacDinh = taoTenFileTheDocGia(docGia->data.MaThe);
        PrintConfig cfg = getCauHinh();
        QString defaultDir;
        if (!cfg.pdfOutputDir.isEmpty()) {
            defaultDir = QDir(cfg.pdfOutputDir).isAbsolute() ? cfg.pdfOutputDir : QApplication::applicationDirPath() + "/" + cfg.pdfOutputDir;
        } else {
            defaultDir = QApplication::applicationDirPath() + "/" + cfg.duongDanThuMucIn + "/" + cfg.thuMucPdf;
        }
        QDir().mkpath(defaultDir);
        QString filePath = QFileDialog::getSaveFileName(
            parent,
            "Lưu Thẻ Độc Giả",
            QDir(defaultDir).filePath(tenFileMacDinh),
            "PDF Files (*.pdf)"
        );

        if (!filePath.isEmpty()) {
            return xuatPDF(&doc, filePath, parent);
        }
    }
    
    return false;
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
    
    // Hỏi người dùng
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("In Hàng Loạt");
    msgBox.setText(QString("Sẵn sàng in %1 thẻ độc giả").arg(danhSach.size()));
    QPushButton* btnIn = msgBox.addButton("🖨️ In Ngay", QMessageBox::AcceptRole);
    QPushButton* btnPDF = msgBox.addButton("💾 Lưu PDF", QMessageBox::ActionRole);
    msgBox.addButton("❌ Hủy", QMessageBox::RejectRole);
    msgBox.exec();
    
    QAbstractButton* clicked = msgBox.clickedButton();
    if (clicked == btnIn) {
        return inDocument(&doc, parent);
    } else if (clicked == btnPDF) {
        QString ngayFile = QDate::currentDate().toString("yyyyMMdd");
        QString tenFileMacDinh = QString("TheDocGia_Batch_%1the_%2.pdf")
            .arg(danhSach.size())
            .arg(ngayFile);

        PrintConfig cfg = getCauHinh();
        QString defaultDir;
        if (!cfg.pdfOutputDir.isEmpty()) {
            defaultDir = QDir(cfg.pdfOutputDir).isAbsolute() ? cfg.pdfOutputDir : QApplication::applicationDirPath() + "/" + cfg.pdfOutputDir;
        } else {
            defaultDir = QApplication::applicationDirPath() + "/" + cfg.duongDanThuMucIn + "/" + cfg.thuMucPdf;
        }
        QDir().mkpath(defaultDir);

        QString filePath = QFileDialog::getSaveFileName(
            parent,
            "Lưu Thẻ Độc Giả Hàng Loạt",
            QDir(defaultDir).filePath(tenFileMacDinh),
            "PDF Files (*.pdf)"
        );

        if (!filePath.isEmpty()) {
            return xuatPDF(&doc, filePath, parent);
        }
    }
    
    return false;
}

bool PrintHelper::inTemMaVachSach(const QString& maSach, const QString& tenSach, QWidget* parent) {
    QString html = taoHTMLTemSach(maSach, tenSach);
    QTextDocument doc;
    doc.setHtml(html);
    
    // Hỏi người dùng
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("In Tem Sách");
    msgBox.setText(QString("In tem cho sách: %1").arg(tenSach));
    QPushButton* btnIn = msgBox.addButton("🖨️ In Ngay", QMessageBox::AcceptRole);
    QPushButton* btnPDF = msgBox.addButton("💾 Lưu PDF", QMessageBox::ActionRole);
    msgBox.addButton("❌ Hủy", QMessageBox::RejectRole);
    msgBox.exec();
    
    QAbstractButton* clicked = msgBox.clickedButton();
    if (clicked == btnIn) {
        return inDocument(&doc, parent);
    } else if (clicked == btnPDF) {
        QString tenFileMacDinh = taoTenFileTemSach(maSach, 1);
        PrintConfig cfg = getCauHinh();
        QString defaultDir;
        if (!cfg.pdfOutputDir.isEmpty()) {
            defaultDir = QDir(cfg.pdfOutputDir).isAbsolute() ? cfg.pdfOutputDir : QApplication::applicationDirPath() + "/" + cfg.pdfOutputDir;
        } else {
            defaultDir = QApplication::applicationDirPath() + "/" + cfg.duongDanThuMucIn + "/" + cfg.thuMucPdf;
        }
        QDir().mkpath(defaultDir);
        QString filePath = QFileDialog::getSaveFileName(
            parent,
            "Lưu Tem Sách",
            QDir(defaultDir).filePath(tenFileMacDinh),
            "PDF Files (*.pdf)"
        );

        if (!filePath.isEmpty()) {
            return xuatPDF(&doc, filePath, parent);
        }
    }
    
    return false;
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
    
    // Hỏi người dùng
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("In Tem Sách Hàng Loạt");
    msgBox.setText(QString("In %1 tem cho: %2").arg(soLuong).arg(tenSach));
    QPushButton* btnIn = msgBox.addButton("🖨️ In Ngay", QMessageBox::AcceptRole);
    QPushButton* btnPDF = msgBox.addButton("💾 Lưu PDF", QMessageBox::ActionRole);
    msgBox.addButton("❌ Hủy", QMessageBox::RejectRole);
    msgBox.exec();
    
    QAbstractButton* clicked = msgBox.clickedButton();
    if (clicked == btnIn) {
        return inDocument(&doc, parent);
    } else if (clicked == btnPDF) {
        QString tenFileMacDinh = taoTenFileTemSach(QString::fromStdString(dauSach->ISBN), soLuong);
        PrintConfig cfg = getCauHinh();
        QString defaultDir;
        if (!cfg.pdfOutputDir.isEmpty()) {
            defaultDir = QDir(cfg.pdfOutputDir).isAbsolute() ? cfg.pdfOutputDir : QApplication::applicationDirPath() + "/" + cfg.pdfOutputDir;
        } else {
            defaultDir = QApplication::applicationDirPath() + "/" + cfg.duongDanThuMucIn + "/" + cfg.thuMucPdf;
        }
        QDir().mkpath(defaultDir);
        QString filePath = QFileDialog::getSaveFileName(
            parent,
            "Lưu Tem Sách Hàng Loạt",
            QDir(defaultDir).filePath(tenFileMacDinh),
            "PDF Files (*.pdf)"
        );

        if (!filePath.isEmpty()) {
            return xuatPDF(&doc, filePath, parent);
        }
    }
    
    return false;
}

bool PrintHelper::inDanhSachDocGia(PTRDG root, QWidget* parent) {
    if (!root) return false;
    
    QString noiDungBang = "<table border='1' cellpadding='5' cellspacing='0' style='width:100%; border-collapse:collapse;'>"
                          "<tr style='background:#f0f0f0;'>"
                          "<th>Mã thẻ</th><th>Họ tên</th><th>Giới tính</th><th>Trạng thái</th>"
                          "</tr>";
    
    // Duyệt cây in-order để thu thập dữ liệu
    std::function<void(PTRDG)> duyetCay = [&](PTRDG p) {
        if (!p) return;
        duyetCay(p->left);
        
        QString hoTen = QString::fromStdString(p->data.Ho + " " + p->data.Ten);
        QString phai = p->data.Phai ? "Nữ" : "Nam";
        QString trangThai = p->data.TrangThai == 1 ? "Hoạt động" : "Khóa";
        
        noiDungBang += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
                          .arg(p->data.MaThe)
                          .arg(hoTen)
                          .arg(phai)
                          .arg(trangThai);
        
        duyetCay(p->right);
    };
    
    duyetCay(root);
    noiDungBang += "</table>";
    
    QString html = taoHTMLDanhSach("DANH SÁCH ĐỘC GIẢ", noiDungBang);
    QTextDocument doc;
    doc.setHtml(html);
    return inDocument(&doc, parent);
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
    return inDocument(&doc, parent);
}

bool PrintHelper::inBaoCaoThongKe(const QString& tieuDe, const QString& noiDungHTML, QWidget* parent) {
    QString html = taoHTMLDanhSach(tieuDe, noiDungHTML);
    QTextDocument doc;
    doc.setHtml(html);
    return inDocument(&doc, parent);
}

bool PrintHelper::inPhieuMuonTra(PTRDG docGia, const QList<QString>& danhSachSach, bool laMuon, QWidget* parent) {
    if (!docGia) return false;
    
    QString loai = laMuon ? "PHIẾU MƯỢN SÁCH" : "PHIẾU TRẢ SÁCH";
    QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
    
    QString html = QString(
        "<html><head>"
        "<style>"
        "body { font-family: 'Times New Roman', serif; margin: 20px; }"
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

bool PrintHelper::inPhieuMuonTuDong(PTRDG docGia, const QString& maSach, const QString& tenSach,
                                     const QString& ngayMuon, QWidget* parent) {
    if (!docGia) return false;
    
    QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
    int maThe = docGia->data.MaThe;
    
    // Tạo tên file: PhieuMuon_MaThe_MaSach_Ngay.pdf
    QString ngayFile = QDate::currentDate().toString("yyyyMMdd");
    QString tenFile = QString("PhieuMuon_%1_%2_%3.pdf")
        .arg(maThe)
        .arg(maSach)
        .arg(ngayFile);
    
    // Tạo mã QR chứa thông tin: MaThe|MaSach|NgayMuon
    QString qrData = QString("%1|%2|%3").arg(maThe).arg(maSach).arg(ngayMuon);
    QString qrCode = taoMaQRHTML(qrData, 150);
    
    QString html = QString(
        "<html><head>"
        "<style>"
        "body { font-family: Arial; margin: 15px; font-size: 11pt; }"
        ".header { text-align: center; font-size: 16pt; font-weight: bold; margin-bottom: 15px; color: #2c3e50; }"
        ".section { margin: 10px 0; padding: 10px; background: #f8f9fa; border-radius: 5px; }"
        ".label { font-weight: bold; color: #555; width: 120px; display: inline-block; }"
        ".value { color: #000; }"
        ".qr-section { text-align: center; margin: 15px 0; padding: 15px; background: white; border: 2px solid #3498db; border-radius: 8px; }"
        ".footer { margin-top: 15px; text-align: center; font-size: 9pt; color: #888; }"
        "</style>"
        "</head><body>"
        "%7"
        "<div class='header'>📚 PHIẾU MƯỢN SÁCH</div>"
        "<div class='section'>"
        "  <div><span class='label'>Họ tên:</span><span class='value'>%1</span></div>"
        "  <div><span class='label'>Mã thẻ:</span><span class='value'>%2</span></div>"
        "  <div><span class='label'>Tên sách:</span><span class='value'>%3</span></div>"
        "  <div><span class='label'>Mã sách:</span><span class='value'>%4</span></div>"
        "  <div><span class='label'>Ngày mượn:</span><span class='value'>%5</span></div>"
        "</div>"
        "<div class='qr-section'>"
        "  <div style='font-size: 10pt; margin-bottom: 10px; color: #555;'>Quét mã QR để trả sách:</div>"
        "  %6"
        "</div>"
        "<div class='footer'>Vui lòng giữ phiếu này để trả sách nhanh hơn</div>"
        "</body></html>"
    ).arg(hoTen).arg(maThe).arg(tenSach).arg(maSach).arg(ngayMuon).arg(qrCode);
    // Insert logo if configured
    QString logoHtml;
    if (s_config.hienThiLogo && !s_config.duongDanLogo.isEmpty() && QFile::exists(s_config.duongDanLogo)) {
        QString logoPath = QDir::toNativeSeparators(s_config.duongDanLogo);
        logoHtml = QString("<div style='text-align:center; margin-bottom:6px;'><img src='file:///%1' style='height:36px;'/></div>");
        logoHtml = logoHtml.arg(logoPath);
    }
    html = html.arg(logoHtml);
    
    QTextDocument doc;
    doc.setHtml(html);
    
    // Hỏi người dùng muốn in hay lưu PDF
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("Chọn Hành Động");
    msgBox.setText("Bạn muốn in ngay hay lưu thành file PDF?");
    QPushButton* btnIn = msgBox.addButton("🖨️ In Ngay", QMessageBox::AcceptRole);
    QPushButton* btnPDF = msgBox.addButton("💾 Lưu PDF", QMessageBox::ActionRole);
    msgBox.addButton("❌ Hủy", QMessageBox::RejectRole);
    msgBox.setDefaultButton(btnIn);
    msgBox.exec();
    
    QAbstractButton* clicked = msgBox.clickedButton();
    if (clicked == btnIn) {
        return inDocument(&doc, parent);
    } else if (clicked == btnPDF) {
        QString ngayFile = QDate::currentDate().toString("yyyyMMdd");
        QString tenFile = QString("PhieuMuon_%1_%2_%3.pdf")
            .arg(maThe)
            .arg(maSach)
            .arg(ngayFile);

        PrintConfig cfg = getCauHinh();
        QString defaultDir;
        if (!cfg.pdfOutputDir.isEmpty()) {
            defaultDir = QDir(cfg.pdfOutputDir).isAbsolute() ? cfg.pdfOutputDir : QApplication::applicationDirPath() + "/" + cfg.pdfOutputDir;
        } else {
            defaultDir = QApplication::applicationDirPath() + "/" + cfg.duongDanThuMucIn + "/" + cfg.thuMucPdf;
        }
        QDir().mkpath(defaultDir);

        QString filePath = QFileDialog::getSaveFileName(
            parent,
            "Lưu Phiếu Mượn",
            QDir(defaultDir).filePath(tenFile),
            "PDF Files (*.pdf)"
        );

        if (!filePath.isEmpty()) {
            return xuatPDF(&doc, filePath, parent);
        }
    }
    
    return false;
}

QString PrintHelper::taoMaVachHTML(const QString& maSo, int fontSize) {
    return QString("<span style='font-family: \"Libre Barcode 128\", \"Code 128\"; font-size: %1px;'>*%2*</span>")
               .arg(fontSize)
               .arg(maSo);
}

// ===== TẠO TÊN FILE TỰ ĐỘNG =====
QString PrintHelper::taoTenFilePhieuMuon(int maThe, const QString& maSach) {
    QString ngay = QDate::currentDate().toString("yyyyMMdd");
    QString gio = QTime::currentTime().toString("HHmmss");
    return QString("PhieuMuon_MT%1_MS%2_%3_%4.pdf")
        .arg(maThe, 5, 10, QChar('0'))
        .arg(maSach)
        .arg(ngay)
        .arg(gio);
}

QString PrintHelper::taoTenFilePhieuTra(int maThe, const QString& maSach) {
    QString ngay = QDate::currentDate().toString("yyyyMMdd");
    QString gio = QTime::currentTime().toString("HHmmss");
    return QString("PhieuTra_MT%1_MS%2_%3_%4.pdf")
        .arg(maThe, 5, 10, QChar('0'))
        .arg(maSach)
        .arg(ngay)
        .arg(gio);
}

QString PrintHelper::taoTenFileTheDocGia(int maThe) {
    QString ngay = QDate::currentDate().toString("yyyyMMdd");
    return QString("TheDocGia_MT%1_%2.pdf")
        .arg(maThe, 5, 10, QChar('0'))
        .arg(ngay);
}

QString PrintHelper::taoTenFileTemSach(const QString& isbn, int soLuong) {
    QString ngay = QDate::currentDate().toString("yyyyMMdd");
    QString isbnClean = isbn;
    isbnClean.replace("-", "").replace(" ", "");
    return QString("TemSach_ISBN%1_SL%2_%3.pdf")
        .arg(isbnClean)
        .arg(soLuong)
        .arg(ngay);
}

QString PrintHelper::taoTenFileBaoCao(const QString& loaiBaoCao) {
    QString ngay = QDate::currentDate().toString("yyyyMMdd");
    QString gio = QTime::currentTime().toString("HHmm");
    return QString("BaoCao_%1_%2_%3.pdf")
        .arg(loaiBaoCao)
        .arg(ngay)
        .arg(gio);
}

QString PrintHelper::taoMaQRHTML(const QString& maSo, int size) {
    // Tạo ảnh QR code
    QImage qrImage = QrCodeGenerator::generateQrImage(maSo, size, 2);
    if (qrImage.isNull()) {
        return QString("<div style='color:red;'>Lỗi tạo QR code</div>");
    }
    // Lưu ảnh vào thư mục cấu hình (ưu tiên s_config.qrOutputDir nếu có)
    PrintConfig cfg = getCauHinh();
    QString qrDir;
    if (!cfg.qrOutputDir.isEmpty()) {
        // nếu người dùng cung cấp đường dẫn tương đối -> hiểu là relative với app dir
        if (QDir(cfg.qrOutputDir).isAbsolute()) qrDir = cfg.qrOutputDir;
        else qrDir = QApplication::applicationDirPath() + "/" + cfg.qrOutputDir;
    } else {
        QString base = QApplication::applicationDirPath() + "/" + cfg.duongDanThuMucIn;
        qrDir = base + "/" + cfg.thuMucQR;
    }

    if (!ensureDirExists(qrDir)) {
        return QString("<div style='color:red;'>Lỗi tạo thư mục QR</div>");
    }

    // tạo tên file an toàn: qr_<maSo>_<timestamp>.png
    QString safeMa = maSo;
    safeMa.replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");
    QString timeStamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmsszzz");
    QString qrPath = QDir(qrDir).filePath(QString("qr_%1_%2.png").arg(safeMa).arg(timeStamp));

    if (!qrImage.save(qrPath)) {
        return QString("<div style='color:red;'>Lỗi lưu QR code</div>");
    }

    // Trả về HTML tag với đường dẫn tuyệt đối
    return QString("<img src='file:///%1' width='%2' height='%2' style='border:1px solid #ccc; padding:2px; background:white;'/>")
               .arg(QDir::toNativeSeparators(qrPath))
               .arg(size);
}

bool PrintHelper::xuatPDF(QTextDocument* doc, const QString& tenFile, QWidget* parent) {
    if (!doc || tenFile.isEmpty()) return false;
    // Ensure target directory exists (if path relative to app dir and under files/pdf)
    QString outPath = tenFile;
    QFileInfo fi(outPath);
    if (!fi.dir().exists()) {
        if (!QDir().mkpath(fi.dir().absolutePath())) {
            QMessageBox::warning(parent, "Lỗi", "Không thể tạo thư mục chứa file PDF.");
            return false;
        }
    }

    QPrinter printer(QPrinter::HighResolution);
    // Respect color setting from configuration
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
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(outPath);
    printer.setPageSize(QPageSize::A4);
    printer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);

    doc->print(&printer);

    QMessageBox::information(parent, "Thành Công",
        QString("Đã lưu file PDF:\n%1").arg(outPath));

    return true;
}

void PrintHelper::setCauHinh(const PrintConfig& config) {
    s_config = config;
    
    QSettings settings("HVPG", "QuanLyThuVien");
    settings.setValue("Print/TenThuVien", config.tenThuVien);
    settings.setValue("Print/TenVietTat", config.tenVietTat);
    settings.setValue("Print/InMauSac", config.inMauSac);
    settings.setValue("Print/HienThiLogo", config.hienThiLogo);
    settings.setValue("Print/DuongDanLogo", config.duongDanLogo);
    settings.setValue("Print/ThuMucIn", config.duongDanThuMucIn);
    settings.setValue("Print/ThuMucQR", config.thuMucQR);
    settings.setValue("Print/ThuMucPDF", config.thuMucPdf);
    settings.setValue("Print/QROutputDir", config.qrOutputDir);
    settings.setValue("Print/PDFOutputDir", config.pdfOutputDir);
    settings.setValue("Print/TempDir", config.tempDir);
    settings.setValue("Print/HeaderText", config.headerText);
    settings.setValue("Print/FooterText", config.footerText);
    settings.setValue("Print/Author", config.author);
}

PrintHelper::PrintConfig PrintHelper::getCauHinh() {
    QSettings settings("HVPG", "QuanLyThuVien");
    
    PrintConfig config;
    config.tenThuVien = settings.value("Print/TenThuVien", config.tenThuVien).toString();
    config.tenVietTat = settings.value("Print/TenVietTat", config.tenVietTat).toString();
    config.inMauSac = settings.value("Print/InMauSac", config.inMauSac).toBool();
    config.hienThiLogo = settings.value("Print/HienThiLogo", config.hienThiLogo).toBool();
    config.duongDanLogo = settings.value("Print/DuongDanLogo", config.duongDanLogo).toString();
    config.duongDanThuMucIn = settings.value("Print/ThuMucIn", config.duongDanThuMucIn).toString();
    config.thuMucQR = settings.value("Print/ThuMucQR", config.thuMucQR).toString();
    config.thuMucPdf = settings.value("Print/ThuMucPDF", config.thuMucPdf).toString();
    config.qrOutputDir = settings.value("Print/QROutputDir", config.qrOutputDir).toString();
    config.pdfOutputDir = settings.value("Print/PDFOutputDir", config.pdfOutputDir).toString();
    config.tempDir = settings.value("Print/TempDir", config.tempDir).toString();
    config.headerText = settings.value("Print/HeaderText", config.headerText).toString();
    config.footerText = settings.value("Print/FooterText", config.footerText).toString();
    config.author = settings.value("Print/Author", config.author).toString();
    
    return config;
}

// ==================== PRIVATE HELPERS ====================

QString PrintHelper::taoHTMLTheDocGia(PTRDG docGia) {
    if (!docGia) return "";
    
    QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
    QString maThe = QString::number(docGia->data.MaThe);
    QString maQR = taoMaQRHTML(maThe, 60); // Sử dụng QR code thay vì barcode
    
    QString logoHtml;
    if (s_config.hienThiLogo && !s_config.duongDanLogo.isEmpty() && QFile::exists(s_config.duongDanLogo)) {
        QString logoPath = QDir::toNativeSeparators(s_config.duongDanLogo);
        logoHtml = QString("<div style='text-align:center; margin-bottom:4px;'><img src='file:///%1' style='height:32px;'/></div>").arg(logoPath);
    }

    return QString(
        "<html><head>"
        "<style>"
        "body { margin: 0; padding: 0; }"
        ".card { width: 85.6mm; height: 54mm; border: 1px solid black; "
        "        padding: 5mm; box-sizing: border-box; font-family: Arial; }"
        ".title { text-align: center; font-size: 14pt; font-weight: bold; margin-bottom: 3mm; }"
        ".name { text-align: center; font-size: 18pt; font-weight: bold; margin: 2mm 0; }"
        ".qrcode { text-align: center; margin: 3mm 0; }"
        ".id { text-align: center; font-size: 12pt; }"
        "</style>"
        "</head><body>"
        "<div class='card'>"
        "%1"
        "  <div class='title'>%2</div>"
        "  <div class='name'>%3</div>"
        "  <div class='qrcode'>%4</div>"
        "  <div class='id'>Mã thẻ: %5</div>"
        "</div>"
        "</body></html>"
    ).arg(logoHtml).arg(s_config.tenVietTat).arg(hoTen).arg(maQR).arg(maThe);
}

QString PrintHelper::taoHTMLA4NhieuThe(const QList<PTRDG>& danhSach, int soCot) {
    QString html = QString(
        "<html><head>"
        "<style>"
        "body { margin: 10mm; padding: 0; font-family: Arial; }"
        "table { width: 100%%; border-collapse: collapse; }"
        "td { padding: 3mm; vertical-align: top; }"
        ".card { width: 85.6mm; height: 54mm; border: 2px solid black; "
        "        padding: 4mm; box-sizing: border-box; background: white; }"
        ".title { text-align: center; font-size: 11pt; font-weight: bold; margin-bottom: 2mm; }"
        ".name { text-align: center; font-size: 14pt; font-weight: bold; margin: 2mm 0; }"
        ".qrcode { text-align: center; margin: 2mm 0; }"
        ".id { text-align: center; font-size: 10pt; }"
        "@media print {"
        "  .page-break { page-break-after: always; }"
        "}"
        "</style>"
        "</head><body>"
    );
    
    int thePerPage = soCot * 5; // 2 cột x 5 hàng = 10 thẻ/trang
    
    QString logoHtml;
    if (s_config.hienThiLogo && !s_config.duongDanLogo.isEmpty() && QFile::exists(s_config.duongDanLogo)) {
        QString logoPath = QDir::toNativeSeparators(s_config.duongDanLogo);
        logoHtml = QString("<div style='text-align:center; margin-bottom:6px;'><img src='file:///%1' style='height:28px;'/></div>").arg(logoPath);
    }

    for (int page = 0; page < (danhSach.size() + thePerPage - 1) / thePerPage; ++page) {
        if (page > 0) {
            html += "<div class='page-break'></div>";
        }
        
        html += "<table>";
        
        for (int row = 0; row < 5; ++row) {
            html += "<tr>";
            for (int col = 0; col < soCot; ++col) {
                int index = page * thePerPage + row * soCot + col;
                
                if (index < danhSach.size()) {
                    PTRDG docGia = danhSach[index];
                    QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
                    QString maThe = QString::number(docGia->data.MaThe);
                    QString maQR = taoMaQRHTML(maThe, 55);
                    
                    html += QString(
                        "<td>"
                        "<div class='card'>"
                        "  %6"
                        "  <div class='title'>%1</div>"
                        "  <div class='name'>%2</div>"
                        "  <div class='qrcode'>%3</div>"
                        "  <div class='id'>Mã: %4</div>"
                        "</div>"
                        "</td>"
                    ).arg(s_config.tenVietTat).arg(hoTen).arg(maQR).arg(maThe).arg(logoHtml);
                } else {
                    html += "<td></td>"; // Ô trống
                }
            }
            html += "</tr>";
        }
        
        html += "</table>";
    }
    
    html += "</body></html>";
    return html;
}

QString PrintHelper::taoHTMLTemSach(const QString& maSach, const QString& tenSach) {
    QString maQR = taoMaQRHTML(maSach, 50); // Sử dụng QR code
    QString logoHtml;
    if (s_config.hienThiLogo && !s_config.duongDanLogo.isEmpty() && QFile::exists(s_config.duongDanLogo)) {
        QString logoPath = QDir::toNativeSeparators(s_config.duongDanLogo);
        logoHtml = QString("<div style='text-align:center; margin-bottom:4px;'><img src='file:///%1' style='height:20px;'/></div>").arg(logoPath);
    }

    return QString(
        "<html><head>"
        "<style>"
        "body { margin: 0; padding: 0; }"
        ".label { width: 50mm; height: 30mm; border: 1px solid black; "
        "         padding: 2mm; box-sizing: border-box; font-family: Arial; }"
        ".title { font-size: 10pt; font-weight: bold; text-align: center; margin-bottom: 2mm; }"
        ".qrcode { text-align: center; margin: 2mm 0; }"
        ".code { text-align: center; font-size: 9pt; }"
        "</style>"
        "</head><body>"
        "<div class='label'>"
        "%1"
        "  <div class='title'>%2</div>"
        "  <div class='qrcode'>%3</div>"
        "  <div class='code'>%4</div>"
        "</div>"
        "</body></html>"
    ).arg(logoHtml).arg(tenSach.left(20)).arg(maQR).arg(maSach);
}

QString PrintHelper::taoHTMLDanhSach(const QString& tieuDe, const QString& noiDungBang) {
    QString logoHtml;
    if (s_config.hienThiLogo && !s_config.duongDanLogo.isEmpty() && QFile::exists(s_config.duongDanLogo)) {
        QString logoPath = QDir::toNativeSeparators(s_config.duongDanLogo);
        logoHtml = QString("<div style='text-align:center; margin-bottom:8px;'><img src='file:///%1' style='height:36px;'/></div>").arg(logoPath);
    }

    return QString(
        "<html><head>"
        "<style>"
        "body { font-family: 'Times New Roman', serif; margin: 20px; }"
        ".header { text-align: center; font-size: 18pt; font-weight: bold; margin-bottom: 10px; }"
        "table { width: 100%%; border-collapse: collapse; }"
        "th { background: #f0f0f0; font-weight: bold; }"
        "td, th { padding: 8px; border: 1px solid black; }"
        "</style>"
        "</head><body>"
        "%1"
        "<div class='header'>%2</div>"
        "%3"
        "</body></html>"
    ).arg(logoHtml).arg(tieuDe).arg(noiDungBang);
}

bool PrintHelper::inDocument(QTextDocument* doc, QWidget* parent) {
    if (!doc) return false;
    
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, parent);
    
    if (dialog.exec() == QDialog::Accepted) {
        doc->print(&printer);
        return true;
    }
    
    return false;
}

bool PrintHelper::kiemTraFontMaVach() {
    QStringList fonts = QFontDatabase::families();
    return fonts.contains("Libre Barcode 128") || fonts.contains("Code 128");
}

// ===== TỰ ĐỘNG IN (AUTO-PRINT) =====

bool PrintHelper::inTheDocGiaTuDong(PTRDG docGia, QWidget* parent) {
    if (!docGia) return false;
    
    QString html = taoHTMLTheDocGia(docGia);
    QTextDocument doc;
    doc.setHtml(html);
    
    // Dialog chọn In/Lưu/Hủy
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("In Thẻ Độc Giả");
    msgBox.setText(QString("In thẻ cho: %1 %2 (Mã: %3)")
        .arg(QString::fromStdString(docGia->data.Ho))
        .arg(QString::fromStdString(docGia->data.Ten))
        .arg(docGia->data.MaThe));
    QPushButton* btnIn = msgBox.addButton("🖨️ In Ngay", QMessageBox::AcceptRole);
    QPushButton* btnPDF = msgBox.addButton("💾 Lưu PDF", QMessageBox::ActionRole);
    msgBox.addButton("❌ Hủy", QMessageBox::RejectRole);
    msgBox.exec();
    
    QAbstractButton* clicked = msgBox.clickedButton();
    if (clicked == btnIn) {
        return inDocument(&doc, parent);
    } else if (clicked == btnPDF) {
        QString tenFileMacDinh = taoTenFileTheDocGia(docGia->data.MaThe);
        return xuatPDF(&doc, tenFileMacDinh, parent);
    }
    return false;
}

bool PrintHelper::inHangLoatTheDocGia(PTRDG root, QWidget* parent) {
    if (!root) return false;
    
    // Đếm số độc giả
    int soLuong = 0;
    std::function<void(PTRDG)> dem = [&](PTRDG p) {
        if (!p) return;
        dem(p->left);
        soLuong++;
        dem(p->right);
    };
    dem(root);
    
    if (soLuong == 0) {
        QMessageBox::information(parent, "Thông Báo", "Không có độc giả nào!");
        return false;
    }
    
    // Hỏi xác nhận
    QMessageBox::StandardButton reply = QMessageBox::question(
        parent,
        "Xác Nhận",
        QString("In %1 thẻ độc giả?\n\nQuá trình này có thể mất vài phút.").arg(soLuong),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) return false;
    
    // Thu thập danh sách
    QList<PTRDG> danhSach;
    std::function<void(PTRDG)> duyet = [&](PTRDG p) {
        if (!p) return;
        duyet(p->left);
        danhSach.append(p);
        duyet(p->right);
    };
    duyet(root);
    
    // In theo batch (10 thẻ/trang)
    return inA4NhieuThe(danhSach, 2, parent);
}

bool PrintHelper::inTemSachTuDong(PTRDS dauSach, QWidget* parent) {
    if (!dauSach) return false;
    
    QString isbn = QString::fromStdString(dauSach->ISBN);
    QString tenSach = QString::fromStdString(dauSach->tenSach);
    
    QString html = taoHTMLTemSach(isbn, tenSach);
    QTextDocument doc;
    doc.setHtml(html);
    
    // Dialog chọn
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("In Tem Sách");
    msgBox.setText(QString("In tem cho: %1").arg(tenSach));
    QPushButton* btnIn = msgBox.addButton("🖨️ In Ngay", QMessageBox::AcceptRole);
    QPushButton* btnPDF = msgBox.addButton("💾 Lưu PDF", QMessageBox::ActionRole);
    msgBox.addButton("❌ Hủy", QMessageBox::RejectRole);
    msgBox.exec();
    
    QAbstractButton* clicked = msgBox.clickedButton();
    if (clicked == btnIn) {
        return inDocument(&doc, parent);
    } else if (clicked == btnPDF) {
        QString tenFileMacDinh = taoTenFileTemSach(isbn, 1);
        return xuatPDF(&doc, tenFileMacDinh, parent);
    }
    return false;
}

bool PrintHelper::inDanhMucSach(PTRDS* dsDauSach, int soLuong, QWidget* parent) {
    if (!dsDauSach || soLuong == 0) return false;
    
    // Tạo bảng HTML
    QString bangHTML = "<table><tr>"
                       "<th>STT</th><th>ISBN</th><th>Tên Sách</th>"
                       "<th>Tác Giả</th><th>Thể Loại</th><th>Số Bản</th>"
                       "</tr>";
    
    for (int i = 0; i < soLuong; i++) {
        if (!dsDauSach[i]) continue;
        
        // Đếm số bản sao
        int soBan = 0;
        PTRDMS p = dsDauSach[i]->dms;
        while (p) {
            soBan++;
            p = p->next;
        }
        
        bangHTML += QString("<tr>"
                           "<td>%1</td><td>%2</td><td>%3</td>"
                           "<td>%4</td><td>%5</td><td>%6</td>"
                           "</tr>")
            .arg(i + 1)
            .arg(QString::fromStdString(dsDauSach[i]->ISBN))
            .arg(QString::fromStdString(dsDauSach[i]->tenSach))
            .arg(QString::fromStdString(dsDauSach[i]->tacGia))
            .arg(QString::fromStdString(dsDauSach[i]->theLoai))
            .arg(soBan);
    }
    bangHTML += "</table>";
    
    QString html = taoHTMLDanhSach("DANH MỤC SÁCH THƯ VIỆN", bangHTML);
    QTextDocument doc;
    doc.setHtml(html);
    
    // Dialog chọn
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("In Danh Mục Sách");
    msgBox.setText(QString("In danh mục %1 đầu sách").arg(soLuong));
    QPushButton* btnIn = msgBox.addButton("🖨️ In Ngay", QMessageBox::AcceptRole);
    QPushButton* btnPDF = msgBox.addButton("💾 Lưu PDF", QMessageBox::ActionRole);
    msgBox.addButton("❌ Hủy", QMessageBox::RejectRole);
    msgBox.exec();
    
    QAbstractButton* clicked = msgBox.clickedButton();
    if (clicked == btnIn) {
        return inDocument(&doc, parent);
    } else if (clicked == btnPDF) {
        QString tenFileMacDinh = taoTenFileBaoCao("DanhMucSach");
        return xuatPDF(&doc, tenFileMacDinh, parent);
    }
    return false;
}

