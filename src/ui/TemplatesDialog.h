#pragma once

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QMap>
#include "../core/Template.h"
#include "../services/TemplateService.h"

namespace CodeShelf {

class TemplatesDialog : public QDialog {
    Q_OBJECT

public:
    explicit TemplatesDialog(TemplateService& tmplService, bool selectionMode = false, QWidget* parent = nullptr);

    QString selectedText() const { return m_substitutedResult; }

signals:
    void templateInserted(const QString& text);

private slots:
    void onTemplateSelected(int row);
    void newTemplate();
    void saveTemplate();
    void deleteTemplate();
    void onPlaceholderChanged();
    void insertIntoEditor();

private:
    void setupUi();
    void loadTemplates();
    void setupPlaceholderInputs(const Template& tmpl);

    TemplateService& m_tmplService;
    bool m_selectionMode{false};
    QString m_substitutedResult;

    QListWidget* m_listWidget{nullptr};
    QLineEdit* m_nameEdit{nullptr};
    QComboBox* m_langCombo{nullptr};
    QTextEdit* m_bodyEdit{nullptr};
    QPushButton* m_saveBtn{nullptr};
    QPushButton* m_deleteBtn{nullptr};
    QPushButton* m_newBtn{nullptr};
    QPushButton* m_insertBtn{nullptr};

    QWidget* m_placeholderContainer{nullptr};
    QFormLayout* m_placeholderLayout{nullptr};
    QMap<QString, QLineEdit*> m_placeholderEdits;
    QTextEdit* m_previewEdit{nullptr};

    QVector<Template> m_templates;
    std::optional<Template> m_currentTemplate;
};

} // namespace CodeShelf
