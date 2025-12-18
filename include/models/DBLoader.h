#ifndef DBLOADER_H
#define DBLOADER_H

#include <QString>
#include <ostream>
#include "DauSach.h"

// Load book catalog from SQLite database into in-memory structures
bool NapDauSachTuDB(const QString &dbPath, PTRDS dsDauSach[], int &soLuongDauSach, std::ostream &out);

#endif // DBLOADER_H
