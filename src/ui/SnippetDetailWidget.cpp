#include "SnippetDetailWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGuiApplication>
#include <QClipboard>
#include <QMessageBox>
#include "../platform/PlatformUtils.h"

namespace CodeShelf {

SnippetDetailWidget::SnippetDetailWidget(SnippetService& snippetService,
                                         CollectionRepository& colRepo,
                                         TagRepository& tagRepo,
                                         RevisionRepository& revRepo,
                                         QWidget* parent)
    : QWidget(parent)
    , m_snippetService(snippetService)
    , m_colRepo(colRepo)
    , m_tagRepo(tagRepo)
    , m_revRepo(revRepo)
{
    setupUi();
    clear();
}

void SnippetDetailWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // Top action toolbar
    auto toolbarLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton(QStringLiteral("Save (Ctrl+S)"), this);
    m_discardBtn = new QPushButton(QStringLiteral("Discard"), this);
    m_copyBtn = new QPushButton(QStringLiteral("Copy Code"), this);
    m_duplicateBtn = new QPushButton(QStringLiteral("Duplicate"), this);
    m_deleteBtn = new QPushButton(QStringLiteral("Move to Trash"), this);
    m_restoreBtn = new QPushButton(QStringLiteral("Restore"), this);
    m_revisionsBtn = new QPushButton(QStringLiteral("History / Diffs"), this);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet(QStringLiteral("color: #666; font-size: 11px;"));

