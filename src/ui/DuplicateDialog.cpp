#include "DuplicateDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>

namespace CodeShelf {

DuplicateDialog::DuplicateDialog(DuplicateService& dupService, SnippetService& snippetService, QWidget* parent)
    : QDialog(parent)
    , m_dupService(dupService)
    , m_snippetService(snippetService)
{
    setWindowTitle(QStringLiteral("Duplicate Detection & Resolution"));
    resize(820, 560);
    setupUi();
    runScan();
}

void DuplicateDialog::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);

    // Top control row
    auto topRow = new QHBoxLayout();
    m_normalizedCheck = new QCheckBox(QStringLiteral("Include normalized matches (ignoring trailing whitespace / line endings)"), this);
    m_normalizedCheck->setChecked(true);
    m_scanBtn = new QPushButton(QStringLiteral("Re-scan"), this);

    topRow->addWidget(m_normalizedCheck);
    topRow->addStretch();
    topRow->addWidget(m_scanBtn);
    mainLayout->addLayout(topRow);

    // Main splitter: Left list of pairs, Right side-by-side preview
    auto splitter = new QSplitter(Qt::Horizontal, this);

    m_dupList = new QListWidget(this);
    m_dupList->setMinimumWidth(260);
    splitter->addWidget(m_dupList);

    auto previewContainer = new QWidget(this);
    auto prevLayout = new QVBoxLayout(previewContainer);
    prevLayout->setContentsMargins(0, 0, 0, 0);

    m_reasonLabel = new QLabel(this);
    m_reasonLabel->setStyleSheet(QStringLiteral("font-weight: bold; padding: 4px;"));
    prevLayout->addWidget(m_reasonLabel);

    auto sideBySide = new QHBoxLayout();
    m_leftPreview = new QTextEdit(this);
    m_leftPreview->setReadOnly(true);
    m_leftPreview->setFont(QFont(QStringLiteral("Consolas"), 9));

    m_rightPreview = new QTextEdit(this);
    m_rightPreview->setReadOnly(true);
    m_rightPreview->setFont(QFont(QStringLiteral("Consolas"), 9));

    sideBySide->addWidget(m_leftPreview);
    sideBySide->addWidget(m_rightPreview);
    prevLayout->addLayout(sideBySide, 1);

    m_trashRightBtn = new QPushButton(QStringLiteral("Move Right Snippet to Trash"), this);
    m_trashRightBtn->setEnabled(false);
    prevLayout->addWidget(m_trashRightBtn);

    splitter->addWidget(previewContainer);
    splitter->setStretchFactor(1, 2);
    mainLayout->addWidget(splitter, 1);

    // Bottom close button
    auto bottomRow = new QHBoxLayout();
    bottomRow->addStretch();
    auto closeBtn = new QPushButton(QStringLiteral("Close"), this);
    bottomRow->addWidget(closeBtn);
    mainLayout->addLayout(bottomRow);

    connect(m_scanBtn, &QPushButton::clicked, this, &DuplicateDialog::runScan);
    connect(m_dupList, &QListWidget::currentRowChanged, this, &DuplicateDialog::onSelectionChanged);
    connect(m_trashRightBtn, &QPushButton::clicked, this, &DuplicateDialog::trashSecondSnippet);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void DuplicateDialog::runScan() {
    m_dupList->clear();
    m_leftPreview->clear();
    m_rightPreview->clear();
    m_reasonLabel->clear();
    m_trashRightBtn->setEnabled(false);

    m_currentPairs = m_dupService.scanEntireLibraryForDuplicates(m_normalizedCheck->isChecked());

    for (const auto& pair : m_currentPairs) {
        QString itemText = QStringLiteral("\"%1\" <--> \"%2\"").arg(pair.first.title(), pair.second.title());
        m_dupList->addItem(itemText);
    }

    if (m_currentPairs.isEmpty()) {
        m_reasonLabel->setText(QStringLiteral("No duplicate snippet pairs detected."));
    } else {
        m_dupList->setCurrentRow(0);
    }
}

void DuplicateDialog::onSelectionChanged() {
    int row = m_dupList->currentRow();
    if (row < 0 || row >= m_currentPairs.size()) {
        m_leftPreview->clear();
        m_rightPreview->clear();
        m_trashRightBtn->setEnabled(false);
        return;
    }

    const auto& pair = m_currentPairs[row];
    m_leftPreview->setPlainText(QStringLiteral("[%1 - %2]\n\n%3").arg(pair.first.title(), pair.first.language(), pair.first.body()));
    m_rightPreview->setPlainText(QStringLiteral("[%1 - %2]\n\n%3").arg(pair.second.title(), pair.second.language(), pair.second.body()));

    if (pair.first.contentHash() == pair.second.contentHash()) {
        m_reasonLabel->setText(QStringLiteral("Reason: Exact SHA-256 content hash match."));
    } else {
        m_reasonLabel->setText(QStringLiteral("Reason: Normalized match (ignoring trailing whitespace / line endings)."));
    }

    m_trashRightBtn->setEnabled(true);
    m_trashRightBtn->setText(QStringLiteral("Move \"%1\" to Trash").arg(pair.second.title()));
}

void DuplicateDialog::trashSecondSnippet() {
    int row = m_dupList->currentRow();
    if (row < 0 || row >= m_currentPairs.size()) return;

    const auto& pair = m_currentPairs[row];
    if (QMessageBox::question(this, QStringLiteral("Move to Trash"),
                              QStringLiteral("Move duplicate snippet \"%1\" to Trash?").arg(pair.second.title()),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QString err;
        if (m_snippetService.moveToTrash(pair.second.id(), err)) {
            emit libraryModified();
            runScan();
        } else {
            QMessageBox::warning(this, QStringLiteral("Error"), err);
        }
    }
}

} // namespace CodeShelf
