#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFontComboBox>
#include <QFileDialog>
#include "../settings/AppSettings.h"

namespace CodeShelf {

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Settings"));
    resize(480, 320);
    setupUi();
}

void SettingsDialog::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    auto formLayout = new QFormLayout();

    // Theme
    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItem(QStringLiteral("System Default"), static_cast<int>(AppTheme::System));
    m_themeCombo->addItem(QStringLiteral("Light (Neutral System)"), static_cast<int>(AppTheme::Light));
    m_themeCombo->addItem(QStringLiteral("Dark (Charcoal Neutral)"), static_cast<int>(AppTheme::Dark));

    auto currentTheme = AppSettings::instance().theme();
    int themeIdx = m_themeCombo->findData(static_cast<int>(currentTheme));
    if (themeIdx >= 0) m_themeCombo->setCurrentIndex(themeIdx);
    formLayout->addRow(QStringLiteral("Theme:"), m_themeCombo);

    // Font
    m_fontCombo = new QFontComboBox(this);
    m_fontCombo->setFontFilters(QFontComboBox::MonospacedFonts);
    m_fontCombo->setCurrentFont(QFont(AppSettings::instance().codeFontFamily()));
    formLayout->addRow(QStringLiteral("Code Font:"), m_fontCombo);

    // Font size
    m_fontSizeSpin = new QSpinBox(this);
    m_fontSizeSpin->setRange(8, 32);
    m_fontSizeSpin->setValue(AppSettings::instance().codeFontSize());
    formLayout->addRow(QStringLiteral("Font Size (pt):"), m_fontSizeSpin);

    // Tab size
    m_tabSizeSpin = new QSpinBox(this);
    m_tabSizeSpin->setRange(1, 16);
    m_tabSizeSpin->setValue(AppSettings::instance().tabSize());
    formLayout->addRow(QStringLiteral("Tab Size (spaces):"), m_tabSizeSpin);

    // Line numbers
    m_lineNumbersCheck = new QCheckBox(QStringLiteral("Show line numbers in code editor"), this);
    m_lineNumbersCheck->setChecked(AppSettings::instance().showLineNumbers());
    formLayout->addRow(QStringLiteral("Line Numbers:"), m_lineNumbersCheck);

    // Backup Directory
    auto backupRow = new QHBoxLayout();
    m_backupDirEdit = new QLineEdit(AppSettings::instance().backupDirectory(), this);
    m_browseBackupBtn = new QPushButton(QStringLiteral("Browse..."), this);
    backupRow->addWidget(m_backupDirEdit, 1);
    backupRow->addWidget(m_browseBackupBtn);
    formLayout->addRow(QStringLiteral("Default Backup Folder:"), backupRow);

    mainLayout->addLayout(formLayout);

    // Bottom action row
    auto bottomRow = new QHBoxLayout();
    bottomRow->addStretch();
    auto saveBtn = new QPushButton(QStringLiteral("Save"), this);
    saveBtn->setDefault(true);
    auto cancelBtn = new QPushButton(QStringLiteral("Cancel"), this);

    bottomRow->addWidget(saveBtn);
    bottomRow->addWidget(cancelBtn);
    mainLayout->addLayout(bottomRow);

    connect(m_browseBackupBtn, &QPushButton::clicked, this, &SettingsDialog::chooseBackupDir);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::saveAndClose);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::chooseBackupDir() {
    QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Select Backup Folder"), m_backupDirEdit->text());
    if (!dir.isEmpty()) {
        m_backupDirEdit->setText(dir);
    }
}

void SettingsDialog::saveAndClose() {
    auto& s = AppSettings::instance();
    s.setTheme(static_cast<AppTheme>(m_themeCombo->currentData().toInt()));
    s.setCodeFontFamily(m_fontCombo->currentFont().family());
    s.setCodeFontSize(m_fontSizeSpin->value());
    s.setTabSize(m_tabSizeSpin->value());
    s.setShowLineNumbers(m_lineNumbersCheck->isChecked());
    s.setBackupDirectory(m_backupDirEdit->text().trimmed());
    s.sync();

    emit settingsChanged();
    accept();
}

} // namespace CodeShelf
