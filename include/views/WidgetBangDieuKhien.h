#ifndef WIDGETBANGDIEUKHIEN_H
#define WIDGETBANGDIEUKHIEN_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGridLayout>
#include <QComboBox>
#include <QTimer>
#include <QShortcut>

class WidgetBangDieuKhien : public QWidget {
    Q_OBJECT

public:
    explicit WidgetBangDieuKhien(QWidget *parent = nullptr);
    ~WidgetBangDieuKhien();

    // Cập nhật dữ liệu thống kê
    void capNhatThongKe();
    
    // Loading state
    void setLoadingState(bool loading);
    
    // Keyboard shortcuts
    void setupShortcuts();

signals:
    void yeuCauXemChiTiet(const QString &loaiCanhBao);
    void yeuCauThemSachMoi();
    void yeuCauDangKyDocGia();
    void yeuCauTaoPhieuMuon();
    void yeuCauSaoLuu();
    void yeuCauXemThongTin();
    void yeuCauMoDemo();
    void yeuCauMoDemoBang();
    void yeuCauXemTop10Sach();
    void yeuCauXemTop10DocGia();

private:
    // Widget chính
    QVBoxLayout *layoutChinh;
    QPushButton *btnRefresh;
    QComboBox *comboBoLocThoiGian;
    QLabel *labelTieuDeTrang;
    
    // Khu vực cảnh báo
    QWidget *khungCanhBao;
    QWidget *cardSachQuaHan;
    QWidget *cardTheSapHet;
    QLabel *labelSachQuaHan;
    QLabel *labelTheSapHetHan;
    QLabel *labelMoTaSachQuaHan;
    QLabel *labelMoTaTheSapHet;
    QPushButton *btnXemSachQuaHan;
    QPushButton *btnXemTheSapHet;
    
    // Khu vực số liệu chính
    QWidget *khungThongKe;
    QLabel *labelTongSoSach;
    QLabel *labelSoDocGia;
    QLabel *labelSachDangMuon;
    QLabel *labelLuotMuonHomNay;
    
    // Khu vực tác vụ nhanh (FIX: Moved before khungTop10 to match initialization order)
    QWidget *khungTacVuNhanh;
    QPushButton *btnThemSachMoi;
    QPushButton *btnDangKyDocGia;
    QPushButton *btnTaoPhieuMuon;
    QPushButton *btnSaoLuu;
    QPushButton *btnThongTin;
    
    // Khu vực Top 10 Statistics
    QWidget *khungTop10;
    QWidget *cardTop10Sach;
    QWidget *cardTop10DocGia;
    QPushButton *btnXemTop10Sach;
    QPushButton *btnXemTop10DocGia;
    
    // Loading & Auto-refresh
    QLabel *labelLoading;
    QTimer *timerAutoRefresh;
    bool isLoading;
    
    // Keyboard shortcuts
    QShortcut *shortcutRefresh;
    QShortcut *shortcutAddBook;
    QShortcut *shortcutAddReader;
    QShortcut *shortcutNewLoan;
    
    // Phương thức khởi tạo giao diện
    void khoiTaoGiaoDien();
    void taoTieuDeVaBoLoc();
    void taoKhungCanhBao();
    void taoKhungThongKe();
    void taoKhungTop10();
    void taoKhungTacVuNhanh();
    
    // Xử lý sự kiện
    void xuLyRefresh();
    void xuLyThayDoiBoLoc(int index);
    
    // Phương thức tạo các thẻ
    QWidget* taoCardCanhBao(const QString &tieuDe, const QString &moTa, 
                             const QString &mauNen, const QString &mauChu,
                             const QString &icon, QLabel **labelSoLieu, 
                             QPushButton **btnHanhDong);
    QWidget* taoCardThongKe(const QString &icon, const QString &tieuDe, 
                            const QString &giaTri, const QString &mauChu);
    QPushButton* taoNutTacVuNhanh(const QString &tieuDe, const QString &icon, const QString &mauNen);
    
    // Tính toán số liệu thống kê
    int demSachQuaHan();
    int demTheSapHetHan();
    int demTongSoSach();
    int demSoDocGia();
    int demSachDangMuon();
    int demLuotMuonHomNay();
    
    // Lấy thông tin chi tiết cho cảnh báo
    QString layThongTinSachQuaHanNhieuNhat();
    QString layThongTinDocGiaQuaHan();
};

#endif // WIDGETBANGDIEUKHIEN_H
