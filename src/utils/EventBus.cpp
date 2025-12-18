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
