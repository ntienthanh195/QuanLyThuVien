#include "utils/TruongNhapLieu.h"
#include <QEvent>
#include <QFocusEvent>
#include <QTimer>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPropertyAnimation>

// ==================== TruongNhapLieu Implementation ====================

const QString TruongNhapLieu::MAU_PRIMARY = "#3B82F6";
const QString TruongNhapLieu::MAU_SUCCESS = "#10B981";
const QString TruongNhapLieu::MAU_ERROR = "#EF4444";
const QString TruongNhapLieu::MAU_BORDER_BINH_THUONG = "#475569";
const QString TruongNhapLieu::MAU_BORDER_FOCUS = "#60A5FA";
const QString TruongNhapLieu::MAU_TEXT = "#F1F5F9";
const QString TruongNhapLieu::MAU_PLACEHOLDER = "#94A3B8";
const QString TruongNhapLieu::MAU_HELPER = "#64748B";

TruongNhapLieu::TruongNhapLieu(QWidget *parent)
    : QWidget(parent)
    , trangThaiHienTai(TRANG_THAI_BINH_THUONG)
    , laBatBuoc(false)
    , daValidate(false)
{
    khoiTaoGiaoDien();
    khoiTaoConnections();
}

TruongNhapLieu::TruongNhapLieu(const QString &nhanPlaceholder, QWidget *parent)
    : TruongNhapLieu(parent)
{
    datNhan(nhanPlaceholder);
    datPlaceholder(nhanPlaceholder);
}

TruongNhapLieu::~TruongNhapLieu() {
}

void TruongNhapLieu::khoiTaoGiaoDien() {
    layoutChinh = new QVBoxLayout(this);
    layoutChinh->setSpacing(6);
    layoutChinh->setContentsMargins(0, 0, 0, 0);
    
    // Container cho input và icon
    QWidget *inputContainer = new QWidget(this);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputContainer);
    inputLayout->setSpacing(0);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    
    // Label nhãn (floating label)
    labelNhan = new QLabel(this);
    labelNhan->setStyleSheet(layStyleNhan());
    labelNhan->hide(); // Ẩn ban đầu, hiện khi focus
    
    // Input field
    inputField = new QLineEdit(inputContainer);
    inputField->setMinimumHeight(44);  // Touch-friendly
    inputField->setStyleSheet(layStyleInput());
    inputField->installEventFilter(this);
    
    // Icon trạng thái (✓ hoặc ✗)
    iconTrangThai = new QLabel(inputContainer);
    iconTrangThai->setFixedSize(24, 24);
    iconTrangThai->setAlignment(Qt::AlignCenter);
    iconTrangThai->hide();
    
    inputLayout->addWidget(inputField);
    inputLayout->addWidget(iconTrangThai);
    inputLayout->addSpacing(12);
    
    // Label thông báo (helper text hoặc error)
    labelThongBao = new QLabel(this);
    labelThongBao->setWordWrap(true);
    labelThongBao->setStyleSheet(QString("color: %1; font-size: 13px; margin-top: 4px;").arg(MAU_HELPER));
    labelThongBao->hide();
    
    layoutChinh->addWidget(labelNhan);
    layoutChinh->addWidget(inputContainer);
    layoutChinh->addWidget(labelThongBao);
    
    setLayout(layoutChinh);
}

void TruongNhapLieu::khoiTaoConnections() {
    connect(inputField, &QLineEdit::textChanged, this, [this](const QString &text) {
        capNhatViTriNhan();
        emit textThayDoi(text);
        
        // Auto-validate nếu đã từng validate
        if (daValidate && inputField->validator()) {
            int pos = 0;
            QString textCopy = text;
            QValidator::State state = inputField->validator()->validate(textCopy, pos);
            
            if (state == QValidator::Acceptable) {
                hienThiThanhCong();
                emit giaTriHopLe(true);
            } else if (!text.isEmpty()) {
                emit giaTriHopLe(false);
            } else {
                xoaLoi();
            }
        }
    });
    
    connect(inputField, &QLineEdit::returnPressed, this, &TruongNhapLieu::enterDuocNhan);
}

