/**
 * @file ManHinhPreviewImport.cpp
 * @path src/views/dialogs/ManHinhPreviewImport.cpp
 * @description Giao diện xem trước và xác nhận dữ liệu trước khi nhập (Import) từ file.
 * Giúp người dùng kiểm tra lại dữ liệu hợp lệ/lỗi trước khi chính thức ghi vào cơ sở dữ liệu.
 * @project Library Management System (QuanLyThuVien)
 */
#include "views/dialogs/ManHinhPreviewImport.h"

#include "utils/LanguageManager.h"
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QBrush>
#include <QColor>

// Internal Helper Model for CSV
class CsvTableModel : public QAbstractTableModel {
public:
    CsvTableModel(const QStringList &headers, const QList<QStringList> &data, int validColIdx, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_headers(headers), m_data(data), m_validColIdx(validColIdx) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_data.size();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_headers.size();
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_data.size()) return QVariant();
        if (role == Qt::DisplayRole) {
            const QStringList &row = m_data[index.row()];
            if (index.column() < row.size()) return row[index.column()];
        }
        if (role == Qt::BackgroundRole) {
            // Check 'valid' column (usually 'TRUE' or 'FALSE')
            if (m_validColIdx >= 0) {
                QString val = m_data[index.row()].value(m_validColIdx).toUpper();
                if (val == "FALSE" || val == "0") return QBrush(QColor("#4a1a1a")); // Dark Red
            }
        }
        if (role == Qt::ForegroundRole) {
            return QColor("#E0E0E0");
        }
        return QVariant();
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override {
        if (!index.isValid()) return Qt::NoItemFlags;
        // Allow editing for all cells except the Valid column itself (optional)
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
        if (index.isValid() && role == Qt::EditRole) {
            QString newValue = value.toString();
            // Update data
            if (index.row() < m_data.size() && index.column() < m_data[index.row()].size()) {
                m_data[index.row()][index.column()] = newValue;
                // Re-validate row? For now, we only update the value.
                // ideally we should re-run validation logic here, but that's complex without the ImportManager logic.
                // User can visually correct it.
                // Or we can simple-check: Is Empty?
                emit dataChanged(index, index, {role});
                return true;
            }
        }
        return false;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
             if (section < m_headers.size()) return m_headers[section];
        }
        return QVariant();
    }
    
    // Helper to get data back
    QList<QStringList> getData() const { return m_data; }
    int getValidColIdx() const { return m_validColIdx; }

private:
    QStringList m_headers;
    QList<QStringList> m_data;
    int m_validColIdx;
};

ManHinhPreviewImport::ManHinhPreviewImport(const ImportManager::PreviewResult &result, const QString &excelPath, ImportManager *importer, QWidget *parent)
    : QDialog(parent), m_result(result), m_excelPath(excelPath), m_importer(importer)
{
    setWindowTitle(LanguageManager::instance().getText("TITLE_PREVIEW_IMPORT"));
    setMinimumSize(900, 600);
    setupUI();
}

ManHinhPreviewImport::~ManHinhPreviewImport() {}

