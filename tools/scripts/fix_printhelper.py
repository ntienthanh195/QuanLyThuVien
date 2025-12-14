
import os

path = r'e:\Qt\QuanLyThuVien\src\utils\PrintHelper.cpp'

if not os.path.exists(path):
    print(f"File not found: {path}")
    exit(1)

with open(path, 'r', encoding='utf-8', errors='ignore') as f:
    text = f.read()

# Find the end of getCauHinh
start_marker = 'PrintHelper::PrintConfig PrintHelper::getCauHinh() {'
start_idx = text.find(start_marker)

if start_idx == -1:
    print('Error: Could not find getCauHinh')
    exit(1)

# Find the closing brace of getCauHinh
# It ends with 'return config;\n\n}' usually.
# Let's be robust and find the matching brace for getCauHinh
brace_count = 0
found_start = False
cutoff_point = -1

for i in range(start_idx, len(text)):
    if text[i] == '{':
        brace_count += 1
        found_start = True
    elif text[i] == '}':
        brace_count -= 1
        if found_start and brace_count == 0:
            cutoff_point = i + 1
            break

if cutoff_point == -1:
    print('Error: Could not find closing brace of getCauHinh')
    exit(1)

clean_content = text[:cutoff_point]

# Ensure we have the headers/helpers from the first part.
# Append the NEW Slip Logic.

