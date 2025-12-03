#ifndef MANHINHBACKUP_H
#define MANHINHBACKUP_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "utils/BackupManager.h"

class ManHinhBackup : public QDialog {
    Q_OBJECT

public:
    explicit ManHinhBackup(QWidget *parent = nullptr);

private slots:
    void onTaoBackupClicked();
    void onPhucHoiClicked();
    void onXoaClicked();
    void onXoaCuClicked();
    void onLamMoiClicked();
    void onBackupSelected();

private:
    void thietLapGiaoDien();
    void apDungPhongCach();
    void capNhatDanhSachBackup();
    
    QListWidget* danhSachBackup;
    QPushButton* nutTaoBackup;
    QPushButton* nutPhucHoi;
    QPushButton* nutXoa;
    QPushButton* nutXoaCu;
    QPushButton* nutLamMoi;
    QPushButton* nutDong;
    QLabel* nhanThongTin;
    
    std::vector<ThongTinBackup> cacBackup;
};

#endif // MANHINHBACKUP_H
