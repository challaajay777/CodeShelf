#include "SqliteHelper.h"
#include <QByteArray>

namespace CodeShelf {

SqliteStatement::SqliteStatement(sqlite3* db, const QString& sql)
    : m_db(db)
{
    if (m_db) {
        const QByteArray utf8 = sql.toUtf8();
        sqlite3_prepare_v2(m_db, utf8.constData(), utf8.size(), &m_stmt, nullptr);
    }
}

SqliteStatement::~SqliteStatement() {
    if (m_stmt) {
        sqlite3_finalize(m_stmt);
        m_stmt = nullptr;
    }
}

SqliteStatement::SqliteStatement(SqliteStatement&& other) noexcept
    : m_db(other.m_db)
    , m_stmt(other.m_stmt)
{
    other.m_db = nullptr;
    other.m_stmt = nullptr;
}

SqliteStatement& SqliteStatement::operator=(SqliteStatement&& other) noexcept {
    if (this != &other) {
        if (m_stmt) {
            sqlite3_finalize(m_stmt);
        }
        m_db = other.m_db;
        m_stmt = other.m_stmt;
        other.m_db = nullptr;
        other.m_stmt = nullptr;
    }
    return *this;
}

QString SqliteStatement::lastError() const {
    if (m_db) {
        return QString::fromUtf8(sqlite3_errmsg(m_db));
    }
    return QStringLiteral("No database connection");
}

bool SqliteStatement::bindText(int index, const QString& text) {
    if (!m_stmt) return false;
    const QByteArray utf8 = text.toUtf8();
    return sqlite3_bind_text(m_stmt, index, utf8.constData(), utf8.size(), SQLITE_TRANSIENT) == SQLITE_OK;
}

bool SqliteStatement::bindInt(int index, int value) {
    if (!m_stmt) return false;
    return sqlite3_bind_int(m_stmt, index, value) == SQLITE_OK;
}

bool SqliteStatement::bindInt64(int index, qint64 value) {
    if (!m_stmt) return false;
    return sqlite3_bind_int64(m_stmt, index, value) == SQLITE_OK;
}

bool SqliteStatement::bindDouble(int index, double value) {
    if (!m_stmt) return false;
    return sqlite3_bind_double(m_stmt, index, value) == SQLITE_OK;
}

bool SqliteStatement::bindNull(int index) {
    if (!m_stmt) return false;
    return sqlite3_bind_null(m_stmt, index) == SQLITE_OK;
}

bool SqliteStatement::bindDateTime(int index, const QDateTime& dt) {
    if (!dt.isValid()) {
        return bindNull(index);
    }
    return bindText(index, dt.toUTC().toString(Qt::ISODateWithMs));
}

int SqliteStatement::step() {
    if (!m_stmt) return SQLITE_ERROR;
    return sqlite3_step(m_stmt);
}

bool SqliteStatement::execute() {
    int res = step();
    return (res == SQLITE_DONE || res == SQLITE_OK);
}

bool SqliteStatement::reset() {
    if (!m_stmt) return false;
    return sqlite3_reset(m_stmt) == SQLITE_OK && sqlite3_clear_bindings(m_stmt) == SQLITE_OK;
}

QString SqliteStatement::getText(int col) const {
    if (!m_stmt || isNull(col)) return QString();
    const unsigned char* text = sqlite3_column_text(m_stmt, col);
    int bytes = sqlite3_column_bytes(m_stmt, col);
    return QString::fromUtf8(reinterpret_cast<const char*>(text), bytes);
}

int SqliteStatement::getInt(int col) const {
    if (!m_stmt) return 0;
    return sqlite3_column_int(m_stmt, col);
}

qint64 SqliteStatement::getInt64(int col) const {
    if (!m_stmt) return 0;
    return sqlite3_column_int64(m_stmt, col);
}

double SqliteStatement::getDouble(int col) const {
    if (!m_stmt) return 0.0;
    return sqlite3_column_double(m_stmt, col);
}

bool SqliteStatement::isNull(int col) const {
    if (!m_stmt) return true;
    return sqlite3_column_type(m_stmt, col) == SQLITE_NULL;
}

QDateTime SqliteStatement::getDateTime(int col) const {
    const QString str = getText(col);
    if (str.isEmpty()) return QDateTime();
    return QDateTime::fromString(str, Qt::ISODateWithMs);
}

// Transaction
SqliteTransaction::SqliteTransaction(sqlite3* db)
    : m_db(db)
{
    if (m_db) {
        if (sqlite3_get_autocommit(m_db) != 0) {
            char* errMsg = nullptr;
            if (sqlite3_exec(m_db, "BEGIN IMMEDIATE TRANSACTION;", nullptr, nullptr, &errMsg) == SQLITE_OK) {
                m_active = true;
                m_owner = true;
            } else {
                if (errMsg) sqlite3_free(errMsg);
            }
        } else {
            // Already participating in an outer active transaction
            m_active = true;
            m_owner = false;
        }
    }
}

SqliteTransaction::~SqliteTransaction() {
    if (m_active && m_db && m_owner) {
        rollback();
    }
}

bool SqliteTransaction::commit() {
    if (!m_active || !m_db) return false;
    if (!m_owner) {
        m_active = false;
        return true;
    }
    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, &errMsg);
    if (rc == SQLITE_OK) {
        m_active = false;
        return true;
    }
    if (errMsg) sqlite3_free(errMsg);
    return false;
}

void SqliteTransaction::rollback() {
    if (!m_active || !m_db) return;
    char* errMsg = nullptr;
    sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, &errMsg);
    if (errMsg) sqlite3_free(errMsg);
    m_active = false;
}

} // namespace CodeShelf
