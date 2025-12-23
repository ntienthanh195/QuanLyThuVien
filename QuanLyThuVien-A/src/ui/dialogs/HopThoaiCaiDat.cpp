#include "ui/dialogs/HopThoaiCaiDat.h"
#include "utils/LanguageManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QSettings>
#include <QMessageBox>
#include "utils/SettingsManager.h"
#include "utils/AuditLogger.h"

HopThoaiCaiDat::HopThoaiCaiDat(QWidget *parent) 
    : QDialog(parent)
    , tabs(nullptr)
    , lblTenThuVien(nullptr)
    , lblDiaChi(nullptr)
    , oTenThuVien(nullptr)
    , oDiaChi(nullptr)
    , lblTienPhat(nullptr)
    , lblNgayMuon(nullptr)
    , lblLanGiaHan(nullptr)
    , lblSachMuon(nullptr)
    , oTienPhatMoiNgay(nullptr)
    , oSoNgayMuonToiDa(nullptr)
    , oSoLanGiaHanToiDa(nullptr)
    , oSoSachMuonToiDa(nullptr)
    , lblThoiHanThe(nullptr)
    , lblPhiMatSach(nullptr)
    , oThoiHanThe(nullptr)
    , oPhiMatSach(nullptr)
    , nutLuu(nullptr)
    , nutHuy(nullptr)
    , nutMacDinh(nullptr)
{
    setWindowTitle(LanguageManager::instance().getText("DIALOG_SETTINGS_TITLE"));
    setMinimumWidth(500);
    setStyleSheet("QDialog { background-color: #1e1e2d; color: #ffffff; } QLabel { color: #e0e0e0; }");
    
    thietLapGiaoDien();
    loadData();
    
    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &HopThoaiCaiDat::capNhatNgonNgu);
    capNhatNgonNgu(); // Initial set
}

void HopThoaiCaiDat::capNhatNgonNgu() {
    setWindowTitle(LanguageManager::instance().getText("DIALOG_SETTINGS_TITLE"));
    
    if (tabs) {
        tabs->setTabText(0, LanguageManager::instance().getText("TAB_GENERAL_INFO"));
        tabs->setTabText(1, LanguageManager::instance().getText("TAB_BORROW_RULES"));
    }
    
    if (lblTenThuVien) lblTenThuVien->setText(LanguageManager::instance().getText("LBL_LIBRARY_NAME") + ":");
    if (lblDiaChi) lblDiaChi->setText(LanguageManager::instance().getText("LBL_ADDRESS") + ":");
    
    if (lblTienPhat) lblTienPhat->setText(LanguageManager::instance().getText("LBL_FINE_PER_DAY") + ":");
    if (lblNgayMuon) lblNgayMuon->setText(LanguageManager::instance().getText("LBL_MAX_BORROW_DAYS") + ":");
    if (lblLanGiaHan) lblLanGiaHan->setText(LanguageManager::instance().getText("LBL_MAX_RENEWALS") + ":");
    if (lblSachMuon) lblSachMuon->setText(LanguageManager::instance().getText("LBL_MAX_BORROW_BOOKS") + ":");
    if (lblThoiHanThe) lblThoiHanThe->setText(LanguageManager::instance().getText("LBL_CARD_VALIDITY_MONTHS"));
    if (lblPhiMatSach) lblPhiMatSach->setText(LanguageManager::instance().getText("LBL_LOST_BOOK_FEE"));
    
    
    if (nutLuu) nutLuu->setText(LanguageManager::instance().getText("BTN_SAVE_CHANGES"));
    if (nutHuy) nutHuy->setText(LanguageManager::instance().getText("BTN_CANCEL"));
    if (nutMacDinh) nutMacDinh->setText(LanguageManager::instance().getText("BTN_RESTORE_DEFAULT"));

}

