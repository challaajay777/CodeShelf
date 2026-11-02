#include "TemplatesDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QMessageBox>

namespace CodeShelf {

TemplatesDialog::TemplatesDialog(TemplateService& tmplService, bool selectionMode, QWidget* parent)
    : QDialog(parent)
    , m_tmplService(tmplService)
    , m_selectionMode(selectionMode)
{
    setWindowTitle(QStringLiteral("Code Templates"));
    resize(820, 560);
    setupUi();
    loadTemplates();
}

void TemplatesDialog::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);

    auto splitter = new QSplitter(Qt::Horizontal, this);

    // Left pane: Template list and buttons
    auto leftContainer = new QWidget(this);
    auto leftLayout = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    m_listWidget = new QListWidget(this);
    leftLayout->addWidget(m_listWidget, 1);

    auto leftBtnRow = new QHBoxLayout();
    m_newBtn = new QPushButton(QStringLiteral("New"), this);
    m_deleteBtn = new QPushButton(QStringLiteral("Delete"), this);
    leftBtnRow->addWidget(m_newBtn);
    leftBtnRow->addWidget(m_deleteBtn);
    leftLayout->addLayout(leftBtnRow);

    splitter->addWidget(leftContainer);

    // Right pane: Template editor & Placeholder substitution preview
    auto rightContainer = new QWidget(this);
    auto rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    auto formLayout = new QFormLayout();
    m_nameEdit = new QLineEdit(this);
    m_langCombo = new QComboBox(this);
    for (const QString& lang : allSupportedLanguages()) {
        m_langCombo->addItem(lang);
    }
    formLayout->addRow(QStringLiteral("Template Name:"), m_nameEdit);
    formLayout->addRow(QStringLiteral("Language:"), m_langCombo);
    rightLayout->addLayout(formLayout);

    rightLayout->addWidget(new QLabel(QStringLiteral("Template Body (use {{variable}} for placeholders):")));
    m_bodyEdit = new QTextEdit(this);
    m_bodyEdit->setFont(QFont(QStringLiteral("Consolas"), 9));
    rightLayout->addWidget(m_bodyEdit, 2);

    m_saveBtn = new QPushButton(QStringLiteral("Save Template"), this);
    rightLayout->addWidget(m_saveBtn);

    // Placeholder inputs
    m_placeholderContainer = new QWidget(this);
    m_placeholderLayout = new QFormLayout(m_placeholderContainer);
    m_placeholderLayout->setContentsMargins(0, 4, 0, 4);
    rightLayout->addWidget(new QLabel(QStringLiteral("Variable Values:")));
    rightLayout->addWidget(m_placeholderContainer);

    // Substituted preview
    rightLayout->addWidget(new QLabel(QStringLiteral("Substituted Output Preview:")));
    m_previewEdit = new QTextEdit(this);
    m_previewEdit->setReadOnly(true);
    m_previewEdit->setFont(QFont(QStringLiteral("Consolas"), 9));
    rightLayout->addWidget(m_previewEdit, 2);

    splitter->addWidget(rightContainer);
    splitter->setStretchFactor(1, 2);
    mainLayout->addWidget(splitter, 1);

    // Bottom action row
    auto bottomRow = new QHBoxLayout();
    bottomRow->addStretch();
    m_insertBtn = new QPushButton(QStringLiteral("Insert into Editor"), this);
    m_insertBtn->setVisible(m_selectionMode);
    auto closeBtn = new QPushButton(QStringLiteral("Close"), this);

    bottomRow->addWidget(m_insertBtn);
    bottomRow->addWidget(closeBtn);
    mainLayout->addLayout(bottomRow);

    connect(m_listWidget, &QListWidget::currentRowChanged, this, &TemplatesDialog::onTemplateSelected);
    connect(m_newBtn, &QPushButton::clicked, this, &TemplatesDialog::newTemplate);
    connect(m_saveBtn, &QPushButton::clicked, this, &TemplatesDialog::saveTemplate);
    connect(m_deleteBtn, &QPushButton::clicked, this, &TemplatesDialog::deleteTemplate);
    connect(m_bodyEdit, &QTextEdit::textChanged, this, &TemplatesDialog::onPlaceholderChanged);
    connect(m_insertBtn, &QPushButton::clicked, this, &TemplatesDialog::insertIntoEditor);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void TemplatesDialog::loadTemplates() {
    m_listWidget->clear();
    m_templates = m_tmplService.getAllTemplates();

    for (const auto& tmpl : m_templates) {
        m_listWidget->addItem(tmpl.name());
    }

    if (!m_templates.isEmpty()) {
        m_listWidget->setCurrentRow(0);
    } else {
        newTemplate();
    }
}

