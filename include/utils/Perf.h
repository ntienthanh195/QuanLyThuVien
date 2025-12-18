#pragma once

#include <QElapsedTimer>
#include <QDebug>
#include <QString>

// Simple RAII performance timer for quick instrumentation.
// Usage: {
//   PerfTimer t("label");
//   ... work ...
// } // logs elapsed ms when destructed
struct PerfTimer {
    explicit PerfTimer(const QString &label)
        : m_label(label) {
        m_timer.start();
    }

    ~PerfTimer() {
        qint64 ms = m_timer.elapsed();
        qDebug().noquote() << "[Perf]" << m_label << ':' << ms << "ms";
    }

private:
    QString m_label;
    QElapsedTimer m_timer;
};