    toolbarLayout->addWidget(m_saveBtn);
    toolbarLayout->addWidget(m_discardBtn);
    toolbarLayout->addWidget(m_copyBtn);
    toolbarLayout->addWidget(m_duplicateBtn);
    toolbarLayout->addWidget(m_deleteBtn);
    toolbarLayout->addWidget(m_restoreBtn);
    toolbarLayout->addWidget(m_revisionsBtn);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_statusLabel);

    mainLayout->addLayout(toolbarLayout);

    // Metadata header form
    auto formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);

    // Title line
    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText(QStringLiteral("Snippet title (required)..."));
    formLayout->addRow(QStringLiteral("Title:"), m_titleEdit);

    // Options row: Language, Type, Collection
    auto metaRow = new QHBoxLayout();
    m_languageCombo = new QComboBox(this);
    for (const QString& lang : allSupportedLanguages()) {
        m_languageCombo->addItem(lang);
    }

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(QStringLiteral("Code"), static_cast<int>(SnippetType::Code));
    m_typeCombo->addItem(QStringLiteral("Technical Note"), static_cast<int>(SnippetType::TechnicalNote));
    m_typeCombo->addItem(QStringLiteral("Command Reference"), static_cast<int>(SnippetType::CommandReference));
    m_typeCombo->addItem(QStringLiteral("Configuration Example"), static_cast<int>(SnippetType::ConfigurationExample));
    m_typeCombo->addItem(QStringLiteral("SQL"), static_cast<int>(SnippetType::SqlQuery));
    m_typeCombo->addItem(QStringLiteral("Pseudocode"), static_cast<int>(SnippetType::Pseudocode));

    m_collectionCombo = new QComboBox(this);
    populateCollections();

    m_favoriteCheck = new QCheckBox(QStringLiteral("Favorite"), this);
    m_pinnedCheck = new QCheckBox(QStringLiteral("Pinned"), this);

    metaRow->addWidget(new QLabel(QStringLiteral("Language:")));
    metaRow->addWidget(m_languageCombo);
    metaRow->addWidget(new QLabel(QStringLiteral("Type:")));
    metaRow->addWidget(m_typeCombo);
    metaRow->addWidget(new QLabel(QStringLiteral("Collection:")));
    metaRow->addWidget(m_collectionCombo);
    metaRow->addWidget(m_favoriteCheck);
    metaRow->addWidget(m_pinnedCheck);
    metaRow->addStretch();

    formLayout->addRow(QStringLiteral("Attributes:"), metaRow);

    // Tags & URL
    auto tagUrlRow = new QHBoxLayout();
    m_tagsEdit = new QLineEdit(this);
    m_tagsEdit->setPlaceholderText(QStringLiteral("comma, separated, tags"));

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText(QStringLiteral("https://... (optional)"));
    m_openUrlBtn = new QPushButton(QStringLiteral("Open"), this);

    tagUrlRow->addWidget(m_tagsEdit, 2);
    tagUrlRow->addWidget(new QLabel(QStringLiteral("Source URL:")));
    tagUrlRow->addWidget(m_urlEdit, 2);
    tagUrlRow->addWidget(m_openUrlBtn);

    formLayout->addRow(QStringLiteral("Tags & Source:"), tagUrlRow);

    // Description
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setFixedHeight(48);
    m_descriptionEdit->setPlaceholderText(QStringLiteral("Optional notes, purpose, or usage instructions..."));
    formLayout->addRow(QStringLiteral("Description:"), m_descriptionEdit);

    mainLayout->addLayout(formLayout);

    // Code Editor
    m_codeEditor = new CodeEditor(this);
    mainLayout->addWidget(m_codeEditor, 1);

    // Connect signals
    connect(m_saveBtn, &QPushButton::clicked, this, &SnippetDetailWidget::saveCurrentSnippet);
    connect(m_discardBtn, &QPushButton::clicked, this, &SnippetDetailWidget::discardChanges);
    connect(m_copyBtn, &QPushButton::clicked, this, &SnippetDetailWidget::copySnippetCode);
    connect(m_duplicateBtn, &QPushButton::clicked, this, &SnippetDetailWidget::duplicateSnippet);
    connect(m_deleteBtn, &QPushButton::clicked, this, &SnippetDetailWidget::moveToTrash);
    connect(m_restoreBtn, &QPushButton::clicked, this, &SnippetDetailWidget::restoreFromTrash);
    connect(m_openUrlBtn, &QPushButton::clicked, this, &SnippetDetailWidget::openSourceUrl);
    connect(m_revisionsBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentSnippet.has_value()) {
            emit showRevisionsRequested(m_currentSnippet->id());
        }
    });

    connect(m_titleEdit, &QLineEdit::textChanged, this, &SnippetDetailWidget::onContentModified);
    connect(m_descriptionEdit, &QTextEdit::textChanged, this, &SnippetDetailWidget::onContentModified);
    connect(m_codeEditor, &CodeEditor::textChanged, this, &SnippetDetailWidget::onContentModified);
    connect(m_tagsEdit, &QLineEdit::textChanged, this, &SnippetDetailWidget::onContentModified);
    connect(m_urlEdit, &QLineEdit::textChanged, this, &SnippetDetailWidget::onContentModified);
    connect(m_languageCombo, &QComboBox::currentIndexChanged, this, &SnippetDetailWidget::onContentModified);
    connect(m_typeCombo, &QComboBox::currentIndexChanged, this, &SnippetDetailWidget::onContentModified);
    connect(m_collectionCombo, &QComboBox::currentIndexChanged, this, &SnippetDetailWidget::onContentModified);
    connect(m_favoriteCheck, &QCheckBox::toggled, this, &SnippetDetailWidget::onContentModified);
    connect(m_pinnedCheck, &QCheckBox::toggled, this, &SnippetDetailWidget::onContentModified);
}

void SnippetDetailWidget::populateCollections() {
    m_collectionCombo->clear();
    m_collectionCombo->addItem(QStringLiteral("(Unfiled)"), QString());
    const auto collections = m_colRepo.getAll();
    for (const auto& col : collections) {
        m_collectionCombo->addItem(col.name(), col.id());
    }
}

