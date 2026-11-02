#include "TemplateRepository.h"
#include <QMutexLocker>

namespace CodeShelf {

TemplateRepository::TemplateRepository(Database& db)
    : m_db(db)
{
}

bool TemplateRepository::create(Template& tmpl, QString& errorOut) {
    auto val = tmpl.validate();
    if (!val.valid) {
        errorOut = val.errorMessage;
        return false;
    }

    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "INSERT INTO templates (id, name, description, body, language, created_at, updated_at) "
        "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7);"
    ));
    stmt.bindText(1, tmpl.id());
    stmt.bindText(2, tmpl.name());
    stmt.bindText(3, tmpl.description());
    stmt.bindText(4, tmpl.body());
    stmt.bindText(5, tmpl.language());
    stmt.bindDateTime(6, tmpl.createdAt());
    stmt.bindDateTime(7, tmpl.updatedAt());

    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

bool TemplateRepository::update(const Template& tmpl, QString& errorOut) {
    auto val = tmpl.validate();
    if (!val.valid) {
        errorOut = val.errorMessage;
        return false;
    }

    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "UPDATE templates SET name = ?1, description = ?2, body = ?3, language = ?4, updated_at = ?5 "
        "WHERE id = ?6;"
    ));
    stmt.bindText(1, tmpl.name());
    stmt.bindText(2, tmpl.description());
    stmt.bindText(3, tmpl.body());
    stmt.bindText(4, tmpl.language());
    stmt.bindDateTime(5, QDateTime::currentDateTimeUtc());
    stmt.bindText(6, tmpl.id());

    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

bool TemplateRepository::deleteTemplate(const QString& id, QString& errorOut) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral("DELETE FROM templates WHERE id = ?1;"));
    stmt.bindText(1, id);
    if (!stmt.execute()) {
        errorOut = stmt.lastError();
        return false;
    }
    return true;
}

std::optional<Template> TemplateRepository::getById(const QString& id) {
    QMutexLocker locker(&m_db.mutex());
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT id, name, description, body, language, created_at, updated_at "
        "FROM templates WHERE id = ?1;"
    ));
    stmt.bindText(1, id);
    if (stmt.step() == SQLITE_ROW) {
        Template tmpl;
        tmpl.setId(stmt.getText(0));
        tmpl.setName(stmt.getText(1));
        tmpl.setDescription(stmt.getText(2));
        tmpl.setBody(stmt.getText(3));
        tmpl.setLanguage(stmt.getText(4));
        tmpl.setCreatedAt(stmt.getDateTime(5));
        tmpl.setUpdatedAt(stmt.getDateTime(6));
        return tmpl;
    }
    return std::nullopt;
}

QVector<Template> TemplateRepository::getAll() {
    QMutexLocker locker(&m_db.mutex());
    QVector<Template> templates;
    SqliteStatement stmt(m_db.rawDb(), QStringLiteral(
        "SELECT id, name, description, body, language, created_at, updated_at "
        "FROM templates ORDER BY name COLLATE NOCASE ASC;"
    ));
    while (stmt.step() == SQLITE_ROW) {
        Template tmpl;
        tmpl.setId(stmt.getText(0));
        tmpl.setName(stmt.getText(1));
        tmpl.setDescription(stmt.getText(2));
        tmpl.setBody(stmt.getText(3));
        tmpl.setLanguage(stmt.getText(4));
        tmpl.setCreatedAt(stmt.getDateTime(5));
        tmpl.setUpdatedAt(stmt.getDateTime(6));
        templates.append(tmpl);
    }
    return templates;
}

} // namespace CodeShelf
