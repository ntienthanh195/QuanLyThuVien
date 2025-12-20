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
    QProgressDialog progress("Đang kiểm tra và cài đặt thư viện...", QString(), 0, 0, parent);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.show();
    QCoreApplication::processEvents();

    // 1. Check if pip exists
    QProcess pipCheck;
    setupProcessEnv(pipCheck);
    pipCheck.start(m_pythonExe, QStringList() << "-m" << "pip" << "--version");
    pipCheck.waitForFinished(3000);
    
    if (pipCheck.exitCode() != 0) {
        // Pip missing, try to bootstrap
        QProcess bootstrapProc;
        setupProcessEnv(bootstrapProc);
        progress.setLabelText("Đang khôi phục Pip (PackageManager)...");
        bootstrapProc.start(m_pythonExe, QStringList() << "-m" << "ensurepip" << "--default-pip");
        if (!bootstrapProc.waitForFinished(60000) || bootstrapProc.exitCode() != 0) {
             progress.close();
             QString err = QString::fromUtf8(bootstrapProc.readAllStandardError());
             QString msg = "Không tìm thấy Pip (trình quản lý gói) và không thể tự cài đặt.\n";
             if (err.contains("externally-managed-environment")) {
                 msg += "\nMôi trường Python của bạn đang được quản lý bởi hệ thống (thường gặp trên MSYS2/MinGW).\n"
                        "Vui lòng mở terminal và chạy lệnh sau để khắc phục:\n"
                        "pacman -S mingw-w64-x86_64-python-pip mingw-w64-x86_64-python-openpyxl";
             } else {
                 msg += "Chi tiết lỗi: " + err;
             }
             QMessageBox::warning(parent, "Cảnh báo Pip", msg);
             progress.show();
        }
    }

    progress.setLabelText("Đang cài đặt thư viện 'openpyxl'...");
    QCoreApplication::processEvents();

    QProcess installProc;
    setupProcessEnv(installProc);
    installProc.start(m_pythonExe, QStringList() << "-m" << "pip" << "install" << "openpyxl" << "--user" << "--prefer-binary" << "--break-system-packages");
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
        
        // Try self-healing if it's an externally managed environment
        if (stdErr.contains("externally-managed-environment")) {
            progress.setLabelText("Phát hiện môi trường hệ thống bị khóa. Đang tạo môi trường ảo riêng biệt...");
            QCoreApplication::processEvents();
            
            QString repoRoot = FilePathManager::getRootPath();
            QString venvPath = QDir(repoRoot).filePath(".venv");
            
            QProcess venvProc;
            setupProcessEnv(venvProc);
            venvProc.start(m_pythonExe, QStringList() << "-m" << "venv" << venvPath);
            
            if (venvProc.waitForFinished(120000) && venvProc.exitCode() == 0) {
                // Determine venv python path (handle both Windows/Unix styles)
                QString venvPython = QDir(venvPath).filePath("Scripts/python.exe");
                if (!QFile::exists(venvPython)) {
                    venvPython = QDir(venvPath).filePath("bin/python.exe");
                }
                
                if (QFile::exists(venvPython)) {
                    m_pythonExe = venvPython;
                    progress.setLabelText("Đang cài đặt thư viện vào môi trường ảo mới...");
                    QCoreApplication::processEvents();
                    
                    QProcess venvInstall;
                    setupProcessEnv(venvInstall);
                    venvInstall.start(m_pythonExe, QStringList() << "-m" << "pip" << "install" << "openpyxl" << "--prefer-binary");
                    
                    if (venvInstall.waitForFinished(180000) && venvInstall.exitCode() == 0) {
                        progress.close();
                        return true;
                    }
                }
            }
        }

        progress.close();
        QMessageBox msgBox(parent);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Lỗi Cài Đặt Thư Viện");
        
        if (stdErr.contains("externally-managed-environment")) {
            msgBox.setText("Môi trường Python hiện tại không cho phép cài đặt thư viện trực tiếp và quá trình tự sửa lỗi thất bại.");
            msgBox.setInformativeText("CÁCH KHẮC PHỤC THỦ CÔNG:\n"
                                     "1. Mở terminal (MSYS2/MinGW).\n"
                                     "2. Chạy lệnh: pacman -S mingw-w64-x86_64-python-openpyxl\n"
                                     "3. Khởi động lại ứng dụng.");
        } else {
            msgBox.setText("Không thể tự động cài đặt thư viện 'openpyxl'.");
            msgBox.setInformativeText("Vui lòng chụp lại thông báo lỗi này và gửi cho kỹ thuật viên.");
            msgBox.setDetailedText("Output:\n" + stdOut + "\nError:\n" + stdErr);
        }
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
