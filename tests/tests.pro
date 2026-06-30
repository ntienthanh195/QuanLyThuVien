QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

# Add C++17 support
CONFIG += c++17

# Include paths
INCLUDEPATH += ../include \
               ../include/models \
               ../include/utils \
               ../include/ui

# Test sources
SOURCES += test_importmanager.cpp \
           importmanager_internal.cpp

# Link minimal sources needed for parse functions
SOURCES += ../src/utils/XuLyChuoi.cpp

# Headers
HEADERS += importmanager_internal.h \
           ../include/utils/XuLyChuoi.h

# Qt SQL module (needed by ImportManager)
QT += sql core

# Output
TARGET = test_importmanager
DESTDIR = ../bin/tests

# Defines
DEFINES += QT_DEPRECATED_WARNINGS