slip_code = r'''

// ==================== PHIẾU MƯỢN / TRẢ SÁCH ====================

QString PrintHelper::taoHTMLPhieuMuon(PTRDG docGia, const QString& maSach, const QString& tenSach, const QString& ngayGiaoDich) {
    if (!docGia) return "";
    QString qrCode = taoMaQRHTML(maSach, 64);
    QString logoHtml;
    if (s_config.hienThiLogo && !s_config.duongDanLogo.isEmpty() && QFile::exists(s_config.duongDanLogo)) {
        QString logoPath = QDir::toNativeSeparators(s_config.duongDanLogo);
        logoHtml = QString("<img src='file:///%1' style='height:30px; margin-bottom:5px;'/>").arg(logoPath);
    }

    return QString(
        "<html><head><style>"
        "body { font-family: 'Arial', sans-serif; width: 72mm; margin: 0; padding: 2px; }"
        ".ticket { border: 1px dashed #333; padding: 5px; border-radius: 4px; }"
        ".header { text-align: center; border-bottom: 2px solid #000; padding-bottom: 5px; margin-bottom: 5px; }"
        ".title { font-size: 14pt; font-weight: bold; margin: 5px 0; }"
        ".info { font-size: 10pt; line-height: 1.4; }"
        ".footer { margin-top: 10px; font-size: 8pt; text-align: center; font-style: italic; }"
        ".qr { text-align: center; margin-top: 5px; }"
        "</style></head><body>"
        "<div class='ticket'>"
        "  <div class='header'>"
        "    %1<br>"
        "    <b>%2</b>"
        "    <div class='title'>PHIẾU MƯỢN SÁCH</div>"
        "  </div>"
        "  <div class='info'>"
        "    <b>Độc giả:</b> %3 %4<br>"
        "    <b>Mã thẻ:</b> %5<br>"
        "    <hr style='border-top: 1px dotted #ccc;'>"
        "    <b>Sách:</b> %6<br>"
        "    <b>Mã sách:</b> %7<br>"
        "    <b>Ngày mượn:</b> %8"
        "  </div>"
        "  <div class='qr'>%9</div>"
        "  <div class='footer'>"
        "    Vui lòng giữ phiếu để đối chiếu.<br>"
        "    Xin cảm ơn!"
        "  </div>"
        "</div>"
        "</body></html>"
    ).arg(logoHtml)
     .arg(s_config.tenVietTat)
     .arg(QString::fromStdString(docGia->data.Ho))
     .arg(QString::fromStdString(docGia->data.Ten))
     .arg(docGia->data.MaThe)
     .arg(tenSach)
     .arg(maSach)
     .arg(ngayGiaoDich)
     .arg(qrCode);
}

bool PrintHelper::inPhieuMuonTuDong(PTRDG docGia, const QString& maSach, const QString& tenSach, const QString& ngayMuon, QWidget* parent) {
    QString html = taoHTMLPhieuMuon(docGia, maSach, tenSach, ngayMuon);
    QTextDocument doc;
    doc.setHtml(html);
    
    // Auto-Print (Silent)
    if (s_config.silentPrint) {
        return inDocument(&doc, parent);
    }

    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("In Phiếu Mượn");
    msgBox.setText("In phiếu mượn cho độc giả?");
    QPushButton* btnIn = msgBox.addButton("🖨️ In Ngay", QMessageBox::AcceptRole);
    QPushButton* btnPDF = msgBox.addButton("💾 Lưu PDF", QMessageBox::ActionRole);
    msgBox.addButton("❌ Hủy", QMessageBox::RejectRole);
    msgBox.exec();
    
    if (msgBox.clickedButton() == btnIn) {
        return inDocument(&doc, parent);
    } else if (msgBox.clickedButton() == btnPDF) {
        QString tenFile = taoTenFilePhieuMuon(docGia->data.MaThe, maSach);
        return xuatPDF(&doc, tenFile, parent);
    }
    return false;
}

bool PrintHelper::inPhieuTraTuDong(PTRDG docGia, const QString& maSach, const QString& tenSach, const QString& ngayTra, QWidget* parent) {
    QString html = taoHTMLPhieuMuon(docGia, maSach, tenSach, ngayTra);
    html.replace("PHIẾU MƯỢN SÁCH", "PHIẾU TRẢ SÁCH");
    html.replace("Ngày mượn:", "Ngày trả:");
    
    QTextDocument doc;
    doc.setHtml(html);
    
    if (s_config.silentPrint) {
        return inDocument(&doc, parent);
    }

    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("In Phiếu Trả");
    msgBox.setText("In phiếu trả sách?");
    QPushButton* btnIn = msgBox.addButton("🖨️ In Ngay", QMessageBox::AcceptRole);
    QPushButton* btnPDF = msgBox.addButton("💾 Lưu PDF", QMessageBox::ActionRole);
    msgBox.addButton("❌ Hủy", QMessageBox::RejectRole);
    msgBox.exec();
    
    if (msgBox.clickedButton() == btnIn) {
        return inDocument(&doc, parent);
    } else if (msgBox.clickedButton() == btnPDF) {
        QString tenFile = taoTenFilePhieuTra(docGia->data.MaThe, maSach);
        return xuatPDF(&doc, tenFile, parent);
    }
    return false;
}

// Append missing helper definition if it was cut off
// Wait, taoHTMLTheDocGia was typically after getCauHinh.
// Let's redefine it here to be safe? No, that causes redefinition errors if it exists.
// We truncated after getCauHinh. Previously taoHTMLTheDocGia was AFTER getCauHinh.
// So we MUST redefine it here, OR we assume we don't need it?
// inTheDocGiaTuDong calls it. So we need it.

QString PrintHelper::taoHTMLTheDocGia(PTRDG docGia) {
    if (!docGia) return "";
    
    QString hoTen = QString::fromStdString(docGia->data.Ho + " " + docGia->data.Ten);
    QString maThe = QString::number(docGia->data.MaThe);
    QString maQR = taoMaQRHTML(maThe, 55);
    
    return QString(
        "<html><head><style>"
        "body { margin: 0; padding: 0; font-family: Arial; }"
        ".card { width: 85.6mm; height: 53.5mm; border: 1px solid #333; border-radius: 4mm; padding: 5px; position: relative; }"
        ".header { font-size: 8pt; text-align: center; border-bottom: 1px solid #ccc; }"
        ".content { margin-top: 10px; }"
        ".footer { position: absolute; bottom: 5px; width: 100%%; text-align: center; font-size: 6pt; }"
        "</style></head><body>"
        "<div class='card'>"
        "  <div class='header'>%1</div>"
        "  <div class='content'>"
        "    <b>Name:</b> %2<br>"
        "    <b>ID:</b> %3<br>"
        "  </div>"
        "  <div class='footer'>%4</div>"
        "</div>"
        "</body></html>"
    ).arg(s_config.tenThuVien).arg(hoTen).arg(maThe).arg(s_config.tenVietTat);
}

'''

with open(path, 'w', encoding='utf-8') as f:
    f.write(clean_content + slip_code)

print('Successfully repaired PrintHelper.cpp')
