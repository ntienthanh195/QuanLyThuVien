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
    
    void setupHomeScreen();
    void setupSidebar();
    void setupConnections();
    void apDungStylesheet();
    QPushButton* createMenuButton(const QString &text, const QString &iconPath);
    void updateStatusBar(const QString &message, int timeout = 3000);
};

#endif // MAINWINDOW_H
