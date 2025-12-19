    #include "utils/LanguageManager.h"
#include "giao_dien/hop_thoai/HopThoaiThongKe.h"
#include "models/QuanLySach.h"
#include "models/DocGia.h"
#include "utils/ResponsiveHelper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QtCharts/QChart>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

HopThoaiThongKe::HopThoaiThongKe(QWidget *cha) : QDialog(cha) {
    setWindowTitle("📊 " + LanguageManager::instance().getText("TAB_STATS_SYMBOL"));
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    setMinimumSize(ResponsiveHelper::getScaledWidth(900, screenSize), 
                   ResponsiveHelper::getScaledHeight(700, screenSize));
    
    thieLapGiaoDien();
    taiDuLieuThongKe();
}

void HopThoaiThongKe::thieLapGiaoDien() {
    ResponsiveHelper::ScreenSize screenSize = ResponsiveHelper::detectScreenSize();
    QVBoxLayout *boTriChinh = new QVBoxLayout(this);
    int margin = ResponsiveHelper::getScaledMargin(20, screenSize);
    boTriChinh->setContentsMargins(margin, margin, margin, margin);
    
    QLabel *tieuDe = new QLabel("📊 " + LanguageManager::instance().getText("STATS_MAIN_TITLE"));
    int titleSize = ResponsiveHelper::getScaledFontSize(22, screenSize);
    tieuDe->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #FFFFFF;").arg(titleSize));
    tieuDe->setAlignment(Qt::AlignCenter);
    boTriChinh->addWidget(tieuDe);
    
    QHBoxLayout *boTriNoiDung = new QHBoxLayout();
    
    // Cột trái: Thống kê sách
    QGroupBox *nhomSach = new QGroupBox("📚 " + LanguageManager::instance().getText("STATS_BOOKS_GROUP"));
    int fontSize = ResponsiveHelper::getScaledFontSize(14, screenSize);
    nhomSach->setStyleSheet(QString("QGroupBox { background-color: #1E1E2D; border: 2px solid #3699FF; border-radius: 12px; font-size: %1px; color: #FFFFFF; padding-top: 15px; }").arg(fontSize));
    QVBoxLayout *boTriSach = new QVBoxLayout(nhomSach);
    
    QGridLayout *luoiSach = new QGridLayout();
    QString kieuNhan = QString("color: #FFFFFF; padding: 8px; font-size: %1px;").arg(fontSize);
    
    nhanTongDauSach = new QLabel("0");
    nhanTongBanSao = new QLabel("0");
    nhanCoTheMuon = new QLabel("0");
    nhanDangMuon = new QLabel("0");
    nhanThanhLy = new QLabel("0");
    nhanTyLe = new QLabel("0%");
    nhanTheLoai = new QLabel("0");
    
    luoiSach->addWidget(new QLabel(LanguageManager::instance().getText("LBL_TOTAL_TITLES")), 0, 0);
    luoiSach->addWidget(nhanTongDauSach, 0, 1);
    luoiSach->addWidget(new QLabel(LanguageManager::instance().getText("LBL_TOTAL_COPIES_STAT")), 1, 0);
    luoiSach->addWidget(nhanTongBanSao, 1, 1);
    luoiSach->addWidget(new QLabel(LanguageManager::instance().getText("LBL_AVAILABLE_STAT")), 2, 0);
    luoiSach->addWidget(nhanCoTheMuon, 2, 1);
    luoiSach->addWidget(new QLabel(LanguageManager::instance().getText("LBL_BORROWED_STAT")), 3, 0);
    luoiSach->addWidget(nhanDangMuon, 3, 1);
    luoiSach->addWidget(new QLabel(LanguageManager::instance().getText("LBL_LIQUIDATED_STAT")), 4, 0);
    luoiSach->addWidget(nhanThanhLy, 4, 1);
    luoiSach->addWidget(new QLabel(LanguageManager::instance().getText("LBL_BORROW_RATE")), 5, 0);
    luoiSach->addWidget(nhanTyLe, 5, 1);
    luoiSach->addWidget(new QLabel(LanguageManager::instance().getText("LBL_CATEGORIES_STAT")), 6, 0);
    luoiSach->addWidget(nhanTheLoai, 6, 1);
    
    for (int i = 0; i < luoiSach->rowCount(); i++) {
        if (luoiSach->itemAtPosition(i, 0))
            ((QLabel*)luoiSach->itemAtPosition(i, 0)->widget())->setStyleSheet(kieuNhan);
        if (luoiSach->itemAtPosition(i, 1))
            ((QLabel*)luoiSach->itemAtPosition(i, 1)->widget())->setStyleSheet(kieuNhan + " font-weight: bold; color: #1BC5BD;");
    }
    
    boTriSach->addLayout(luoiSach);
    boTriSach->addWidget(taoBieuDoSach());
    boTriNoiDung->addWidget(nhomSach);
    
    // Cột phải: Thống kê độc giả
    QGroupBox *nhomDocGia = new QGroupBox("👥 " + LanguageManager::instance().getText("STATS_READERS_GROUP"));
    nhomDocGia->setStyleSheet(QString("QGroupBox { background-color: #1E1E2D; border: 2px solid #F64E60; border-radius: 12px; font-size: %1px; color: #FFFFFF; padding-top: 15px; }").arg(fontSize));
    QVBoxLayout *boTriDocGia = new QVBoxLayout(nhomDocGia);
    
    QGridLayout *luoiDocGia = new QGridLayout();
    
    nhanTongDocGia = new QLabel("0");
    nhanHoatDong = new QLabel("0");
    nhanBiKhoa = new QLabel("0");
    nhanDangMuonSach = new QLabel("0");
    nhanTongSachMuon = new QLabel("0");
    nhanQuaHan = new QLabel("0");
    
    luoiDocGia->addWidget(new QLabel(LanguageManager::instance().getText("LBL_TOTAL_READERS_STAT")), 0, 0);
    luoiDocGia->addWidget(nhanTongDocGia, 0, 1);
    luoiDocGia->addWidget(new QLabel(LanguageManager::instance().getText("LBL_ACTIVE_STAT")), 1, 0);
    luoiDocGia->addWidget(nhanHoatDong, 1, 1);
    luoiDocGia->addWidget(new QLabel(LanguageManager::instance().getText("LBL_LOCKED_STAT")), 2, 0);
    luoiDocGia->addWidget(nhanBiKhoa, 2, 1);
    luoiDocGia->addWidget(new QLabel(LanguageManager::instance().getText("LBL_READERS_BORROWING_STAT")), 3, 0);
    luoiDocGia->addWidget(nhanDangMuonSach, 3, 1);
    luoiDocGia->addWidget(new QLabel(LanguageManager::instance().getText("LBL_TOTAL_BORROWED_BOOKS_STAT")), 4, 0);
    luoiDocGia->addWidget(nhanTongSachMuon, 4, 1);
    luoiDocGia->addWidget(new QLabel(LanguageManager::instance().getText("LBL_READERS_OVERDUE_STAT")), 5, 0);
    luoiDocGia->addWidget(nhanQuaHan, 5, 1);
    
    for (int i = 0; i < luoiDocGia->rowCount(); i++) {
        if (luoiDocGia->itemAtPosition(i, 0))
            ((QLabel*)luoiDocGia->itemAtPosition(i, 0)->widget())->setStyleSheet(kieuNhan);
        if (luoiDocGia->itemAtPosition(i, 1))
            ((QLabel*)luoiDocGia->itemAtPosition(i, 1)->widget())->setStyleSheet(kieuNhan + " font-weight: bold; color: #FFA800;");
    }
    
    boTriDocGia->addLayout(luoiDocGia);
    boTriDocGia->addWidget(taoBieuDoDocGia());
    boTriNoiDung->addWidget(nhomDocGia);
    
    boTriChinh->addLayout(boTriNoiDung);
    
    // Nút đóng
    QHBoxLayout *boTriNut = new QHBoxLayout();
    int buttonHeight = ResponsiveHelper::getScaledHeight(45, screenSize);
    
    nutDong = new QPushButton("✖ " + LanguageManager::instance().getText("BTN_CLOSE"));
    nutDong->setMinimumHeight(buttonHeight);
    nutDong->setStyleSheet("QPushButton { background-color: #3699FF; color: white; border: none; border-radius: 8px; font-weight: bold; padding: 0 30px; } QPushButton:hover { background-color: #187DE4; }");
    connect(nutDong, &QPushButton::clicked, this, &QDialog::accept);
    
    boTriNut->addStretch();
    boTriNut->addWidget(nutDong);
    boTriChinh->addLayout(boTriNut);
}