void ManHinhPreviewImport::setupUI() {
    // Modern Dark Theme styling
    setStyleSheet(
        "QDialog { background-color: #1E1E1E; color: #E0E0E0; }"
        "QTabWidget::pane { border: 1px solid #3E3E42; background: #252526; }"
        "QTabBar::tab { background: #2D2D30; color: #CCCCCC; padding: 8px 12px; border: 1px solid #3E3E42; border-bottom: none; }"
        "QTabBar::tab:selected { background: #3E3E42; color: #FFFFFF; font-weight: bold; }"
        "QTableView { background-color: #1e1e1e; gridline-color: #3e3e42; color: #e0e0e0; border: none; }"
        "QHeaderView::section { background-color: #2D2D30; color: #E0E0E0; padding: 5px; border: 1px solid #3E3E42; }"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Header
    QLabel *lblTitle = new QLabel(LanguageManager::instance().getText("HEADER_PREVIEW_IMPORT"), this);
    lblTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFFFFF;");
    mainLayout->addWidget(lblTitle);
    
    QLabel *lblSub = new QLabel(LanguageManager::instance().getText("LBL_PREVIEW_NOTE"), this);
    lblSub->setStyleSheet("font-style: italic; color: #AAAAAA; margin-bottom: 5px;");
    mainLayout->addWidget(lblSub);

    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);
    
    QLabel *lblChoose = new QLabel(LanguageManager::instance().getText("LBL_CHOOSE_IMPORT_TYPE"), this);
    lblChoose->setStyleSheet("color: #00BCF2; font-weight: bold; margin-top: 5px;");
    mainLayout->addWidget(lblChoose);

    int bookTabIdx = -1;
    int readerTabIdx = -1;

    if (!m_result.dauSachCsv.isEmpty() && QFile::exists(m_result.dauSachCsv)) {
        QTableView *tv = new QTableView();
        loadCsv(m_result.dauSachCsv, tv);
        bookTabIdx = m_tabWidget->addTab(tv, LanguageManager::instance().getText("TAB_BOOKS")); 
    }
    if (!m_result.banSaoCsv.isEmpty() && QFile::exists(m_result.banSaoCsv)) {
        QTableView *tv = new QTableView();
        loadCsv(m_result.banSaoCsv, tv);
        m_tabWidget->addTab(tv, LanguageManager::instance().getText("TAB_COPIES"));
    }
    if (!m_result.docGiaCsv.isEmpty() && QFile::exists(m_result.docGiaCsv)) {
        QTableView *tv = new QTableView();
        loadCsv(m_result.docGiaCsv, tv); 
        readerTabIdx = m_tabWidget->addTab(tv, LanguageManager::instance().getText("TAB_READERS"));
    }
    if (!m_result.muonTraCsv.isEmpty() && QFile::exists(m_result.muonTraCsv)) {
        QTableView *tv = new QTableView();
        loadCsv(m_result.muonTraCsv, tv);
        m_tabWidget->addTab(tv, LanguageManager::instance().getText("TAB_LOANS"));
    }

    // Smart auto-select tab
    if (m_result.detectedType == "SACH" && bookTabIdx != -1) {
        m_tabWidget->setCurrentIndex(bookTabIdx);
    } else if (m_result.detectedType == "DOCGIA" && readerTabIdx != -1) {
        m_tabWidget->setCurrentIndex(readerTabIdx);
    }

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton *btnCancel = new QPushButton(LanguageManager::instance().getText("BTN_CANCEL"), this);
    btnCancel->setMinimumHeight(40);
    btnCancel->setStyleSheet("background-color: #3C3C3C; color: #FFF; border: none; padding: 0 20px; border-radius: 4px;");
    connect(btnCancel, &QPushButton::clicked, this, &ManHinhPreviewImport::onBtnCancelClicked);
    
    QPushButton *btnApply = new QPushButton(LanguageManager::instance().getText("BTN_APPLY_IMPORT"), this);
    btnApply->setMinimumHeight(40);
    btnApply->setStyleSheet("background-color: #007ACC; color: #FFF; border: none; padding: 0 20px; border-radius: 4px; font-weight: bold;");
    connect(btnApply, &QPushButton::clicked, this, &ManHinhPreviewImport::onBtnApplyClicked);

    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnApply);

    mainLayout->addLayout(btnLayout);
}

