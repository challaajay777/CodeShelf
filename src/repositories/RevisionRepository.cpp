#include "RevisionRepository.h"
#include <QMutexLocker>

namespace CodeShelf {

RevisionRepository::RevisionRepository(Database& db)
    : m_db(db)
{
}

bool RevisionRepository::createRevision(const Revision& rev, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "INSERT INTO snippet_revisions (id, snippet_id, revision_number, title, body, description, language, changed_at, change_note) "
        "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9);"
    ));
    stmt.bindText(1, rev.id());
    stmt.bindText(2, rev.snippetId());
    stmt.bindInt(3, rev.revisionNumber());
    stmt.bindText(4, rev.title());
    stmt.bindText(5, rev.body());
    stmt.bindText(6, rev.description());
    stmt.bindText(7, rev.language());
    stmt.bindDateTime(8, rev.changedAt());
    stmt.bindText(9, rev.changeNote());

    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

std::optional<Revision> RevisionRepository::getById(const QString& id) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT id, snippet_id, revision_number, title, body, description, language, changed_at, change_note "
        "FROM snippet_revisions WHERE id = ?1;"
    ));
    stmt.bindText(1, id);
    if (stmt.step() == SQLITE_ROW) {
        Revision rev;
        rev.setId(stmt.getText(0));
        rev.setSnippetId(stmt.getText(1));
        rev.setRevisionNumber(stmt.getInt(2));
        rev.setTitle(stmt.getText(3));
        rev.setBody(stmt.getText(4));
        rev.setDescription(stmt.getText(5));
        rev.setLanguage(stmt.getText(6));
        rev.setChangedAt(stmt.getDateTime(7));
        rev.setChangeNote(stmt.getText(8));
        return rev;
    }
    return std::nullopt;
}

QVector<Revision> RevisionRepository::getRevisionsForSnippet(const QString& snippetId) {
    QMutexLocker locker(&m_db.mutex());
    QVector<Revision> revisions;
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT id, snippet_id, revision_number, title, body, description, language, changed_at, change_note "
        "FROM snippet_revisions WHERE snippet_id = ?1 ORDER BY revision_number DESC;"
    ));
    stmt.bindText(1, snippetId);
    while (stmt.step() == SQLITE_ROW) {
        Revision rev;
        rev.setId(stmt.getText(0));
        rev.setSnippetId(stmt.getText(1));
        rev.setRevisionNumber(stmt.getInt(2));
        rev.setTitle(stmt.getText(3));
        rev.setBody(stmt.getText(4));
        rev.setDescription(stmt.getText(5));
        rev.setLanguage(stmt.getText(6));
        rev.setChangedAt(stmt.getDateTime(7));
        rev.setChangeNote(stmt.getText(8));
        revisions.append(rev);
    }
    return revisions;
}

int RevisionRepository::getNextRevisionNumber(const QString& snippetId) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT COALESCE(MAX(revision_number), 0) + 1 FROM snippet_revisions WHERE snippet_id = ?1;"
    ));
    stmt.bindText(1, snippetId);
    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }
    return 1;
}

} // namespace CodeShelf
