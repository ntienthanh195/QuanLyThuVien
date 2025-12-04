#ifndef AUTOSAVEMANAGER_H
#define AUTOSAVEMANAGER_H

#include <QObject>
#include <QTimer>
#include <functional>

/**
 * @brief Manager for automatic file saving
 * 
 * Monitors data changes and triggers auto-save operations.
 * Can work in two modes:
 * 1. Immediate save - saves right after change
 * 2. Debounced save - waits for a delay period before saving
 */
class AutoSaveManager : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Save mode enumeration
     */
    enum SaveMode {
        Immediate,      // Save immediately when data changes
        Debounced       // Wait for delay period before saving
    };
    
    /**
     * @brief Constructor
     * @param parent Parent QObject
     */
    explicit AutoSaveManager(QObject* parent = nullptr);
    
    /**
     * @brief Set the save callback function
     * @param callback Function to call when saving
     */
    void setSaveCallback(std::function<void()> callback);
    
    /**
     * @brief Set the save mode
     * @param mode Save mode (Immediate or Debounced)
     */
    void setSaveMode(SaveMode mode);
    
    /**
     * @brief Set the debounce delay (only for Debounced mode)
     * @param delayMs Delay in milliseconds (default: 500ms)
     */
    void setDebounceDelay(int delayMs);
    
    /**
     * @brief Trigger a save operation
     * 
     * Depending on the mode:
     * - Immediate: Saves right away
     * - Debounced: Starts/restarts the timer
     */
    void triggerSave();
    
    /**
     * @brief Force immediate save regardless of mode
     */
    void forceSave();
    
    /**
     * @brief Check if auto-save is enabled
     */
    bool isEnabled() const { return enabled; }
    
    /**
     * @brief Enable/disable auto-save
     */
    void setEnabled(bool enable);
    
signals:
    /**
     * @brief Emitted when save is about to happen
     */
    void savingStarted();
    
    /**
     * @brief Emitted when save is completed
     */
    void savingCompleted();
    
private slots:
    void onTimerTimeout();
    
private:
    std::function<void()> saveCallback;
    QTimer* debounceTimer;
    SaveMode currentMode;
    bool enabled;
    int debounceDelayMs;
};

#endif // AUTOSAVEMANAGER_H