bool TruongNhapLieu::eventFilter(QObject *obj, QEvent *event) {
    if (obj == inputField) {
        if (event->type() == QEvent::FocusIn) {
            datTrangThai(TRANG_THAI_FOCUS);
            labelNhan->show();
            capNhatViTriNhan();
        } else if (event->type() == QEvent::FocusOut) {
            if (inputField->text().isEmpty()) {
                labelNhan->hide();
                datTrangThai(TRANG_THAI_BINH_THUONG);
            } else {
                datTrangThai(trangThaiHienTai == TRANG_THAI_LOI ? TRANG_THAI_LOI : TRANG_THAI_BINH_THUONG);
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void TruongNhapLieu::capNhatViTriNhan() {
    if (!inputField->text().isEmpty() || inputField->hasFocus()) {
        labelNhan->show();
    } else {
        labelNhan->hide();
    }
}

void TruongNhapLieu::capNhatStyle() {
    inputField->setStyleSheet(layStyleInput());
    labelNhan->setStyleSheet(layStyleNhan());
}

QString TruongNhapLieu::layStyleInput() {
    QString borderColor;
    QString focusColor;
    
    switch (trangThaiHienTai) {
        case TRANG_THAI_FOCUS:
            borderColor = MAU_BORDER_FOCUS;
            focusColor = MAU_PRIMARY;
            break;
        case TRANG_THAI_THANH_CONG:
            borderColor = MAU_SUCCESS;
            focusColor = MAU_SUCCESS;
            break;
        case TRANG_THAI_LOI:
            borderColor = MAU_ERROR;
            focusColor = MAU_ERROR;
            break;
        case TRANG_THAI_DISABLED:
            borderColor = "#334155";
            focusColor = "#334155";
            break;
        default:
            borderColor = MAU_BORDER_BINH_THUONG;
            focusColor = MAU_PRIMARY;
    }
    
    return QString(
        "QLineEdit {"
        "    background: rgba(30, 41, 59, 0.5);"
        "    color: %1;"
        "    border: 2px solid %2;"
        "    border-radius: 10px;"
        "    padding: 12px 16px;"
        "    font-size: 15px;"
        "    font-weight: 500;"
        "}"
        "QLineEdit:focus {"
        "    border-color: %3;"
        "    background: rgba(30, 41, 59, 0.7);"
        "    outline: none;"
        "}"
        "QLineEdit::placeholder {"
        "    color: %4;"
        "}"
        "QLineEdit:disabled {"
        "    background: rgba(51, 65, 85, 0.3);"
        "    color: #64748B;"
        "    border-color: #334155;"
        "}"
    )
    .arg(MAU_TEXT)
    .arg(borderColor)
    .arg(focusColor)
    .arg(MAU_PLACEHOLDER);
}

QString TruongNhapLieu::layStyleNhan() {
    QString color = MAU_TEXT;
    if (trangThaiHienTai == TRANG_THAI_LOI) {
        color = MAU_ERROR;
    } else if (trangThaiHienTai == TRANG_THAI_THANH_CONG) {
        color = MAU_SUCCESS;
    } else if (trangThaiHienTai == TRANG_THAI_FOCUS) {
        color = MAU_PRIMARY;
    }
    
    QString batBuocMark = laBatBuoc ? " <span style='color: #EF4444;'>*</span>" : "";
    
    return QString(
        "QLabel {"
        "    color: %1;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    background: transparent;"
        "    margin-bottom: 6px;"
        "}"
    ).arg(color);
}

// ==================== Public Methods ====================

QString TruongNhapLieu::layGiaTri() const {
    return inputField->text();
}

QLineEdit* TruongNhapLieu::layLineEdit() {
    return inputField;
}

bool TruongNhapLieu::laTrong() const {
    return inputField->text().trimmed().isEmpty();
}

bool TruongNhapLieu::hopLe() const {
    if (laBatBuoc && laTrong()) {
        return false;
    }
    
    if (inputField->validator()) {
        int pos = 0;
        QString text = inputField->text();
        return inputField->validator()->validate(text, pos) == QValidator::Acceptable;
    }
    
    return true;
}

void TruongNhapLieu::datGiaTri(const QString &giaTri) {
    inputField->setText(giaTri);
    capNhatViTriNhan();
}

void TruongNhapLieu::datNhan(const QString &nhan) {
    nhanGoc = nhan;
    QString batBuocMark = laBatBuoc ? " <span style='color: #EF4444;'>*</span>" : "";
    labelNhan->setText(nhan + batBuocMark);
}

void TruongNhapLieu::datPlaceholder(const QString &placeholder) {
    inputField->setPlaceholderText(placeholder);
}

void TruongNhapLieu::datTextGiupDo(const QString &text) {
    textGiupDo = text;
    if (trangThaiHienTai != TRANG_THAI_LOI) {
        labelThongBao->setText(text);
        labelThongBao->setStyleSheet(QString("color: %1; font-size: 13px;").arg(MAU_HELPER));
        labelThongBao->setVisible(!text.isEmpty());
    }
}

void TruongNhapLieu::datTextLoi(const QString &text) {
    labelThongBao->setText(text);
    labelThongBao->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 600;").arg(MAU_ERROR));
    labelThongBao->show();
}

void TruongNhapLieu::datValidator(QValidator *validator) {
    inputField->setValidator(validator);
}

void TruongNhapLieu::datReadOnly(bool readOnly) {
    inputField->setReadOnly(readOnly);
    if (readOnly) {
        datTrangThai(TRANG_THAI_DISABLED);
    }
}

void TruongNhapLieu::datPasswordMode(bool isPassword) {
    inputField->setEchoMode(isPassword ? QLineEdit::Password : QLineEdit::Normal);
}

void TruongNhapLieu::datMaxLength(int length) {
    inputField->setMaxLength(length);
}

void TruongNhapLieu::datTrangThai(TrangThaiTruong trangThai) {
    trangThaiHienTai = trangThai;
    capNhatStyle();
}

void TruongNhapLieu::hienThiThanhCong() {
    daValidate = true;
    datTrangThai(TRANG_THAI_THANH_CONG);
    iconTrangThai->setText("✓");
    iconTrangThai->setStyleSheet(QString("color: %1; font-size: 18px; font-weight: bold;").arg(MAU_SUCCESS));
    iconTrangThai->show();
    
    if (!textGiupDo.isEmpty()) {
        labelThongBao->setText(textGiupDo);
        labelThongBao->setStyleSheet(QString("color: %1; font-size: 13px;").arg(MAU_HELPER));
        labelThongBao->show();
    } else {
        labelThongBao->hide();
    }
}

void TruongNhapLieu::hienThiLoi(const QString &thongDiepLoi) {
    daValidate = true;
    datTrangThai(TRANG_THAI_LOI);
    iconTrangThai->setText("✗");
    iconTrangThai->setStyleSheet(QString("color: %1; font-size: 18px; font-weight: bold;").arg(MAU_ERROR));
    iconTrangThai->show();
    datTextLoi(thongDiepLoi);
}

void TruongNhapLieu::xoaLoi() {
    datTrangThai(TRANG_THAI_BINH_THUONG);
    iconTrangThai->hide();
    
    if (!textGiupDo.isEmpty()) {
        labelThongBao->setText(textGiupDo);
        labelThongBao->setStyleSheet(QString("color: %1; font-size: 13px;").arg(MAU_HELPER));
        labelThongBao->show();
    } else {
        labelThongBao->hide();
    }
}

void TruongNhapLieu::datBatBuoc(bool batBuoc) {
    laBatBuoc = batBuoc;
    datNhan(nhanGoc);  // Cập nhật label với dấu *
}

void TruongNhapLieu::focus() {
    inputField->setFocus();
}

void TruongNhapLieu::clearFocus() {
    inputField->clearFocus();
}

// ==================== TruongTimKiem Implementation ====================

TruongTimKiem::TruongTimKiem(QWidget *parent)
    : QWidget(parent)
{
    khoiTaoGiaoDien();
    khoiTaoConnections();
}

TruongTimKiem::~TruongTimKiem() {
}

void TruongTimKiem::khoiTaoGiaoDien() {
    layoutChinh = new QHBoxLayout(this);
    layoutChinh->setSpacing(0);
    layoutChinh->setContentsMargins(0, 0, 0, 0);
    
    // Container với border
    QWidget *container = new QWidget(this);
    QHBoxLayout *containerLayout = new QHBoxLayout(container);
    containerLayout->setSpacing(8);
    containerLayout->setContentsMargins(16, 0, 12, 0);
    
    container->setStyleSheet(
        "QWidget {"
        "    background: rgba(30, 41, 59, 0.5);"
        "    border: 2px solid #475569;"
        "    border-radius: 10px;"
        "}"
        "QWidget:focus-within {"
        "    border-color: #60A5FA;"
        "}"
    );
    container->setMinimumHeight(44);
    
    // Icon search
    iconTimKiem = new QLabel("🔍", container);
    iconTimKiem->setStyleSheet("color: #94A3B8; font-size: 18px; background: transparent; border: none;");
    
    // Input field
    inputField = new QLineEdit(container);
    inputField->setStyleSheet(
        "QLineEdit {"
        "    background: transparent;"
        "    color: #F1F5F9;"
        "    border: none;"
        "    font-size: 15px;"
        "    font-weight: 500;"
        "}"
        "QLineEdit::placeholder {"
        "    color: #94A3B8;"
        "}"
    );
    inputField->setPlaceholderText("Tìm kiếm...");
    
    // Nút xóa (X)
    nutXoa = new QPushButton("✕", container);
    nutXoa->setFixedSize(24, 24);
    nutXoa->setCursor(Qt::PointingHandCursor);
    nutXoa->setStyleSheet(
        "QPushButton {"
        "    background: transparent;"
        "    color: #94A3B8;"
        "    border: none;"
        "    border-radius: 12px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background: rgba(148, 163, 184, 0.2);"
        "    color: #F1F5F9;"
        "}"
    );
    nutXoa->hide();
    
    containerLayout->addWidget(iconTimKiem);
    containerLayout->addWidget(inputField);
    containerLayout->addWidget(nutXoa);
    
    layoutChinh->addWidget(container);
    setLayout(layoutChinh);
    
    // Timer debounce
    timerDebounce = new QTimer(this);
    timerDebounce->setSingleShot(true);
    timerDebounce->setInterval(300);
}

void TruongTimKiem::khoiTaoConnections() {
    connect(inputField, &QLineEdit::textChanged, this, [this](const QString &text) {
        capNhatNutXoa();
        emit textThayDoi(text);
        timerDebounce->start();
    });
    
    connect(timerDebounce, &QTimer::timeout, this, [this]() {
        emit timKiem(inputField->text());
    });
    
    connect(nutXoa, &QPushButton::clicked, this, &TruongTimKiem::xoa);
}

void TruongTimKiem::capNhatNutXoa() {
    nutXoa->setVisible(!inputField->text().isEmpty());
}

QString TruongTimKiem::layGiaTri() const {
    return inputField->text();
}

void TruongTimKiem::datGiaTri(const QString &giaTri) {
    inputField->setText(giaTri);
    capNhatNutXoa();
}

void TruongTimKiem::datPlaceholder(const QString &placeholder) {
    inputField->setPlaceholderText(placeholder);
}

void TruongTimKiem::xoa() {
    inputField->clear();
    inputField->setFocus();
    emit daXoa();
}

void TruongTimKiem::focus() {
    inputField->setFocus();
}
