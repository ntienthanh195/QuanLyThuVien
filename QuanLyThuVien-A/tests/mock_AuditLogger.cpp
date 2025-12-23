#include "utils/AuditLogger.h"

// Mock AuditLogger to prevent linking errors
void AuditLogger::log(const QString& action, const QString& type, const QString& key, const QString& details) {
    // No-op
}

void AuditLogger::log(ActionType action, const QString& table, const QString& key, const QString& details) {
    // No-op
}
