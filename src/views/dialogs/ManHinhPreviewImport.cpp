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
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            if (section < m_headers.size()) return m_headers[section];
        }
        return QVariant();
    }

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

    // Tabs
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);

    if (!m_result.dauSachCsv.isEmpty() && QFile::exists(m_result.dauSachCsv)) {
        QTableView *tv = new QTableView();
        loadCsv(m_result.dauSachCsv, tv);
        m_tabWidget->addTab(tv, LanguageManager::instance().getText("TAB_BOOKS")); 
    }
    if (!m_result.banSaoCsv.isEmpty() && QFile::exists(m_result.banSaoCsv)) {
        QTableView *tv = new QTableView();
        loadCsv(m_result.banSaoCsv, tv);
        m_tabWidget->addTab(tv, LanguageManager::instance().getText("TAB_COPIES"));
    }
    if (!m_result.docGiaCsv.isEmpty() && QFile::exists(m_result.docGiaCsv)) {
        QTableView *tv = new QTableView();
        loadCsv(m_result.docGiaCsv, tv); // Leaving hardcoded for safety
        m_tabWidget->addTab(tv, LanguageManager::instance().getText("TAB_READERS"));
    }
    if (!m_result.muonTraCsv.isEmpty() && QFile::exists(m_result.muonTraCsv)) {
        QTableView *tv = new QTableView();
        loadCsv(m_result.muonTraCsv, tv);
        m_tabWidget->addTab(tv, LanguageManager::instance().getText("TAB_LOANS"));
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

        if (m_importer->runApply(m_excelPath, mode, this)) {
            QMessageBox::information(this, LanguageManager::instance().getText("TITLE_SUCCESS"), LanguageManager::instance().getText("MSG_IMPORT_SUCCESS"));
            accept();
        }
    }
}
