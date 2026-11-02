#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QLineEdit>
#include <QListView>
#include <QLabel>
#include <QTimer>
#include <memory>
#include "../database/Database.h"
#include "../repositories/SnippetRepository.h"
#include "../repositories/CollectionRepository.h"
#include "../repositories/TagRepository.h"
#include "../repositories/RevisionRepository.h"
#include "../repositories/TemplateRepository.h"
#include "../repositories/SmartViewRepository.h"
#include "../services/SnippetService.h"
#include "../services/DuplicateService.h"
#include "../services/TemplateService.h"
#include "../services/BackupService.h"
#include "../services/MaintenanceService.h"
#include "../search/SearchEngine.h"
#include "../import_export/JsonBackupHandler.h"
#include "../models/SnippetListModel.h"
#include "SidebarWidget.h"
#include "SnippetDetailWidget.h"

namespace CodeShelf {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // Snippet CRUD & Navigation
    void onNewSnippet();
    void onSnippetSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void onSnippetSaved(const QString& snippetId);
    void onSnippetDeleted(const QString& snippetId);
    void onSnippetDuplicated(const QString& newSnippetId);
    void onSidebarFilterChanged(SidebarItemType type, const QString& entityId);

    // Search
    void onSearchTextChanged(const QString& text);
    void performSearch();
    void focusSearch();

    // Dialogs & Actions
    void openImportDialog();
    void openExportJsonDialog();
    void openExportMarkdownDialog();
    void openBackupDialog();
    void openDuplicateDialog();
    void openTemplatesDialog();
    void openMaintenanceDialog();
    void openSettingsDialog();
    void openAboutDialog();
    void openKeyboardShortcutsDialog();

    // Revisions & Diff
    void openRevisionsDialog(const QString& snippetId);

    // Theme & Styling
    void applyTheme();

private:
    void setupUi();
    void setupMenus();
    void setupShortcuts();
    void refreshLibraryViews();

    // Database & Repositories
    Database& m_db;
    SnippetRepository m_snippetRepo;
    CollectionRepository m_colRepo;
    TagRepository m_tagRepo;
    RevisionRepository m_revRepo;
    TemplateRepository m_tmplRepo;
    SmartViewRepository m_svRepo;

    // Services
    SnippetService m_snippetService;
    DuplicateService m_dupService;
    TemplateService m_tmplService;
    BackupService m_backupService;
    MaintenanceService m_maintService;
    SearchEngine m_searchEngine;
    JsonBackupHandler m_backupHandler;

    // UI Components
    QSplitter* m_mainSplitter{nullptr};
    SidebarWidget* m_sidebarWidget{nullptr};
    QLineEdit* m_searchEdit{nullptr};
    QListView* m_snippetListView{nullptr};
    SnippetListModel* m_snippetListModel{nullptr};
    SnippetDetailWidget* m_detailWidget{nullptr};

    QTimer* m_searchDebounceTimer{nullptr};
    QLabel* m_statusItemCountLabel{nullptr};

    // Current filter state
    SidebarItemType m_currentFilterType{SidebarItemType::AllSnippets};
    QString m_currentFilterEntityId;
};

} // namespace CodeShelf
