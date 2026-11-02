#include "MainWindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QApplication>
#include <QStyle>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QKeySequence>
#include <QCloseEvent>
#include "DiffDialog.h"
#include "DuplicateDialog.h"
#include "ImportPreviewDialog.h"
#include "TemplatesDialog.h"
#include "MaintenanceDialog.h"
#include "BackupDialog.h"
#include "SettingsDialog.h"
#include "AboutDialog.h"
#include "../search/SearchParser.h"
#include "../import_export/MarkdownExportHandler.h"
#include "../settings/AppSettings.h"

namespace CodeShelf {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_db(Database::instance())
    , m_snippetRepo(m_db)
    , m_colRepo(m_db)
    , m_tagRepo(m_db)
    , m_revRepo(m_db)
    , m_tmplRepo(m_db)
    , m_svRepo(m_db)
    , m_snippetService(m_snippetRepo, m_colRepo, m_tagRepo, m_revRepo)
    , m_dupService(m_snippetRepo)
    , m_tmplService(m_tmplRepo)
    , m_backupService(m_db)
    , m_maintService(m_db, m_snippetRepo, m_colRepo, m_tagRepo)
    , m_searchEngine(m_db)
    , m_backupHandler(m_snippetRepo, m_colRepo, m_tagRepo, m_tmplRepo)
{
    setWindowTitle(QStringLiteral("CodeShelf"));
    setWindowIcon(QIcon(QStringLiteral(":/icons/codeshelf.png")));
    resize(1140, 740);

    setupUi();
    setupMenus();
    setupShortcuts();
    applyTheme();

    refreshLibraryViews();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    auto central = new QWidget(this);
    auto rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(4, 4, 4, 4);
    rootLayout->setSpacing(4);

    m_mainSplitter = new QSplitter(Qt::Horizontal, central);

    // 1. Left Sidebar
    m_sidebarWidget = new SidebarWidget(m_snippetRepo, m_colRepo, m_tagRepo, m_svRepo, m_mainSplitter);
    m_sidebarWidget->setMinimumWidth(200);
    m_sidebarWidget->setMaximumWidth(320);
    m_mainSplitter->addWidget(m_sidebarWidget);

    // 2. Center List Container
    auto centerContainer = new QWidget(m_mainSplitter);
    auto centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setContentsMargins(2, 2, 2, 2);
    centerLayout->setSpacing(4);

    // Search row
    m_searchEdit = new QLineEdit(centerContainer);
    m_searchEdit->setPlaceholderText(QStringLiteral("Search snippets... (Ctrl+F)"));
    m_searchEdit->setClearButtonEnabled(true);
    centerLayout->addWidget(m_searchEdit);

    m_snippetListView = new QListView(centerContainer);
    m_snippetListModel = new SnippetListModel(this);
    m_snippetListView->setModel(m_snippetListModel);
    m_snippetListView->setUniformItemSizes(true);
    centerLayout->addWidget(m_snippetListView, 1);

    m_mainSplitter->addWidget(centerContainer);

    // 3. Right Detail Editor
    m_detailWidget = new SnippetDetailWidget(m_snippetService, m_colRepo, m_tagRepo, m_revRepo, m_mainSplitter);
    m_mainSplitter->addWidget(m_detailWidget);

    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 2);
    m_mainSplitter->setStretchFactor(2, 4);

    rootLayout->addWidget(m_mainSplitter);
    setCentralWidget(central);

    // Status Bar
    m_statusItemCountLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_statusItemCountLabel);
    statusBar()->showMessage(QStringLiteral("Ready"));

    // Search debounce timer
    m_searchDebounceTimer = new QTimer(this);
    m_searchDebounceTimer->setSingleShot(true);
    m_searchDebounceTimer->setInterval(150);

    // Connections
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_searchDebounceTimer, &QTimer::timeout, this, &MainWindow::performSearch);

    connect(m_sidebarWidget, &SidebarWidget::filterChanged, this, &MainWindow::onSidebarFilterChanged);
    connect(m_snippetListView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onSnippetSelectionChanged);

    connect(m_detailWidget, &SnippetDetailWidget::snippetSaved, this, &MainWindow::onSnippetSaved);
    connect(m_detailWidget, &SnippetDetailWidget::snippetDeleted, this, &MainWindow::onSnippetDeleted);
    connect(m_detailWidget, &SnippetDetailWidget::snippetDuplicated, this, &MainWindow::onSnippetDuplicated);
    connect(m_detailWidget, &SnippetDetailWidget::showRevisionsRequested, this, &MainWindow::openRevisionsDialog);
}