void HopThoaiCaiDat::thietLapGiaoDien() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    tabs = new QTabWidget();
    tabs->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #3E3E42; }"
        "QTabBar::tab { background: #2D2D30; color: #BBB; padding: 10px 20px; border: 1px solid #3E3E42; }"
        "QTabBar::tab:selected { background: #0078D4; color: white; }"
    );

    // TAB 1: THÔNG TIN CHUNG
    QWidget *tabChung = new QWidget();
    QFormLayout *formChung = new QFormLayout(tabChung);
    formChung->setSpacing(15);
    
    oTenThuVien = new QLineEdit();
    oTenThuVien->setStyleSheet("padding: 8px; background: #2b2b36; border: 1px solid #3E3E42; color: white;");
    oDiaChi = new QLineEdit();
    oDiaChi->setStyleSheet("padding: 8px; background: #2b2b36; border: 1px solid #3E3E42; color: white;");
    
    lblTenThuVien = new QLabel(LanguageManager::instance().getText("LBL_LIBRARY_NAME") + ":");
    lblDiaChi = new QLabel(LanguageManager::instance().getText("LBL_ADDRESS") + ":");
    
    formChung->addRow(lblTenThuVien, oTenThuVien);
    formChung->addRow(lblDiaChi, oDiaChi);
    
    tabs->addTab(tabChung, LanguageManager::instance().getText("TAB_GENERAL_INFO"));

    // TAB 2: MƯỢN TRẢ
    QWidget *tabMuonTra = new QWidget();
    QFormLayout *formMuonTra = new QFormLayout(tabMuonTra);
    formMuonTra->setSpacing(15);
    
    oTienPhatMoiNgay = new QSpinBox();
    oTienPhatMoiNgay->setRange(0, 1000000);
    oTienPhatMoiNgay->setSuffix("");
    oTienPhatMoiNgay->setStyleSheet("padding: 8px; background: #2b2b36; border: 1px solid #3E3E42; color: white;");
    
    oSoNgayMuonToiDa = new QSpinBox();
    oSoNgayMuonToiDa->setRange(1, 365);
    oSoNgayMuonToiDa->setSuffix("");
    oSoNgayMuonToiDa->setStyleSheet("padding: 8px; background: #2b2b36; border: 1px solid #3E3E42; color: white;");
    
    oSoSachMuonToiDa = new QSpinBox();
    oSoSachMuonToiDa->setRange(1, 20);
    oSoSachMuonToiDa->setSuffix("");
    oSoSachMuonToiDa->setStyleSheet("padding: 8px; background: #2b2b36; border: 1px solid #3E3E42; color: white;");

    oSoLanGiaHanToiDa = new QSpinBox();
    oSoLanGiaHanToiDa->setRange(0, 5);
    oSoLanGiaHanToiDa->setSuffix("");
    oSoLanGiaHanToiDa->setStyleSheet("padding: 8px; background: #2b2b36; border: 1px solid #3E3E42; color: white;");
    
    lblTienPhat = new QLabel(LanguageManager::instance().getText("LBL_FINE_PER_DAY") + ":");
    lblNgayMuon = new QLabel(LanguageManager::instance().getText("LBL_MAX_BORROW_DAYS") + ":");
    lblLanGiaHan = new QLabel(LanguageManager::instance().getText("LBL_MAX_RENEWALS") + ":");
    lblSachMuon = new QLabel(LanguageManager::instance().getText("LBL_MAX_BORROW_BOOKS") + ":");
    
    formMuonTra->addRow(lblTienPhat, oTienPhatMoiNgay);
    formMuonTra->addRow(lblNgayMuon, oSoNgayMuonToiDa);
    formMuonTra->addRow(lblLanGiaHan, oSoLanGiaHanToiDa);
    formMuonTra->addRow(lblSachMuon, oSoSachMuonToiDa);

    // New Fields Phase 8
    oThoiHanThe = new QSpinBox();
    oThoiHanThe->setRange(1, 120); // 1 month to 10 years
    oThoiHanThe->setSuffix(" tháng");
    oThoiHanThe->setStyleSheet("padding: 8px; background: #2b2b36; border: 1px solid #3E3E42; color: white;");

    oPhiMatSach = new QSpinBox();
    oPhiMatSach->setRange(0, 10000000); // Max 10tr
    oPhiMatSach->setSingleStep(1000);
    oPhiMatSach->setSuffix(" đ");
    oPhiMatSach->setStyleSheet("padding: 8px; background: #2b2b36; border: 1px solid #3E3E42; color: white;");
    
    lblThoiHanThe = new QLabel(LanguageManager::instance().getText("LBL_CARD_VALIDITY_MONTHS"));
    lblPhiMatSach = new QLabel(LanguageManager::instance().getText("LBL_LOST_BOOK_FEE"));

    formMuonTra->addRow(lblThoiHanThe, oThoiHanThe);
    formMuonTra->addRow(lblPhiMatSach, oPhiMatSach);
    
    tabs->addTab(tabMuonTra, LanguageManager::instance().getText("TAB_BORROW_RULES"));

    
    mainLayout->addWidget(tabs);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    nutMacDinh = new QPushButton(LanguageManager::instance().getText("BTN_RESTORE_DEFAULT"));
    nutMacDinh->setStyleSheet("background: transparent; color: #888; text-decoration: underline; border: none;");
    
    nutLuu = new QPushButton(LanguageManager::instance().getText("BTN_SAVE_CHANGES"));
    nutLuu->setStyleSheet("background: #0078D4; color: white; padding: 10px 20px; border-radius: 4px; font-weight: bold; border: none;");
    
    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CANCEL"));
    nutHuy->setStyleSheet("background: #2D2D30; color: white; padding: 10px 20px; border-radius: 4px; border: 1px solid #3E3E42;");
    
    btnLayout->addWidget(nutMacDinh);
    btnLayout->addStretch();
    btnLayout->addWidget(nutHuy);
    btnLayout->addWidget(nutLuu);
    
    mainLayout->addLayout(btnLayout);
    
    connect(nutLuu, &QPushButton::clicked, this, &HopThoaiCaiDat::luuCauHinh);
    connect(nutHuy, &QPushButton::clicked, this, &QDialog::reject);
    connect(nutMacDinh, &QPushButton::clicked, this, &HopThoaiCaiDat::macDinh);
}

