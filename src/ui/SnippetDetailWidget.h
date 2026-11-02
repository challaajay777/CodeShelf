#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <optional>
#include "../core/Snippet.h"
#include "../core/Collection.h"
#include "../editor/CodeEditor.h"
#include "../services/SnippetService.h"
#include "../repositories/CollectionRepository.h"
#include "../repositories/TagRepository.h"
#include "../repositories/RevisionRepository.h"

namespace CodeShelf {

class SnippetDetailWidget : public QWidget {
    Q_OBJECT

public:
    explicit SnippetDetailWidget(SnippetService& snippetService,
                                 CollectionRepository& colRepo,
                                 TagRepository& tagRepo,
                                 RevisionRepository& revRepo,
                                 QWidget* parent = nullptr);

    void loadSnippet(const std::optional<Snippet>& snippet);
    std::optional<Snippet> currentSnippet() const { return m_currentSnippet; }

    bool hasUnsavedChanges() const { return m_isModified; }
    bool promptSaveIfModified(); // Returns true if caller can safely proceed

    void clear();

signals:
    void snippetSaved(const QString& snippetId);
    void snippetDeleted(const QString& snippetId);
    void snippetDuplicated(const QString& newSnippetId);
    void showDiffRequested(const QString& oldText, const QString& newText);
    void showRevisionsRequested(const QString& snippetId);

public slots:
    void saveCurrentSnippet();
    void discardChanges();
    void copySnippetCode();
    void duplicateSnippet();
    void moveToTrash();
    void restoreFromTrash();
    void openSourceUrl();
    void insertTemplateText(const QString& templateBody);

private slots:
    void onContentModified();

private:
    void setupUi();
    void populateCollections();

    SnippetService& m_snippetService;
    CollectionRepository& m_colRepo;
    TagRepository& m_tagRepo;
    RevisionRepository& m_revRepo;

    std::optional<Snippet> m_currentSnippet;
    bool m_isModified{false};

    // UI Widgets
    QLineEdit* m_titleEdit{nullptr};
    QComboBox* m_languageCombo{nullptr};
    QComboBox* m_typeCombo{nullptr};
    QComboBox* m_collectionCombo{nullptr};
    QLineEdit* m_tagsEdit{nullptr};
    QLineEdit* m_urlEdit{nullptr};
    QPushButton* m_openUrlBtn{nullptr};
    QTextEdit* m_descriptionEdit{nullptr};
    CodeEditor* m_codeEditor{nullptr};

    QCheckBox* m_favoriteCheck{nullptr};
    QCheckBox* m_pinnedCheck{nullptr};

    QPushButton* m_saveBtn{nullptr};
    QPushButton* m_discardBtn{nullptr};
    QPushButton* m_copyBtn{nullptr};
    QPushButton* m_duplicateBtn{nullptr};
    QPushButton* m_deleteBtn{nullptr};
    QPushButton* m_restoreBtn{nullptr};
    QPushButton* m_revisionsBtn{nullptr};

    QLabel* m_statusLabel{nullptr};
};

} // namespace CodeShelf
