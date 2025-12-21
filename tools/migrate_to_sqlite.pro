QT += core sql
CONFIG += console c++17

TARGET = migrate_to_sqlite
TEMPLATE = app

INCLUDEPATH += ../include

SOURCES += \
    migrate_to_sqlite.cpp \
    ../src/utils/DatabaseManager.cpp
