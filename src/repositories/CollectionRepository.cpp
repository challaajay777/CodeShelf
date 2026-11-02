#include "CollectionRepository.h"
#include <QMutexLocker>

namespace CodeShelf {

CollectionRepository::CollectionRepository(Database& db)
    : m_db(db)
{
}

bool CollectionRepository::create(Collection& collection, QString& errorOut) {
    auto val = collection.validate();
    if (!val.valid) {
        errorOut = val.errorMessage;
        return false;
    }

    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "INSERT INTO collections (id, name, description, sort_order, created_at, updated_at) "
        "VALUES (?1, ?2, ?3, ?4, ?5, ?6);"
    ));
    stmt.bindText(1, collection.id());
    stmt.bindText(2, collection.name());
    stmt.bindText(3, collection.description());
    stmt.bindInt(4, collection.sortOrder());
    stmt.bindDateTime(5, collection.createdAt());
    stmt.bindDateTime(6, collection.updatedAt());

    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

bool CollectionRepository::update(const Collection& collection, QString& errorOut) {
    auto val = collection.validate();
    if (!val.valid) {
        errorOut = val.errorMessage;
        return false;
    }

    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "UPDATE collections SET name = ?1, description = ?2, sort_order = ?3, updated_at = ?4 "
        "WHERE id = ?5;"
    ));
    stmt.bindText(1, collection.name());
    stmt.bindText(2, collection.description());
    stmt.bindInt(3, collection.sortOrder());
    stmt.bindDateTime(4, QDateTime::currentDateTimeUtc());
    stmt.bindText(5, collection.id());

    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

bool CollectionRepository::deleteCollection(const QString& id, bool deleteSnippets, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteTransaction tx(m_db.rawDb());

    if (deleteSnippets) {
        // Soft delete snippets in this collection
        SqliteStatement delSnips(m_db.rawDb(), QStringLiteral(
            "UPDATE snippets SET deleted_at = ?1 WHERE collection_id = ?2;"
        ));
        delSnips.bindDateTime(1, QDateTime::currentDateTimeUtc());
        delSnips.bindText(2, id);
        if (!delSnips.execute()) {
            errorOut = delSnips.lastError();
            return false;
        }
    } else {
        // Default behavior: unfile snippets (set collection_id to NULL)
        SqliteStatement unfileSnips(m_db.rawDb(), QStringLiteral(
            "UPDATE snippets SET collection_id = NULL WHERE collection_id = ?1;"
        ));
        unfileSnips.bindText(1, id);
        if (!unfileSnips.execute()) {
            errorOut = unfileSnips.lastError();
            return false;
        }
    }

    SqliteStatement delStmt(m_db.rawDb(), QStringLiteral("DELETE FROM collections WHERE id = ?1;"));
    delStmt.bindText(1, id);
    if (!delStmt.execute()) {
        errorOut = delStmt.lastError();
        return false;
    }

    return tx.commit();
}

std::optional<Collection> CollectionRepository::getById(const QString& id) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT id, name, description, sort_order, created_at, updated_at "
        "FROM collections WHERE id = ?1;"
    ));
    stmt.bindText(1, id);
    if (stmt.step() == SQLITE_ROW) {
        Collection col;
        col.setId(stmt.getText(0));
        col.setName(stmt.getText(1));
        col.setDescription(stmt.getText(2));
        col.setSortOrder(stmt.getInt(3));
        col.setCreatedAt(stmt.getDateTime(4));
        col.setUpdatedAt(stmt.getDateTime(5));
        return col;
    }
    return std::nullopt;
}

QVector<Collection> CollectionRepository::getAll() {
    QMutexLocker locker(&m_db.mutex());
    QVector<Collection> result;
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT id, name, description, sort_order, created_at, updated_at "
        "FROM collections ORDER BY sort_order ASC, name COLLATE NOCASE ASC;"
    ));
    while (stmt.step() == SQLITE_ROW) {
        Collection col;
        col.setId(stmt.getText(0));
        col.setName(stmt.getText(1));
        col.setDescription(stmt.getText(2));
        col.setSortOrder(stmt.getInt(3));
        col.setCreatedAt(stmt.getDateTime(4));
        col.setUpdatedAt(stmt.getDateTime(5));
        result.append(col);
    }
    return result;
}

int CollectionRepository::getSnippetCount(const QString& collectionId) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT COUNT(*) FROM snippets WHERE collection_id = ?1 AND deleted_at IS NULL;"
    ));
    stmt.bindText(1, collectionId);
    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }
    return 0;
}

} // namespace CodeShelf
