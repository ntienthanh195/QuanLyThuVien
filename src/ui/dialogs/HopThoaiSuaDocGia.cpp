#include "utils/LanguageManager.h"
#include "ui/dialogs/HopThoaiSuaDocGia.h"
#include "models/DocGia.h"
#include "utils/ResponsiveHelper.h"
#include "utils/XuLyChuoi.h"
#include "utils/EventBus.h"
#include "utils/ToastNotification.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QTimer>
#include <QScrollArea>

HopThoaiSuaDocGia::HopThoaiSuaDocGia(int maThe, QWidget *cha)
    : QDialog(cha), maDocGia(maThe), docGiaHienTai({}) {
    setWindowTitle(LanguageManager::instance().getText("DIALOG_EDIT_READER_TITLE"));
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    setMinimumWidth(ResponsiveHelper::getScaledWidth(550, screenSize));
    
    extern PTRDG rootDocGia;
    docGiaHienTai = timDocGia(rootDocGia, maThe);
    
    // Apply Deep Night Premium Theme
    this->setStyleSheet("QDialog { background-color: #1e1e2d; color: #ffffff; }");

    thieLapGiaoDien();
    taiDuLieuDocGia();
}

void HopThoaiSuaDocGia::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(25, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    
    QLabel *tieuDe = new QLabel("👤 " + LanguageManager::instance().getText("TITLE_EDIT_READER_HEADER"));
    int titleSize = ResponsiveHelper::getScaledFontSize(20, screenSize);
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF;").arg(titleSize));
    tieuDe->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(tieuDe);
    
    // Scroll Area Setup
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; } QScrollBar:vertical { width: 8px; background: #1b1b29; } QScrollBar::handle:vertical { background: #565674; border-radius: 4px; }");

    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 10, 0); 
    scrollLayout->setSpacing(15);
    
    QGroupBox *nhomForm = new QGroupBox(LanguageManager::instance().getText("GROUP_READER_INFO"), this);
    int fontSize = ResponsiveHelper::getScaledFontSize(14, screenSize);
    nhomForm->setStyleSheet(QString(
        "QGroupBox {"
        "   background-color: #1e1e2d;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   margin-top: 12px;"
        "   padding-top: 18px;"
        "   font-weight: bold;"
        "   font-size: %1px;"
        "   color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   left: 10px;"
        "   top: -10px;"
        "   padding: 0 5px;"
        "   color: #5D9CEC;"
        "   background-color: #1e1e2d;"
        "   font-size: 12px;"
        "}"
    ).arg(fontSize));
    QFormLayout *boTriForm = new QFormLayout(nhomForm);
    boTriForm->setHorizontalSpacing(20);
    boTriForm->setVerticalSpacing(12);
    boTriForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    int fieldHeight = ResponsiveHelper::getScaledHeight(40, screenSize);
    QString kieuTruong = QString(
        "QLineEdit, QComboBox {"
        "   padding: 0 10px;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   font-size: %1px;"
        "   background-color: #1b1b29;"
        "   color: #ffffff;"
        "   min-height: %2px;"
        "   selection-background-color: #5D9CEC;"
        "}"
        "QLineEdit:focus, QComboBox:focus {"
        "   border: 1px solid #5D9CEC;"
        "   background-color: #1b1b29;"
        "}"
    ).arg(fontSize).arg(fieldHeight);
    
    oNhapMaThe = new QLineEdit();
    oNhapMaThe->setReadOnly(true);
    oNhapMaThe->setStyleSheet("background-color: #151521; color: #565674; padding: 0 10px; border: 1px solid #323248; border-radius: 6px; min-height: " + QString::number(fieldHeight) + "px;");
    QLabel *lblMaThe = new QLabel(LanguageManager::instance().getText("LBL_READER_ID"));
    lblMaThe->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblMaThe, oNhapMaThe);
    
    oNhapHo = new QLineEdit();
    oNhapHo->setStyleSheet(kieuTruong);
    connect(oNhapHo, &QLineEdit::textChanged, this, &HopThoaiSuaDocGia::kiemTraDuLieu);
    connect(oNhapHo, &QLineEdit::textChanged, this, &HopThoaiSuaDocGia::capNhatPreviewHoTen);
    QLabel *lblHo = new QLabel(LanguageManager::instance().getText("LBL_HO_DEM"));
    lblHo->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblHo, oNhapHo);
    
    oNhapTen = new QLineEdit();
    oNhapTen->setStyleSheet(kieuTruong);
    connect(oNhapTen, &QLineEdit::textChanged, this, &HopThoaiSuaDocGia::kiemTraDuLieu);
    connect(oNhapTen, &QLineEdit::textChanged, this, &HopThoaiSuaDocGia::capNhatPreviewHoTen);
    QLabel *lblTen = new QLabel(LanguageManager::instance().getText("LBL_TEN"));
    lblTen->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblTen, oNhapTen);
    
    hopChonGioiTinh = new QComboBox();
    hopChonGioiTinh->addItem(LanguageManager::instance().getText("GENDER_MALE"), 0);
    hopChonGioiTinh->addItem(LanguageManager::instance().getText("GENDER_FEMALE"), 1);
    hopChonGioiTinh->setStyleSheet(kieuTruong);
    QLabel *lblGioiTinh = new QLabel(LanguageManager::instance().getText("TABLE_HEADER_GENDER") + ":");
    lblGioiTinh->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblGioiTinh, hopChonGioiTinh);

    // Phase 2 New Fields
    hopChonLoaiDocGia = new QComboBox();
    hopChonLoaiDocGia->addItem(LanguageManager::instance().getText("READER_TYPE_STUDENT"), 0);
    hopChonLoaiDocGia->addItem(LanguageManager::instance().getText("READER_TYPE_TEACHER"), 1);
    hopChonLoaiDocGia->setStyleSheet(kieuTruong);

    oNhapLop = new QLineEdit(); 
    oNhapLop->setStyleSheet(kieuTruong);
    
    oNhapSDT = new QLineEdit();
    oNhapSDT->setStyleSheet(kieuTruong);
    connect(oNhapSDT, &QLineEdit::textChanged, this, &HopThoaiSuaDocGia::kiemTraDuLieu);

    oNhapEmail = new QLineEdit();
    oNhapEmail->setStyleSheet(kieuTruong);

    oNhapNgaySinh = new QDateEdit();
    oNhapNgaySinh->setDisplayFormat("dd/MM/yyyy");
    oNhapNgaySinh->setCalendarPopup(true);
    oNhapNgaySinh->setStyleSheet(kieuTruong);

    QLabel *lblLoai = new QLabel(LanguageManager::instance().getText("LBL_READER_TYPE"));
    lblLoai->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    QLabel *lblLop = new QLabel(LanguageManager::instance().getText("LBL_CLASS"));
    lblLop->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    QLabel *lblSDT = new QLabel(LanguageManager::instance().getText("LBL_PHONE") + " (*)");
    lblSDT->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    QLabel *lblEmail = new QLabel(LanguageManager::instance().getText("LBL_EMAIL"));
    lblEmail->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    QLabel *lblDOB = new QLabel(LanguageManager::instance().getText("LBL_DOB"));
    lblDOB->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");

    boTriForm->addRow(lblLoai, hopChonLoaiDocGia);
    boTriForm->addRow(lblLop, oNhapLop);
    boTriForm->addRow(lblDOB, oNhapNgaySinh);
    boTriForm->addRow(lblSDT, oNhapSDT);
    boTriForm->addRow(lblEmail, oNhapEmail);
    
    hopChonTrangThai = new QComboBox();
    hopChonTrangThai->addItem(LanguageManager::instance().getText("STATUS_ACTIVE"), 0);
    hopChonTrangThai->addItem(LanguageManager::instance().getText("STATUS_LOCKED"), 1);
    hopChonTrangThai->setStyleSheet(kieuTruong);
    QLabel *lblTrangThai = new QLabel(LanguageManager::instance().getText("LBL_STATUS"));
    lblTrangThai->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 12px; min-width: 100px;");
    boTriForm->addRow(lblTrangThai, hopChonTrangThai);
    
    scrollLayout->addWidget(nhomForm);
    
    nhanSoSachDangMuon = new QLabel("");
    nhanSoSachDangMuon->setStyleSheet("color: #B5B5C3; padding: 10px; background-color: #252534; border-radius: 8px;");
    nhanSoSachDangMuon->setAlignment(Qt::AlignCenter);
    scrollLayout->addWidget(nhanSoSachDangMuon);
    
    nhanTrangThai = new QLabel("");
    nhanTrangThai->setAlignment(Qt::AlignCenter);
    scrollLayout->addWidget(nhanTrangThai);

    nhanPreviewHoTen = new QLabel("");
    nhanPreviewHoTen->setStyleSheet("color: #A1F0E8; font-weight: 600; padding: 6px; background: transparent;");
    nhanPreviewHoTen->setAlignment(Qt::AlignCenter);
    scrollLayout->addWidget(nhanPreviewHoTen);

    // FINALIZE
    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);
    boTriChinh->addWidget(scrollArea);
    
    QHBoxLayout *boTriNut = new QHBoxLayout();
    int buttonHeight = ResponsiveHelper::getScaledHeight(45, screenSize);
    int buttonWidth = ResponsiveHelper::getScaledWidth(130, screenSize);
    
    nutLuu = new QPushButton(LanguageManager::instance().getText("BTN_SAVE_CHANGES"));
    nutLuu->setMinimumSize(buttonWidth, buttonHeight);
    nutLuu->setCursor(Qt::PointingHandCursor);
    nutLuu->setStyleSheet(
        "QPushButton {"
        "   background-color: #5D9CEC;"
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 6px;"
        "   font-weight: bold;"
        "   font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #4A89DC; }"
        "QPushButton:pressed { background-color: #3b6bbf; }"
    );
    connect(nutLuu, &QPushButton::clicked, this, &HopThoaiSuaDocGia::xuLyNutLuu);
    
    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CANCEL"));
    nutHuy->setMinimumSize(buttonWidth, buttonHeight);
    nutHuy->setCursor(Qt::PointingHandCursor);
    nutHuy->setStyleSheet(
        "QPushButton {"
        "   background-color: #2b2b40;"
        "   color: #ffffff;"
        "   border: 1px solid #323248;"
        "   border-radius: 6px;"
        "   font-weight: 600;"
        "   font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #323248; border-color: #f64e60; }"
        "QPushButton:pressed { background-color: #1b1b29; }"
    );
    connect(nutHuy, &QPushButton::clicked, this, &QDialog::reject);
    
    boTriNut->addStretch();
    boTriNut->addWidget(nutLuu);
    boTriNut->addWidget(nutHuy);
    boTriChinh->addLayout(boTriNut);
}

