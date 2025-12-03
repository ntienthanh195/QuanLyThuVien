cmake_minimum_required(VERSION 3.16)

project(LibraryManagement VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Tìm Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Qt Designer UI files directory
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/models
    ${CMAKE_CURRENT_SOURCE_DIR}/include/views
    ${CMAKE_CURRENT_SOURCE_DIR}/include/views/dialogs
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
)

# Source files
set(MODEL_SOURCES
    src/models/DocGia.cpp
    src/models/MuonTra.cpp
    src/models/QuanLySach.cpp
    src/models/KiemTraDuLieu.cpp
    src/models/ThaoTacFile.cpp
    src/models/NhapLieu.cpp
)

set(VIEW_SOURCES
    src/views/MainWindow.cpp
    src/views/BookManagementWidget.cpp
    src/views/ReaderManagementWidget.cpp
    src/views/LoanManagementWidget.cpp
    src/views/AboutDialog.cpp
    src/views/dialogs/AddBookDialog.cpp
    src/views/dialogs/BookDetailsDialog.cpp
    src/views/dialogs/AddCopyDialog.cpp
    src/views/dialogs/EditBookDialog.cpp
    src/views/dialogs/AddReaderDialog.cpp
    src/views/dialogs/EditReaderDialog.cpp
    src/views/dialogs/Top10BooksDialog.cpp
    src/views/dialogs/OverdueReportDialog.cpp
)

set(UTIL_SOURCES
    src/utils/NgayThang.cpp
    src/utils/ThongBao.cpp
    src/utils/XuLyChuoi.cpp
    src/utils/FileManager.cpp
)

set(HEADER_FILES
    include/Constants.h
    include/models/DauSach.h
    include/models/DocGia.h
    include/models/QuanLySach.h
    include/models/KiemTraDuLieu.h
    include/models/ThaoTacFile.h
    include/models/NhapLieu.h
    include/views/MainWindow.h
    include/views/BookManagementWidget.h
    include/views/ReaderManagementWidget.h
    include/views/LoanManagementWidget.h
    include/views/AboutDialog.h
    include/views/dialogs/AddBookDialog.h
    include/views/dialogs/BookDetailsDialog.h
    include/views/dialogs/AddCopyDialog.h
    include/views/dialogs/EditBookDialog.h
    include/views/dialogs/AddReaderDialog.h
    include/views/dialogs/EditReaderDialog.h
    include/views/dialogs/Top10BooksDialog.h
    include/views/dialogs/OverdueReportDialog.h
    include/utils/NgayThang.h
    include/utils/ThongBao.h
    include/utils/XuLyChuoi.h
    include/utils/FileManager.h
)

set(UI_FILES
    # UI files tạm thời comment out
    # ui/MainWindow.ui
    # ui/BookManagementWidget.ui
    # ui/ReaderManagementWidget.ui
    # ui/LoanManagementWidget.ui
    # ui/AboutDialog.ui
)

# Resource files
set(RESOURCE_FILES
    resources/resources.qrc
)

# Create executable
qt_add_executable(LibraryManagement
    src/main.cpp
    ${MODEL_SOURCES}
    ${VIEW_SOURCES}
    ${UTIL_SOURCES}
    ${HEADER_FILES}
    ${UI_FILES}
    ${RESOURCE_FILES}
)

# Link Qt libraries
target_link_libraries(LibraryManagement PRIVATE 
    Qt6::Core
    Qt6::Widgets
)

# Windows specific settings
if(WIN32)
    set_target_properties(LibraryManagement PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Install
install(TARGETS LibraryManagement
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
