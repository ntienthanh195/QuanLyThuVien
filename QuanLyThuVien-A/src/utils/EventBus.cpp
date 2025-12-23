/**
 * @file EventBus.cpp
 * @path src/utils/EventBus.cpp
 * @description Hệ thống Bus truyền tin trung tâm (Observer Pattern).
 * Giúp các Widget khác nhau giao tiếp và cập nhật dữ liệu thời gian thực mà không phụ thuộc trực tiếp vào nhau.
 * @project Library Management System (QuanLyThuVien)
 */
#include "utils/EventBus.h"

#include <QMutex>

EventBus::EventBus(QObject* parent) : QObject(parent) {
}

EventBus& EventBus::instance() {
    static QMutex mutex;
    static EventBus* inst = nullptr;
    if (!inst) {
        QMutexLocker locker(&mutex);
        if (!inst) inst = new EventBus(nullptr);
    }
    return *inst;
}
