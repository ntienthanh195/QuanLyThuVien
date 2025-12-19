#ifndef IMPORTMANAGER_H
#define IMPORTMANAGER_H

#include <QString>
#include <QStringList>
#include <QObject>
#include <QWidget>
#include <QProcess>

class ImportManager : public QObject {
    Q_OBJECT
public:
    struct PreviewResult {
        bool success;
        QString message; // Log or Error
        QString dauSachCsv;
        QString banSaoCsv;
        QString docGiaCsv; // Optional
        QString muonTraCsv; // Optional
    };

    explicit ImportManager(QObject *parent = nullptr);
    static ImportManager& instance(); // Singleton for easier access

    // Determines python executable path logic (system or venv)
    QString getPythonExecutable();

    // Finds the script path
    QString getScriptPath();

    // Ensures dependencies (openpyxl) are installed
    bool ensureDependencies(QWidget *parent);

    // Runs preview mode
    PreviewResult runPreview(const QString &excelFile);

    // Runs application to DB
    bool runApply(const QString &excelFile, const QString &mode, QWidget *parent);

private:
    QString m_pythonExe;
    QString m_scriptPath;
    
    // Helper to isolate python environment
    void setupProcessEnv(QProcess &proc);
};

#endif // IMPORTMANAGER_H