void HopThoaiSuaDocGia::taiDuLieuDocGia() {
    if (!docGiaHienTai) {
        QMessageBox::critical(this, LanguageManager::instance().getText("TITLE_ERROR"), LanguageManager::instance().getText("MSG_READER_NOT_FOUND"));
        reject();
        return;
    }
    
    oNhapMaThe->setText(QString::number(docGiaHienTai->data.MaThe));
    oNhapHo->setText(QString::fromStdString(docGiaHienTai->data.Ho));
    oNhapTen->setText(QString::fromStdString(docGiaHienTai->data.Ten));
    hopChonGioiTinh->setCurrentIndex(docGiaHienTai->data.Phai == "Nu" ? 1 : 0);
    hopChonTrangThai->setCurrentIndex(docGiaHienTai->data.TrangThai);
    
    // Phase 2 Load
    oNhapLop->setText(QString::fromStdString(docGiaHienTai->data.Lop));
    oNhapSDT->setText(QString::fromStdString(docGiaHienTai->data.SDT));
    oNhapEmail->setText(QString::fromStdString(docGiaHienTai->data.Email));
    hopChonLoaiDocGia->setCurrentIndex(docGiaHienTai->data.LoaiDocGia);
    if (!docGiaHienTai->data.NgaySinh.empty()) {
        oNhapNgaySinh->setDate(QDate::fromString(QString::fromStdString(docGiaHienTai->data.NgaySinh), "dd/MM/yyyy"));
    } else {
        oNhapNgaySinh->setDate(QDate::currentDate().addYears(-15));
    }
    
    nhanSoSachDangMuon->setText(LanguageManager::instance().getText("LBL_BORROWED_COUNT_HINT").arg(docGiaHienTai->data.soSachDangMuon));
}

