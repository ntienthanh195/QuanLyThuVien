#ifndef IMPORTMANAGER_H
#define IMPORTMANAGER_H

#include <QString>
#include <QStringList>
#include <QObject>
#include <QWidget>

// Forward declaration to avoid including heavy headers here if possible, 
// but QXlsx is often included in cpp. 
// Standard Qt classes

class ImportManager : public QObject {
    Q_OBJECT
public:
    struct PreviewResult {
        bool success;
        QString message; // Log or Error
        
        // Structures to hold preview data instead of CSV paths
        // For simplicity during migration, we can still use CSV paths if we want to 
        // generate temporary CSVs from C++, OR we can change this to QList<QStringList>
        // But to minimize changes to the UI (ManHinhPreviewImport) which likely expects CSVs,
        // we might stick to generating Temp CSVs from QXlsx in Step 1.
        
        // However, for a "Native" approach, passing objects is better.
        // Let's assume for now we keep the struct interface compatible with the UI 
        // which likely reads these CSV files. 
        // We will generate these CSVs internally in C++.
        QString dauSachCsv;
        QString banSaoCsv;
        QString docGiaCsv; 
        QString muonTraCsv;
        
        // Smart Detection Results
        QString detectedType; // "SACH" or "DOCGIA"
    };

    explicit ImportManager(QObject *parent = nullptr);
    static ImportManager& instance(); // Singleton

    // No longer need ensureDependencies or getPythonExecutable

    // Runs preview mode (Native C++ Reading)
    PreviewResult runPreview(const QString &excelFile);

    // Runs application to DB (Native C++ Reading)
    bool runApply(const QString &excelFile, const QString &mode, QWidget *parent);
    
    // Runs application from Edited Data (In-Memory)
    bool runApplyData(const QList<QStringList> &data, const QString &type, const QString &mode, QWidget *parent);

private:
    // Helper to generate a temp CSV from QList<QStringList> data
    // This maintains compatibility with the existing Preview UI that reads CSVs
    QString createTempCsv(const QString &prefix, const QList<QStringList> &data, const QString &type = "");
};

#endif // IMPORTMANAGER_H
