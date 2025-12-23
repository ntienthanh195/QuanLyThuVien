#include "utils/CustomPrintDialog.h"
#include "utils/PrintHelper.h"
#include "utils/FilePathManager.h"
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QAbstractScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPrintPreviewWidget>
#include <QPrinter>
#include <QPrinterInfo>
#include <QTextDocument>
#include <QFileDialog>
#include <QApplication>
#include <QTimer> // Add QTimer

CustomPrintDialog::CustomPrintDialog(QTextDocument* document,
                                     const QString& title,
                                     const QString& suggestedFileName,
                                     const QString& defaultPrinter,
                                     QWidget* parent)
    : QDialog(parent)
    , m_document(document)
    , m_suggestedFileName(suggestedFileName)
    , m_defaultPrinter(defaultPrinter)
    , m_printer(nullptr)
{
    setWindowTitle(title);
    setModal(true);
    resize(1100, 700);
    
    setupUI();
    setupConnections(); // Connect signals first
    populatePrinters(); // Then trigger changes
    
    // Force initial preview update after a short delay to ensure layout is ready
    QTimer::singleShot(100, this, [this](){
        if (m_printerCombo->count() > 0) {
            onPrinterChanged(m_printerCombo->currentIndex());
        } else {
             m_previewWidget->updatePreview();
        }
    });
}

CustomPrintDialog::~CustomPrintDialog() {
    if (m_printer) {
        delete m_printer;
    }
}

