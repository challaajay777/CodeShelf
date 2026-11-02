cmake_minimum_required(VERSION 3.24)

project(CodeShelf
    VERSION 0.1.0
    DESCRIPTION "A minimal, offline-first native desktop library for organizing and searching reusable code snippets."
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Compile options & warnings
if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Dependencies
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Sql Test)
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)

# Includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Core & UI Library sources
set(CODESHELF_SOURCES
    # Core entities & utilities
    src/core/Snippet.h
    src/core/Snippet.cpp
    src/core/Collection.h
    src/core/Collection.cpp
    src/core/Tag.h
    src/core/Tag.cpp
    src/core/Revision.h
    src/core/Revision.cpp
    src/core/Template.h
    src/core/Template.cpp
    src/core/SmartView.h
    src/core/SmartView.cpp
    src/core/Enums.h
    src/core/Validation.h
    src/core/Validation.cpp
    src/core/ContentHash.h
    src/core/ContentHash.cpp
    src/core/DiffGenerator.h
    src/core/DiffGenerator.cpp

    # Database & Migrations
    src/database/Database.h
    src/database/Database.cpp
    src/database/Migrations.h
    src/database/Migrations.cpp
    src/database/SqliteHelper.h
    src/database/SqliteHelper.cpp

    # Repositories
    src/repositories/SnippetRepository.h
    src/repositories/SnippetRepository.cpp
    src/repositories/CollectionRepository.h
    src/repositories/CollectionRepository.cpp
    src/repositories/TagRepository.h
    src/repositories/TagRepository.cpp
    src/repositories/RevisionRepository.h
    src/repositories/RevisionRepository.cpp
    src/repositories/TemplateRepository.h
    src/repositories/TemplateRepository.cpp
    src/repositories/SmartViewRepository.h
    src/repositories/SmartViewRepository.cpp

    # Search
    src/search/SearchQuery.h
    src/search/SearchQuery.cpp
    src/search/SearchParser.h
    src/search/SearchParser.cpp
    src/search/SearchEngine.h
    src/search/SearchEngine.cpp

    # Services
    src/services/SnippetService.h
    src/services/SnippetService.cpp
    src/services/DuplicateService.h
    src/services/DuplicateService.cpp
    src/services/TemplateService.h
    src/services/TemplateService.cpp
    src/services/BackupService.h
    src/services/BackupService.cpp
    src/services/MaintenanceService.h
    src/services/MaintenanceService.cpp

    # Import / Export
    src/import_export/JsonBackupHandler.h
    src/import_export/JsonBackupHandler.cpp
    src/import_export/MarkdownExportHandler.h
    src/import_export/MarkdownExportHandler.cpp

    # Platform & Settings
    src/platform/PlatformUtils.h
    src/platform/PlatformUtils.cpp
    src/settings/AppSettings.h
    src/settings/AppSettings.cpp

    # Models
    src/models/SnippetListModel.h
    src/models/SnippetListModel.cpp
    src/models/SidebarTreeModel.h
    src/models/SidebarTreeModel.cpp

    # Editor
    src/editor/CodeEditor.h
    src/editor/CodeEditor.cpp
    src/editor/LineNumberArea.h
    src/editor/LineNumberArea.cpp

    # UI Widgets & Dialogs
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/SidebarWidget.h
    src/ui/SidebarWidget.cpp
    src/ui/SnippetDetailWidget.h
    src/ui/SnippetDetailWidget.cpp
    src/ui/DuplicateDialog.h
    src/ui/DuplicateDialog.cpp
    src/ui/DiffDialog.h
    src/ui/DiffDialog.cpp
    src/ui/ImportPreviewDialog.h
    src/ui/ImportPreviewDialog.cpp
    src/ui/TemplatesDialog.h
    src/ui/TemplatesDialog.cpp
    src/ui/MaintenanceDialog.h
    src/ui/MaintenanceDialog.cpp
    src/ui/BackupDialog.h
    src/ui/BackupDialog.cpp
    src/ui/SettingsDialog.h
    src/ui/SettingsDialog.cpp
    src/ui/AboutDialog.h
    src/ui/AboutDialog.cpp

    # Resources
    resources/resources.qrc
)

# Static library for core logic & UI
add_library(CodeShelfCore STATIC ${CODESHELF_SOURCES})
target_include_directories(CodeShelfCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(CodeShelfCore PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    SQLite3::SQLite3
    Threads::Threads
)

# Main Application Executable
add_executable(codeshelf
    src/app/main.cpp
)
target_link_libraries(codeshelf PRIVATE CodeShelfCore)

# Tests
enable_testing()
add_subdirectory(tests)
