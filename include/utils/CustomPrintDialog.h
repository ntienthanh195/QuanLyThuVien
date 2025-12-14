#ifndef CUSTOMPRINTDIALOG_H
#define CUSTOMPRINTDIALOG_H

#include <QDialog>
#include <QString>

class QTextDocument;
class QPrintPreviewWidget;
class QComboBox;
class QCheckBox;
class QPushButton;
class QPrinter;

/**
 * @brief Custom Print Dialog with inline preview
 * Dark-themed dialog for print operations with live preview
 */
class CustomPrintDialog : public QDialog {
    Q_OBJECT

public:
    enum DialogResult {
        PRINT_ACCEPTED = QDialog::Accepted,
        PDF_EXPORT = QDialog::Accepted + 1,
        CANCELLED = QDialog::Rejected
    };

    explicit CustomPrintDialog(QTextDocument* document,
                               const QString& title,
                               const QString& suggestedFileName = QString(),
                               const QString& defaultPrinter = QString(),
                               QWidget* parent = nullptr);
    ~CustomPrintDialog();

    QString selectedPrinterName() const;
    QString exportPath() const { return m_exportPath; }

private slots:
    void onPrintClicked();
    void onPdfClicked();
    void onPrinterChanged(int index);
    void updatePreview(QPrinter* printer);

private:
    void setupUI();
    void setupConnections();
    void populatePrinters();

    // UI Components
    QTextDocument* m_document;
    QPrintPreviewWidget* m_previewWidget;
    QComboBox* m_printerCombo;
    QCheckBox* m_colorCheckbox;
    QPushButton* m_btnPrint;
    QPushButton* m_btnPdf;
    QPushButton* m_btnCancel;

    // Data
    QString m_suggestedFileName;
    QString m_defaultPrinter;
    QString m_exportPath;
    QPrinter* m_printer;
};

#endif // CUSTOMPRINTDIALOG_H
