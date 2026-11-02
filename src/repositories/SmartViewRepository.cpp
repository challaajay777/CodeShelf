#include "SmartViewRepository.h"
#include <QMutexLocker>

namespace CodeShelf {

SmartViewRepository::SmartViewRepository(Database& db)
    : m_db(db)
{
}

bool SmartViewRepository::create(SmartView& sv, QString& errorOut) {
    auto val = sv.validate();
    if (!val.valid) {
        errorOut = val.errorMessage;
        return false;
    }

    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "INSERT INTO smart_views (id, name, query_string, sort_order, created_at) "
        "VALUES (?1, ?2, ?3, ?4, ?5);"
    ));
    stmt.bindText(1, sv.id());
    stmt.bindText(2, sv.name());
    stmt.bindText(3, sv.queryString());
    stmt.bindInt(4, sv.sortOrder());
    stmt.bindDateTime(5, sv.createdAt());

    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

bool SmartViewRepository::update(const SmartView& sv, QString& errorOut) {
    auto val = sv.validate();
    if (!val.valid) {
        errorOut = val.errorMessage;
        return false;
    }

    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "UPDATE smart_views SET name = ?1, query_string = ?2, sort_order = ?3 "
        "WHERE id = ?4;"
    ));
    stmt.bindText(1, sv.name());
    stmt.bindText(2, sv.queryString());
    stmt.bindInt(3, sv.sortOrder());
    stmt.bindText(4, sv.id());

    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

bool SmartViewRepository::deleteSmartView(const QString& id, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("DELETE FROM smart_views WHERE id = ?1;"));
    stmt.bindText(1, id);
    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

std::optional<SmartView> SmartViewRepository::getById(const QString& id) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT id, name, query_string, sort_order, created_at "
        "FROM smart_views WHERE id = ?1;"
    ));
    stmt.bindText(1, id);
    if (stmt.step() == SQLITE_ROW) {
        SmartView sv;
        sv.setId(stmt.getText(0));
        sv.setName(stmt.getText(1));
        sv.setQueryString(stmt.getText(2));
        sv.setSortOrder(stmt.getInt(3));
        sv.setCreatedAt(stmt.getDateTime(4));
        return sv;
    }
    return std::nullopt;
}

QVector<SmartView> SmartViewRepository::getAll() {
    QMutexLocker locker(&m_db.mutex());
    QVector<SmartView> result;
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT id, name, query_string, sort_order, created_at "
        "FROM smart_views ORDER BY sort_order ASC, name COLLATE NOCASE ASC;"
    ));
    while (stmt.step() == SQLITE_ROW) {
        SmartView sv;
        sv.setId(stmt.getText(0));
        sv.setName(stmt.getText(1));
        sv.setQueryString(stmt.getText(2));
        sv.setSortOrder(stmt.getInt(3));
        sv.setCreatedAt(stmt.getDateTime(4));
        result.append(sv);
    }
    return result;
}

} // namespace CodeShelf
