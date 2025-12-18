#include "models/DBLoader.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QFile>
#include <QDebug>
#include <map>
#include "utils/Perf.h"

bool NapDauSachTuDB(const QString &dbPath, PTRDS dsDauSach[], int &soLuongDauSach, std::ostream &out) {
    PerfTimer perf("NapDauSachTuDB");
    if (!QFile::exists(dbPath)) {
        std::cout << "DB not found: " << dbPath.toStdString() << std::endl;
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "loader_conn");
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        std::cout << "Failed to open DB: " << db.lastError().text().toStdString() << std::endl;
        QSqlDatabase::removeDatabase("loader_conn");
        return false;
    }

    QSqlQuery q(db);

    // Read dau_sach
    if (!q.exec("SELECT id, isbn, ten_sach, so_trang, tac_gia, nam_xuat_ban, the_loai FROM dau_sach")) {
        std::cout << "Query failed: " << q.lastError().text().toStdString() << std::endl;
        db.close();
        QSqlDatabase::removeDatabase("loader_conn");
        return false;
    }

    std::map<int,int> idToIndex;
    soLuongDauSach = 0;

    while (q.next()) {
        int id = q.value(0).toInt();
        QString isbn = q.value(1).toString();
        QString ten = q.value(2).toString();
        int soTrang = q.value(3).toInt();
        QString tacGia = q.value(4).toString();
        int nam = q.value(5).toInt();
        QString theLoai = q.value(6).toString();

        PTRDS d = new DauSach();
        d->ISBN = isbn.toStdString();
        d->tenSach = ten.toStdString();
        d->soTrang = soTrang;
        d->tacGia = tacGia.toStdString();
        d->namXuatBan = nam;
        d->theLoai = theLoai.toStdString();
        d->dms = nullptr;
        d->tongBanSao = 0;
        d->soLuotMuon = 0;

        if (soLuongDauSach < MAX_DAUSACH) {
            dsDauSach[soLuongDauSach] = d;
            idToIndex[id] = soLuongDauSach;
            soLuongDauSach++;
        } else {
            delete d; // overflow - skip
        }
    }

    // Read ban_sao and attach to corresponding dau_sach
    if (!q.exec("SELECT ma_sach, dau_sach_id, trang_thai, vi_tri FROM ban_sao")) {
        std::cout << "Query ban_sao failed: " << q.lastError().text().toStdString() << std::endl;
        db.close();
        QSqlDatabase::removeDatabase("loader_conn");
        return false;
    }

    while (q.next()) {
        QString maSach = q.value(0).toString();
        int dauId = q.value(1).toInt();
        int trangThai = q.value(2).toInt();
        QString viTri = q.value(3).toString();

        auto it = idToIndex.find(dauId);
        if (it == idToIndex.end()) {
            std::cout << "Warning: ban_sao references missing dau_sach id=" << dauId << std::endl;
            continue;
        }
        PTRDS parent = dsDauSach[it->second];
        if (!parent) continue;

        PTRDMS node = new DanhMucSach();
        node->maSach = maSach.toStdString();
        node->trangThai = static_cast<TrangThaiSach>(trangThai);
        node->viTri = viTri.toStdString();
        node->next = parent->dms;
        parent->dms = node;
        parent->tongBanSao++;
    }

    db.close();
    QSqlDatabase::removeDatabase("loader_conn");
    return true;
}
