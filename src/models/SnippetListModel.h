#pragma once

#include <QAbstractListModel>
#include <QVector>
#include "../core/Snippet.h"

namespace CodeShelf {

class SnippetListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum SnippetRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        LanguageRole,
        SnippetTypeRole,
        FavoriteRole,
        PinnedRole,
        UpdatedAtRole,
        TagsRole,
        DeletedRole,
        ObjectRole
    };

    explicit SnippetListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setSnippets(const QVector<Snippet>& snippets);
    const QVector<Snippet>& snippets() const { return m_snippets; }

    std::optional<Snippet> getSnippetAt(int row) const;
    std::optional<Snippet> getSnippetById(const QString& id) const;

    void updateSnippet(const Snippet& snippet);
    void removeSnippetById(const QString& id);

private:
    QVector<Snippet> m_snippets;
};

} // namespace CodeShelf
