
#include "giao_dien/hop_thoai/HopThoaiThanhToan.h"
#include "utils/LanguageManager.h"
#include "utils/PrintHelper.h"
#include "utils/ToastNotification.h"
#include "utils/ResponsiveHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>

HopThoaiThanhToan::HopThoaiThanhToan(PTRDG docGia, const QString& lyDo, long long soTien, QWidget *parent)
    : QDialog(parent), m_docGia(docGia), m_lyDo(lyDo), m_soTien(soTien) {
    setWindowTitle(LanguageManager::instance().getText("DIALOG_PAYMENT_TITLE"));
    setMinimumWidth(400);
    
    // Deep Night Theme
    setStyleSheet("QDialog { background-color: #1e1e2d; color: #ffffff; }");
    
    thietLapGiaoDien();
}

bool HopThoaiThanhToan::hienThiThanhToan(PTRDG docGia, const QString& lyDo, long long soTien, QWidget *parent) {
    HopThoaiThanhToan dialog(docGia, lyDo, soTien, parent);
    return dialog.exec() == QDialog::Accepted;
}

void HopThoaiThanhToan::thietLapGiaoDien() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *title = new QLabel(LanguageManager::instance().getText("HEADER_PAYMENT"));
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #FFFFFF; margin-bottom: 10px;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);
    
    QGroupBox *infoGroup = new QGroupBox(LanguageManager::instance().getText("GROUP_PAYMENT_INFO"));
    infoGroup->setStyleSheet(
        "QGroupBox { background-color: #252526; border: 1px solid #3E3E42; border-radius: 6px; padding-top: 15px; font-weight: bold; color: #E0E0E0; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: #5BA3F5; background-color: #1e1e2d; }"
    );
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    
    auto addRow = [&](QString label, QString value, bool isHighlight = false) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("color: #CCCCCC;");
        QLabel *val = new QLabel(value);
        val->setStyleSheet(isHighlight ? "color: #FF5252; font-weight: bold; font-size: 16px;" : "color: #FFFFFF; font-weight: 500;");
        val->setAlignment(Qt::AlignRight);
        row->addWidget(lbl);
        row->addStretch();
        row->addWidget(val);
        infoLayout->addLayout(row);
    };
    
    addRow(LanguageManager::instance().getText("LBL_READER_NAME") + ":", QString::fromStdString(m_docGia->data.Ho + " " + m_docGia->data.Ten));
    addRow(LanguageManager::instance().getText("LBL_REASON") + ":", m_lyDo);
    addRow(LanguageManager::instance().getText("LBL_AMOUNT") + ":", QString("%L1 %2").arg(m_soTien).arg(LanguageManager::instance().getText("CURRENCY_UNIT")), true);
    
    mainLayout->addWidget(infoGroup);
    
    hopInBienLai = new QCheckBox(LanguageManager::instance().getText("CB_PRINT_RECEIPT"));
    hopInBienLai->setChecked(true);
    hopInBienLai->setStyleSheet("color: #E0E0E0; font-size: 14px; margin-top: 10px;");
    mainLayout->addWidget(hopInBienLai);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    
    nutThanhToan = new QPushButton(LanguageManager::instance().getText("BTN_CONFIRM_PAYMENT"));
    nutThanhToan->setMinimumHeight(40);
    nutThanhToan->setCursor(Qt::PointingHandCursor);
    nutThanhToan->setStyleSheet(
        "QPushButton { background-color: #0078D4; color: white; border: none; border-radius: 4px; font-weight: 600; padding: 0 20px; }"
        "QPushButton:hover { background-color: #106EBE; }"
    );
    
    nutHuy = new QPushButton(LanguageManager::instance().getText("BTN_CANCEL"));
    nutHuy->setMinimumHeight(40);
    nutHuy->setCursor(Qt::PointingHandCursor);
    nutHuy->setStyleSheet(
        "QPushButton { background-color: #2D2D30; color: white; border: 1px solid #3E3E42; border-radius: 4px; font-weight: 600; padding: 0 20px; }"
        "QPushButton:hover { background-color: #3E3E42; border: 1px solid #555; }"
    );
    
    btnLayout->addWidget(nutHuy);
    btnLayout->addWidget(nutThanhToan);
    mainLayout->addLayout(btnLayout);
    
    connect(nutThanhToan, &QPushButton::clicked, this, &HopThoaiThanhToan::xuLyThanhToan);
    connect(nutHuy, &QPushButton::clicked, this, &HopThoaiThanhToan::xuLyHuy);
}

void HopThoaiThanhToan::xuLyThanhToan() {
    // Logic thực tế cập nhật số tiền sẽ ở bên ngoài (caller) hoặc gọi API backend nếu có
    // Ở đây ta giả lập thanh toán thành công
    
    if (hopInBienLai->isChecked()) {
        PrintHelper::inBienLaiPhat(m_docGia, m_lyDo, m_soTien, this);
    }
    
    ToastNotification::show(LanguageManager::instance().getText("MSG_PAYMENT_SUCCESS"), ToastNotification::SUCCESS);
    accept();
}

void HopThoaiThanhToan::xuLyHuy() {
    reject();
}