void TemplatesDialog::onTemplateSelected(int row) {
    if (row < 0 || row >= m_templates.size()) {
        m_currentTemplate = std::nullopt;
        m_deleteBtn->setEnabled(false);
        m_insertBtn->setEnabled(false);
        return;
    }

    m_currentTemplate = m_templates[row];
    m_nameEdit->setText(m_currentTemplate->name());
    int langIdx = m_langCombo->findText(m_currentTemplate->language());
    if (langIdx >= 0) m_langCombo->setCurrentIndex(langIdx);
    m_bodyEdit->setPlainText(m_currentTemplate->body());
    m_deleteBtn->setEnabled(true);
    m_insertBtn->setEnabled(m_selectionMode);

    setupPlaceholderInputs(*m_currentTemplate);
}

void TemplatesDialog::newTemplate() {
    m_currentTemplate = std::nullopt;
    m_nameEdit->clear();
    m_nameEdit->setPlaceholderText(QStringLiteral("e.g. C++ Class Header"));
    m_langCombo->setCurrentIndex(0);
    m_bodyEdit->clear();
    m_deleteBtn->setEnabled(false);
    m_insertBtn->setEnabled(false);
    m_previewEdit->clear();

    // Clear placeholder inputs
    QLayoutItem* child = nullptr;
    while ((child = m_placeholderLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    m_placeholderEdits.clear();
}

void TemplatesDialog::saveTemplate() {
    Template tmpl;
    if (m_currentTemplate.has_value()) {
        tmpl = *m_currentTemplate;
    }

    tmpl.setName(m_nameEdit->text());
    tmpl.setLanguage(m_langCombo->currentText());
    tmpl.setBody(m_bodyEdit->toPlainText());

    auto val = tmpl.validate();
    if (!val.valid) {
        QMessageBox::warning(this, QStringLiteral("Validation Error"), val.errorMessage);
        return;
    }

    QString err;
    if (m_tmplService.saveTemplate(tmpl, err)) {
        loadTemplates();
    } else {
        QMessageBox::warning(this, QStringLiteral("Error"), err);
    }
}

void TemplatesDialog::deleteTemplate() {
    if (!m_currentTemplate.has_value()) return;

    if (QMessageBox::question(this, QStringLiteral("Delete Template"),
                              QStringLiteral("Delete template \"%1\"?").arg(m_currentTemplate->name()),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QString err;
        if (m_tmplService.deleteTemplate(m_currentTemplate->id(), err)) {
            loadTemplates();
        } else {
            QMessageBox::warning(this, QStringLiteral("Error"), err);
        }
    }
}

void TemplatesDialog::setupPlaceholderInputs(const Template& tmpl) {
    // Clear old inputs
    QLayoutItem* child = nullptr;
    while ((child = m_placeholderLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    m_placeholderEdits.clear();

    const QStringList vars = tmpl.extractPlaceholders();
    for (const QString& v : vars) {
        auto edit = new QLineEdit(this);
        edit->setPlaceholderText(QStringLiteral("Value for %1").arg(v));
        connect(edit, &QLineEdit::textChanged, this, &TemplatesDialog::onPlaceholderChanged);
        m_placeholderLayout->addRow(v + QStringLiteral(":"), edit);
        m_placeholderEdits[v] = edit;
    }

    onPlaceholderChanged();
}

void TemplatesDialog::onPlaceholderChanged() {
    Template tmpl;
    tmpl.setBody(m_bodyEdit->toPlainText());

    QMap<QString, QString> values;
    for (auto it = m_placeholderEdits.constBegin(); it != m_placeholderEdits.constEnd(); ++it) {
        values[it.key()] = it.value()->text();
    }

    m_substitutedResult = tmpl.applySubstitutions(values);
    m_previewEdit->setPlainText(m_substitutedResult);
}

void TemplatesDialog::insertIntoEditor() {
    onPlaceholderChanged();
    emit templateInserted(m_substitutedResult);
    accept();
}

} // namespace CodeShelf