void CustomPrintDialog::setupUI() {
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header
    QWidget* header = new QWidget();
    header->setStyleSheet("background-color: #2b2b40; border-bottom: 2px solid #5D9CEC;");
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 15, 20, 15);
    
    QLabel* titleLabel = new QLabel(windowTitle());
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #E0E0E0;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    
    mainLayout->addWidget(header);
    
    // Content area (Split view)
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    
    // Left panel - Controls
    QWidget* leftPanel = new QWidget();
    leftPanel->setMinimumWidth(280);
    leftPanel->setMaximumWidth(320);
    leftPanel->setStyleSheet("background-color: #1e1e2d; border-right: 1px solid #3E3E42;");
    
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(20, 20, 20, 20);
    leftLayout->setSpacing(15);
    
    // Printer selection
    QLabel* lblPrinter = new QLabel("Máy in:");
    lblPrinter->setStyleSheet("color: #999; font-size: 12px;");
    leftLayout->addWidget(lblPrinter);
    
    m_printerCombo = new QComboBox();
    m_printerCombo->setStyleSheet(
        "QComboBox { background: #252530; color: #E0E0E0; border: 1px solid #3E3E42; "
        "border-radius: 4px; padding: 8px; font-size: 13px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox:hover { border-color: #5D9CEC; }"
    );
    leftLayout->addWidget(m_printerCombo);
    
    // Options
    m_colorCheckbox = new QCheckBox("In màu");
    m_colorCheckbox->setStyleSheet("color: #E0E0E0; font-size: 13px;");
    m_colorCheckbox->setChecked(PrintHelper::getCauHinh().inMauSac);
    leftLayout->addWidget(m_colorCheckbox);
    
    leftLayout->addSpacing(20);
    
    // Action buttons
    m_btnPrint = new QPushButton("In Ngay");
    m_btnPrint->setCursor(Qt::PointingHandCursor);
    m_btnPrint->setMinimumHeight(45);
    m_btnPrint->setStyleSheet(
        "QPushButton { background: #5D9CEC; color: white; border: none; "
        "border-radius: 6px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background: #4A8CD9; }"
        "QPushButton:pressed { background: #3A7BC8; }"
    );
    leftLayout->addWidget(m_btnPrint);
    
    m_btnPdf = new QPushButton("Lưu PDF");
    m_btnPdf->setCursor(Qt::PointingHandCursor);
    m_btnPdf->setMinimumHeight(40);
    m_btnPdf->setStyleSheet(
        "QPushButton { background: #2b2b40; color: #E0E0E0; border: 1px solid #3E3E42; "
        "border-radius: 6px; font-size: 13px; }"
        "QPushButton:hover { background: #36364d; border-color: #5D9CEC; }"
    );
    leftLayout->addWidget(m_btnPdf);
    
    m_btnCancel = new QPushButton("Hủy");
    m_btnCancel->setCursor(Qt::PointingHandCursor);
    m_btnCancel->setMinimumHeight(40);
    m_btnCancel->setStyleSheet(
        "QPushButton { background: transparent; color: #999; border: 1px solid #3E3E42; "
        "border-radius: 6px; font-size: 13px; }"
        "QPushButton:hover { color: #E0E0E0; border-color: #555; }"
    );
    leftLayout->addWidget(m_btnCancel);
    
    leftLayout->addSpacing(30);
    
    // Zoom Controls (Phase 10)
    QLabel* lblZoom = new QLabel("Phóng to/Thu nhỏ:");
    lblZoom->setStyleSheet("color: #999; font-size: 11px;");
    leftLayout->addWidget(lblZoom);
    
    QHBoxLayout* zoomLayout = new QHBoxLayout();
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setRange(10, 300); // 10% to 300%
    m_zoomSlider->setValue(100);
    m_zoomSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: #252530; height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: #5D9CEC; border: none; width: 14px; height: 14px; margin: -5px 0; border-radius: 7px; }"
    );
    
    m_zoomLabel = new QLabel("100%");
    m_zoomLabel->setStyleSheet("color: #E0E0E0; font-size: 11px; min-width: 30px;");
    
    zoomLayout->addWidget(m_zoomSlider);
    zoomLayout->addWidget(m_zoomLabel);
    leftLayout->addLayout(zoomLayout);
    
    leftLayout->addStretch();
    
    // Right panel - Preview
    QWidget* rightPanel = new QWidget();
    rightPanel->setStyleSheet("background-color: #252530;");
    
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(15, 15, 15, 15);
    
    // Preview widget
    m_printer = new QPrinter(QPrinter::HighResolution);
    
    // Apply Settings to Printer for Preview Accuracy
    PrintHelper::PrintConfig cfg = PrintHelper::getCauHinh();
    
    // Page Size
    QPageSize::PageSizeId paperId = QPageSize::A4;
    QString ps = cfg.pageSize.toUpper();
    if (ps == "A3") paperId = QPageSize::A3;
    else if (ps == "A5") paperId = QPageSize::A5;
    else if (ps == "LETTER") paperId = QPageSize::Letter;
    else if (ps == "LEGAL") paperId = QPageSize::Legal;
    m_printer->setPageSize(QPageSize(paperId));
    
    // Orientation
    m_printer->setPageOrientation(cfg.orientation.toLower() == "landscape" ? QPageLayout::Landscape : QPageLayout::Portrait);
    
    m_previewWidget = new QPrintPreviewWidget(m_printer);
    m_previewWidget->setStyleSheet("background-color: #2b2b40; border: 1px solid #3E3E42; border-radius: 4px;");
    
    // Install Event Filter for Zoom/Pan
    m_previewWidget->installEventFilter(this);
    for (auto child : m_previewWidget->findChildren<QWidget*>()) {
        child->installEventFilter(this);
        // Important: Set tracking to get mouse move events while dragging
        child->setMouseTracking(true); 
    }

    rightLayout->addWidget(m_previewWidget);
    
    // Assemble panels
    contentLayout->addWidget(leftPanel);
    contentLayout->addWidget(rightPanel, 1);
    
    mainLayout->addLayout(contentLayout, 1);
    
    // Apply global dark theme
    setStyleSheet("QDialog { background-color: #1e1e2d; }");
}

void CustomPrintDialog::setupConnections() {
    connect(m_btnPrint, &QPushButton::clicked, this, &CustomPrintDialog::onPrintClicked);
    connect(m_btnPdf, &QPushButton::clicked, this, &CustomPrintDialog::onPdfClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_printerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CustomPrintDialog::onPrinterChanged);
    connect(m_previewWidget, &QPrintPreviewWidget::paintRequested,
            this, &CustomPrintDialog::updatePreview);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &CustomPrintDialog::onZoomChanged);
}

void CustomPrintDialog::populatePrinters() {
    QStringList printers = QPrinterInfo::availablePrinterNames();
    m_printerCombo->addItems(printers);
    
    // Select default printer
    if (!m_defaultPrinter.isEmpty()) {
        int idx = m_printerCombo->findText(m_defaultPrinter);
        if (idx >= 0) {
            m_printerCombo->setCurrentIndex(idx);
        }
    } else {
        // Use system default
        QString defaultPrinter = QPrinterInfo::defaultPrinter().printerName();
        int idx = m_printerCombo->findText(defaultPrinter);
        if (idx >= 0) {
            m_printerCombo->setCurrentIndex(idx);
        }
    }
}

