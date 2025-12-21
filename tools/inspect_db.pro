QT += core sql
CONFIG += console c++17
TARGET = inspect_db
TEMPLATE = app

SOURCES += \
    inspect_db.cpp \
    ../src/utils/DatabaseManager.cpp

HEADERS += \
    ../include/utils/DatabaseManager.h