void MainWindow::setupMenus() {
    // File Menu
    auto fileMenu = menuBar()->addMenu(QStringLiteral("&File"));

    auto newAction = fileMenu->addAction(QStringLiteral("&New Snippet"), this, &MainWindow::onNewSnippet);
    newAction->setShortcut(QKeySequence::New);

    auto saveAction = fileMenu->addAction(QStringLiteral("&Save Snippet"), m_detailWidget, &SnippetDetailWidget::saveCurrentSnippet);
    saveAction->setShortcut(QKeySequence::Save);

    auto dupAction = fileMenu->addAction(QStringLiteral("&Duplicate Snippet"), m_detailWidget, &SnippetDetailWidget::duplicateSnippet);
    dupAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+D")));

    fileMenu->addSeparator();

    fileMenu->addAction(QStringLiteral("&Import Backup (JSON)..."), this, &MainWindow::openImportDialog);
    fileMenu->addAction(QStringLiteral("Export Library (&JSON)..."), this, &MainWindow::openExportJsonDialog);
    fileMenu->addAction(QStringLiteral("Export Selected to &Markdown..."), this, &MainWindow::openExportMarkdownDialog);

    fileMenu->addSeparator();

    fileMenu->addAction(QStringLiteral("&Backup & Restore..."), this, &MainWindow::openBackupDialog);

    fileMenu->addSeparator();

    auto exitAction = fileMenu->addAction(QStringLiteral("E&xit"), this, &QWidget::close);
    exitAction->setShortcut(QKeySequence::Quit);

    // Edit Menu
    auto editMenu = menuBar()->addMenu(QStringLiteral("&Edit"));
    auto copyAction = editMenu->addAction(QStringLiteral("Copy Snippet &Code"), m_detailWidget, &SnippetDetailWidget::copySnippetCode);
    copyAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+C")));

    auto trashAction = editMenu->addAction(QStringLiteral("&Move to Trash"), m_detailWidget, &SnippetDetailWidget::moveToTrash);
    trashAction->setShortcut(QKeySequence::Delete);

    // View Menu
    auto viewMenu = menuBar()->addMenu(QStringLiteral("&View"));
    auto refreshAct = viewMenu->addAction(QStringLiteral("&Refresh Library"), this, &MainWindow::refreshLibraryViews);
    refreshAct->setShortcut(QKeySequence::Refresh);

    viewMenu->addSeparator();
    viewMenu->addAction(QStringLiteral("Preferences / &Settings..."), this, &MainWindow::openSettingsDialog);

    // Tools Menu
    auto toolsMenu = menuBar()->addMenu(QStringLiteral("&Tools"));
    toolsMenu->addAction(QStringLiteral("Duplicate &Detection..."), this, &MainWindow::openDuplicateDialog);
    toolsMenu->addAction(QStringLiteral("Code &Templates..."), this, &MainWindow::openTemplatesDialog);
    toolsMenu->addAction(QStringLiteral("Database &Maintenance..."), this, &MainWindow::openMaintenanceDialog);

    // Help Menu
    auto helpMenu = menuBar()->addMenu(QStringLiteral("&Help"));
    helpMenu->addAction(QStringLiteral("&Keyboard Shortcuts"), this, &MainWindow::openKeyboardShortcutsDialog);
    helpMenu->addAction(QStringLiteral("&About CodeShelf"), this, &MainWindow::openAboutDialog);
}

void MainWindow::setupShortcuts() {
    auto findShortcut = new QShortcut(QKeySequence::Find, this);
    connect(findShortcut, &QShortcut::activated, this, &MainWindow::focusSearch);
}

