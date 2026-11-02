#include "SnippetRepository.h"
#include <QMutexLocker>

namespace CodeShelf {

SnippetRepository::SnippetRepository(Database& db)
    : m_db(db)
    , m_tagRepo(db)
    , m_revRepo(db)
{
}

QString SnippetRepository::buildOrderByClause(SortField field, SortDirection dir) {
    QString col;
    switch (field) {
        case SortField::Title: col = QStringLiteral("title COLLATE NOCASE"); break;
        case SortField::Language: col = QStringLiteral("language COLLATE NOCASE"); break;
        case SortField::CreatedAt: col = QStringLiteral("created_at"); break;
        case SortField::UpdatedAt: col = QStringLiteral("updated_at"); break;
    }
    const QString direction = (dir == SortDirection::Ascending) ? QStringLiteral("ASC") : QStringLiteral("DESC");
    return QStringLiteral(" ORDER BY pinned DESC, %1 %2").arg(col, direction);
}

bool SnippetRepository::create(Snippet& snippet, QString& errorOut) {
    auto val = snippet.validate();
    if (!val.valid) {
        errorOut = val.errorMessage;
        return false;
    }

    snippet.refreshContentHash();
    snippet.setUpdatedAt(QDateTime::currentDateTimeUtc());

    QMutexLocker locker(&m_db.mutex());
    SqliteTransaction tx(m_db.rawDb());

    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "INSERT INTO snippets (id, title, body, description, language, snippet_type, collection_id, "
        "favorite, pinned, created_at, updated_at, deleted_at, content_hash, source_url) "
        "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14);"
    ));
    stmt.bindText(1, snippet.id());
    stmt.bindText(2, snippet.title());
    stmt.bindText(3, snippet.body());
    stmt.bindText(4, snippet.description());
    stmt.bindText(5, snippet.language());
    stmt.bindText(6, snippetTypeToString(snippet.snippetType()));
    if (snippet.collectionId().isEmpty()) {
        stmt.bindNull(7);
    } else {
        stmt.bindText(7, snippet.collectionId());
    }
    stmt.bindInt(8, snippet.isFavorite() ? 1 : 0);
    stmt.bindInt(9, snippet.isPinned() ? 1 : 0);
    stmt.bindDateTime(10, snippet.createdAt());
    stmt.bindDateTime(11, snippet.updatedAt());
    stmt.bindDateTime(12, snippet.deletedAt());
    stmt.bindText(13, snippet.contentHash());
    stmt.bindText(14, snippet.sourceUrl());

    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }

    // Save tags
    if (!m_tagRepo.setTagsForSnippet(snippet.id(), snippet.tags(), errorOut)) {
        return false;
    }

    if (!tx.commit()) {
        errorOut = QStringLiteral("Failed to commit snippet creation.");
        return false;
    }

    syncFtsIndex(snippet);
    return true;
}

bool SnippetRepository::update(const Snippet& snippet, bool createRevisionSnapshot, QString& errorOut, const QString& changeNote) {
    auto val = snippet.validate();
    if (!val.valid) {
        errorOut = val.errorMessage;
        return false;
    }

    QMutexLocker locker(&m_db.mutex());

    // Fetch existing state to check if revision snapshot is needed
    auto existing = getById(snippet.id());
    if (existing.has_value() && createRevisionSnapshot) {
        // Only create revision if content, title, description, or language changed
        if (existing->body() != snippet.body() ||
            existing->title() != snippet.title() ||
            existing->description() != snippet.description() ||
            existing->language() != snippet.language()) {

            int nextRev = m_revRepo.getNextRevisionNumber(snippet.id());
            Revision rev(snippet.id(), nextRev, existing->title(), existing->body(),
                         existing->description(), existing->language(), changeNote);
            m_revRepo.createRevision(rev, errorOut);
        }
    }

    SqliteTransaction tx(m_db.rawDb());

    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "UPDATE snippets SET title = ?1, body = ?2, description = ?3, language = ?4, "
        "snippet_type = ?5, collection_id = ?6, favorite = ?7, pinned = ?8, updated_at = ?9, "
        "content_hash = ?10, source_url = ?11 WHERE id = ?12;"
    ));
    stmt.bindText(1, snippet.title());
    stmt.bindText(2, snippet.body());
    stmt.bindText(3, snippet.description());
    stmt.bindText(4, snippet.language());
    stmt.bindText(5, snippetTypeToString(snippet.snippetType()));
    if (snippet.collectionId().isEmpty()) {
        stmt.bindNull(6);
    } else {
        stmt.bindText(6, snippet.collectionId());
    }
    stmt.bindInt(7, snippet.isFavorite() ? 1 : 0);
    stmt.bindInt(8, snippet.isPinned() ? 1 : 0);
    stmt.bindDateTime(9, QDateTime::currentDateTimeUtc());
    stmt.bindText(10, ContentHash::computeExactHash(snippet.body()));
    stmt.bindText(11, snippet.sourceUrl());
    stmt.bindText(12, snippet.id());

    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }

    if (!m_tagRepo.setTagsForSnippet(snippet.id(), snippet.tags(), errorOut)) {
        return false;
    }

    if (!tx.commit()) {
        errorOut = QStringLiteral("Failed to commit snippet update.");
        return false;
    }

    syncFtsIndex(snippet);
    return true;
}

