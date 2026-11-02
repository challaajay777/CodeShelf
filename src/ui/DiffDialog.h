#pragma once

#include <QDialog>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include "../core/Revision.h"
#include "../repositories/RevisionRepository.h"
#include "../repositories/SnippetRepository.h"
#include "../services/SnippetService.h"

namespace CodeShelf {

class DiffDialog : public QDialog {
    Q_OBJECT

public:
    DiffDialog(const QString& snippetId,
               SnippetRepository& snippetRepo,
               RevisionRepository& revRepo,
               SnippetService& snippetService,
               QWidget* parent = nullptr);

signals:
    void revisionRestored(const QString& snippetId);

private slots:
    void computeAndDisplayDiff();
    void restoreSelectedRevision();

private:
    void setupUi();

    QString m_snippetId;
    SnippetRepository& m_snippetRepo;
    RevisionRepository& m_revRepo;
    SnippetService& m_snippetService;

    QComboBox* m_leftRevisionCombo{nullptr};
    QComboBox* m_rightRevisionCombo{nullptr};
    QTableWidget* m_diffTable{nullptr};
    QPushButton* m_restoreBtn{nullptr};
    QLabel* m_summaryLabel{nullptr};

    QVector<Revision> m_revisions;
};

} // namespace CodeShelf