void SnippetDetailWidget::loadSnippet(const std::optional<Snippet>& snippet) {
    populateCollections();
    m_currentSnippet = snippet;

    blockSignals(true);
    m_titleEdit->blockSignals(true);
    m_descriptionEdit->blockSignals(true);
    m_codeEditor->blockSignals(true);
    m_tagsEdit->blockSignals(true);
    m_urlEdit->blockSignals(true);
    m_languageCombo->blockSignals(true);
    m_typeCombo->blockSignals(true);
    m_collectionCombo->blockSignals(true);
    m_favoriteCheck->blockSignals(true);
    m_pinnedCheck->blockSignals(true);

    if (snippet.has_value()) {
        const Snippet& s = *snippet;
        m_titleEdit->setText(s.title());
        m_descriptionEdit->setPlainText(s.description());
        m_codeEditor->setPlainText(s.body());
        m_tagsEdit->setText(s.tags().join(QStringLiteral(", ")));
        m_urlEdit->setText(s.sourceUrl());

        int langIdx = m_languageCombo->findText(s.language());
        if (langIdx >= 0) m_languageCombo->setCurrentIndex(langIdx);

        int typeIdx = m_typeCombo->findData(static_cast<int>(s.snippetType()));
        if (typeIdx >= 0) m_typeCombo->setCurrentIndex(typeIdx);

        int colIdx = m_collectionCombo->findData(s.collectionId());
        if (colIdx >= 0) m_collectionCombo->setCurrentIndex(colIdx);
        else m_collectionCombo->setCurrentIndex(0);

        m_favoriteCheck->setChecked(s.isFavorite());
        m_pinnedCheck->setChecked(s.isPinned());

        bool isDeleted = s.isDeleted();
        m_deleteBtn->setVisible(!isDeleted);
        m_restoreBtn->setVisible(isDeleted);
        m_revisionsBtn->setEnabled(true);
        m_copyBtn->setEnabled(true);
        m_duplicateBtn->setEnabled(!isDeleted);
        m_saveBtn->setEnabled(true);
        m_discardBtn->setEnabled(false);

        m_statusLabel->setText(QStringLiteral("Updated %1").arg(s.updatedAt().toLocalTime().toString(QStringLiteral("yyyy-MM-dd hh:mm"))));
    } else {
        clear();
    }

    m_titleEdit->blockSignals(false);
    m_descriptionEdit->blockSignals(false);
    m_codeEditor->blockSignals(false);
    m_tagsEdit->blockSignals(false);
    m_urlEdit->blockSignals(false);
    m_languageCombo->blockSignals(false);
    m_typeCombo->blockSignals(false);
    m_collectionCombo->blockSignals(false);
    m_favoriteCheck->blockSignals(false);
    m_pinnedCheck->blockSignals(false);
    blockSignals(false);

    m_isModified = false;
}

void SnippetDetailWidget::clear() {
    m_currentSnippet = std::nullopt;
    m_titleEdit->clear();
    m_descriptionEdit->clear();
    m_codeEditor->clear();
    m_tagsEdit->clear();
    m_urlEdit->clear();
    m_languageCombo->setCurrentIndex(0);
    m_typeCombo->setCurrentIndex(0);
    m_collectionCombo->setCurrentIndex(0);
    m_favoriteCheck->setChecked(false);
    m_pinnedCheck->setChecked(false);

    m_saveBtn->setEnabled(false);
    m_discardBtn->setEnabled(false);
    m_copyBtn->setEnabled(false);
    m_duplicateBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
    m_restoreBtn->setVisible(false);
    m_revisionsBtn->setEnabled(false);
    m_statusLabel->clear();

    m_isModified = false;
}

void SnippetDetailWidget::onContentModified() {
    if (!m_currentSnippet.has_value()) {
        if (!m_titleEdit->text().isEmpty() || !m_codeEditor->toPlainText().isEmpty()) {
            m_isModified = true;
            m_saveBtn->setEnabled(true);
            m_discardBtn->setEnabled(true);
            m_statusLabel->setText(QStringLiteral("New snippet (unsaved)"));
        }
        return;
    }

    m_isModified = true;
    m_saveBtn->setEnabled(true);
    m_discardBtn->setEnabled(true);
    m_statusLabel->setText(QStringLiteral("Modified (unsaved changes)"));
}

bool SnippetDetailWidget::promptSaveIfModified() {
    if (!m_isModified) return true;

    if (QGuiApplication::platformName() == QStringLiteral("offscreen")) {
        discardChanges();
        return true;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(QStringLiteral("Unsaved Changes"));
    msgBox.setText(QStringLiteral("You have unsaved changes in this snippet."));
    msgBox.setInformativeText(QStringLiteral("Do you want to save your changes before proceeding?"));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);

    int ret = msgBox.exec();
    if (ret == QMessageBox::Save) {
        saveCurrentSnippet();
        return !m_isModified; // Proceed if saved successfully
    } else if (ret == QMessageBox::Discard) {
        discardChanges();
        return true;
    }
    return false; // Cancel
}