bool SnippetRepository::duplicate(const QString& snippetId, Snippet& duplicatedOut, QString& errorOut) {
    auto orig = getById(snippetId);
    if (!orig.has_value()) {
        errorOut = QStringLiteral("Source snippet not found.");
        return false;
    }

    duplicatedOut = *orig;
    duplicatedOut.setId(Validation::generateUuid());
    duplicatedOut.setTitle(orig->title() + QStringLiteral(" (Copy)"));
    duplicatedOut.setCreatedAt(QDateTime::currentDateTimeUtc());
    duplicatedOut.setUpdatedAt(duplicatedOut.createdAt());
    duplicatedOut.setDeletedAt(QDateTime());

    return create(duplicatedOut, errorOut);
}

std::optional<Snippet> SnippetRepository::getById(const QString& id) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT id, title, body, description, language, snippet_type, collection_id, "
        "favorite, pinned, created_at, updated_at, deleted_at, content_hash, source_url "
        "FROM snippets WHERE id = ?1;"
    ));
    stmt.bindText(1, id);
    if (stmt.step() == SQLITE_ROW) {
        Snippet s;
        s.setId(stmt.getText(0));
        s.setTitle(stmt.getText(1));
        s.setBody(stmt.getText(2));
        s.setDescription(stmt.getText(3));
        s.setLanguage(stmt.getText(4));
        s.setSnippetType(stringToSnippetType(stmt.getText(5)));
        s.setCollectionId(stmt.getText(6));
        s.setFavorite(stmt.getInt(7) == 1);
        s.setPinned(stmt.getInt(8) == 1);
        s.setCreatedAt(stmt.getDateTime(9));
        s.setUpdatedAt(stmt.getDateTime(10));
        s.setDeletedAt(stmt.getDateTime(11));
        s.setContentHash(stmt.getText(12));
        s.setSourceUrl(stmt.getText(13));
        s.setTags(m_tagRepo.getTagsForSnippet(s.id()));
        return s;
    }
    return std::nullopt;
}

QVector<Snippet> SnippetRepository::getAllActive(SortField field, SortDirection dir) {
    QMutexLocker locker(&m_db.mutex());
    QVector<Snippet> result;
    const QString sql = QStringLiteral(
        "SELECT id, title, body, description, language, snippet_type, collection_id, "
        "favorite, pinned, created_at, updated_at, deleted_at, content_hash, source_url "
        "FROM snippets WHERE deleted_at IS NULL"
    ) + buildOrderByClause(field, dir);

    SqliteStatement stmt(m_db.rawDb(), sql);
    while (stmt.step() == SQLITE_ROW) {
        Snippet s;
        s.setId(stmt.getText(0));
        s.setTitle(stmt.getText(1));
        s.setBody(stmt.getText(2));
        s.setDescription(stmt.getText(3));
        s.setLanguage(stmt.getText(4));
        s.setSnippetType(stringToSnippetType(stmt.getText(5)));
        s.setCollectionId(stmt.getText(6));
        s.setFavorite(stmt.getInt(7) == 1);
        s.setPinned(stmt.getInt(8) == 1);
        s.setCreatedAt(stmt.getDateTime(9));
        s.setUpdatedAt(stmt.getDateTime(10));
        s.setDeletedAt(stmt.getDateTime(11));
        s.setContentHash(stmt.getText(12));
        s.setSourceUrl(stmt.getText(13));
        s.setTags(m_tagRepo.getTagsForSnippet(s.id()));
        result.append(s);
    }
    return result;
}