void HopThoaiThongKe::taiDuLieuThongKe() {
    // Lấy thống kê sách
    ThongKeSach tkSach = LayThongKeSach();
    nhanTongDauSach->setText(QString::number(tkSach.tongSoDauSach));
    nhanTongBanSao->setText(QString::number(tkSach.tongSoBanSao));
    nhanCoTheMuon->setText(QString::number(tkSach.soBanSaoCoTheMuon));
    nhanDangMuon->setText(QString::number(tkSach.soBanSaoDangMuon));
    nhanThanhLy->setText(QString::number(tkSach.soBanSaoThanhLy));
    nhanTyLe->setText(QString::number(tkSach.tyLeChoMuon, 'f', 1) + "%");
    nhanTheLoai->setText(QString::number(tkSach.soTheLoaiKhacNhau));
    
    // Lấy thống kê độc giả
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    ThongKeDocGia tkDocGia = LayThongKeDocGia(rootDocGia, dsDauSach, soLuongDauSach);
    nhanTongDocGia->setText(QString::number(tkDocGia.tongSoDocGia));
    nhanHoatDong->setText(QString::number(tkDocGia.docGiaHoatDong));
    nhanBiKhoa->setText(QString::number(tkDocGia.docGiaBiKhoa));
    nhanDangMuonSach->setText(QString::number(tkDocGia.docGiaDangMuonSach));
    nhanTongSachMuon->setText(QString::number(tkDocGia.tongSoSachDangMuon));
    nhanQuaHan->setText(QString::number(tkDocGia.docGiaCoSachQuaHan));
}

