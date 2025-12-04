// PATCH CHO WidgetMuonTraSach.cpp  
// Copy 2 hàm này vào cuối file nguon/giao_dien/WidgetMuonTraSach.cpp (sau dòng 837)

void WidgetMuonTraSach::xuLyNutQuetMaDocGia() {
    HopThoaiQuetMaVach *dialogQuet = new HopThoaiQuetMaVach(HopThoaiQuetMaVach::QUET_DOC_GIA, this);
    if (dialogQuet->exec() == QDialog::Accepted) {
        QString maThe = dialogQuet->layMaQuet();
        if (!maThe.isEmpty()) {
            oNhapTimDocGia->setText(maThe);
            xuLyTimKiem();
        }
    }
    delete dialogQuet;
}

void WidgetMuonTraSach::xuLyNutQuetMaSach() {
    if (!docGiaHienTai) {
        QMessageBox::warning(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Vui lòng chọn độc giả trước!"));
        return;
    }
    HopThoaiQuetMaVach *dialogQuet = new HopThoaiQuetMaVach(HopThoaiQuetMaVach::QUET_SACH, this);
    if (dialogQuet->exec() == QDialog::Accepted) {
        QString isbn = dialogQuet->layMaQuet();
        if (!isbn.isEmpty()) {
            oNhapISBN->setText(isbn);
            extern PTRDS dsDauSach[];
            extern int soLuongDauSach;
            bool timThay = false;
            for (int i = 0; i < soLuongDauSach; i++) {
                if (dsDauSach[i] && QString::fromStdString(dsDauSach[i]->ISBN) == isbn) {
                    timThay = true;
                    hopXacNhanMuon->setChecked(true);
                    break;
                }
            }
            if (!timThay) {
                QMessageBox::warning(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Không tìm thấy sách với ISBN: ") + isbn);
            }
        }
    }
    delete dialogQuet;
}