void MainWindow::applyTheme() {
    auto theme = AppSettings::instance().theme();

    if (theme == AppTheme::Dark) {
        // Charcoal neutral palette (no neon, no glow, no bright blue/magenta)
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(45, 48, 50));
        darkPalette.setColor(QPalette::WindowText, QColor(220, 220, 220));
        darkPalette.setColor(QPalette::Base, QColor(32, 34, 36));
        darkPalette.setColor(QPalette::AlternateBase, QColor(40, 42, 44));
        darkPalette.setColor(QPalette::ToolTipBase, QColor(220, 220, 220));
        darkPalette.setColor(QPalette::ToolTipText, QColor(32, 34, 36));
        darkPalette.setColor(QPalette::Text, QColor(220, 220, 220));
        darkPalette.setColor(QPalette::Button, QColor(50, 54, 57));
        darkPalette.setColor(QPalette::ButtonText, QColor(220, 220, 220));
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(100, 150, 210));
        darkPalette.setColor(QPalette::Highlight, QColor(70, 110, 160));
        darkPalette.setColor(QPalette::HighlightedText, Qt::white);
        qApp->setPalette(darkPalette);
    } else if (theme == AppTheme::Light) {
        // Clean neutral light palette
        QPalette lightPalette;
        lightPalette.setColor(QPalette::Window, QColor(245, 246, 248));
        lightPalette.setColor(QPalette::WindowText, QColor(30, 30, 30));
        lightPalette.setColor(QPalette::Base, Qt::white);
        lightPalette.setColor(QPalette::AlternateBase, QColor(240, 242, 245));
        lightPalette.setColor(QPalette::Text, QColor(30, 30, 30));
        lightPalette.setColor(QPalette::Button, QColor(235, 238, 242));
        lightPalette.setColor(QPalette::ButtonText, QColor(30, 30, 30));
        lightPalette.setColor(QPalette::Highlight, QColor(50, 100, 170));
        lightPalette.setColor(QPalette::HighlightedText, Qt::white);
        qApp->setPalette(lightPalette);
    } else {
        // System default
        if (QApplication::style()) {
            QApplication::setPalette(QApplication::style()->standardPalette());
        }
    }

    if (m_detailWidget) {
        // Apply editor font settings
        m_detailWidget->update();
    }
}

void MainWindow::onSidebarFilterChanged(SidebarItemType type, const QString& entityId) {
    if (!m_detailWidget->promptSaveIfModified()) {
        return;
    }

    m_currentFilterType = type;
    m_currentFilterEntityId = entityId;
    m_searchEdit->clear();
    refreshLibraryViews();
}

void MainWindow::onSearchTextChanged(const QString& /*text*/) {
    m_searchDebounceTimer->start();
}

void MainWindow::focusSearch() {
    m_searchEdit->setFocus();
    m_searchEdit->selectAll();
}

void MainWindow::performSearch() {
    const QString queryText = m_searchEdit->text().trimmed();

    if (queryText.isEmpty()) {
        refreshLibraryViews();
        return;
    }

    SearchQuery q = SearchParser::parse(queryText);
    if (m_currentFilterType == SidebarItemType::Trash) {
        q.inTrash = true;
    }

    auto results = m_searchEngine.search(q);
    QVector<Snippet> snips;
    snips.reserve(results.size());
    for (const auto& r : results) {
        snips.append(r.snippet);
    }

    m_snippetListModel->setSnippets(snips);
    m_statusItemCountLabel->setText(QStringLiteral("%1 results found").arg(snips.size()));

    if (!snips.isEmpty()) {
        m_snippetListView->setCurrentIndex(m_snippetListModel->index(0, 0));
    } else {
        m_detailWidget->clear();
    }
}

