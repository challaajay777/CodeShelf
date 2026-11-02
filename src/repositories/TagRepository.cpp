#include "TagRepository.h"
#include <QMutexLocker>

namespace CodeShelf {

TagRepository::TagRepository(Database& db)
    : m_db(db)
{
}

Tag TagRepository::getOrCreateTag(const QString& displayName, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    const QString norm = Validation::normalizeTag(displayName);
    const QString trimmed = displayName.trimmed();

    // Check if exists
    SqliteStatement selectStmt(m_db.rawDb(), QStringLiteral("SELECT id, normalized_name, display_name, created_at FROM tags WHERE normalized_name = ?1;"));
    selectStmt.bindText(1, norm);
    if (selectStmt.step() == SQLITE_ROW) {
        Tag tag;
        tag.setId(selectStmt.getText(0));
        tag.setNormalizedName(selectStmt.getText(1));
        tag.setDisplayName(selectStmt.getText(2));
        tag.setCreatedAt(selectStmt.getDateTime(3));
        return tag;
    }

    // Insert new tag
    Tag newTag(trimmed);
    auto val = newTag.validate();
    if (!val.valid) {
        errorOut = val.errorMessage;
        return Tag();
    }

    SqliteStatement insertStmt(m_db.rawDb(), QStringLiteral("INSERT INTO tags (id, normalized_name, display_name, created_at) VALUES (?1, ?2, ?3, ?4);"));
    insertStmt.bindText(1, newTag.id());
    insertStmt.bindText(2, newTag.normalizedName());
    insertStmt.bindText(3, newTag.displayName());
    insertStmt.bindDateTime(4, newTag.createdAt());

    if (!insertStmt.execute()) {
        errorOut = insertStmt.lastError();
        return Tag();
    }

    return newTag;
}

std::optional<Tag> TagRepository::getById(const QString& id) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("SELECT id, normalized_name, display_name, created_at FROM tags WHERE id = ?1;"));
    stmt.bindText(1, id);
    if (stmt.step() == SQLITE_ROW) {
        Tag tag;
        tag.setId(stmt.getText(0));
        tag.setNormalizedName(stmt.getText(1));
        tag.setDisplayName(stmt.getText(2));
        tag.setCreatedAt(stmt.getDateTime(3));
        return tag;
    }
    return std::nullopt;
}

std::optional<Tag> TagRepository::getByNormalizedName(const QString& normalizedName) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("SELECT id, normalized_name, display_name, created_at FROM tags WHERE normalized_name = ?1;"));
    stmt.bindText(1, normalizedName);
    if (stmt.step() == SQLITE_ROW) {
        Tag tag;
        tag.setId(stmt.getText(0));
        tag.setNormalizedName(stmt.getText(1));
        tag.setDisplayName(stmt.getText(2));
        tag.setCreatedAt(stmt.getDateTime(3));
        return tag;
    }
    return std::nullopt;
}

QVector<Tag> TagRepository::getAllTagsWithCounts() {
    QMutexLocker locker(&m_db.mutex());
    QVector<Tag> tags;
    const char* sql = R"(
        SELECT t.id, t.normalized_name, t.display_name, t.created_at,
               COUNT(st.snippet_id) AS usage_count
        FROM tags t
        LEFT JOIN snippet_tags st ON t.id = st.tag_id
        LEFT JOIN snippets s ON st.snippet_id = s.id AND s.deleted_at IS NULL
        GROUP BY t.id
        ORDER BY t.display_name COLLATE NOCASE ASC;
    )";

    SqliteStatement stmt(m_db.rawDb(), QString::fromUtf8(sql));
    while (stmt.step() == SQLITE_ROW) {
        Tag tag;
        tag.setId(stmt.getText(0));
        tag.setNormalizedName(stmt.getText(1));
        tag.setDisplayName(stmt.getText(2));
        tag.setCreatedAt(stmt.getDateTime(3));
        tag.setUsageCount(stmt.getInt(4));
        tags.append(tag);
    }
    return tags;
}