void CustomPrintDialog::onPrinterChanged(int index) {
    if (index < 0) return;
    
    QString printerName = m_printerCombo->itemText(index);
    m_printer->setPrinterName(printerName);
    
    // Re-apply settings because setting printer name can sometimes reset printer state
    PrintHelper::PrintConfig cfg = PrintHelper::getCauHinh();
    QPageSize::PageSizeId paperId = QPageSize::A4;
    QString ps = cfg.pageSize.toUpper();
    if (ps == "A3") paperId = QPageSize::A3;
    else if (ps == "A5") paperId = QPageSize::A5;
    else if (ps == "LETTER") paperId = QPageSize::Letter;
    else if (ps == "LEGAL") paperId = QPageSize::Legal;
    m_printer->setPageSize(QPageSize(paperId));
    m_printer->setPageOrientation(cfg.orientation.toLower() == "landscape" ? QPageLayout::Landscape : QPageLayout::Portrait);

    m_previewWidget->updatePreview();
}

void CustomPrintDialog::updatePreview(QPrinter* printer) {
    if (!m_document || !printer) return;
    
    // Apply color mode
    if (m_colorCheckbox && m_colorCheckbox->isChecked()) {
        printer->setColorMode(QPrinter::Color);
    } else {
        printer->setColorMode(QPrinter::GrayScale);
    }
    
    m_document->print(printer);
}

void CustomPrintDialog::onPrintClicked() {
    if (m_printer && m_document) {
        // Force color mode update before final print
        m_printer->setColorMode( (m_colorCheckbox && m_colorCheckbox->isChecked()) ? QPrinter::Color : QPrinter::GrayScale );
        
        // Execute print directly to the configured printer
        m_document->print(m_printer);
    }
    done(PRINT_ACCEPTED);
}

void CustomPrintDialog::onPdfClicked() {
    PrintHelper::PrintConfig cfg = PrintHelper::getCauHinh();
    QString defaultDir;
    
    if (!cfg.pdfOutputDir.isEmpty()) {
        defaultDir = cfg.pdfOutputDir;
    } else {
        defaultDir = FilePathManager::getFilePath("pdf");
    }
    
    // Use suggested filename if provided, otherwise generate generic one
    QString fileName = m_suggestedFileName;
    if (fileName.isEmpty()) {
        fileName = QString("Document_%1.pdf")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    }
    
    m_exportPath = QFileDialog::getSaveFileName(
        this,
        "Lưu PDF",
        defaultDir + "/" + fileName,
        "PDF Files (*.pdf)"
    );
    
    if (!m_exportPath.isEmpty()) {
        done(PDF_EXPORT);
    }
}

void CustomPrintDialog::onZoomChanged(int value) {
    if (!m_previewWidget) return;
    double factor = value / 100.0;
    m_previewWidget->setZoomFactor(factor);
    m_zoomLabel->setText(QString("%1%").arg(value));
}

QString CustomPrintDialog::selectedPrinterName() const {
    return m_printerCombo->currentText();
}

bool CustomPrintDialog::eventFilter(QObject *obj, QEvent *event) {
    // Helper to find scroll area
    auto getScrollArea = [&]() -> QAbstractScrollArea* {
        return m_previewWidget->findChild<QAbstractScrollArea*>();
    };

    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        // Fix for Trackpad: 
        // - Standard Wheel (2 fingers) -> Scroll (Pan) [Pass through]
        // - Ctrl + Wheel (Pinch) -> Zoom [Consume]
        if (wheelEvent->modifiers() & Qt::ControlModifier) { 
            int delta = wheelEvent->angleDelta().y();
            if (delta != 0) {
                int currentVal = m_zoomSlider->value();
                // Slower zoom step for smoother trackpad pinch
                int step = (delta > 0) ? 5 : -5; 
                m_zoomSlider->setValue(currentVal + step);
                return true;
            }
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            m_isDragging = true;
            m_lastMousePos = mouseEvent->globalPosition().toPoint();
            m_previewWidget->setCursor(Qt::ClosedHandCursor);
            return true;
        }
    } else if (event->type() == QEvent::MouseMove) {
        if (m_isDragging) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint delta = mouseEvent->globalPosition().toPoint() - m_lastMousePos;
            m_lastMousePos = mouseEvent->globalPosition().toPoint();

            QAbstractScrollArea* scroll = getScrollArea();
            if (scroll) {
                scroll->horizontalScrollBar()->setValue(scroll->horizontalScrollBar()->value() - delta.x());
                scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->value() - delta.y());
            }
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (m_isDragging) {
            m_isDragging = false;
            m_previewWidget->setCursor(Qt::ArrowCursor);
            return true;
        }
    }
    
    return QDialog::eventFilter(obj, event);
}