void HopThoaiSuaDocGia::kiemTraDuLieu() {
    bool hopLe = !oNhapHo->text().trimmed().isEmpty() && 
                 !oNhapTen->text().trimmed().isEmpty() &&
                 !oNhapSDT->text().trimmed().isEmpty();
    nutLuu->setEnabled(hopLe);
}

void HopThoaiSuaDocGia::xuLyNutLuu() {
    if (!docGiaHienTai) return;
    
    // Disable buttons
    nutLuu->setEnabled(false);
    nutHuy->setEnabled(false);
    nutLuu->setText(LanguageManager::instance().getText("BTN_PROCESSING"));
    
    nhanTrangThai->setText(LanguageManager::instance().getText("MSG_UPDATING"));
    nhanTrangThai->setStyleSheet("color: #FFA800; font-weight: bold;");
    QApplication::processEvents();
    
    std::string ho = oNhapHo->text().trimmed().toStdString();
    std::string ten = oNhapTen->text().trimmed().toStdString();
    
    // CHUẨN HÓA HỌA TÊN
    std::string hoChuan, tenChuan;
    
    std::string loiHo = ChuanHoaTenUnicode(ho, 1, 50, hoChuan);
    if (!loiHo.empty()) {
        nhanTrangThai->setText("✕ " + QString::fromStdString(loiHo));
        nhanTrangThai->setStyleSheet("color: #E06D6D; font-weight: bold; font-size: 13px;");
        oNhapHo->setFocus();
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_SAVE"));
        return;
    }
    
    std::string loiTen = ChuanHoaTenUnicode(ten, 1, 20, tenChuan);
    if (!loiTen.empty()) {
        nhanTrangThai->setText("✕ " + QString::fromStdString(loiTen));
        nhanTrangThai->setStyleSheet("color: #E06D6D; font-weight: bold; font-size: 13px;");
        oNhapTen->setFocus();
        nutLuu->setEnabled(true);
        nutHuy->setEnabled(true);
        nutLuu->setText(LanguageManager::instance().getText("BTN_SAVE"));
        return;
    }
    
    // Tạo struct mới với dữ liệu đã cập nhật và chuẩn hóa
    TheDocGia duLieuMoi = docGiaHienTai->data;
    duLieuMoi.Ho = hoChuan;
    duLieuMoi.Ten = tenChuan;
    duLieuMoi.Phai = (hopChonGioiTinh->currentIndex() == 1 ? "Nu" : "Nam"); // 0=Nam, 1=Nữ
    duLieuMoi.TrangThai = (TrangThaiTheDocGia)hopChonTrangThai->currentIndex();
    // Phase 2 Save
    // Phase 2 Save
    duLieuMoi.LoaiDocGia = hopChonLoaiDocGia->currentIndex();
    
    std::string lopChuan = ChuanHoaKhoangTrang(oNhapLop->text().trimmed().toStdString());
    duLieuMoi.Lop = ChuyenInHoa(lopChuan);
    
    std::string sdtChuan = LayChiCacKyTuSo(oNhapSDT->text().trimmed().toStdString());
    duLieuMoi.SDT = sdtChuan;
    
    std::string emailChuan = ChuanHoaKhoangTrang(oNhapEmail->text().trimmed().toStdString());
    duLieuMoi.Email = ChuyenInThuong(emailChuan);
    duLieuMoi.NgaySinh = oNhapNgaySinh->date().toString("dd/MM/yyyy").toStdString();
    
    extern PTRDG rootDocGia;
    hieuChinhDocGia(rootDocGia, duLieuMoi);
    EventBus::instance().docGiaUpdated(duLieuMoi.MaThe); // REALTIME UPDATE
    
    nhanTrangThai->setText(LanguageManager::instance().getText("MSG_UPDATE_SUCCESS_VAR")
                          .arg(QString::fromStdString(hoChuan))
                          .arg(QString::fromStdString(tenChuan)));
    nhanTrangThai->setStyleSheet("color: #78C2AD; font-weight: bold; font-size: 13px;");
    
    emit docGiaDaSua();
    
    // Show Toast
    ToastNotification::show(LanguageManager::instance().getText("MSG_UPDATE_SUCCESS_VAR")
                           .arg(QString::fromStdString(hoChuan))
                           .arg(QString::fromStdString(tenChuan)), ToastNotification::SUCCESS);
                           
    accept(); // Exit immediately
    // QTimer removed
}

void HopThoaiSuaDocGia::capNhatPreviewHoTen() {
    std::string ho = oNhapHo->text().trimmed().toStdString();
    std::string ten = oNhapTen->text().trimmed().toStdString();
    std::string hoChuan, tenChuan;
    ChuanHoaTenUnicode(ho, 1, 50, hoChuan);
    ChuanHoaTenUnicode(ten, 1, 20, tenChuan);
    std::string full = hoChuan + " " + tenChuan;
    nhanPreviewHoTen->setText(QString::fromStdString(full));
}