void SnippetDetailWidget::saveCurrentSnippet() {
    Snippet snip;
    if (m_currentSnippet.has_value()) {
        snip = *m_currentSnippet;
    }

    snip.setTitle(m_titleEdit->text());
    snip.setDescription(m_descriptionEdit->toPlainText());
    snip.setBody(m_codeEditor->toPlainText());
    snip.setLanguage(m_languageCombo->currentText());
    snip.setSnippetType(static_cast<SnippetType>(m_typeCombo->currentData().toInt()));
    snip.setCollectionId(m_collectionCombo->currentData().toString());
    snip.setFavorite(m_favoriteCheck->isChecked());
    snip.setPinned(m_pinnedCheck->isChecked());
    snip.setSourceUrl(m_urlEdit->text());

    QStringList tagList;
    const QString rawTags = m_tagsEdit->text();
    for (const QString& t : rawTags.split(QLatin1Char(','))) {
        if (!t.trimmed().isEmpty()) {
            tagList.append(t.trimmed());
        }
    }
    snip.setTags(tagList);

    auto val = snip.validate();
    if (!val.valid) {
        QMessageBox::warning(this, QStringLiteral("Validation Error"), val.errorMessage);
        return;
    }

    QString err;
    if (!m_snippetService.saveSnippet(snip, true, err)) {
        QMessageBox::warning(this, QStringLiteral("Save Failed"), err);
        return;
    }

    m_currentSnippet = snip;
    m_isModified = false;
    m_saveBtn->setEnabled(true);
    m_discardBtn->setEnabled(false);
    m_statusLabel->setText(QStringLiteral("Saved"));

    emit snippetSaved(snip.id());
}

void SnippetDetailWidget::discardChanges() {
    if (m_currentSnippet.has_value()) {
        loadSnippet(m_currentSnippet);
    } else {
        clear();
    }
}

void SnippetDetailWidget::copySnippetCode() {
    QString code = m_codeEditor->toPlainText();
    if (m_codeEditor->textCursor().hasSelection()) {
        code = m_codeEditor->textCursor().selectedText();
        // Convert Unicode paragraph separators (0x2029) back to newlines if Qt replaced them
        code.replace(QChar(0x2029), QLatin1Char('\n'));
    }

    QGuiApplication::clipboard()->setText(code);
    m_statusLabel->setText(QStringLiteral("Code copied to clipboard"));
}

void SnippetDetailWidget::duplicateSnippet() {
    if (!m_currentSnippet.has_value()) return;

    Snippet dup;
    QString err;
    if (m_snippetService.duplicateSnippet(m_currentSnippet->id(), dup, err)) {
        emit snippetDuplicated(dup.id());
    } else {
        QMessageBox::warning(this, QStringLiteral("Duplicate Error"), err);
    }
}

void SnippetDetailWidget::moveToTrash() {
    if (!m_currentSnippet.has_value()) return;

    if (QMessageBox::question(this, QStringLiteral("Move to Trash"),
                              QStringLiteral("Move snippet '%1' to Trash?").arg(m_currentSnippet->title()),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QString err;
        if (m_snippetService.moveToTrash(m_currentSnippet->id(), err)) {
            emit snippetDeleted(m_currentSnippet->id());
            clear();
        } else {
            QMessageBox::warning(this, QStringLiteral("Error"), err);
        }
    }
}

void SnippetDetailWidget::restoreFromTrash() {
    if (!m_currentSnippet.has_value()) return;

    QString err;
    if (m_snippetService.restoreFromTrash(m_currentSnippet->id(), err)) {
        m_currentSnippet->setDeletedAt(QDateTime());
        loadSnippet(m_currentSnippet);
        emit snippetSaved(m_currentSnippet->id());
    } else {
        QMessageBox::warning(this, QStringLiteral("Error"), err);
    }
}

void SnippetDetailWidget::openSourceUrl() {
    const QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) return;

    QString err;
    if (!PlatformUtils::openSafeUrl(url, err)) {
        QMessageBox::warning(this, QStringLiteral("Cannot Open URL"), err);
    }
}

void SnippetDetailWidget::insertTemplateText(const QString& templateBody) {
    m_codeEditor->insertPlainText(templateBody);
    onContentModified();
}

} // namespace CodeShelf
