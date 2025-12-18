#include "utils/AutoSaveManager.h"
#include <QDebug>

AutoSaveManager::AutoSaveManager(QObject* parent)
    : QObject(parent)
    , saveCallback(nullptr)
    , debounceTimer(new QTimer(this))
    , currentMode(Immediate)
    , enabled(true)
    , debounceDelayMs(500)
{
    debounceTimer->setSingleShot(true);
    connect(debounceTimer, &QTimer::timeout, this, &AutoSaveManager::onTimerTimeout);
}

void AutoSaveManager::setSaveCallback(std::function<void()> callback) {
    saveCallback = callback;
}

void AutoSaveManager::setSaveMode(SaveMode mode) {
    currentMode = mode;
}

void AutoSaveManager::setDebounceDelay(int delayMs) {
    debounceDelayMs = delayMs;
}

void AutoSaveManager::triggerSave() {
    if (!enabled || !saveCallback) {
        return;
    }
    
    if (currentMode == Immediate) {
        // Save immediately
        emit savingStarted();
        saveCallback();
        emit savingCompleted();
    } else {
        // Debounced mode: restart timer
        debounceTimer->stop();
        debounceTimer->start(debounceDelayMs);
    }
}

void AutoSaveManager::forceSave() {
    if (!saveCallback) {
        return;
    }
    
    // Stop any pending debounced save
    debounceTimer->stop();
    
    // Save immediately
    emit savingStarted();
    saveCallback();
    emit savingCompleted();
}

void AutoSaveManager::setEnabled(bool enable) {
    enabled = enable;
    if (!enabled) {
        debounceTimer->stop();
    }
}

void AutoSaveManager::onTimerTimeout() {
    if (!saveCallback) {
        return;
    }
    
    emit savingStarted();
    saveCallback();
    emit savingCompleted();
}
