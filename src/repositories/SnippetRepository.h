#pragma once

#include <QString>
#include <QVector>
#include <optional>
#include "../core/Snippet.h"
#include "../core/Enums.h"
#include "../database/Database.h"
#include "TagRepository.h"
#include "RevisionRepository.h"

namespace CodeShelf {

class SnippetRepository {
public:
    explicit SnippetRepository(Database& db = Database::instance());

    bool create(Snippet& snippet, QString& errorOut);
    bool update(const Snippet& snippet, bool createRevisionSnapshot, QString& errorOut, const QString& changeNote = QString());
    bool duplicate(const QString& snippetId, Snippet& duplicatedOut, QString& errorOut);

    std::optional<Snippet> getById(const QString& id);
    QVector<Snippet> getAllActive(SortField field = SortField::UpdatedAt, SortDirection dir = SortDirection::Descending);
    QVector<Snippet> getByCollection(const QString& collectionId, SortField field = SortField::UpdatedAt, SortDirection dir = SortDirection::Descending);
    QVector<Snippet> getUnfiled(SortField field = SortField::UpdatedAt, SortDirection dir = SortDirection::Descending);
    QVector<Snippet> getFavorites(SortField field = SortField::UpdatedAt, SortDirection dir = SortDirection::Descending);
    QVector<Snippet> getPinned(SortField field = SortField::UpdatedAt, SortDirection dir = SortDirection::Descending);
    QVector<Snippet> getTrash();

    bool softDelete(const QString& id, QString& errorOut);
    bool restore(const QString& id, QString& errorOut);
    bool permanentDelete(const QString& id, QString& errorOut);
    bool emptyTrash(QString& errorOut);

    bool setFavorite(const QString& id, bool fav, QString& errorOut);
    bool setPinned(const QString& id, bool pin, QString& errorOut);
    bool moveToCollection(const QString& snippetId, const QString& collectionId, QString& errorOut);

    QVector<Snippet> findByContentHash(const QString& hash);
    int countActive();
    int countTrash();

private:
    Database& m_db;
    TagRepository m_tagRepo;
    RevisionRepository m_revRepo;

    void syncFtsIndex(const Snippet& snippet);
    void removeFtsIndex(const QString& snippetId);
    static QString buildOrderByClause(SortField field, SortDirection dir);
};

} // namespace CodeShelf
