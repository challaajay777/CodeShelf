#include "SnippetListModel.h"

namespace CodeShelf {

SnippetListModel::SnippetListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int SnippetListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_snippets.size();
}

QVariant SnippetListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_snippets.size()) {
        return QVariant();
    }

    const Snippet& snip = m_snippets.at(index.row());

    switch (role) {
        case Qt::DisplayRole:
        case TitleRole:
            return snip.title();
        case LanguageRole:
            return snip.language();
        case SnippetTypeRole:
            return snippetTypeToString(snip.snippetType());
        case FavoriteRole:
            return snip.isFavorite();
        case PinnedRole:
            return snip.isPinned();
        case UpdatedAtRole:
            return snip.updatedAt().toLocalTime().toString(QStringLiteral("yyyy-MM-dd hh:mm"));
        case TagsRole:
            return snip.tags().join(QStringLiteral(", "));
        case DeletedRole:
            return snip.isDeleted();
        case IdRole:
            return snip.id();
        default:
            break;
    }

    return QVariant();
}

void SnippetListModel::setSnippets(const QVector<Snippet>& snippets) {
    beginResetModel();
    m_snippets = snippets;
    endResetModel();
}

std::optional<Snippet> SnippetListModel::getSnippetAt(int row) const {
    if (row >= 0 && row < m_snippets.size()) {
        return m_snippets.at(row);
    }
    return std::nullopt;
}

std::optional<Snippet> SnippetListModel::getSnippetById(const QString& id) const {
    for (const auto& snip : m_snippets) {
        if (snip.id() == id) {
            return snip;
        }
    }
    return std::nullopt;
}

void SnippetListModel::updateSnippet(const Snippet& snippet) {
    for (int i = 0; i < m_snippets.size(); ++i) {
        if (m_snippets[i].id() == snippet.id()) {
            m_snippets[i] = snippet;
            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx);
            return;
        }
    }
}

void SnippetListModel::removeSnippetById(const QString& id) {
    for (int i = 0; i < m_snippets.size(); ++i) {
        if (m_snippets[i].id() == id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_snippets.removeAt(i);
            endRemoveRows();
            return;
        }
    }
}

} // namespace CodeShelf