void ManHinhPreviewImport::loadCsv(const QString &csvPath, QTableView *tableView) {
    QFile file(csvPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    //in.setCodec("UTF-8"); // Removed deprecated setCodec, defaults to UTF8

    // Simple CSV parser (not robust for commas in quotes, but Python csv writer is standard)
    // Actually, simple split(',') is risky if cells contain commas.
    // Let's implement a slightly better parser or assume standard simple csv.
    // Since we control generation, we can assume standard CSV format.
    // Ideally we should use a proper CSV parser.
    // For now, let's use a simple regex split or assumes no embedded commas for simplicity
    // OR: Use standard Regex for CSV: (?:^|,)(?=[^"]|(")?)"?((?(1)[^"]*|[^,"]*))"?(?=,|$)
    
    // Better: Read line, use a simple state machine or regex.
    // Python csv module quotes fields with commas.
    
    QStringList headers;
    QList<QStringList> data;
    int validColIdx = -1;

    bool isHeader = true;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        
        // Basic split - IMPROVE IF NEEDED
        QStringList row;
        bool inQuote = false;
        QString field;
        for (int i = 0; i < line.length(); ++i) {
            QChar c = line[i];
            if (c == '"') {
                inQuote = !inQuote;
            } else if (c == ',' && !inQuote) {
                row.append(field);
                field.clear();
            } else {
                field.append(c);
            }
        }
        row.append(field); // Last field

        // Cleanup quotes
        for (int i = 0; i < row.size(); ++i) {
             if (row[i].startsWith('"') && row[i].endsWith('"')) {
                 row[i] = row[i].mid(1, row[i].length()-2);
                 // Unescape double quotes if any "" -> "
                 row[i].replace("\"\"", "\"");
             }
        }

        if (isHeader) {
            headers = row;
            // Find 'valid' column
            for (int i=0; i<headers.size(); ++i) {
                if (headers[i].compare("valid", Qt::CaseInsensitive) == 0) {
                    validColIdx = i;
                    break;
                }
            }
            isHeader = false;
        } else {
            data.append(row);
        }
    }

    CsvTableModel *model = new CsvTableModel(headers, data, validColIdx, this);
    tableView->setModel(model);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setAlternatingRowColors(true);
    tableView->resizeColumnsToContents();
}

void ManHinhPreviewImport::onBtnCancelClicked() {
    reject();
}

void ManHinhPreviewImport::onBtnApplyClicked() {
    // Check if there are invalid rows
    // Iterate all tabs? Or just active one? 
    // Ideally we should check ALL tabs or just the one we are importing.
    // Current flow: Import Everything.
    
    // Simple check: Loop through all tabs, get model, check for Invalid rows
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        QTableView *tv = qobject_cast<QTableView*>(m_tabWidget->widget(i));
        if (tv) {
            CsvTableModel *model = static_cast<CsvTableModel*>(tv->model());
            if (model) {
                int validIdx = model->getValidColIdx();
                if (validIdx >= 0) {
                     QList<QStringList> data = model->getData();
                     for (const QStringList &row : data) {
                         if (validIdx < row.size()) {
                             if (row[validIdx].toUpper() == "FALSE" || row[validIdx] == "0") {
                                 QMessageBox::warning(this, LanguageManager::instance().getText("TITLE_WARNING"), 
                                     "Dữ liệu còn dòng không hợp lệ (màu đỏ). Vui lòng sửa hoặc xóa trước khi nhập.");
                                 m_tabWidget->setCurrentIndex(i);
                                 return; // Block
                             }
                         }
                     }
                }
            }
        }
    }

    // Ask for mode
    QStringList modes; 
    modes << LanguageManager::instance().getText("OPTION_MERGE") 
          << LanguageManager::instance().getText("OPTION_REPLACE") 
          << LanguageManager::instance().getText("OPTION_SKIP");
    bool ok;
    QString item = QInputDialog::getItem(this, LanguageManager::instance().getText("DIALOG_IMPORT_MODE_TITLE"), 
        LanguageManager::instance().getText("DIALOG_IMPORT_MODE_MSG"), modes, 0, false, &ok);
    
    if (ok && !item.isEmpty()) {
        QString mode = "merge";
        if (item.contains("Replace")) mode = "replace";
        if (item.contains("Skip")) mode = "skip";

        // Determine Data to Apply (From Current Tab)
        QWidget *current = m_tabWidget->currentWidget();
        QTableView *tv = qobject_cast<QTableView*>(current);
        bool success = false;
        
        if (tv) {
            CsvTableModel *model = static_cast<CsvTableModel*>(tv->model());
            if (model) {
                QList<QStringList> data = model->getData();
                // Detect Type
                QString type = "SACH"; // Default
                QVariant head = model->headerData(0, Qt::Horizontal, Qt::DisplayRole);
                QString headStr = head.toString();
                if (headStr.contains("HoDem", Qt::CaseInsensitive)) {
                    type = "DOCGIA";
                }
                
                success = m_importer->runApplyData(data, type, mode, this);
            }
        } else {
            // Fallback to original file-based apply if somehow no table (shouldn't happen)
            success = m_importer->runApply(m_excelPath, mode, this);
        }

        if (success) {
            QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_IMPORT_SUCCESS"));
            accept();
        }
    }
}