// Include SettingsManager
#include "utils/SettingsManager.h"
#include "utils/AuditLogger.h"
#include "utils/PrintHelper.h"

// ... (in loadData)
void HopThoaiCaiDat::loadData() {
    auto& store = SettingsManager::instance();
    oTenThuVien->setText(store.getString("Print/TenThuVien", "Thư Viện Chùa Chantarangsay"));
    oDiaChi->setText(store.getString("general/library_address", "P.Xuân Hòa, TP.HCM"));
    oTienPhatMoiNgay->setValue(store.getInt("TIEN_PHAT_MOI_NGAY", 5000));
    oSoNgayMuonToiDa->setValue(store.getInt("SO_NGAY_MUON_TOI_DA", 7));
    oSoLanGiaHanToiDa->setValue(store.getInt("SO_LAN_GIA_HAN_TOI_DA", 1));
    if (oSoSachMuonToiDa) oSoSachMuonToiDa->setValue(store.getInt("SO_SACH_MUON_TOI_DA", 3));
    
    if (oThoiHanThe) oThoiHanThe->setValue(store.getInt("THOI_HAN_THE_THANG", 6));
    if (oPhiMatSach) oPhiMatSach->setValue(store.getInt("PHI_MAT_SACH", 20000));

}

void HopThoaiCaiDat::luuCauHinh() {
    auto& store = SettingsManager::instance();
    store.set("Print/TenThuVien", oTenThuVien->text());
    store.set("general/library_address", oDiaChi->text());
    store.set("TIEN_PHAT_MOI_NGAY", oTienPhatMoiNgay->value(), "int");
    store.set("SO_NGAY_MUON_TOI_DA", oSoNgayMuonToiDa->value(), "int");
    store.set("SO_LAN_GIA_HAN_TOI_DA", oSoLanGiaHanToiDa->value(), "int");
    if (oSoSachMuonToiDa) store.set("SO_SACH_MUON_TOI_DA", oSoSachMuonToiDa->value(), "int");
    if (oThoiHanThe) store.set("THOI_HAN_THE_THANG", oThoiHanThe->value(), "int");
    if (oPhiMatSach) store.set("PHI_MAT_SACH", oPhiMatSach->value(), "int");
    

    // Log
    AuditLogger::log(AuditLogger::ACTION_SETTINGS, "CAI_DAT", "", "Cap nhat cau hinh he thong (HopThoaiCaiDat)");
    
    QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_SAVE_SETTINGS_SUCCESS"));
    accept();
}

void HopThoaiCaiDat::macDinh() {
    oTenThuVien->setText("PHÁT TRIỂN: CHĂM RỐCH THI");
    oDiaChi->setText("AN GIANG");
    oTienPhatMoiNgay->setValue(5000);
    oSoNgayMuonToiDa->setValue(7);
    oSoLanGiaHanToiDa->setValue(1);
    if (oSoSachMuonToiDa) oSoSachMuonToiDa->setValue(3);
    if (oThoiHanThe) oThoiHanThe->setValue(6);
    if (oPhiMatSach) oPhiMatSach->setValue(20000);
    
}
