#include "giao_dien/hop_thoai/HopThoaiCaiDat.h"
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
    , lblCheDoIn(nullptr)
    , lblMayInReceipt(nullptr)
    , lblMayInReport(nullptr)
    , oInYenLang(nullptr)
    , oMayInReceipt(nullptr)
    , oMayInReport(nullptr)
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
        tabs->setTabText(2, LanguageManager::instance().getText("TAB_PRINT_CONFIG"));
    }
    
    if (lblTenThuVien) lblTenThuVien->setText(LanguageManager::instance().getText("LBL_LIBRARY_NAME") + ":");
    if (lblDiaChi) lblDiaChi->setText(LanguageManager::instance().getText("LBL_ADDRESS") + ":");
    
    if (lblTienPhat) lblTienPhat->setText(LanguageManager::instance().getText("LBL_FINE_PER_DAY") + ":");
    if (lblNgayMuon) lblNgayMuon->setText(LanguageManager::instance().getText("LBL_MAX_BORROW_DAYS") + ":");
    if (lblLanGiaHan) lblLanGiaHan->setText(LanguageManager::instance().getText("LBL_MAX_RENEWALS") + ":");
    if (lblSachMuon) lblSachMuon->setText(LanguageManager::instance().getText("LBL_MAX_BORROW_BOOKS") + ":");
    
    if (lblCheDoIn) lblCheDoIn->setText(LanguageManager::instance().getText("LBL_PRINT_MODE") + ":");
    if (oInYenLang) oInYenLang->setText(LanguageManager::instance().getText("CB_SILENT_PRINT"));
    if (lblMayInReceipt) lblMayInReceipt->setText(LanguageManager::instance().getText("LBL_PRINTER_RECEIPT") + ":");
    if (lblMayInReport) lblMayInReport->setText(LanguageManager::instance().getText("LBL_PRINTER_REPORT") + ":");
    
    if (nutLuu) nutLuu->setText(LanguageManager::instance().getText("BTN_SAVE_CHANGES"));
    if (nutHuy) nutHuy->setText(LanguageManager::instance().getText("BTN_CANCEL"));
    if (nutMacDinh) nutMacDinh->setText(LanguageManager::instance().getText("BTN_RESTORE_DEFAULT"));

    if (oMayInReceipt && oMayInReceipt->count() > 0) oMayInReceipt->setItemText(0, LanguageManager::instance().getText("OPTION_SYSTEM_DEFAULT"));
    if (oMayInReport && oMayInReport->count() > 0) oMayInReport->setItemText(0, LanguageManager::instance().getText("OPTION_SYSTEM_DEFAULT"));
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
    
    tabs->addTab(tabMuonTra, LanguageManager::instance().getText("TAB_BORROW_RULES"));

    // TAB 3: IN ẤN (New Integration)
    QWidget *tabInAn = new QWidget();
    QFormLayout *formInAn = new QFormLayout(tabInAn);
    formInAn->setSpacing(15);

    oInYenLang = new QCheckBox(LanguageManager::instance().getText("CB_SILENT_PRINT"));
    oInYenLang->setStyleSheet("QCheckBox { color: #e0e0e0; }");

    oMayInReceipt = new QComboBox();
    oMayInReceipt->setStyleSheet("padding: 8px; background: #2b2b36; border: 1px solid #3E3E42; color: white;");
    
    oMayInReport = new QComboBox();
    oMayInReport->setStyleSheet("padding: 8px; background: #2b2b36; border: 1px solid #3E3E42; color: white;");

    // Populate Printers
    QString defaultPrinterText = LanguageManager::instance().getText("OPTION_SYSTEM_DEFAULT");
    QStringList printers = QPrinterInfo::availablePrinterNames();
    oMayInReceipt->addItem(defaultPrinterText);
    oMayInReport->addItem(defaultPrinterText);
    oMayInReceipt->addItems(printers);
    oMayInReport->addItems(printers);

    lblCheDoIn = new QLabel(LanguageManager::instance().getText("LBL_PRINT_MODE") + ":");
    lblMayInReceipt = new QLabel(LanguageManager::instance().getText("LBL_PRINTER_RECEIPT") + ":");
    lblMayInReport = new QLabel(LanguageManager::instance().getText("LBL_PRINTER_REPORT") + ":");

    formInAn->addRow(lblCheDoIn, oInYenLang);
    formInAn->addRow(lblMayInReceipt, oMayInReceipt);
    formInAn->addRow(lblMayInReport, oMayInReport);

    tabs->addTab(tabInAn, LanguageManager::instance().getText("TAB_PRINT_CONFIG"));
    
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

    // Print Config
    oInYenLang->setChecked(store.getBool("Print/Silent", false));
    
    QString receiptPrinter = store.getString("Print/Receipt", "");
    QString reportPrinter = store.getString("Print/Report", "");
    
    if (!receiptPrinter.isEmpty()) oMayInReceipt->setCurrentText(receiptPrinter);
    if (!reportPrinter.isEmpty()) oMayInReport->setCurrentText(reportPrinter);
}

void HopThoaiCaiDat::luuCauHinh() {
    auto& store = SettingsManager::instance();
    store.set("Print/TenThuVien", oTenThuVien->text());
    store.set("general/library_address", oDiaChi->text());
    store.set("TIEN_PHAT_MOI_NGAY", oTienPhatMoiNgay->value(), "int");
    store.set("SO_NGAY_MUON_TOI_DA", oSoNgayMuonToiDa->value(), "int");
    store.set("SO_LAN_GIA_HAN_TOI_DA", oSoLanGiaHanToiDa->value(), "int");
    if (oSoSachMuonToiDa) store.set("SO_SACH_MUON_TOI_DA", oSoSachMuonToiDa->value(), "int");
    
    // Save Print Config
    store.set("Print/Silent", oInYenLang->isChecked(), "bool");
    store.set("Print/Receipt", oMayInReceipt->currentText());
    store.set("Print/Report", oMayInReport->currentText());

    // Apply to PrintHelper immediately
    PrintHelper::PrintConfig cfg = PrintHelper::getCauHinh();
    cfg.tenThuVien = oTenThuVien->text();
    QString defaultPrinterText = LanguageManager::instance().getText("OPTION_SYSTEM_DEFAULT");
    cfg.printerReceipt = oMayInReceipt->currentText() == defaultPrinterText ? "" : oMayInReceipt->currentText();
    cfg.printerReport = oMayInReport->currentText() == defaultPrinterText ? "" : oMayInReport->currentText();
    cfg.silentPrint = oInYenLang->isChecked();
    PrintHelper::setCauHinh(cfg);

    // Log
    AuditLogger::log("SETTINGS_UPDATE", "CAI_DAT", "", "Cap nhat cau hinh he thong");
    
    QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_SAVE_SETTINGS_SUCCESS"));
    accept();
}

void HopThoaiCaiDat::macDinh() {
    oTenThuVien->setText("Thư Viện Chùa Chantarangsay");
    oDiaChi->setText("P.Xuân Hòa, TP.HCM");
    oTienPhatMoiNgay->setValue(5000);
    oSoNgayMuonToiDa->setValue(7);
    oSoLanGiaHanToiDa->setValue(1);
    if (oSoSachMuonToiDa) oSoSachMuonToiDa->setValue(3);
    
    oInYenLang->setChecked(false);
    oMayInReceipt->setCurrentIndex(0);
    oMayInReport->setCurrentIndex(0);
}
