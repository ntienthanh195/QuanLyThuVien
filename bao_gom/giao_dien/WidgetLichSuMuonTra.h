/**
 * @file WidgetLichSuMuonTra.h
 * @path bao_gom/giao_dien/WidgetLichSuMuonTra.h
 * @description Header cho Widget xem lịch sử mượn trả và báo cáo tài chính.
 */
#ifndef WIDGETLICHSUMUONTRA_H
#define WIDGETLICHSUMUONTRA_H

#include <QWidget>
#include <QTableView>
#include <QSqlQueryModel>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class WidgetLichSuMuonTra : public QWidget {
    Q_OBJECT

public:
    explicit WidgetLichSuMuonTra(QWidget *parent = nullptr);
    ~WidgetLichSuMuonTra();

    void lamMoi();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void xuLyTimKiem();
    void xuLyXuatBaoCao();
    void capNhatNgonNgu();
    void xuLyMenuChuotPhai(const QPoint &pos);

private:
    void thieLapGiaoDien();
    void thieLapBang();
    QString xayDungCauLenhSQL();

    // UI Elements
    QLineEdit *oNhapTimKiem;
    QDateEdit *ngayBatDau;
    QDateEdit *ngayKetThuc;
    QComboBox *hopChonTrangThai;
    QPushButton *nutTimKiem;
    QPushButton *nutXuatBaoCao;
    QPushButton *nutLuuTruLichSu;
    QCheckBox *chkBaoGomLuuTru;
    
    QTableView *bangLichSu;
    QSqlQueryModel *modelLichSu;
    
    QLabel *nhanThongKe;
};

#endif // WIDGETLICH SUMUONTRA_H
