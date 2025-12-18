#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>
#include <QListWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class WidgetBangDieuKhien;
class WidgetQuanLySach;
class WidgetQuanLyDocGia;
class WidgetMuonTraSach;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    // Helper function để tạo Status Badge (Modern UI)
    static QWidget* TaoTagTrangThai(int trangThai);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showBookManagement();
    void showReaderManagement();
    void showLoanManagement();
    void showAbout();
    void showSettings();
    void showTop10Report();
    void showStatistics();
    void showBackupDialog();
    void showOverdueReport();
    void showDemoPhase1();
    void showDemoBang();
    void returnToHome();

private:
    Ui::MainWindow *ui;
    
    // Central widget va cac man hinh
    QStackedWidget *stackedWidget;
    QWidget *homeWidget;
    WidgetBangDieuKhien *bangDieuKhien;
    WidgetQuanLySach *bookWidget;
    WidgetQuanLyDocGia *readerWidget;
    WidgetMuonTraSach *loanWidget;
    
    // Sidebar navigation
    QListWidget *menuSidebar;
    
    // Home Screen Widget Members (Promoted for Localization)
    QLabel *lblHomeTitle = nullptr;
    QPushButton *btnHomeBooks = nullptr;
    QPushButton *btnHomeReaders = nullptr;
    QPushButton *btnHomeLoans = nullptr;
    QPushButton *btnHomeTop10 = nullptr;
    QPushButton *btnHomeStats = nullptr;
    QPushButton *btnHomeBackup = nullptr;
    QPushButton *btnHomeOverdue = nullptr;
    QPushButton *btnHomeAbout = nullptr;
    QPushButton *btnHomeExit = nullptr;

public slots:
    void capNhatNgonNgu();

private:
    void setupHomeScreen();
    void setupSidebar();
    void setupConnections();
    void apDungStylesheet();
    QPushButton* createMenuButton(const QString &text, const QString &iconPath);
    void updateStatusBar(const QString &message, int timeout = 3000);
    
    // Global Scanner Members
    QString barcodeBuffer;
    qint64 lastKeyTime;
    
protected:
    void keyPressEvent(QKeyEvent *event) override;
    // closeEvent is already protected in line 31, no need to duplicate visibility modifier if already in protected section.
    // Wait, the replaced block is in 'private'. I need to move keyPressEvent to 'protected'.
    // Or I can put it in the existing protected section.

};

#endif // MAINWINDOW_H
