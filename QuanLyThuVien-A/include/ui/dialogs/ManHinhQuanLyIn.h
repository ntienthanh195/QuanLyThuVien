#ifndef MANHINHQUANLYIN_H
#define MANHINHQUANLYIN_H

#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QStackedWidget>
#include <QGroupBox>
#include <QScrollArea>
#include "models/DocGia.h"
#include "models/DauSach.h"

class ManHinhQuanLyIn : public QDialog {
    Q_OBJECT
public:
    explicit ManHinhQuanLyIn(PTRDG caNhan = nullptr, const std::vector<PTRDG>& danhSach = {}, QWidget *parent = nullptr);
    ~ManHinhQuanLyIn();

    // Context Modes
    enum Mode {
        ALL,
        BOOK_CONTEXT,
        READER_CONTEXT
    };
    void setMode(Mode mode);
    
    // Context Setters
    void setBookContext(PTRDS sach);
    void setCurrentTab(int index);

private slots:
    void moManHinhCauHinh();

private:
    void thieLapGiaoDien();
    void thieLapDanhSachChucNang();
    
    // UI Elements
    QListWidget *danhSachChucNang;
    QStackedWidget *vungHienThi;
    QLabel *nhanTieuDe;
    
    // Dashboard Elements (New)
    QScrollArea *scrollArea;
    QGroupBox *groupReader;
    QGroupBox *groupBook;
    QGroupBox *groupSys;
    
    // Report Buttons (for context switching)
    QPushButton *btnReportUser;
    QPushButton *btnReportBook;
    
    // Search widgets
    class SearchableListWidget* readerSearchList;
    class SearchableListWidget* bookSearchList;
    
    // Data Context
    PTRDG docGiaHienTai;
    std::vector<PTRDG> m_danhSach;
    PTRDS sachHienTai = nullptr;
    
    QWidget* taoTrangTheDocGia();
    QWidget* taoTrangTemSach();
    QWidget* taoTrangBaoCao();
    
    void xuLyInTheDocGia();
    void xuLyInDanhSach();
    void xuLyInTheHangLoat(); // New
    
    void xuLyInTemSach();
    void xuLyInDanhMucSach();
    void xuLyInTemHangLoat(); // New
    
    // New Handlers
    void xuLyInBaoCaoDocGia();
    void xuLyInBaoCaoSach();
    
    // Toggle states for Select All buttons
    bool m_readerSelectAllToggle = true;
    bool m_bookSelectAllToggle = true;
};

#endif // MANHINHQUANLYIN_H
