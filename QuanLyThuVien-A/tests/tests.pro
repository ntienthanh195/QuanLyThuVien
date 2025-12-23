QT += core testlib sql

# Define output directory
DESTDIR = ../build/tests

# Include project headers
INCLUDEPATH += ../include \
               ../include/utils \
               ../include/models
INCLUDEPATH += ../src

# Sources to test (Core Logic)
SOURCES += ../src/utils/NgayThang.cpp \
           ../src/utils/XuLyChuoi.cpp \
           ../src/utils/ThongBao.cpp \
           ../src/models/DocGia.cpp \
           ../src/models/MuonTra.cpp \
           mock_SettingsManager.cpp \
           mock_DatabaseManager.cpp \
           mock_AuditLogger.cpp \
           mock_EventBus.cpp \
           mock_FileSystem.cpp \
           mock_NhapLieu.cpp \
           mock_DauSach.cpp


# Test Sources
SOURCES += main.cpp

# Headers
HEADERS += tst_NgayThang.h tst_DocGia.h tst_MuonTra.h

# Config
CONFIG += console
CONFIG -= app_bundle