void MainWindow::refreshLibraryViews() {
    m_sidebarWidget->refresh();

    QVector<Snippet> snips;
    switch (m_currentFilterType) {
        case SidebarItemType::AllSnippets:
            snips = m_snippetRepo.getAllActive();
            break;
        case SidebarItemType::Favorites:
            snips = m_snippetRepo.getFavorites();
            break;
        case SidebarItemType::Pinned:
            snips = m_snippetRepo.getPinned();
            break;
        case SidebarItemType::Unfiled:
            snips = m_snippetRepo.getUnfiled();
            break;
        case SidebarItemType::CollectionItem:
            snips = m_snippetRepo.getByCollection(m_currentFilterEntityId);
            break;
        case SidebarItemType::TagItem: {
            SearchQuery q;
            q.tagFilter = m_currentFilterEntityId;
            auto res = m_searchEngine.search(q);
            for (const auto& r : res) snips.append(r.snippet);
            break;
        }
        case SidebarItemType::SmartViewItem: {
            auto sv = m_svRepo.getById(m_currentFilterEntityId);
            if (sv.has_value()) {
                SearchQuery q = SearchParser::parse(sv->queryString());
                auto res = m_searchEngine.search(q);
                for (const auto& r : res) snips.append(r.snippet);
            }
            break;
        }
        case SidebarItemType::Trash:
            snips = m_snippetRepo.getTrash();
            break;
        default:
            snips = m_snippetRepo.getAllActive();
            break;
    }

    m_snippetListModel->setSnippets(snips);
    m_statusItemCountLabel->setText(QStringLiteral("%1 snippet%2").arg(snips.size()).arg(snips.size() == 1 ? "" : "s"));

    if (!snips.isEmpty()) {
        m_snippetListView->setCurrentIndex(m_snippetListModel->index(0, 0));
    } else {
        m_detailWidget->clear();
    }
}

void MainWindow::onSnippetSelectionChanged(const QModelIndex& current, const QModelIndex& previous) {
    if (previous.isValid() && m_detailWidget->hasUnsavedChanges()) {
        if (!m_detailWidget->promptSaveIfModified()) {
            // Cancel navigation - restore previous index
            m_snippetListView->setCurrentIndex(previous);
            return;
        }
    }

    if (!current.isValid()) {
        m_detailWidget->clear();
        return;
    }

    auto snip = m_snippetListModel->getSnippetAt(current.row());
    m_detailWidget->loadSnippet(snip);
}

void MainWindow::onNewSnippet() {
    if (!m_detailWidget->promptSaveIfModified()) {
        return;
    }

    Snippet newSnip;
    newSnip.setTitle(QStringLiteral("Untitled Snippet"));
    if (m_currentFilterType == SidebarItemType::CollectionItem) {
        newSnip.setCollectionId(m_currentFilterEntityId);
    }

    m_detailWidget->loadSnippet(newSnip);
}

void MainWindow::onSnippetSaved(const QString& snippetId) {
    refreshLibraryViews();
    // Select saved snippet
    for (int i = 0; i < m_snippetListModel->snippets().size(); ++i) {
        if (m_snippetListModel->snippets()[i].id() == snippetId) {
            m_snippetListView->setCurrentIndex(m_snippetListModel->index(i, 0));
            break;
        }
    }
}

void MainWindow::onSnippetDeleted(const QString& /*snippetId*/) {
    refreshLibraryViews();
}

void MainWindow::onSnippetDuplicated(const QString& newSnippetId) {
    refreshLibraryViews();
    for (int i = 0; i < m_snippetListModel->snippets().size(); ++i) {
        if (m_snippetListModel->snippets()[i].id() == newSnippetId) {
            m_snippetListView->setCurrentIndex(m_snippetListModel->index(i, 0));
            break;
        }
    }
}

void MainWindow::openImportDialog() {
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Select CodeShelf Backup"), QString(),
                                                QStringLiteral("CodeShelf Backup (*.json);;All Files (*.*)"));
    if (path.isEmpty()) return;

    ImportPreviewSummary preview;
    QString err;
    if (!m_backupHandler.parseAndPreview(path, preview, err)) {
        QMessageBox::critical(this, QStringLiteral("Import Error"), err);
        return;
    }

    ImportPreviewDialog dlg(preview, m_backupHandler, this);
    connect(&dlg, &ImportPreviewDialog::importCompleted, this, &MainWindow::refreshLibraryViews);
    dlg.exec();
}

void MainWindow::openExportJsonDialog() {
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Export CodeShelf Library"),
                                                QStringLiteral("codeshelf_export.json"),
                                                QStringLiteral("JSON Files (*.json)"));
    if (path.isEmpty()) return;

    QString err;
    if (m_backupHandler.exportBackup(path, {}, true, true, err)) {
        QMessageBox::information(this, QStringLiteral("Export Complete"), QStringLiteral("Library exported successfully to:\n%1").arg(path));
    } else {
        QMessageBox::critical(this, QStringLiteral("Export Failed"), err);
    }
}

