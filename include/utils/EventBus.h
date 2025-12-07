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

signals:
    // Độc giả
    void docGiaAdded(int maThe);
    void docGiaUpdated(int maThe);
    void docGiaRemoved(int maThe);

    // Đầu sách
    void dauSachAdded(const QString& isbn);
    void dauSachUpdated(const QString& isbn);
    void dauSachRemoved(const QString& isbn);

    // Mượn - trả
    void muonTraAdded(int maThe, const QString& maSach);
    void muonTraUpdated(int maThe, const QString& maSach);
    void muonTraRemoved(int maThe, const QString& maSach);

    // General: any data file or DB changed externally
    void dataChanged();

private:
    explicit EventBus(QObject* parent = nullptr);
    ~EventBus() {}
    Q_DISABLE_COPY(EventBus)
};

#endif // EVENTBUS_H