QChartView* HopThoaiThongKe::taoBieuDoSach() {
    ThongKeSach tk = LayThongKeSach();
    
    QPieSeries *series = new QPieSeries();
    series->append(LanguageManager::instance().getText("OPTION_STATUS_AVAILABLE_SYMBOL"), tk.soBanSaoCoTheMuon);
    series->append(LanguageManager::instance().getText("STATUS_BORROWED"), tk.soBanSaoDangMuon);
    series->append(LanguageManager::instance().getText("OPTION_STATUS_LIQUIDATED_SYMBOL"), tk.soBanSaoThanhLy);
    
    series->setLabelsVisible(true);
    series->setLabelsPosition(QPieSlice::LabelOutside);
    
    for (auto slice : series->slices()) {
        slice->setLabel(QString("%1: %2 (%3%)")
            .arg(slice->label())
            .arg((int)slice->value())
            .arg(slice->percentage() * 100, 0, 'f', 1));
    }
    
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(LanguageManager::instance().getText("CHART_BOOK_STATUS"));
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setBackgroundBrush(QBrush(QColor("#1E1E2D")));
    chart->setTitleBrush(QBrush(Qt::white));
    chart->legend()->setLabelColor(Qt::white);
    
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(250);
    
    return chartView;
}

QChartView* HopThoaiThongKe::taoBieuDoDocGia() {
    extern PTRDG rootDocGia;
    extern PTRDS dsDauSach[];
    extern int soLuongDauSach;
    
    ThongKeDocGia tk = LayThongKeDocGia(rootDocGia, dsDauSach, soLuongDauSach);
    
    QPieSeries *series = new QPieSeries();
    series->append(LanguageManager::instance().getText("STATUS_ACTIVE"), tk.docGiaHoatDong);
    series->append(LanguageManager::instance().getText("STATUS_LOCKED"), tk.docGiaBiKhoa);
    
    series->setLabelsVisible(true);
    series->setLabelsPosition(QPieSlice::LabelOutside);
    
    for (auto slice : series->slices()) {
        slice->setLabel(QString("%1: %2 (%3%)")
            .arg(slice->label())
            .arg((int)slice->value())
            .arg(slice->percentage() * 100, 0, 'f', 1));
    }
    
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(LanguageManager::instance().getText("CHART_READER_STATUS"));
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setBackgroundBrush(QBrush(QColor("#1E1E2D")));
    chart->setTitleBrush(QBrush(Qt::white));
    chart->legend()->setLabelColor(Qt::white);
    
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(250);
    
    return chartView;
}