QVector<Snippet> SnippetRepository::getByCollection(const QString& collectionId, SortField field, SortDirection dir) {
    QMutexLocker locker(&m_db.mutex());
    QVector<Snippet> result;
    const QString sql = QStringLiteral(
        "SELECT id, title, body, description, language, snippet_type, collection_id, "
        "favorite, pinned, created_at, updated_at, deleted_at, content_hash, source_url "
        "FROM snippets WHERE deleted_at IS NULL AND collection_id = ?1"
    ) + buildOrderByClause(field, dir);

    SqliteStatement stmt(m_db.rawDb(), sql);
    stmt.bindText(1, collectionId);
    while (stmt.step() == SQLITE_ROW) {
        Snippet s;
        s.setId(stmt.getText(0));
        s.setTitle(stmt.getText(1));
        s.setBody(stmt.getText(2));
        s.setDescription(stmt.getText(3));
        s.setLanguage(stmt.getText(4));
        s.setSnippetType(stringToSnippetType(stmt.getText(5)));
        s.setCollectionId(stmt.getText(6));
        s.setFavorite(stmt.getInt(7) == 1);
        s.setPinned(stmt.getInt(8) == 1);
        s.setCreatedAt(stmt.getDateTime(9));
        s.setUpdatedAt(stmt.getDateTime(10));
        s.setDeletedAt(stmt.getDateTime(11));
        s.setContentHash(stmt.getText(12));
        s.setSourceUrl(stmt.getText(13));
        s.setTags(m_tagRepo.getTagsForSnippet(s.id()));
        result.append(s);
    }
    return result;
}

QVector<Snippet> SnippetRepository::getUnfiled(SortField field, SortDirection dir) {
    QMutexLocker locker(&m_db.mutex());
    QVector<Snippet> result;
    const QString sql = QStringLiteral(
        "SELECT id, title, body, description, language, snippet_type, collection_id, "
        "favorite, pinned, created_at, updated_at, deleted_at, content_hash, source_url "
        "FROM snippets WHERE deleted_at IS NULL AND (collection_id IS NULL OR collection_id = '')"
    ) + buildOrderByClause(field, dir);

    SqliteStatement stmt(m_db.rawDb(), sql);
    while (stmt.step() == SQLITE_ROW) {
        Snippet s;
        s.setId(stmt.getText(0));
        s.setTitle(stmt.getText(1));
        s.setBody(stmt.getText(2));
        s.setDescription(stmt.getText(3));
        s.setLanguage(stmt.getText(4));
        s.setSnippetType(stringToSnippetType(stmt.getText(5)));
        s.setCollectionId(stmt.getText(6));
        s.setFavorite(stmt.getInt(7) == 1);
        s.setPinned(stmt.getInt(8) == 1);
        s.setCreatedAt(stmt.getDateTime(9));
        s.setUpdatedAt(stmt.getDateTime(10));
        s.setDeletedAt(stmt.getDateTime(11));
        s.setContentHash(stmt.getText(12));
        s.setSourceUrl(stmt.getText(13));
        s.setTags(m_tagRepo.getTagsForSnippet(s.id()));
        result.append(s);
    }
    return result;
}