void MainWindow::openExportMarkdownDialog() {
    auto current = m_detailWidget->currentSnippet();
    if (!current.has_value()) {
        QMessageBox::information(this, QStringLiteral("Export"), QStringLiteral("Please select a snippet to export to Markdown."));
        return;
    }

    QString defaultName = current->title().trimmed().replace(QRegularExpression(QStringLiteral("[^a-zA-Z0-9_-]")), QStringLiteral("_")) + QStringLiteral(".md");
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Export Snippet to Markdown"), defaultName, QStringLiteral("Markdown Files (*.md)"));
    if (path.isEmpty()) return;

    QString md = MarkdownExportHandler::snippetToMarkdown(*current);
    QString err;
    if (MarkdownExportHandler::exportToFile(path, md, err)) {
        QMessageBox::information(this, QStringLiteral("Export Complete"), QStringLiteral("Snippet exported to Markdown:\n%1").arg(path));
    } else {
        QMessageBox::critical(this, QStringLiteral("Export Failed"), err);
    }
}

void MainWindow::openBackupDialog() {
    BackupDialog dlg(m_backupService, this);
    connect(&dlg, &BackupDialog::databaseRestored, this, &MainWindow::refreshLibraryViews);
    dlg.exec();
}

void MainWindow::openDuplicateDialog() {
    DuplicateDialog dlg(m_dupService, m_snippetService, this);
    connect(&dlg, &DuplicateDialog::libraryModified, this, &MainWindow::refreshLibraryViews);
    dlg.exec();
}

void MainWindow::openTemplatesDialog() {
    TemplatesDialog dlg(m_tmplService, true, this);
    connect(&dlg, &TemplatesDialog::templateInserted, m_detailWidget, &SnippetDetailWidget::insertTemplateText);
    dlg.exec();
}

void MainWindow::openMaintenanceDialog() {
    MaintenanceDialog dlg(m_maintService, this);
    connect(&dlg, &MaintenanceDialog::databaseCompacted, this, &MainWindow::refreshLibraryViews);
    dlg.exec();
}

void MainWindow::openSettingsDialog() {
    SettingsDialog dlg(this);
    connect(&dlg, &SettingsDialog::settingsChanged, this, [this]() {
        applyTheme();
        refreshLibraryViews();
    });
    dlg.exec();
}

void MainWindow::openAboutDialog() {
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::openKeyboardShortcutsDialog() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(QStringLiteral("Keyboard Shortcuts"));
    msgBox.setText(QStringLiteral(
        "<h3>Keyboard Shortcuts</h3>"
        "<table>"
        "<tr><td><b>Ctrl+N</b></td><td>New snippet</td></tr>"
        "<tr><td><b>Ctrl+S</b></td><td>Save active snippet</td></tr>"
        "<tr><td><b>Ctrl+F</b></td><td>Focus global search</td></tr>"
        "<tr><td><b>Ctrl+D</b></td><td>Duplicate active snippet</td></tr>"
        "<tr><td><b>Ctrl+Shift+C</b></td><td>Copy snippet code</td></tr>"
        "<tr><td><b>Delete</b></td><td>Move active snippet to Trash</td></tr>"
        "<tr><td><b>F5</b></td><td>Refresh library views</td></tr>"
        "<tr><td><b>Ctrl+Q</b></td><td>Exit CodeShelf</td></tr>"
        "</table>"
    ));
    msgBox.exec();
}

void MainWindow::openRevisionsDialog(const QString& snippetId) {
    DiffDialog dlg(snippetId, m_snippetRepo, m_revRepo, m_snippetService, this);
    connect(&dlg, &DiffDialog::revisionRestored, this, [this](const QString& snipId) {
        onSnippetSaved(snipId);
    });
    dlg.exec();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_detailWidget && !m_detailWidget->promptSaveIfModified()) {
        event->ignore();
        return;
    }
    event->accept();
}

} // namespace CodeShelf
