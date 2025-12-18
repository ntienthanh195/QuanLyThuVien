#ifndef HOPTHOAIIN_H
#define HOPTHOAIIN_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include "models/DocGia.h"
#include "models/DauSach.h"

/**
 * @brief HopThoaiIn - Dialog chọn chức năng in ấn
 * Thiết kế đơn giản với 3 nhóm chức năng chính
 */
class HopThoaiIn : public QDialog {
    Q_OBJECT

public:
    explicit HopThoaiIn(QWidget *parent = nullptr);
    ~HopThoaiIn();

    // Thiết lập context để in
    void setDocGia(PTRDG dg);
    void setDauSach(PTRDS ds);
    void setDanhSachDocGia(PTRDG root);
    void setDanhSachSach(PTRDS root);

private slots:
    // Slot xử lý in thẻ
    void xuLyInTheDocGia();
    void xuLyInNhieuThe();
    
    // Slot xử lý in tem
    void xuLyInTemSach();
    void xuLyInNhieuTem();
    
    // Slot xử lý in báo cáo
    void xuLyInDanhSachDocGia();
    void xuLyInDanhSachSach();
    void xuLyInPhieuMuon();
    void xuLyInPhieuTra();
    
    // Slot cấu hình
    void xuLyCauHinh();

private:
    void taoGiaoDien();
    void taoNhomInThe();
    void taoNhomInTem();
    void taoNhomInBaoCao();
    void taoNutDieuKhien();
    
    // Layouts
    QVBoxLayout* layoutChinh;
    
    // Groups
    QGroupBox* nhomInThe;
    QGroupBox* nhomInTem;
    QGroupBox* nhomInBaoCao;
    
    // Buttons - In thẻ
    QPushButton* nutInThe;
    QPushButton* nutInNhieuThe;
    
    // Buttons - In tem
    QPushButton* nutInTem;
    QPushButton* nutInNhieuTem;
    QSpinBox* spinSoLuongTem;
    
    // Buttons - In báo cáo
    QPushButton* nutInDSDocGia;
    QPushButton* nutInDSSach;
    QPushButton* nutInPhieuMuon;
    QPushButton* nutInPhieuTra;
    
    // Buttons - Control
    QPushButton* nutCauHinh;
    QPushButton* nutDong;
    
    // Data context
    PTRDG docGiaHienTai;
    PTRDS dauSachHienTai;
    PTRDG rootDocGia;
    PTRDS rootSach;
};

#endif // HOPTHOAIIN_H
