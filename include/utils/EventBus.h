/**
 * @file EventBus.h
 * @path include/utils/EventBus.h
 * @description Header của hệ thống Bus truyền tin (Singleton).
 * @project Library Management System (QuanLyThuVien)
 */
#ifndef EVENTBUS_H

#define EVENTBUS_H

#include <QObject>
#include <QString>
#include <QVariant>

// EventBus: singleton QObject để phát tín hiệu khi dữ liệu thay đổi
class EventBus : public QObject {
    Q_OBJECT
public:
    static EventBus& instance();
    
    void emitDataChanged() { emit dataChanged(); }

signals:
    // Độc giả
    void docGiaAdded(int maThe);
    void docGiaUpdated(int maThe);
    void docGiaRemoved(int maThe);

    // Đầu sách
    void dauSachAdded(const QString& isbn);
    void dauSachUpdated(const QString& isbn);
    void dauSachRemoved(const QString& isbn);

    // Người dùng (Nhân viên/Admin)
    void nguoiDungAdded(const QString& username);
    void nguoiDungUpdated(const QString& username);
    void nguoiDungRemoved(const QString& username);

    // Mượn - trả
    void muonTraAdded(int maThe, const QString& maSach);
    void muonTraUpdated(int maThe, const QString& maSach);
    void muonTraRemoved(int maThe, const QString& maSach);

    // System Log
    void newSystemLog(const QString& action, const QString& details);

    // General: any data file or DB changed externally
    void dataChanged();

    // Settings
    void settingsUpdated();

private:
    explicit EventBus(QObject* parent = nullptr);
    ~EventBus() {}
    Q_DISABLE_COPY(EventBus)
};

#endif // EVENTBUS_H
