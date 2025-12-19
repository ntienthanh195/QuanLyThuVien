#include "utils/ImportManager.h"
#include "utils/FilePathManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QProgressDialog>
#include <QDebug>

ImportManager::ImportManager(QObject *parent) : QObject(parent) {
    m_pythonExe = getPythonExecutable();
    m_scriptPath = getScriptPath();
}

ImportManager& ImportManager::instance() {
    static ImportManager _instance;
    return _instance;
}

QString ImportManager::getPythonExecutable() {
    QString repoRoot = FilePathManager::getRootPath();
    
    // 1. Check Portable Runtime (Bundled with App) - Deployment Priority
    QString appDir = QCoreApplication::applicationDirPath();
    QStringList portablePaths;
    portablePaths << QDir(appDir).filePath("python_runtime/python.exe");
    portablePaths << QDir(appDir).filePath("python/python.exe");
    portablePaths << QDir(appDir).filePath("runtime/python.exe");
    
    // Developer Convenience: Check Source Root (Repo Root)
    portablePaths << QDir(repoRoot).filePath("python_runtime/python.exe");
    
    for (const QString &path : portablePaths) {
        if (QFile::exists(path)) return path;
    }

    // 2. Check Dev Venv (Repo Root or App Root)
    QString venvPython = QDir(repoRoot).filePath(".venv/Scripts/python.exe");
    if (QFile::exists(venvPython)) return venvPython;
    
    QString appVenv = QDir(appDir).filePath(".venv/Scripts/python.exe");
    if (QFile::exists(appVenv)) return appVenv;

    // 3. Fallback to System Python
    return "python"; 
}

QString ImportManager::getScriptPath() {
    QString repoRoot = FilePathManager::getRootPath();
    QStringList candidates;
    candidates << QDir(repoRoot).filePath("tools/import_from_excel.py");
    candidates << QDir(QCoreApplication::applicationDirPath()).filePath("tools/import_from_excel.py");
    candidates << QDir(QCoreApplication::applicationDirPath()).filePath("../tools/import_from_excel.py");
    candidates << QDir(QDir::currentPath()).filePath("tools/import_from_excel.py");
    candidates << QDir(repoRoot).filePath("../tools/import_from_excel.py");

    for (const QString &cand : candidates) {
        if (QFile::exists(cand)) return cand;
    }
    return QString();
}

bool ImportManager::ensureDependencies(QWidget *parent) {
    if (m_pythonExe.isEmpty()) return false;
    
    // Quick check if we can run a simple import
    QProcess proc;
    setupProcessEnv(proc);
    proc.start(m_pythonExe, QStringList() << "-c" << "import openpyxl");
    proc.waitForFinished(5000);
    if (proc.exitCode() == 0) return true;

    // Install if missing
    QProgressDialog progress("Đang cài đặt thư viện hỗ trợ Excel...", QString(), 0, 0, parent);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.show();
    QCoreApplication::processEvents();

    QProcess installProc;
    setupProcessEnv(installProc);
    installProc.start(m_pythonExe, QStringList() << "-m" << "pip" << "install" << "openpyxl" << "--user" << "--prefer-binary");
    // Timeout after 3 minutes (180s) to avoid hanging forever
    if (!installProc.waitForFinished(180000)) {
        installProc.kill();
        progress.close();
        QMessageBox::critical(parent, "Lỗi Quá Giờ", "Quá trình tải thư viện mất quá nhiều thời gian.\nVui lòng kiểm tra kết nối mạng và thử lại.");
        return false;
    }
    
    // Close progress before showing error
    progress.close();

    if (installProc.exitCode() != 0) {
        QString stdOut = QString::fromUtf8(installProc.readAllStandardOutput());
        QString stdErr = QString::fromUtf8(installProc.readAllStandardError());
        QString detailedLog = "Output:\n" + stdOut + "\nError:\n" + stdErr;

        QMessageBox msgBox(parent);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Lỗi Cài Đặt Thư Viện");
        msgBox.setText("Không thể tự động cài đặt thư viện 'openpyxl'.");
        msgBox.setInformativeText("Vui lòng chụp lại thông báo lỗi này và gửi cho kỹ thuật viên.");
        msgBox.setDetailedText(detailedLog);
        msgBox.exec();
        return false;
    }
    return true;
}

ImportManager::PreviewResult ImportManager::runPreview(const QString &excelFile) {
    PreviewResult result;
    result.success = false;

    if (m_scriptPath.isEmpty()) {
        result.message = "Không tìm thấy file script import_from_excel.py";
        return result;
    }

    QProcess proc;
    // Output directly to 'tools' folder for simplicity via script default, 
    // or explicit output arg. Let's use explicit output temp dir logic later if needed
    // For now script defaults to 'tools' or explicit '--out'
    
    // Use a temp folder logic or just use tools
    QString outDir = QDir(FilePathManager::getRootPath()).filePath("tools");
    
    QStringList args;
    args << m_scriptPath << "--file" << excelFile << "--preview" << "--out" << outDir;
    
    setupProcessEnv(proc);
    proc.start(m_pythonExe, args);
    if (!proc.waitForStarted(10000)) {
        result.message = "Không thể khởi chạy Python process.";
        return result;
    }
    proc.waitForFinished(-1);

    QString stdoutStr = QString::fromUtf8(proc.readAllStandardOutput());
    QString stderrStr = QString::fromUtf8(proc.readAllStandardError());
    
    // Check if script output contains preview CSV paths
    // The script prints "Preview CSVs: \n - path"
    // We can just construct paths based on outDir since the script uses fixed names inside outDir
    result.dauSachCsv = QDir(outDir).filePath("preview_dausach.csv");
    result.banSaoCsv = QDir(outDir).filePath("preview_bansao.csv");
    result.docGiaCsv = QDir(outDir).filePath("preview_docgia.csv");
    result.muonTraCsv = QDir(outDir).filePath("preview_muontra.csv");

    if (proc.exitCode() == 0) {
        result.success = true;
        result.message = stdoutStr;
    } else {
        result.success = false;
        result.message = stderrStr + "\n" + stdoutStr;
    }
    return result;
}

bool ImportManager::runApply(const QString &excelFile, const QString &mode, QWidget *parent) {
    if (m_scriptPath.isEmpty()) return false;

    QProgressDialog progress("Đang cập nhật cơ sở dữ liệu...", QString(), 0, 0, parent);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.show();
    QCoreApplication::processEvents();

    QProcess proc;
    QStringList args;
    args << m_scriptPath << "--file" << excelFile << "--to-db" << "--mode" << mode;
    
    setupProcessEnv(proc);
    proc.start(m_pythonExe, args);
    proc.waitForFinished(-1);
    
    if (proc.exitCode() != 0) {
        QString err = QString::fromUtf8(proc.readAllStandardError());
        QMessageBox::critical(parent, "Lỗi Import", "Có lỗi xảy ra:\n" + err);
        return false;
    }
    
    return true;
}

void ImportManager::setupProcessEnv(QProcess &proc) {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // CRITICAL: Unset PYTHONHOME/PYTHONPATH to prevent conflicts with system/toolchain Python
    env.remove("PYTHONHOME");
    env.remove("PYTHONPATH");
    proc.setProcessEnvironment(env);
}