bool TagRepository::renameTag(const QString& id, const QString& newDisplayName, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    const QString trimmed = newDisplayName.trimmed();
    auto val = Validation::validateTagName(trimmed);
    if (!val.valid) {
        errorOut = val.errorMessage;
        return false;
    }

    const QString norm = Validation::normalizeTag(trimmed);

    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("UPDATE tags SET display_name = ?1, normalized_name = ?2 WHERE id = ?3;"));
    stmt.bindText(1, trimmed);
    stmt.bindText(2, norm);
    stmt.bindText(3, id);

    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

bool TagRepository::mergeTags(const QString& sourceTagId, const QString& targetTagId, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteTransaction tx(m_db.rawDb());

    // Update snippet_tags: replace sourceTagId with targetTagId where not already present
    const char* updateSql = R"(
        UPDATE OR IGNORE snippet_tags
        SET tag_id = ?1
        WHERE tag_id = ?2;
    )";
    SqliteStatement updateStmt(m_db.rawDb(), QString::fromUtf8(updateSql));
    updateStmt.bindText(1, targetTagId);
    updateStmt.bindText(2, sourceTagId);
    if (!updateStmt.execute()) {
        errorOut = updateStmt.lastError();
        return false;
    }

    // Delete remaining source tags
    SqliteStatement delStmt(m_db.rawDb(), QStringLiteral("DELETE FROM tags WHERE id = ?1;"));
    delStmt.bindText(1, sourceTagId);
    if (!delStmt.execute()) {
        errorOut = delStmt.lastError();
        return false;
    }

    return tx.commit();
}

bool TagRepository::deleteTag(const QString& id, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("DELETE FROM tags WHERE id = ?1;"));
    stmt.bindText(1, id);
    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

QStringList TagRepository::getTagsForSnippet(const QString& snippetId) {
    QMutexLocker locker(&m_db.mutex());
    QStringList tags;
    const char* sql = R"(
        SELECT t.display_name
        FROM tags t
        JOIN snippet_tags st ON t.id = st.tag_id
        WHERE st.snippet_id = ?1
        ORDER BY t.display_name COLLATE NOCASE ASC;
    )";
    SqliteStatement stmt(m_db.rawDb(), QString::fromUtf8(sql));
    stmt.bindText(1, snippetId);
    while (stmt.step() == SQLITE_ROW) {
        tags.append(stmt.getText(0));
    }
    return tags;
}

bool TagRepository::setTagsForSnippet(const QString& snippetId, const QStringList& tags, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteTransaction tx(m_db.rawDb());

    // Delete existing snippet_tags
    SqliteStatement delStmt(m_db.rawDb(), QStringLiteral("DELETE FROM snippet_tags WHERE snippet_id = ?1;"));
    delStmt.bindText(1, snippetId);
    if (!delStmt.execute()) {
        errorOut = delStmt.lastError();
        return false;
    }

    for (const QString& tagStr : tags) {
        const QString trimmed = tagStr.trimmed();
        if (trimmed.isEmpty()) continue;

        // Find or create tag
        const QString norm = Validation::normalizeTag(trimmed);
        QString tagId;

        SqliteStatement findStmt(m_db.rawDb(), QStringLiteral("SELECT id FROM tags WHERE normalized_name = ?1;"));
        findStmt.bindText(1, norm);
        if (findStmt.step() == SQLITE_ROW) {
            tagId = findStmt.getText(0);
        } else {
            Tag newTag(trimmed);
            SqliteStatement insTag(m_db.rawDb(), QStringLiteral("INSERT INTO tags (id, normalized_name, display_name, created_at) VALUES (?1, ?2, ?3, ?4);"));
            insTag.bindText(1, newTag.id());
            insTag.bindText(2, newTag.normalizedName());
            insTag.bindText(3, newTag.displayName());
            insTag.bindDateTime(4, newTag.createdAt());
            if (!insTag.execute()) {
                errorOut = insTag.lastError();
                return false;
            }
            tagId = newTag.id();
        }

        // Insert snippet_tags
        SqliteStatement linkStmt(m_db.rawDb(), QStringLiteral("INSERT OR IGNORE INTO snippet_tags (snippet_id, tag_id) VALUES (?1, ?2);"));
        linkStmt.bindText(1, snippetId);
        linkStmt.bindText(2, tagId);
        if (!linkStmt.execute()) {
            errorOut = linkStmt.lastError();
            return false;
        }
    }

    return tx.commit();
}

} // namespace CodeShelf
