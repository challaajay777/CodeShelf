#pragma once

#include <QDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include "../services/DuplicateService.h"
#include "../services/SnippetService.h"

namespace CodeShelf {

class DuplicateDialog : public QDialog {
    Q_OBJECT

public:
    DuplicateDialog(DuplicateService& dupService, SnippetService& snippetService, QWidget* parent = nullptr);

signals:
    void libraryModified();

private slots:
    void runScan();
    void onSelectionChanged();
    void trashSecondSnippet();

private:
    void setupUi();

    DuplicateService& m_dupService;
    SnippetService& m_snippetService;

    QCheckBox* m_normalizedCheck{nullptr};
    QPushButton* m_scanBtn{nullptr};
    QListWidget* m_dupList{nullptr};
    QLabel* m_reasonLabel{nullptr};
    QTextEdit* m_leftPreview{nullptr};
    QTextEdit* m_rightPreview{nullptr};
    QPushButton* m_trashRightBtn{nullptr};

    QVector<QPair<Snippet, Snippet>> m_currentPairs;
};

} // namespace CodeShelf
