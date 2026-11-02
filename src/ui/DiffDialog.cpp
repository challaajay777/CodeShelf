#include "DiffDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include "../core/DiffGenerator.h"

namespace CodeShelf {

DiffDialog::DiffDialog(const QString& snippetId,
                       SnippetRepository& snippetRepo,
                       RevisionRepository& revRepo,
                       SnippetService& snippetService,
                       QWidget* parent)
    : QDialog(parent)
    , m_snippetId(snippetId)
    , m_snippetRepo(snippetRepo)
    , m_revRepo(revRepo)
    , m_snippetService(snippetService)
{
    setWindowTitle(QStringLiteral("Revision History & Code Comparison"));
    resize(760, 520);
    setupUi();
    computeAndDisplayDiff();
}

void DiffDialog::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);

    // Revisions selector row
    auto topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel(QStringLiteral("Base (Old):"), this));
    m_leftRevisionCombo = new QComboBox(this);
    topRow->addWidget(m_leftRevisionCombo, 1);

    topRow->addWidget(new QLabel(QStringLiteral("Target (New):"), this));
    m_rightRevisionCombo = new QComboBox(this);
    topRow->addWidget(m_rightRevisionCombo, 1);

    mainLayout->addLayout(topRow);

    // Populate revisions
    m_revisions = m_revRepo.getRevisionsForSnippet(m_snippetId);
    auto current = m_snippetRepo.getById(m_snippetId);

    m_rightRevisionCombo->addItem(QStringLiteral("Current Live Snippet"), QStringLiteral("current"));
    for (const auto& rev : m_revisions) {
        QString label = QStringLiteral("Rev #%1 (%2) - %3")
                            .arg(rev.revisionNumber())
                            .arg(rev.changedAt().toLocalTime().toString(QStringLiteral("yyyy-MM-dd hh:mm")))
                            .arg(rev.changeNote().isEmpty() ? QStringLiteral("Auto snapshot") : rev.changeNote());
        m_leftRevisionCombo->addItem(label, rev.id());
        m_rightRevisionCombo->addItem(label, rev.id());
    }

    if (m_revisions.isEmpty()) {
        m_leftRevisionCombo->addItem(QStringLiteral("(No prior revisions saved)"), QString());
    }

    // Diff table: Line # (Old), Line # (New), Marker (+/-/ ), Text
    m_diffTable = new QTableWidget(this);
    m_diffTable->setColumnCount(4);
    m_diffTable->setHorizontalHeaderLabels({QStringLiteral("Old"), QStringLiteral("New"), QStringLiteral("Mark"), QStringLiteral("Content")});
    m_diffTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_diffTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_diffTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_diffTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_diffTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_diffTable->verticalHeader()->setVisible(false);
    m_diffTable->setShowGrid(false);
    m_diffTable->setFont(QFont(QStringLiteral("Consolas"), 9));

    mainLayout->addWidget(m_diffTable, 1);

    // Bottom action row
    auto bottomRow = new QHBoxLayout();
    m_summaryLabel = new QLabel(this);
    m_restoreBtn = new QPushButton(QStringLiteral("Restore Selected Base Revision"), this);
    auto closeBtn = new QPushButton(QStringLiteral("Close"), this);

    bottomRow->addWidget(m_summaryLabel);
    bottomRow->addStretch();
    bottomRow->addWidget(m_restoreBtn);
    bottomRow->addWidget(closeBtn);

    mainLayout->addLayout(bottomRow);

    connect(m_leftRevisionCombo, &QComboBox::currentIndexChanged, this, &DiffDialog::computeAndDisplayDiff);
    connect(m_rightRevisionCombo, &QComboBox::currentIndexChanged, this, &DiffDialog::computeAndDisplayDiff);
    connect(m_restoreBtn, &QPushButton::clicked, this, &DiffDialog::restoreSelectedRevision);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void DiffDialog::computeAndDisplayDiff() {
    m_diffTable->setRowCount(0);

    QString oldText;
    QString newText;

    const QString leftRevId = m_leftRevisionCombo->currentData().toString();
    const QString rightRevId = m_rightRevisionCombo->currentData().toString();

    // Fetch left
    if (!leftRevId.isEmpty()) {
        auto optRev = m_revRepo.getById(leftRevId);
        if (optRev.has_value()) {
            oldText = optRev->body();
        }
    }

    // Fetch right
    if (rightRevId == QStringLiteral("current")) {
        auto optSnip = m_snippetRepo.getById(m_snippetId);
        if (optSnip.has_value()) {
            newText = optSnip->body();
        }
    } else if (!rightRevId.isEmpty()) {
        auto optRev = m_revRepo.getById(rightRevId);
        if (optRev.has_value()) {
            newText = optRev->body();
        }
    }

    auto diffLines = DiffGenerator::generateDiff(oldText, newText, true);

    int additions = 0;
    int deletions = 0;

    m_diffTable->setRowCount(diffLines.size());
    for (int i = 0; i < diffLines.size(); ++i) {
        const auto& dl = diffLines[i];

        auto oldNumItem = new QTableWidgetItem(dl.oldLineNumber > 0 ? QString::number(dl.oldLineNumber) : QString());
        auto newNumItem = new QTableWidgetItem(dl.newLineNumber > 0 ? QString::number(dl.newLineNumber) : QString());
        auto markItem = new QTableWidgetItem(dl.prefixMarker().trimmed());
        auto textItem = new QTableWidgetItem(dl.text);

        oldNumItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        newNumItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        markItem->setTextAlignment(Qt::AlignCenter);

        // Muted neutral background tints with text markers
        QColor rowBg = palette().color(QPalette::Base);
        if (dl.type == DiffLineType::Addition) {
            ++additions;
            rowBg = QColor(220, 245, 220); // Muted green
        } else if (dl.type == DiffLineType::Deletion) {
            ++deletions;
            rowBg = QColor(255, 230, 230); // Muted red
        }

        oldNumItem->setBackground(rowBg);
        newNumItem->setBackground(rowBg);
        markItem->setBackground(rowBg);
        textItem->setBackground(rowBg);

        m_diffTable->setItem(i, 0, oldNumItem);
        m_diffTable->setItem(i, 1, newNumItem);
        m_diffTable->setItem(i, 2, markItem);
        m_diffTable->setItem(i, 3, textItem);
    }

    m_summaryLabel->setText(QStringLiteral("+%1 lines added, -%2 lines deleted").arg(additions).arg(deletions));
    m_restoreBtn->setEnabled(!leftRevId.isEmpty());
}

void DiffDialog::restoreSelectedRevision() {
    const QString leftRevId = m_leftRevisionCombo->currentData().toString();
    if (leftRevId.isEmpty()) return;

    if (QMessageBox::question(this, QStringLiteral("Restore Revision"),
                              QStringLiteral("Restore this revision? A new revision snapshot of your current snippet will be preserved first."),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QString err;
        if (m_snippetService.restoreRevision(m_snippetId, leftRevId, err)) {
            emit revisionRestored(m_snippetId);
            accept();
        } else {
            QMessageBox::warning(this, QStringLiteral("Restore Error"), err);
        }
    }
}

} // namespace CodeShelf