QVector<Snippet> SnippetRepository::getFavorites(SortField field, SortDirection dir) {
    QMutexLocker locker(&m_db.mutex());
    QVector<Snippet> result;
    const QString sql = QStringLiteral(
        "SELECT id, title, body, description, language, snippet_type, collection_id, "
        "favorite, pinned, created_at, updated_at, deleted_at, content_hash, source_url "
        "FROM snippets WHERE deleted_at IS NULL AND favorite = 1"
    ) + buildOrderByClause(field, dir);

    SqliteStatement stmt(m_db.rawDb(), sql);
    while (stmt.step() == SQLITE_ROW) {
        Snippet s;
        s.setId(stmt.getText(0));
        s.setTitle(stmt.getText(1));
        s.setBody(stmt.getText(2));
        s.setDescription(stmt.getText(3));
        s.setLanguage(stmt.getText(4));
        s.setSnippetType(stringToSnippetType(stmt.getText(5)));
        s.setCollectionId(stmt.getText(6));
        s.setFavorite(stmt.getInt(7) == 1);
        s.setPinned(stmt.getInt(8) == 1);
        s.setCreatedAt(stmt.getDateTime(9));
        s.setUpdatedAt(stmt.getDateTime(10));
        s.setDeletedAt(stmt.getDateTime(11));
        s.setContentHash(stmt.getText(12));
        s.setSourceUrl(stmt.getText(13));
        s.setTags(m_tagRepo.getTagsForSnippet(s.id()));
        result.append(s);
    }
    return result;
}

QVector<Snippet> SnippetRepository::getPinned(SortField field, SortDirection dir) {
    QMutexLocker locker(&m_db.mutex());
    QVector<Snippet> result;
    const QString sql = QStringLiteral(
        "SELECT id, title, body, description, language, snippet_type, collection_id, "
        "favorite, pinned, created_at, updated_at, deleted_at, content_hash, source_url "
        "FROM snippets WHERE deleted_at IS NULL AND pinned = 1"
    ) + buildOrderByClause(field, dir);

    SqliteStatement stmt(m_db.rawDb(), sql);
    while (stmt.step() == SQLITE_ROW) {
        Snippet s;
        s.setId(stmt.getText(0));
        s.setTitle(stmt.getText(1));
        s.setBody(stmt.getText(2));
        s.setDescription(stmt.getText(3));
        s.setLanguage(stmt.getText(4));
        s.setSnippetType(stringToSnippetType(stmt.getText(5)));
        s.setCollectionId(stmt.getText(6));
        s.setFavorite(stmt.getInt(7) == 1);
        s.setPinned(stmt.getInt(8) == 1);
        s.setCreatedAt(stmt.getDateTime(9));
        s.setUpdatedAt(stmt.getDateTime(10));
        s.setDeletedAt(stmt.getDateTime(11));
        s.setContentHash(stmt.getText(12));
        s.setSourceUrl(stmt.getText(13));
        s.setTags(m_tagRepo.getTagsForSnippet(s.id()));
        result.append(s);
    }
    return result;
}

QVector<Snippet> SnippetRepository::getTrash() {
    QMutexLocker locker(&m_db.mutex());
    QVector<Snippet> result;
    const char* sql = "SELECT id, title, body, description, language, snippet_type, collection_id, "
                      "favorite, pinned, created_at, updated_at, deleted_at, content_hash, source_url "
                      "FROM snippets WHERE deleted_at IS NOT NULL ORDER BY deleted_at DESC;";
    SqliteStatement stmt(m_db.rawDb(), QString::fromUtf8(sql));
    while (stmt.step() == SQLITE_ROW) {
        Snippet s;
        s.setId(stmt.getText(0));
        s.setTitle(stmt.getText(1));
        s.setBody(stmt.getText(2));
        s.setDescription(stmt.getText(3));
        s.setLanguage(stmt.getText(4));
        s.setSnippetType(stringToSnippetType(stmt.getText(5)));
        s.setCollectionId(stmt.getText(6));
        s.setFavorite(stmt.getInt(7) == 1);
        s.setPinned(stmt.getInt(8) == 1);
        s.setCreatedAt(stmt.getDateTime(9));
        s.setUpdatedAt(stmt.getDateTime(10));
        s.setDeletedAt(stmt.getDateTime(11));
        s.setContentHash(stmt.getText(12));
        s.setSourceUrl(stmt.getText(13));
        s.setTags(m_tagRepo.getTagsForSnippet(s.id()));
        result.append(s);
    }
    return result;
}

bool SnippetRepository::softDelete(const QString& id, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "UPDATE snippets SET deleted_at = ?1 WHERE id = ?2;"
    ));
    stmt.bindDateTime(1, QDateTime::currentDateTimeUtc());
    stmt.bindText(2, id);
    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    removeFtsIndex(id);
    return true;
}

