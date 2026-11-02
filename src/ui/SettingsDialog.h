#pragma once

#include <QDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

namespace CodeShelf {

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

signals:
    void settingsChanged();

private slots:
    void saveAndClose();
    void chooseBackupDir();

private:
    void setupUi();

    QComboBox* m_themeCombo{nullptr};
    QFontComboBox* m_fontCombo{nullptr};
    QSpinBox* m_fontSizeSpin{nullptr};
    QSpinBox* m_tabSizeSpin{nullptr};
    QCheckBox* m_lineNumbersCheck{nullptr};
    QLineEdit* m_backupDirEdit{nullptr};
    QPushButton* m_browseBackupBtn{nullptr};
};

} // namespace CodeShelf