bool SnippetRepository::restore(const QString& id, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "UPDATE snippets SET deleted_at = NULL WHERE id = ?1;"
    ));
    stmt.bindText(1, id);
    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }

    auto snip = getById(id);
    if (snip.has_value()) {
        syncFtsIndex(*snip);
    }
    return true;
}

bool SnippetRepository::permanentDelete(const QString& id, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("DELETE FROM snippets WHERE id = ?1;"));
    stmt.bindText(1, id);
    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    removeFtsIndex(id);
    return true;
}

bool SnippetRepository::emptyTrash(QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("DELETE FROM snippets WHERE deleted_at IS NOT NULL;"));
    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

bool SnippetRepository::setFavorite(const QString& id, bool fav, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("UPDATE snippets SET favorite = ?1 WHERE id = ?2;"));
    stmt.bindInt(1, fav ? 1 : 0);
    stmt.bindText(2, id);
    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

bool SnippetRepository::setPinned(const QString& id, bool pin, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("UPDATE snippets SET pinned = ?1 WHERE id = ?2;"));
    stmt.bindInt(1, pin ? 1 : 0);
    stmt.bindText(2, id);
    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

bool SnippetRepository::moveToCollection(const QString& snippetId, const QString& collectionId, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("UPDATE snippets SET collection_id = ?1 WHERE id = ?2;"));
    if (collectionId.isEmpty()) {
        stmt.bindNull(1);
    } else {
        stmt.bindText(1, collectionId);
    }
    stmt.bindText(2, snippetId);
    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

QVector<Snippet> SnippetRepository::findByContentHash(const QString& hash) {
    QMutexLocker locker(&m_db.mutex());
    QVector<Snippet> result;
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT id, title, body, description, language, snippet_type, collection_id, "
        "favorite, pinned, created_at, updated_at, deleted_at, content_hash, source_url "
        "FROM snippets WHERE content_hash = ?1 AND deleted_at IS NULL;"
    ));
    stmt.bindText(1, hash);
    while (stmt.step() == SQLITE_ROW) {
        Snippet s;
        s.setId(stmt.getText(0));
        s.setTitle(stmt.getText(1));
        s.setBody(stmt.getText(2));
        s.setDescription(stmt.getText(3));
        s.setLanguage(stmt.getText(4));
        s.setSnippetType(stringToSnippetType(stmt.getText(5)));
        s.setCollectionId(stmt.getText(6));
        s.setFavorite(stmt.getInt(7) == 1);
        s.setPinned(stmt.getInt(8) == 1);
        s.setCreatedAt(stmt.getDateTime(9));
        s.setUpdatedAt(stmt.getDateTime(10));
        s.setDeletedAt(stmt.getDateTime(11));
        s.setContentHash(stmt.getText(12));
        s.setSourceUrl(stmt.getText(13));
        s.setTags(m_tagRepo.getTagsForSnippet(s.id()));
        result.append(s);
    }
    return result;
}

int SnippetRepository::countActive() {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("SELECT COUNT(*) FROM snippets WHERE deleted_at IS NULL;"));
    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }
    return 0;
}

int SnippetRepository::countTrash() {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("SELECT COUNT(*) FROM snippets WHERE deleted_at IS NOT NULL;"));
    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }
    return 0;
}

void SnippetRepository::syncFtsIndex(const Snippet& snippet) {
    if (!m_db.isFts5Available()) return;

    removeFtsIndex(snippet.id());
    if (snippet.isDeleted()) return;

    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "INSERT INTO snippets_fts (snippet_id, title, description, body, tags) "
        "VALUES (?1, ?2, ?3, ?4, ?5);"
    ));
    stmt.bindText(1, snippet.id());
    stmt.bindText(2, snippet.title());
    stmt.bindText(3, snippet.description());
    stmt.bindText(4, snippet.body());
    stmt.bindText(5, snippet.tags().join(QLatin1Char(' ')));
    stmt.execute();
}

void SnippetRepository::removeFtsIndex(const QString& snippetId) {
    if (!m_db.isFts5Available()) return;
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("DELETE FROM snippets_fts WHERE snippet_id = ?1;"));
    stmt.bindText(1, snippetId);
    stmt.execute();
}

} // namespace CodeShelf
